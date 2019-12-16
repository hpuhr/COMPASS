#include "postprocesstask.h"
#include "taskmanager.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "dbobjectmanager.h"
#include "dbobject.h"
#include "dboactivedatasourcesdbjob.h"
#include "dbominmaxdbjob.h"
#include "jobmanager.h"
#include "stringconv.h"

#include <QProgressDialog>
#include <QMessageBox>
#include <QApplication>

using namespace Utils;

const std::string PostProcessTask::DONE_PROPERTY_NAME = "post_processed";

PostProcessTask::PostProcessTask(const std::string& class_id, const std::string& instance_id,
                                   TaskManager& task_manager)
    : Task("PostProcessTask", "Post-Process", false, false, task_manager),
      Configurable (class_id, instance_id, &task_manager)
{
}

QWidget* PostProcessTask::widget ()
{
    if (!widget_)
    {
        widget_.reset(new PostProcessTaskWidget(*this));
    }

    return widget_.get();
}

void PostProcessTask::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    throw std::runtime_error ("PostProcessTask: generateSubConfigurable: unknown class_id "+class_id );
}

bool PostProcessTask::checkPrerequisites ()
{
    if (!ATSDB::instance().interface().ready())
        return false;

    if (ATSDB::instance().interface().hasProperty(DONE_PROPERTY_NAME))
        done_ = ATSDB::instance().interface().getProperty(DONE_PROPERTY_NAME) == "1";

    //done_ = ATSDB::instance().interface().isPostProcessed();

    return ATSDB::instance().objectManager().hasData();
}

bool PostProcessTask::isRecommended ()
{
    if (!checkPrerequisites())
        return false;

    if (!ATSDB::instance().objectManager().hasData())
        return false;

    return !done_;
}

bool PostProcessTask::canRun()
{
    return ATSDB::instance().objectManager().hasData();
}


void PostProcessTask::run ()
{
    assert (!done_);

    loginf << "PostProcessTask: run: post-processing started";
//    connect (&ATSDB::instance().interface(), &DBInterface::postProcessingDoneSignal,
//             this, &PostProcessTask::postProcessingDoneSlot, Qt::UniqueConnection);

    task_manager_.appendInfo("PostProcessTask: started");

    start_time_ = boost::posix_time::microsec_clock::local_time();

    DBInterface& db_interface = ATSDB::instance().interface();

    loginf << "PostProcessTask: postProcess: creating jobs";

    assert (ATSDB::instance().objectManager().hasData());

    unsigned int dbos_with_data=0;

    for (auto obj_it : ATSDB::instance().objectManager())
        if (obj_it.second->hasData())
            ++dbos_with_data;

    assert (!postprocess_dialog_);
    postprocess_dialog_ = new QProgressDialog (tr(""), tr(""), 0, static_cast<int>(2*dbos_with_data));
    postprocess_dialog_->setWindowTitle("Post-Processing Status");
    postprocess_dialog_->setCancelButton(nullptr);
    postprocess_dialog_->setWindowModality(Qt::ApplicationModal);
    postprocess_dialog_->show();

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    if (!db_interface.existsMinMaxTable())
        db_interface.createMinMaxTable();
    else
        db_interface.clearTableContent (TABLE_NAME_MINMAX);

    for (auto obj_it : ATSDB::instance().objectManager())
    {
        if (!obj_it.second->hasData())
            continue;

        {
            DBOActiveDataSourcesDBJob* job = new DBOActiveDataSourcesDBJob (ATSDB::instance().interface(),
                                                                            *obj_it.second);

            std::shared_ptr<Job> shared_job = std::shared_ptr<Job> (job);
            connect (job, SIGNAL(doneSignal()), this, SLOT(postProcessingJobDoneSlot()), Qt::QueuedConnection);
            JobManager::instance().addDBJob(shared_job);
            postprocess_jobs_.push_back(shared_job);
        }
        {
            DBOMinMaxDBJob* job = new DBOMinMaxDBJob (ATSDB::instance().interface(), *obj_it.second);
            std::shared_ptr<Job> shared_job = std::shared_ptr<Job> (job);
            connect (job, SIGNAL(doneSignal()), this, SLOT(postProcessingJobDoneSlot()), Qt::QueuedConnection);
            JobManager::instance().addDBJob(shared_job);
            postprocess_jobs_.push_back(shared_job);
        }
    }

    assert (postprocess_jobs_.size() == 2*dbos_with_data);
    postprocess_job_num_ = postprocess_jobs_.size();
}

void PostProcessTask::postProcessingJobDoneSlot()
{
    loginf << "PostProcessTask: postProcessingJobDoneSlot: " << postprocess_jobs_.size() << " active jobs" ;

    Job* job_sender = static_cast <Job*> (QObject::sender());
    assert (job_sender);
    assert (postprocess_jobs_.size() > 0);
    assert (postprocess_dialog_);

    bool found=false;
    for (auto job_it = postprocess_jobs_.begin(); job_it != postprocess_jobs_.end(); job_it++)
    {
        Job *current = job_it->get();
        if (current == job_sender)
        {
            postprocess_jobs_.erase(job_it);
            found = true;
            break;
        }
    }
    assert (found);

    if (postprocess_jobs_.size() == 0)
    {
        loginf << "PostProcessTask: postProcessingJobDoneSlot: done";
        //setPostProcessed(true);

        stop_time_ = boost::posix_time::microsec_clock::local_time();

        boost::posix_time::time_duration diff = stop_time_ - start_time_;

        std::string time_str = String::timeStringFromDouble(diff.total_milliseconds()/1000.0, false);

        ATSDB::instance().interface().setProperty(DONE_PROPERTY_NAME, "1");

        delete postprocess_dialog_;
        postprocess_dialog_=nullptr;

        task_manager_.appendSuccess("PostProcessTask: done after "+time_str);
        done_ = true;

        QApplication::restoreOverrideCursor();

        //emit postProcessingDoneSignal();

        emit doneSignal(name_);
    }
    else
        postprocess_dialog_->setValue(postprocess_job_num_-postprocess_jobs_.size());
}

//void PostProcessTask::postProcessingDoneSlot ()
//{
//    loginf << "PostProcessTask: postProcessingDoneSlot";
//    done_ = true;
//}
