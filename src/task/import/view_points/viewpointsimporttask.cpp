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

#include "viewpointsimporttask.h"
#include "viewpointsimporttaskwidget.h"
#include "taskmanager.h"
#include "atsdb.h"

ViewPointsImportTask::ViewPointsImportTask(const std::string& class_id, const std::string& instance_id,
                                           TaskManager& task_manager)
    : Task("ViewPointsImportTask", "Import View Points", true, false, task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_import_view_points.json")
{
    tooltip_ =
            "Allows import of view points and associated datasets. This task can not "
            "be run, but is performed using the 'Import' button.";

    createSubConfigurables(); // no thing
}

TaskWidget* ViewPointsImportTask::widget()
{
    if (!widget_)
    {
        widget_.reset(new ViewPointsImportTaskWidget(*this));

//        connect(widget_.get(), &ViewPointsImportTaskWidget::databaseOpenedSignal, this,
//                &ViewPointsImportTask::databaseOpenedSlot);
        connect(&task_manager_, &TaskManager::expertModeChangedSignal, widget_.get(),
                &ViewPointsImportTaskWidget::expertModeChangedSlot);
    }

    return widget_.get();
}

void ViewPointsImportTask::deleteWidget() { widget_.reset(nullptr); }

void ViewPointsImportTask::generateSubConfigurable(const std::string& class_id,
                                               const std::string& instance_id)
{
    throw std::runtime_error("ViewPointsImportTask: generateSubConfigurable: unknown class_id " +
                             class_id);
}

bool ViewPointsImportTask::checkPrerequisites()
{
    return true; //!ATSDB::instance().interface().ready();  // only usable if not already connected
}

bool ViewPointsImportTask::isRecommended()
{
    if (!checkPrerequisites())
        return false;

    return true;
}

bool ViewPointsImportTask::isRequired()
{
    if (!checkPrerequisites())
        return false;

    return true;
}
