/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "managedbobjectstask.h"

#include "atsdb.h"
#include "dbinterface.h"
#include "dbschemamanager.h"
#include "managedbobjectstaskwidget.h"
#include "taskmanager.h"

ManageDBObjectsTask::ManageDBObjectsTask(const std::string& class_id,
                                         const std::string& instance_id, TaskManager& task_manager)
    : Task("ManageDBObjectsTask", "Manage DBObjects", true, true, task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_manage_dbojects.json")
{
    tooltip_ =
        "Allows management of DBObjects and is reserved for expert users. This task can not be run,"
        " but is performed using the GUI elements.";
}

TaskWidget* ManageDBObjectsTask::widget()
{
    if (!widget_)
    {
        widget_.reset(new ManageDBObjectsTaskWidget(*this));

        connect(&task_manager_, &TaskManager::expertModeChangedSignal, widget_.get(),
                &ManageDBObjectsTaskWidget::expertModeChangedSlot);
    }

    return widget_.get();
}

void ManageDBObjectsTask::deleteWidget() { widget_.reset(nullptr); }

void ManageDBObjectsTask::generateSubConfigurable(const std::string& class_id,
                                                  const std::string& instance_id)
{
    throw std::runtime_error("ManageDBObjectsTask: generateSubConfigurable: unknown class_id " +
                             class_id);
}

bool ManageDBObjectsTask::checkPrerequisites()
{
    if (!ATSDB::instance().interface().ready())
        return false;

    return ATSDB::instance().schemaManager().isLocked();
}
