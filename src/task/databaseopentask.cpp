#include "databaseopentask.h"
#include "taskmanager.h"

DatabaseOpenTask::DatabaseOpenTask(const std::string& class_id, const std::string& instance_id,
                                   TaskManager& task_manager)
    : Configurable (class_id, instance_id, &task_manager), Task("DatabaseOpenTask", "Open a Database", true,
                                                                task_manager)
{
}

QWidget* DatabaseOpenTask::widget ()
{
    if (!widget_)
    {
        widget_.reset(new DatabaseOpenTaskWidget(*this));
    }

    return widget_.get();
}

void DatabaseOpenTask::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    throw std::runtime_error ("DatabaseOpenTask: generateSubConfigurable: unknown class_id "+class_id );
}
