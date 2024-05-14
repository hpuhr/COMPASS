#include "reconstructortask.h"

#include "compass.h"
#include "reconstructortaskdialog.h"
#include "datasourcemanager.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variableset.h"
#include "stringconv.h"
#include "taskmanager.h"
#include "viewmanager.h"
#include "buffer.h"
#include "simplereconstructor.h"
#include "simpleaccuracyestimator.h"
#include "timeconv.h"
#include "number.h"
#include "dbinterface.h"
#include "metavariable.h"
#include "util/async.h"

#if USE_EXPERIMENTAL_SOURCE == true
#include "probimmreconstructor.h"
#include "complexaccuracyestimator.h"
#endif

#include <QApplication>
#include <QMessageBox>
#include <QProgressDialog>
#include <QThread>

#include <malloc.h>

#include <future>

using namespace std;
using namespace Utils;
using namespace dbContent;

const std::string ReconstructorTask::ScoringUMReconstructorName {"Scoring + UMKalman"};

#if USE_EXPERIMENTAL_SOURCE == true
const std::string ReconstructorTask::ProbImmReconstructorName {"Probabilistic + IMM"};
#endif

ReconstructorTask::ReconstructorTask(const std::string& class_id, const std::string& instance_id,
                                     TaskManager& task_manager)
    : Task(task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_reconstructor.json")
{
    tooltip_ = "Associate target reports and calculate reference trajectories based on all DB Content.";

    registerParameter("use_dstypes", &use_dstypes_, nlohmann::json::object());
    registerParameter("use_data_sources", &use_data_sources_, nlohmann::json::object());
    registerParameter("use_data_sources_lines", &use_data_sources_lines_, nlohmann::json::object());

    registerParameter("current_reconstructor_str", &current_reconstructor_str_, {});

    if (!current_reconstructor_str_.size()
        || (current_reconstructor_str_ != ScoringUMReconstructorName
#if USE_EXPERIMENTAL_SOURCE == true
            && current_reconstructor_str_ != ProbImmReconstructorName
#endif
            ))
        current_reconstructor_str_ = ScoringUMReconstructorName;

    createSubConfigurables();
}

ReconstructorTask::~ReconstructorTask()
{
}

void ReconstructorTask::generateSubConfigurable(const std::string& class_id,
                                                const std::string& instance_id)
{
    if (class_id == "SimpleReconstructor")
    {
        assert(!simple_reconstructor_);

        std::unique_ptr<AccuracyEstimatorBase> acc_estimator;
        acc_estimator.reset(new SimpleAccuracyEstimator());

        simple_reconstructor_.reset(new SimpleReconstructor(class_id, instance_id, *this, std::move(acc_estimator)));
        assert(simple_reconstructor_);
    }
    else if (class_id == "ProbIMMReconstructor")
    {
#if USE_EXPERIMENTAL_SOURCE == true

        assert(!probimm_reconstructor_);

        std::unique_ptr<AccuracyEstimatorBase> acc_estimator;
        acc_estimator.reset(new ComplexAccuracyEstimator());

        probimm_reconstructor_.reset(new ProbIMMReconstructor(class_id, instance_id, *this, std::move(acc_estimator)));
        assert(probimm_reconstructor_);

#endif
    }
    else
        throw std::runtime_error("ReconstructorTask: generateSubConfigurable: unknown class_id " + class_id);
}

ReconstructorTaskDialog* ReconstructorTask::dialog()
{
    if (!dialog_)
    {
        dialog_.reset(new ReconstructorTaskDialog(*this));

        connect(dialog_.get(), &ReconstructorTaskDialog::runSignal,
                this, &ReconstructorTask::dialogRunSlot);

        connect(dialog_.get(), &ReconstructorTaskDialog::cancelSignal,
                this, &ReconstructorTask::dialogCancelSlot);
    }

    assert(dialog_);
    return dialog_.get();
}

bool ReconstructorTask::canRun()
{
    assert (currentReconstructor());

    return COMPASS::instance().dbContentManager().hasData() && currentReconstructor()->hasNextTimeSlice();
}

void ReconstructorTask::updateProgress(const QString& msg, bool add_slice_progress)
{
    logdbg << "ReconstructorTask: updateProgress: slice " << add_slice_progress
           << " dialog " << (progress_dialog_ != nullptr);

    if (!progress_dialog_)
        return;

    int ns   = currentReconstructor()->numSlices();
    int sidx = std::max(0, std::min((int)current_slice_idx_, ns - 1));

    QString slice_p;
    if (ns >= 0)
        slice_p = " " + QString::number(sidx + 1) + "/" + QString::number(ns);

    QString pmsg = msg;
    if (add_slice_progress)
        pmsg += slice_p;

    double time_elapsed_s= Time::partialSeconds(boost::posix_time::microsec_clock::local_time() - run_start_time_);

    double progress = 0.0;
    if (ns >= 1)
    {
        progress = (double) sidx / (double) ns;
    }

    pmsg += ("\n\nElapsed: "+String::timeStringFromDouble(time_elapsed_s, false)).c_str();

    if (ns && sidx) // do remaining time estimate if possible
    {
        double seconds_per_slice = time_elapsed_s / (float) (sidx + 1);

        int num_slices_remaining = ns - sidx; // not -1, since will display and async run
        double time_remaining_s = num_slices_remaining * seconds_per_slice;

        logdbg << "ReconstructorTask: updateProgress: current_slice_idx " << sidx
               << " ns " << ns << " num_slices_remaining " << num_slices_remaining
               << " time_remaining_s " << time_remaining_s;

        pmsg += ("\tRemaining: " + String::timeStringFromDouble(time_remaining_s, false)).c_str();
    }

    progress_dialog_->setLabelText(pmsg);
    progress_dialog_->setValue(progress_dialog_->maximum() * progress);
    
    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();
    while ((boost::posix_time::microsec_clock::local_time() - start_time).total_milliseconds() < 50)
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

std::string ReconstructorTask::currentReconstructorStr() const
{
    return current_reconstructor_str_;
}

void ReconstructorTask::currentReconstructorStr(const std::string& value)
{

#if USE_EXPERIMENTAL_SOURCE == true
    assert (value == ReconstructorTask::ScoringUMReconstructorName
           || value == ReconstructorTask::ProbImmReconstructorName);
#else
    assert (value == ReconstructorTask::ScoringUMReconstructorName);
#endif

    current_reconstructor_str_ = value;
}

ReconstructorBase* ReconstructorTask::currentReconstructor() const
{
#if USE_EXPERIMENTAL_SOURCE == true
    assert (current_reconstructor_str_ == ReconstructorTask::ScoringUMReconstructorName
           || current_reconstructor_str_ == ReconstructorTask::ProbImmReconstructorName);

    if (current_reconstructor_str_ == ReconstructorTask::ScoringUMReconstructorName)
        return dynamic_cast<ReconstructorBase*> (simple_reconstructor_.get());
    else
        return dynamic_cast<ReconstructorBase*> (probimm_reconstructor_.get());
#else
    assert (current_reconstructor_str_ == ReconstructorTask::ScoringUMReconstructorName);

    return dynamic_cast<ReconstructorBase*> (simple_reconstructor_.get());
#endif
}

SimpleReconstructor* ReconstructorTask::simpleReconstructor() const
{
    return simple_reconstructor_.get();
}

#if USE_EXPERIMENTAL_SOURCE == true

ProbIMMReconstructor* ReconstructorTask::probIMMReconstructor() const
{
    return probimm_reconstructor_.get();
}

#endif

std::set<unsigned int> ReconstructorTask::disabledDataSources() const
{
    std::set<unsigned int> disabled_ds;

    disabled_ds.insert(Number::dsIdFrom(currentReconstructor()->settings().ds_sac,
                                        currentReconstructor()->settings().ds_sic));

    for (auto& ds_it : COMPASS::instance().dataSourceManager().dbDataSources())
    {
        if (ds_it->dsType() == "Radar")
            disabled_ds.insert(ds_it->id());
    }

    return disabled_ds;
}

const std::set<unsigned int>& ReconstructorTask::debugUTNs() const
{
    return debug_utns_;
}

void ReconstructorTask::debugUTNs(const std::set<unsigned int>& utns)
{
    loginf << "ReconstructorTask: debugRecNums: values '" << String::compress(utns, ',') << "'";

    debug_utns_ = utns;
}

const std::set<unsigned long>& ReconstructorTask::debugRecNums() const
{
    return debug_rec_nums_;
}

void ReconstructorTask::debugRecNums(const std::set<unsigned long>& rec_nums)
{
    loginf << "ReconstructorTask: debugRecNums: values '" << String::compress(rec_nums, ',') << "'";

    debug_rec_nums_ = rec_nums;
}

void ReconstructorTask::dialogRunSlot()
{
    loginf << "ReconstructorTask: dialogRunSlot";

    assert (dialog_);
    dialog_->hide();

    assert (canRun());
    run ();
}

void ReconstructorTask::dialogCancelSlot()
{
    loginf << "ReconstructorTask: dialogCancelSlot";

    assert (dialog_);
    dialog_->hide();
}

void ReconstructorTask::run()
{
    assert(canRun());

    loading_slice_ = nullptr;
    processing_slice_ = nullptr;
    processing_data_slice_ = false;
    writing_slice_ = nullptr;

    cancelled_ = false;

    current_slice_idx_ = 0;

    loginf << "ReconstructorTask: run: started";

    run_start_time_ = boost::posix_time::microsec_clock::local_time();

    progress_dialog_.reset(new QProgressDialog("Reconstructing...", "Cancel", 0, 100));
    progress_dialog_->setAutoClose(false);
    connect(progress_dialog_.get(), &QProgressDialog::canceled,
            this, &ReconstructorTask::runCancelSlot, Qt::QueuedConnection);

    progress_dialog_->show();

    if (cancelled_)
        return;

    updateProgress("Deleting Previous References", false);

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();
    dbcontent_man.clearData();

    delcalcref_future_ = std::async(std::launch::async, [&] {
        {
            deleteCalculatedReferences();

            if (cancelled_)
                return;

            QMetaObject::invokeMethod(this, "deleteCalculatedReferencesDoneSlot", Qt::QueuedConnection);
        }});
}

void ReconstructorTask::deleteCalculatedReferencesDoneSlot()
{
    loginf << "ReconstructorTask: deleteCalculatedReferencesDoneSlot";

    if (cancelled_)
        return;

    updateProgress("Deleting Previous Targets", false);

    DBContentManager& cont_man = COMPASS::instance().dbContentManager();

    deltgts_future_ = std::async(std::launch::async, [&] {
        {
            cont_man.clearTargetsInfo();

            if (cancelled_)
                return;

            QMetaObject::invokeMethod(this, "deleteTargetsDoneSlot", Qt::QueuedConnection);
        }});
}

void ReconstructorTask::deleteTargetsDoneSlot()
{
    loginf << "ReconstructorTask: deleteTargetsDoneSlot";

    if (cancelled_)
        return;

    updateProgress("Deleting Previous Associations", false);

    DBContentManager& cont_man = COMPASS::instance().dbContentManager();

    delassocs_future_ = std::async(std::launch::async, [&] {
        {
            for (auto& dbcont_it : cont_man)
            {
                if (dbcont_it.second->existsInDB())
                    COMPASS::instance().interface().clearAssociations(*dbcont_it.second);
            }

            if (cancelled_)
                return;

            QMetaObject::invokeMethod(this, "deleteAssociationsDoneSlot", Qt::QueuedConnection);
        }});
}

void ReconstructorTask::deleteAssociationsDoneSlot()
{
    loginf << "ReconstructorTask: deleteAssociationsDoneSlot";

    if (cancelled_)
        return;

    updateProgress("Initializing", false);

    COMPASS::instance().viewManager().disableDataDistribution(true);

    currentReconstructor()->reset();

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    connect(&dbcontent_man, &DBContentManager::loadedDataSignal,
            this, &ReconstructorTask::loadedDataSlot);
    connect(&dbcontent_man, &DBContentManager::loadingDoneSignal,
            this, &ReconstructorTask::loadingDoneSlot);

    if (cancelled_)
        return;

    loadDataSlice();
}

void ReconstructorTask::loadDataSlice()
{
    assert (currentReconstructor());
    assert (currentReconstructor()->hasNextTimeSlice());
    assert (!loading_slice_);

    loginf << "ReconstructorTask: loadDataSlice";

    loading_slice_ = currentReconstructor()->getNextTimeSlice();

    assert (loading_slice_);

    loginf << "ReconstructorTask: loadDataSlice: min " << Time::toString(loading_slice_->slice_begin_)
           << " max " << Time::toString(loading_slice_->next_slice_begin_)
           << " last " << loading_slice_->is_last_slice_;

    string timestamp_filter;

    timestamp_filter = "timestamp >= " + to_string(Time::toLong(loading_slice_->slice_begin_));

    if (loading_slice_->is_last_slice_)
        timestamp_filter += " AND timestamp <= " + to_string(Time::toLong(loading_slice_->next_slice_begin_));
    else
        timestamp_filter += " AND timestamp < " + to_string(Time::toLong(loading_slice_->next_slice_begin_));

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    for (auto& dbcont_it : dbcontent_man)
    {
        logdbg << "ReconstructorTask: loadDataSlice: " << dbcont_it.first
               << " has data " << dbcont_it.second->hasData();

        if (!dbcont_it.second->hasData())
            continue;

        VariableSet read_set = currentReconstructor()->getReadSetFor(dbcont_it.first);

        dbcont_it.second->load(read_set, false, false, timestamp_filter);
    }
}

void ReconstructorTask::processDataSlice()
{
    loginf << "ReconstructorTask: processDataSlice";

    if (cancelled_)
        return;

    assert (loading_slice_);
    assert (!processing_slice_);

    processing_slice_ = std::move(loading_slice_);

    assert (!processing_data_slice_);

    processing_data_slice_ = true;

    logdbg << "ReconstructorTask: processDataSlice: processing1 first slice "
           << !processing_slice_->first_slice_
           << " remove ts " << Time::toString(processing_slice_->remove_before_time_);

    assert (processing_slice_);

    logdbg << "ReconstructorTask: processDataSlice: processing2 first slice "
           << !processing_slice_->first_slice_
           << " remove ts " << Time::toString(processing_slice_->remove_before_time_);

    process_future_ = std::async(std::launch::async, [&] {

        logdbg << "ReconstructorTask: processDataSlice: async process";

        if (cancelled_)
            return;

        logdbg << "ReconstructorTask: processDataSlice: async processing first slice "
               << !processing_slice_->first_slice_
               << " remove ts " << Time::toString(processing_slice_->remove_before_time_);

        assert (!currentReconstructor()->processing());
        currentReconstructor()->processSlice();

                // wait for previous writing done
        while (writing_slice_)
            QThread::msleep(1);

        logdbg << "ReconstructorTask: processDataSlice: done";

        QMetaObject::invokeMethod(this, "processingDoneSlot", Qt::QueuedConnection);
    });
}


void ReconstructorTask::writeDataSlice()
{
    loginf << "ReconstructorTask: writeDataSlice";

    assert (writing_slice_);

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    for (auto& buf_it : writing_slice_->assoc_data_)
    {
        logdbg << "ReconstructorTask: writeDataSlice: association dbcontent " << buf_it.first;

        DBContent& dbcontent = dbcontent_man.dbContent(buf_it.first);

        dbcontent.updateData(
            dbcontent_man.metaVariable(DBContent::meta_var_rec_num_.name()).getFor(buf_it.first), buf_it.second);
    }

    if (writing_slice_->first_slice_)
        connect(&dbcontent_man, &DBContentManager::insertDoneSignal,
                this, &ReconstructorTask::writeDoneSlot);

    loginf << "ReconstructorTask: writeDataSlice: references dbcontent";
    dbcontent_man.insertData(writing_slice_->reftraj_data_);
}


void ReconstructorTask::loadedDataSlot(const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset)
{
    assert (loading_slice_);
}

void ReconstructorTask::loadingDoneSlot()
{
    loginf << "ReconstructorTask: loadingDoneSlot";

    if (cancelled_)
        return;

    assert (currentReconstructor());
    assert (loading_slice_);

    bool last_slice = loading_slice_->is_last_slice_;

    loginf << "ReconstructorTask: loadingDoneSlot: is_last_slice " << last_slice
           << " current_slice_idx " << current_slice_idx_;

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();
    loading_slice_->data_ = dbcontent_man.data();
    assert (loading_slice_->data_.size());

    dbcontent_man.clearData(); // clear previous

    if (cancelled_)
        return;

            // check if not already processing
    while (currentReconstructor()->processing() || processing_data_slice_)
    {
        if (cancelled_)
            return;

        QCoreApplication::processEvents();
        QThread::msleep(1);
    }

    if (cancelled_)
        return;

    loginf << "ReconstructorTask: loadingDoneSlot: calling process";

    updateProgress("Processing slice", true);
    ++current_slice_idx_;

    assert (!processing_data_slice_);

    loginf << "ReconstructorTask: loadingDoneSlot: processing first slice "
           << !loading_slice_->first_slice_
           << " remove ts " << Time::toString(loading_slice_->remove_before_time_);

    processDataSlice();
    assert (!loading_slice_);
    assert (processing_data_slice_);

    if (cancelled_)
        return;

    if (last_slice) // disconnect everything
    {
        disconnect(&dbcontent_man, &DBContentManager::loadedDataSignal,
                   this, &ReconstructorTask::loadedDataSlot);
        disconnect(&dbcontent_man, &DBContentManager::loadingDoneSignal,
                   this, &ReconstructorTask::loadingDoneSlot);

        COMPASS::instance().viewManager().disableDataDistribution(false);
    }
    else // do next load
    {
        loginf << "ReconstructorTask: loadingDoneSlot: next slice";

        if (cancelled_)
            return;

        loadDataSlice();
    }
}

void ReconstructorTask::processingDoneSlot()
{
    loginf << "ReconstructorTask: processingDoneSlot";

            // processing done
    assert(currentReconstructor()->currentSlice().processing_done_);
    assert (processing_data_slice_);
    assert (processing_slice_);
    assert (!writing_slice_);

    processing_data_slice_ = false;

    if (cancelled_)
        return;

    assert (!writing_slice_);
    writing_slice_ = std::move(processing_slice_);
    writeDataSlice(); // starts the async jobs
}

void ReconstructorTask::writeDoneSlot()
{
    loginf << "ReconstructorTask: writeDoneSlot: last " << writing_slice_->is_last_slice_;

    assert (writing_slice_);
    writing_slice_->write_done_ = true;

    if (cancelled_)
    {
        writing_slice_ = nullptr;

        malloc_trim(0); // release unused memory

        done_ = true;

        return;
    }

    if (writing_slice_->is_last_slice_)
    {
        DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

        disconnect(&dbcontent_man, &DBContentManager::insertDoneSignal,
                   this, &ReconstructorTask::writeDoneSlot);

        currentReconstructor()->saveTargets();
        currentReconstructor()->reset();

        double time_elapsed_s= Time::partialSeconds(boost::posix_time::microsec_clock::local_time() - run_start_time_);

        loginf << "ReconstructorTask: writeDoneSlot: done after "
               << String::timeStringFromDouble(time_elapsed_s, false);


        COMPASS::instance().dbContentManager().setAssociationsIdentifier("All");

        malloc_trim(0); // release unused memory

        done_ = true;

                //close progress dialog
        progress_dialog_.reset();

        emit doneSignal();
    }

    writing_slice_ = nullptr;
}

void ReconstructorTask::runCancelSlot()
{
    loginf << "ReconstructorTask: runCancelSlot";

    assert (progress_dialog_);

    progress_dialog_->setLabelText("Cancelling");
    progress_dialog_->setCancelButton(nullptr);

    cancelled_ = true;

    Async::waitAndProcessEventsFor(50);

    currentReconstructor()->cancel();

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    disconnect(&dbcontent_man, &DBContentManager::loadedDataSignal,
               this, &ReconstructorTask::loadedDataSlot);
    disconnect(&dbcontent_man, &DBContentManager::loadingDoneSignal,
               this, &ReconstructorTask::loadingDoneSlot);

    COMPASS::instance().viewManager().disableDataDistribution(false);

    disconnect(&dbcontent_man, &DBContentManager::insertDoneSignal,
               this, &ReconstructorTask::writeDoneSlot);

    while (currentReconstructor()->processing())
        Async::waitAndProcessEventsFor(10);

    currentReconstructor()->reset();
    loading_slice_ = nullptr;

    done_ = true;

            //close progress dialog
    progress_dialog_.reset();

    emit doneSignal();

    loginf << "ReconstructorTask: runCancelSlot: done";
}

bool ReconstructorTask::useDStype(const std::string& ds_type) const
{
    if (!use_dstypes_.contains(ds_type))
        return true;

    return use_dstypes_.at(ds_type);
}

void ReconstructorTask::useDSType(const std::string& ds_type, bool value)
{
    use_dstypes_[ds_type] = value;
}

bool ReconstructorTask::useDataSource(unsigned int ds_id) const
{
    string ds_id_str = to_string(ds_id);

    if (!use_data_sources_.contains(ds_id_str))
        return true;

    return use_data_sources_.at(ds_id_str);
}

void ReconstructorTask::useDataSource(unsigned int ds_id, bool value)
{
    use_data_sources_[to_string(ds_id)] = value;
}

bool ReconstructorTask::useDataSourceLine(unsigned int ds_id, unsigned int line_id) const
{
    string ds_id_str = to_string(ds_id);
    string line_id_str = to_string(line_id);

    if (!use_data_sources_lines_.contains(ds_id_str))
        return true;

    if (!use_data_sources_lines_.at(ds_id_str).contains(line_id_str))
        return true;

    return use_data_sources_lines_.at(ds_id_str).at(line_id_str);
}

void ReconstructorTask::useDataSourceLine(unsigned int ds_id, unsigned int line_id, bool value)
{
    use_data_sources_lines_[to_string(ds_id)][to_string(line_id)] = value;
}

std::set<unsigned int> ReconstructorTask::unusedDSIDs() const
{
    std::set<unsigned int> unused_ds = disabledDataSources();

    for (auto& ds_it : COMPASS::instance().dataSourceManager().dbDataSources())
    {
        if (unused_ds.count(ds_it->id()))
            continue;

        if (!useDStype(ds_it->dsType()) || !useDataSource(ds_it->id()))
            unused_ds.insert(ds_it->id());
    }

    return unused_ds;
}

std::map<unsigned int, std::set<unsigned int>> ReconstructorTask::unusedDSIDLines() const
{
    std::set<unsigned int> unused_ds = unusedDSIDs();

    std::map<unsigned int, std::set<unsigned int>> unused_lines;

    for (auto& ds_it : COMPASS::instance().dataSourceManager().dbDataSources())
    {
        if (unused_ds.count(ds_it->id()))
            continue;

        for (unsigned int line_id = 0; line_id < 4; ++line_id)
        {
            if (!useDataSourceLine(ds_it->id(), line_id))
                unused_lines[ds_it->id()].insert(line_id);
        }
    }

    return unused_lines;
}

ReconstructorBase::DataSlice& ReconstructorTask::processingSlice()
{
    assert (processing_slice_);
    return *processing_slice_;
}

void ReconstructorTask::checkSubConfigurables()
{
    if (!simple_reconstructor_)
    {
        generateSubConfigurable("SimpleReconstructor", "SimpleReconstructor0");
        assert (simple_reconstructor_);
    }

#if USE_EXPERIMENTAL_SOURCE == true
    if (!probimm_reconstructor_)
    {
        generateSubConfigurable("ProbIMMReconstructor", "ProbIMMReconstructor0");
        assert (probimm_reconstructor_);
    }
#endif
}

void ReconstructorTask::deleteCalculatedReferences()
{
    loginf << "ReconstructorTask: deleteCalculatedReferences: delete_all_calc_reftraj "
           << currentReconstructor()->settings().delete_all_calc_reftraj;

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    loginf << "ReconstructorTask: deleteCalculatedReferences: deleting";

    if (currentReconstructor()->settings().delete_all_calc_reftraj)
        dbcontent_man.dbContent("RefTraj").deleteDBContentData(
            currentReconstructor()->settings().ds_sac, currentReconstructor()->settings().ds_sic);
    else
        dbcontent_man.dbContent("RefTraj").deleteDBContentData(
            currentReconstructor()->settings().ds_sac, currentReconstructor()->settings().ds_sic,
            currentReconstructor()->settings().ds_line);

    while (dbcontent_man.dbContent("RefTraj").isDeleting())
    {
        loginf << "ReconstructorTask: deleteCalculatedReferences: waiting on delete done";
        QThread::msleep(1000);
    }

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    unsigned int ds_id = Number::dsIdFrom(currentReconstructor()->settings().ds_sac,
                                          currentReconstructor()->settings().ds_sic);

            // clear counts
    if (ds_man.hasDBDataSource(ds_id))
    {
        dbContent::DBDataSource& ds = ds_man.dbDataSource(ds_id);

        if (currentReconstructor()->settings().delete_all_calc_reftraj)
            ds.clearNumInserted("RefTraj");
        else
            ds.clearNumInserted("RefTraj", currentReconstructor()->settings().ds_line);
    }

            // emit done in run

    loginf << "ReconstructorTask: deleteCalculatedReferences: done";
}

