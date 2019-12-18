#include "manageschematask.h"
#include "manageschemataskwidget.h"
#include "taskmanager.h"
#include "atsdb.h"
#include "dbinterface.h"

ManageSchemaTask::ManageSchemaTask(const std::string& class_id, const std::string& instance_id,
                                   TaskManager& task_manager)
    : Task("ManageSchemaTask", "Manage DB Schema", true, true, task_manager),
      Configurable (class_id, instance_id, &task_manager, "task_manage_schema.json")
{
}

TaskWidget* ManageSchemaTask::widget ()
{
    if (!widget_)
    {
        widget_.reset(new ManageSchemaTaskWidget(*this));

        connect (&task_manager_, &TaskManager::expertModeChangedSignal,
                 widget_.get(), &ManageSchemaTaskWidget::expertModeChangedSlot);
    }

    return widget_.get();
}

void ManageSchemaTask::deleteWidget ()
{
    widget_.reset(nullptr);
}

void ManageSchemaTask::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    throw std::runtime_error ("ManageSchemaTask: generateSubConfigurable: unknown class_id "+class_id );
}

bool ManageSchemaTask::checkPrerequisites ()
{
    return ATSDB::instance().interface().ready();
}

