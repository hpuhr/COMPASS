#include "reconstructortask.h"

#include "compass.h"
#include "reconstructortaskdialog.h"
#include "reconstructortaskjob.h"
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
    return true;
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

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

//    if (!cache_)
//        cache_ = std::make_shared<dbContent::Cache> (dbcontent_man);

//    cache_->add(data_);

    disconnect(&dbcontent_man, &DBContentManager::loadedDataSignal,
               this, &ReconstructorTask::loadedDataSlot);
    disconnect(&dbcontent_man, &DBContentManager::loadingDoneSignal,
               this, &ReconstructorTask::loadingDoneSlot);

//    assert(status_dialog_);
//    status_dialog_->setStatus("Loading done, starting association");

    dbcontent_man.clearData();

    COMPASS::instance().viewManager().disableDataDistribution(false);

    loginf << "ReconstructorTask: loadingDoneSlot: data loading done";

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

}
