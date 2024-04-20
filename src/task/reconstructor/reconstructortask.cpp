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
#include "probimmreconstructor.h"
#include "simpleaccuracyestimator.h"
#include "complexaccuracyestimator.h"
#include "timeconv.h"
#include "number.h"
#include "dbinterface.h"
#include "metavariable.h"

#include <QApplication>
#include <QMessageBox>
#include <QProgressDialog>
#include <QThread>

using namespace std;
using namespace Utils;
using namespace dbContent;

const std::string ReconstructorTask::ScoringUMReconstructorName {"Scoring + UMKalman"};
const std::string ReconstructorTask::ProbImmReconstructorName {"Probabilistic + IMM"};

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
            && current_reconstructor_str_ != ProbImmReconstructorName))
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
        assert(!probimm_reconstructor_);

        std::unique_ptr<AccuracyEstimatorBase> acc_estimator;
        acc_estimator.reset(new ComplexAccuracyEstimator());

        probimm_reconstructor_.reset(new ProbIMMReconstructor(class_id, instance_id, *this, std::move(acc_estimator)));
        assert(probimm_reconstructor_);
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

    int ns = currentReconstructor()->numSlices();
    QString slice_p;
    if (ns >= 0)
        slice_p = " " + QString::number(current_slice_idx_ + 1) + "/" + QString::number(ns);

    QString pmsg = msg;
    if (add_slice_progress)
        pmsg += slice_p;

    double time_elapsed_s= Time::partialSeconds(boost::posix_time::microsec_clock::local_time() - run_start_time_);

    double progress = 0.0;
    if (ns >= 1)
    {
        progress = (double) current_slice_idx_ / (double) ns;
    }

    pmsg += ("\n\nElapsed: "+String::timeStringFromDouble(time_elapsed_s, false)).c_str();

    if (ns && current_slice_idx_) // do remaining time estimate if possible
    {
        double seconds_per_slice = time_elapsed_s / (float) (current_slice_idx_ + 1);

        int num_slices_remaining = ns - (current_slice_idx_ + 1);
        double time_remaining_s = num_slices_remaining * seconds_per_slice;

        logdbg << "ReconstructorTask: updateProgress: current_slice_idx " << current_slice_idx_
               << " ns " << ns << " num_slices_remaining " << num_slices_remaining
               << " time_remaining_s " << time_remaining_s;

        pmsg += ("\tRemaining: "+String::timeStringFromDouble(time_remaining_s, false)).c_str();

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
    assert (value == ReconstructorTask::ScoringUMReconstructorName
           || value == ReconstructorTask::ProbImmReconstructorName);

    current_reconstructor_str_ = value;
}

ReconstructorBase* ReconstructorTask::currentReconstructor() const
{
    assert (current_reconstructor_str_ == ReconstructorTask::ScoringUMReconstructorName
           || current_reconstructor_str_ == ReconstructorTask::ProbImmReconstructorName);

    if (current_reconstructor_str_ == ReconstructorTask::ScoringUMReconstructorName)
        return dynamic_cast<ReconstructorBase*> (simple_reconstructor_.get());
    else
        return dynamic_cast<ReconstructorBase*> (probimm_reconstructor_.get());
}

SimpleReconstructor* ReconstructorTask::simpleReconstructor() const
{
    return simple_reconstructor_.get();
}

ProbIMMReconstructor* ReconstructorTask::probIMMReconstructor() const
{
    return probimm_reconstructor_.get();
}

std::set<unsigned int> ReconstructorTask::disabledDataSources() const
{
    std::set<unsigned int> disabled_ds;

    disabled_ds.insert(Number::dsIdFrom(currentReconstructor()->baseSettings().ds_sac,
                                        currentReconstructor()->baseSettings().ds_sic));

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

    current_slice_idx_ = 0;

    loginf << "ReconstructorTask: run: started";

    run_start_time_ = boost::posix_time::microsec_clock::local_time();

    progress_dialog_.reset(new QProgressDialog);
    progress_dialog_->setCancelButton(nullptr);
    progress_dialog_->setMinimum(0);
    progress_dialog_->setMaximum(100);
    progress_dialog_->setWindowTitle("Reconstructing...");

    progress_dialog_->show();

    updateProgress("Deleting Previous References", false);

    deleteCalculatedReferences();

    updateProgress("Deleting Previous Targets", false);

    DBContentManager& cont_man = COMPASS::instance().dbContentManager();
    cont_man.clearTargetsInfo();

    updateProgress("Deleting Previous Associations", false);

    for (auto& dbcont_it : cont_man)
    {
        if (dbcont_it.second->existsInDB())
            COMPASS::instance().interface().clearAssociations(*dbcont_it.second);
    }

    updateProgress("Initializing", false);

    COMPASS::instance().viewManager().disableDataDistribution(true);

    currentReconstructor()->reset();

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    connect(&dbcontent_man, &DBContentManager::loadedDataSignal,
            this, &ReconstructorTask::loadedDataSlot);
    connect(&dbcontent_man, &DBContentManager::loadingDoneSignal,
            this, &ReconstructorTask::loadingDoneSlot);

    loadDataSlice();
}

