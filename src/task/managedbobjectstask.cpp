#include "managedbobjectstask.h"
#include "managedbobjectstaskwidget.h"
#include "taskmanager.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "dbschemamanager.h"

ManageDBObjectsTask::ManageDBObjectsTask(const std::string& class_id, const std::string& instance_id,
                                   TaskManager& task_manager)
    : Task("ManageDBObjectsTask", "Manage DBObjects", true, true, task_manager),
      Configurable (class_id, instance_id, &task_manager, "task_manage_dbojects.json")
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

void ManageDBObjectsTask::deleteWidget ()
{
    widget_.reset(nullptr);
}

void ManageDBObjectsTask::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    throw std::runtime_error ("ManageDBObjectsTask: generateSubConfigurable: unknown class_id "+class_id );
}

bool ManageDBObjectsTask::checkPrerequisites ()
{
    if (!ATSDB::instance().interface().ready())
        return false;

    return ATSDB::instance().schemaManager().isLocked();
}

