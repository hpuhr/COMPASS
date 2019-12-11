#include "managedbobjectstask.h"
#include "taskmanager.h"

ManageDBObjectsTask::ManageDBObjectsTask(const std::string& class_id, const std::string& instance_id,
                                   TaskManager& task_manager)
    : Configurable (class_id, instance_id, &task_manager), Task("ManageDBObjectsTask", "Manage DBObjects", true,
                                                                task_manager)
{
}

QWidget* ManageDBObjectsTask::widget ()
{
    if (!widget_)
    {
        widget_.reset(new ManageDBObjectsTaskWidget(*this));
    }

    return widget_.get();
}

void ManageDBObjectsTask::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    throw std::runtime_error ("ManageDBObjectsTask: generateSubConfigurable: unknown class_id "+class_id );
}