void ReconstructorTask::loadDataSlice()
{
    assert (currentReconstructor());
    assert (currentReconstructor()->hasNextTimeSlice());
    assert (!loading_slice_);

    updateProgress("Loading slice", true);

            //boost::posix_time::ptime min_ts, max_ts;

    loading_slice_ = currentReconstructor()->getNextTimeSlice();
    //assert (min_ts <= max_ts);

            //bool last_slice = !currentReconstructor()->hasNextTimeSlice();

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
        loginf << "ReconstructorTask: loadDataSlice: " << dbcont_it.first
               << " has data " << dbcont_it.second->hasData();

        if (!dbcont_it.second->hasData())
            continue;

        VariableSet read_set = currentReconstructor()->getReadSetFor(dbcont_it.first);

        dbcont_it.second->load(read_set, false, false, timestamp_filter);
    }
}

void ReconstructorTask::writeDataSlice()
{
    loginf << "ReconstructorTask: writeDataSlice";

    assert (writing_slice_);

    bool is_last_slice = writing_slice_->is_last_slice_;

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();
    DBInterface& db_interface = COMPASS::instance().interface();

            // TODO move to DB job

    for (auto& buf_it : writing_slice_->assoc_data_)
    {
        loginf << "ReconstructorTask: writeDataSlice: association dbcontent " << buf_it.first;

        DBContent& dbcontent = dbcontent_man.dbContent(buf_it.first);

        string rec_num_col_name =
            dbcontent_man.metaVariable(DBContent::meta_var_rec_num_.name()).getFor(buf_it.first).dbColumnName();

        db_interface.updateBuffer(dbcontent.dbTableName(), rec_num_col_name, buf_it.second);
    }

    for (auto& buf_it : writing_slice_->reftraj_data_)
    {
        loginf << "ReconstructorTask: writeDataSlice: references dbcontent " << buf_it.first;

        DBContent& dbcontent = dbcontent_man.dbContent(buf_it.first);

        string rec_num_col_name =
            dbcontent_man.metaVariable(DBContent::meta_var_rec_num_.name()).getFor(buf_it.first).dbColumnName();

        dbcontent.insertData(buf_it.second);
    }

    writing_slice_->write_done_ = true;

    if (is_last_slice)
        currentReconstructor()->saveTargets();

    writing_slice_ = nullptr;
}


void ReconstructorTask::loadedDataSlot(const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset)
{
    assert (loading_slice_);
}

void ReconstructorTask::loadingDoneSlot()
{
    assert (currentReconstructor());
    assert (loading_slice_);

    bool last_slice = loading_slice_->is_last_slice_;

    loginf << "ReconstructorTask: loadingDoneSlot: is_last_slice " << last_slice;

    updateProgress("Processing slice", true);

    //std::map<std::string, std::shared_ptr<Buffer>> data = std::move(data_); // move out of there

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();
    loading_slice_->data_ = dbcontent_man.data();

    dbcontent_man.clearData(); // clear previous

    ++current_slice_idx_;

            // TODO: do async, check if not already processing
    assert (!currentReconstructor()->hasCurrentSlice());
    currentReconstructor()->processSlice(std::move(loading_slice_));

    // processing done
    assert(currentReconstructor()->currentSlice().processing_done_);

    // do write
    assert (!writing_slice_);
    writing_slice_ = currentReconstructor()->moveCurrentSlice();
    writeDataSlice();

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
        loadDataSlice();
    }

    if (last_slice)
    {
        double time_elapsed_s= Time::partialSeconds(boost::posix_time::microsec_clock::local_time() - run_start_time_);

        loginf << "ReconstructorTask: loadingDoneSlot: data loading done after "
               << String::timeStringFromDouble(time_elapsed_s, false);

        currentReconstructor()->reset();

        COMPASS::instance().dbContentManager().setAssociationsIdentifier("All");

        done_ = true;

                //close progress dialog
        progress_dialog_.reset();

        emit doneSignal();
    }

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

void ReconstructorTask::checkSubConfigurables()
{
    if (!simple_reconstructor_)
    {
        generateSubConfigurable("SimpleReconstructor", "SimpleReconstructor0");
        assert (simple_reconstructor_);
    }

    if (!probimm_reconstructor_)
    {
        generateSubConfigurable("ProbIMMReconstructor", "ProbIMMReconstructor0");
        assert (probimm_reconstructor_);
    }
}

void ReconstructorTask::deleteCalculatedReferences()
{
    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();
    dbcontent_man.clearData();

    if (currentReconstructor()->baseSettings().delete_all_calc_reftraj)
        dbcontent_man.dbContent("RefTraj").deleteDBContentData(
            currentReconstructor()->baseSettings().ds_sac, currentReconstructor()->baseSettings().ds_sic);
    else
        dbcontent_man.dbContent("RefTraj").deleteDBContentData(
            currentReconstructor()->baseSettings().ds_sac, currentReconstructor()->baseSettings().ds_sic,
            currentReconstructor()->baseSettings().ds_line);

    while (dbcontent_man.dbContent("RefTraj").isDeleting())
    {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep(10);
    }

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    unsigned int ds_id = Number::dsIdFrom(currentReconstructor()->baseSettings().ds_sac,
                                          currentReconstructor()->baseSettings().ds_sic);

            // clear counts
    if (ds_man.hasDBDataSource(ds_id))
    {
        dbContent::DBDataSource& ds = ds_man.dbDataSource(ds_id);

        if (currentReconstructor()->baseSettings().delete_all_calc_reftraj)
            ds.clearNumInserted("RefTraj");
        else
            ds.clearNumInserted("RefTraj", currentReconstructor()->baseSettings().ds_line);
    }
}

