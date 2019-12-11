#include "manageschematask.h"
#include "taskmanager.h"

ManageSchemaTask::ManageSchemaTask(const std::string& class_id, const std::string& instance_id,
                                   TaskManager& task_manager)
    : Configurable (class_id, instance_id, &task_manager), Task("ManageSchemaTask", "Manage DB Schema", true,
                                                                task_manager)
{
}

QWidget* ManageSchemaTask::widget ()
{
    if (!widget_)
    {
        widget_.reset(new ManageSchemaTaskWidget(*this));
    }

    return widget_.get();
}

void ManageSchemaTask::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    throw std::runtime_error ("ManageSchemaTask: generateSubConfigurable: unknown class_id "+class_id );
}
