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


#include "managedatasourcestask.h"
#include "managedatasourcestaskwidget.h"
#include "taskmanager.h"
#include "atsdb.h"
#include "dbinterface.h"

ManageDataSourcesTask::ManageDataSourcesTask(const std::string& class_id, const std::string& instance_id,
                                   TaskManager& task_manager)
    : Task("ManageDataSourcesTask", "Manage Data Sources", true, false, task_manager),
      Configurable (class_id, instance_id, &task_manager, "task_manage_datasources.json")
{
}

TaskWidget* ManageDataSourcesTask::widget ()
{
    if (!widget_)
    {
        widget_.reset(new ManageDataSourcesTaskWidget(*this));

        connect (&task_manager_, &TaskManager::expertModeChangedSignal,
                 widget_.get(), &ManageDataSourcesTaskWidget::expertModeChangedSlot);
    }

    return widget_.get();
}

void ManageDataSourcesTask::deleteWidget ()
{
    widget_.reset(nullptr);
}

void ManageDataSourcesTask::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    throw std::runtime_error ("ManageDataSourcesTask: generateSubConfigurable: unknown class_id "+class_id );
}

bool ManageDataSourcesTask::checkPrerequisites ()
{
    if (!ATSDB::instance().interface().ready())
        return false;

    return true;
}

bool ManageDataSourcesTask::isRecommended ()
{
    return false;
}
