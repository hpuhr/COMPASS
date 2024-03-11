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
#include "timeconv.h"

#include <QApplication>
#include <QMessageBox>

using namespace std;
using namespace Utils;
using namespace dbContent;

ReconstructorTask::ReconstructorTask(const std::string& class_id, const std::string& instance_id,
                                     TaskManager& task_manager)
    : Task(task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_reconstructor.json")
{
    tooltip_ = "Associate target reports and calculate reference trajectories based on all DB Content.";

    reconstructor_.reset(new SimpleReconstructor());
}

ReconstructorTask::~ReconstructorTask()
{

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
    assert (reconstructor_);

    return COMPASS::instance().dbContentManager().hasData() && reconstructor_->hasNextTimeSlice();
}

void ReconstructorTask::run()
{
    assert(canRun());

    loginf << "ReconstructorTask: run: started";

    deleteCalculatedReferences();

    COMPASS::instance().viewManager().disableDataDistribution(true);

    reconstructor_.reset(new SimpleReconstructor());

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
    loginf << "ReconstructorTask: loadingDoneSlot";

    assert (reconstructor_);

    bool last_slice = !reconstructor_->hasNextTimeSlice();

    std::map<std::string, std::shared_ptr<Buffer>> data = std::move(data_); // move out of there

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    dbcontent_man.clearData(); // clear previous

    if (last_slice) // disconnect everything
    {
        disconnect(&dbcontent_man, &DBContentManager::loadedDataSignal,
                   this, &ReconstructorTask::loadedDataSlot);
        disconnect(&dbcontent_man, &DBContentManager::loadingDoneSignal,
                   this, &ReconstructorTask::loadingDoneSlot);

        COMPASS::instance().viewManager().disableDataDistribution(false);

        loginf << "ReconstructorTask: loadingDoneSlot: data loading done";

    }
    else // do next load
    {
        loadDataSlice();
    }

    // check if not already processing
    assert (reconstructor_->processSlice(std::move(data)));

//    if (!cache_)
//        cache_ = std::make_shared<dbContent::Cache> (dbcontent_man);

//    cache_->add(data_);


//    assert(status_dialog_);
//    status_dialog_->setStatus("Loading done, starting association");



            //assert(!create_job_);

//    job_ = std::make_shared<ReconstructorTaskJob>(
//        *this, COMPASS::instance().interface(), cache_);

//    connect(job_.get(), &ReconstructorTaskJob::doneSignal, this,
//            &ReconstructorTask::createDoneSlot, Qt::QueuedConnection);
//    connect(job_.get(), &ReconstructorTaskJob::obsoleteSignal, this,
//            &ReconstructorTask::createObsoleteSlot, Qt::QueuedConnection);
//    connect(job_.get(), &ReconstructorTaskJob::statusSignal, this,
//            &ReconstructorTask::associationStatusSlot, Qt::QueuedConnection);

//    JobManager::instance().addDBJob(create_job_);

}

void ReconstructorTask::closeStatusDialogSlot()
{
//    assert(status_dialog_);
//    status_dialog_->close();
//    status_dialog_ = nullptr;
}


void ReconstructorTask::deleteCalculatedReferences()
{
    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();
    dbcontent_man.clearData();

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    for (auto& ds_it : ds_man.dbDataSources())
    {
        if (ds_it->isCalculatedReferenceSource())
        {
            //status_dialog_->setStatus("Deleting From Data Source " + ds_it->name());

            dbcontent_man.dbContent("RefTraj").deleteDBContentData(ds_it->sac(), ds_it->sic());

            while (dbcontent_man.dbContent("RefTraj").isDeleting())
            {
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                QThread::msleep(10);
            }

            ds_it->clearNumInserted("RefTraj");
        }
    }
}

void ReconstructorTask::loadDataSlice()
{
    assert (reconstructor_);
    assert (reconstructor_->hasNextTimeSlice());

    boost::posix_time::ptime min_ts, max_ts;

    std::tie(min_ts, max_ts) = reconstructor_->getNextTimeSlice();
    assert (min_ts <= max_ts);

    bool last_slice = !reconstructor_->hasNextTimeSlice();

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

        VariableSet read_set = reconstructor_->getReadSetFor(dbo_it.first);

        dbo_it.second->load(read_set, false, false, timestamp_filter);
    }
}
