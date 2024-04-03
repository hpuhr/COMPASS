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

    loginf << "ReconstructorTask: run: started";

    deleteCalculatedReferences();

    DBContentManager& cont_man = COMPASS::instance().dbContentManager();
    cont_man.clearTargetsInfo();

    COMPASS::instance().viewManager().disableDataDistribution(true);

    currentReconstructor()->reset();

    QMessageBox box;
    box.setText("Running Reconstruction...");
    box.show();

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    connect(&dbcontent_man, &DBContentManager::loadedDataSignal,
            this, &ReconstructorTask::loadedDataSlot);
    connect(&dbcontent_man, &DBContentManager::loadingDoneSignal,
            this, &ReconstructorTask::loadingDoneSlot);

    loadDataSlice();

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

    std::map<std::string, std::shared_ptr<Buffer>> data = std::move(data_); // move out of there

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    dbcontent_man.clearData(); // clear previous

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

    for (auto& dbo_it : dbcontent_man)
    {
        if (!dbo_it.second->hasData())
            continue;

        VariableSet read_set = currentReconstructor()->getReadSetFor(dbo_it.first);

        dbo_it.second->load(read_set, false, false, timestamp_filter);
    }
}
