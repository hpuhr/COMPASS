#include "reconstructortask.h"

#include "compass.h"
#include "reconstructortaskdialog.h"
#include "datasourcemanager.h"
#include "dbinterface.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/variableset.h"
#include "jobmanager.h"
#include "dbcontent/variable/metavariable.h"
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

#include <QApplication>
#include <QMessageBox>
#include <QProgressDialog>

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

void ReconstructorTask::run()
{
    assert(canRun());

    current_slice_idx_ = 0;

    loginf << "ReconstructorTask: run: started";

    //calculated_reftraj_ds_id = Number::dsIdFrom(currentReconstructor()->ds_sac_, currentReconstructor()->ds_sic_);

    progress_dialog_.reset(new QProgressDialog);
    progress_dialog_->setCancelButton(nullptr);
    progress_dialog_->setMinimum(0);
    progress_dialog_->setMinimum(100);
    progress_dialog_->setWindowTitle("Reconstructing...");
    progress_dialog_->show();

    updateProgress("Initializing", false);

    deleteCalculatedReferences();

    DBContentManager& cont_man = COMPASS::instance().dbContentManager();
    cont_man.clearTargetsInfo();

    COMPASS::instance().viewManager().disableDataDistribution(true);

    currentReconstructor()->reset();

    // QMessageBox box;
    // box.setText("Running Reconstruction...");
    // box.show();

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    connect(&dbcontent_man, &DBContentManager::loadedDataSignal,
            this, &ReconstructorTask::loadedDataSlot);
    connect(&dbcontent_man, &DBContentManager::loadingDoneSignal,
            this, &ReconstructorTask::loadingDoneSlot);

    loadDataSlice();
}

void ReconstructorTask::updateProgress(const QString& msg, bool add_slice_progress)
{
    if (!progress_dialog_)
        return;

    int ns = currentReconstructor()->numSlices();
    QString slice_p;
    if (ns >= 0)
        slice_p = " " + QString::number(current_slice_idx_ + 1) + "/" + QString::number(ns);

    QString pmsg = msg;
    if (add_slice_progress)
        pmsg += slice_p;

    double progress = 0.0;
    if (ns >= 1)
        progress = (double)current_slice_idx_ / (double)ns;

    //@TODO: helmut (progress bar not updated)
    progress_dialog_->setLabelText(pmsg);
    progress_dialog_->setValue(progress_dialog_->maximum() * progress);
    
    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();
    while ((boost::posix_time::microsec_clock::local_time() - start_time).total_milliseconds() < 50)
        qApp->processEvents();
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

    disabled_ds.insert(Number::dsIdFrom(currentReconstructor()->ds_sac_,
                                        currentReconstructor()->ds_sic_));

    for (auto& ds_it : COMPASS::instance().dataSourceManager().dbDataSources())
    {
        if (ds_it->dsType() == "Radar")
            disabled_ds.insert(ds_it->id());
    }

    return disabled_ds;
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

void ReconstructorTask::loadedDataSlot(const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset)
{
    data_ = data;

//    assert (status_dialog_);
//    status_dialog_->updateTime();
}

void ReconstructorTask::loadingDoneSlot()
{
    assert (currentReconstructor());

    bool last_slice = !currentReconstructor()->hasNextTimeSlice();

    loginf << "ReconstructorTask: loadingDoneSlot: last_slice " << last_slice;

    updateProgress("Processing slice", true);

    std::map<std::string, std::shared_ptr<Buffer>> data = std::move(data_); // move out of there

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    dbcontent_man.clearData(); // clear previous

    ++current_slice_idx_;

    // TODO: do async, check if not already processing
    assert (currentReconstructor()->processSlice(std::move(data)));

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
        loginf << "ReconstructorTask: loadingDoneSlot: data loading done";
        currentReconstructor()->reset();

        COMPASS::instance().dbContentManager().setAssociationsIdentifier("All");

        done_ = true;

        //close progress dialog
        progress_dialog_.reset();

        emit doneSignal();
    }

//    assert(status_dialog_);
//    status_dialog_->setStatus("Loading done, starting association");

}

void ReconstructorTask::closeStatusDialogSlot()
{
//    assert(status_dialog_);
//    status_dialog_->close();
//    status_dialog_ = nullptr;
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

    dbcontent_man.dbContent("RefTraj").deleteDBContentData(
        currentReconstructor()->ds_sac_, currentReconstructor()->ds_sic_,
        currentReconstructor()->ds_line_);

    while (dbcontent_man.dbContent("RefTraj").isDeleting())
    {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep(10);
    }

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    unsigned int ds_id = Number::dsIdFrom(currentReconstructor()->ds_sac_,
                                          currentReconstructor()->ds_sic_);

    // clear counts
    if (ds_man.hasDBDataSource(ds_id))
    {
        dbContent::DBDataSource& ds = ds_man.dbDataSource(ds_id);

        ds.clearNumInserted("RefTraj", currentReconstructor()->ds_line_);
    }

//    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

//    for (auto& ds_it : ds_man.dbDataSources())
//    {
//        if (ds_it->isCalculatedReferenceSource())
//        {
//            //status_dialog_->setStatus("Deleting From Data Source " + ds_it->name());

//            dbcontent_man.dbContent("RefTraj").deleteDBContentData(
//                currentReconstructor()->ds_sac_, currentReconstructor()->ds_sic_,
//                currentReconstructor()->ds_line_);

//            while (dbcontent_man.dbContent("RefTraj").isDeleting())
//            {
//                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
//                QThread::msleep(10);
//            }

//            ds_it->clearNumInserted("RefTraj");
//        }
//    }
}

void ReconstructorTask::loadDataSlice()
{
    assert (currentReconstructor());
    assert (currentReconstructor()->hasNextTimeSlice());

    updateProgress("Loading slice", true);

    boost::posix_time::ptime min_ts, max_ts;

    std::tie(min_ts, max_ts) = currentReconstructor()->getNextTimeSlice();
    assert (min_ts <= max_ts);

    bool last_slice = !currentReconstructor()->hasNextTimeSlice();

    loginf << "ReconstructorTask: loadDataSlice: min " << Time::toString(min_ts)
           << " max " << Time::toString(max_ts) << " last " << last_slice;

    string timestamp_filter;

    timestamp_filter = "timestamp >= " + to_string(Time::toLong(min_ts));

    if (last_slice)
        timestamp_filter += " AND timestamp <= " + to_string(Time::toLong(max_ts));
    else
        timestamp_filter += " AND timestamp < " + to_string(Time::toLong(max_ts));

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
