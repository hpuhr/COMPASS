#include "postprocesstask.h"
#include "taskmanager.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "dbobjectmanager.h"

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

    done_ = ATSDB::instance().interface().isPostProcessed();

    return ATSDB::instance().objectManager().hasData();
}

void PostProcessTask::run ()
{
    assert (!done_);

    loginf << "PostProcessTask: run: post-processing started";
    connect (&ATSDB::instance().interface(), &DBInterface::postProcessingDoneSignal,
             this, &PostProcessTask::postProcessingDoneSlot, Qt::UniqueConnection);

    ATSDB::instance().interface().postProcess();
}

void PostProcessTask::postProcessingDoneSlot ()
{
    loginf << "PostProcessTask: postProcessingDoneSlot";
    done_ = true;
}
