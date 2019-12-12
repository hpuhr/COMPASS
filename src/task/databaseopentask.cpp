#include "databaseopentask.h"
#include "taskmanager.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "dbinterfacewidget.h"

DatabaseOpenTask::DatabaseOpenTask(const std::string& class_id, const std::string& instance_id,
                                   TaskManager& task_manager)
    : Task("DatabaseOpenTask", "Open a Database", true, false, task_manager),
      Configurable (class_id, instance_id, &task_manager)
{
}

QWidget* DatabaseOpenTask::widget ()
{
    if (!widget_)
    {
        widget_.reset(new DatabaseOpenTaskWidget(*this));

        QObject::connect(ATSDB::instance().interface().widget(), &DBInterfaceWidget::databaseOpenedSignal,
                         this, &DatabaseOpenTask::databaseOpenedSlot);
    }

    return widget_.get();
}

void DatabaseOpenTask::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    throw std::runtime_error ("DatabaseOpenTask: generateSubConfigurable: unknown class_id "+class_id );
}

bool DatabaseOpenTask::checkPrerequisites ()
{
    if (ATSDB::instance().interface().ready())
        return false;

    return true;
}

void DatabaseOpenTask::databaseOpenedSlot()
{
    loginf << "DatabaseOpenTask: databaseOpenedSlot";

    emit doneSignal(name_);
}
