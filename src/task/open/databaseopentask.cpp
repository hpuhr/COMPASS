/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "databaseopentask.h"

#include "atsdb.h"
#include "databaseopentaskwidget.h"
#include "dbconnection.h"
#include "dbinterface.h"
#include "taskmanager.h"

DatabaseOpenTask::DatabaseOpenTask(const std::string& class_id, const std::string& instance_id,
                                   TaskManager& task_manager)
    : Task("DatabaseOpenTask", "Open a Database", true, false, task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_db_open.json")
{
    tooltip_ =
        "Allows creating new databases, or managing and accessing existing ones. This task can not "
        "be run, but is performed using the 'Open' button.";
}

void DatabaseOpenTask::useConnection(const std::string& connection_type)
{
    ATSDB::instance().interface().useConnection(connection_type);

    if (widget_)
        widget_->updateUsedConnection();
}

TaskWidget* DatabaseOpenTask::widget()
{
    if (!widget_)
    {
        widget_.reset(new DatabaseOpenTaskWidget(*this, ATSDB::instance().interface()));

        connect(widget_.get(), &DatabaseOpenTaskWidget::databaseOpenedSignal, this,
                &DatabaseOpenTask::databaseOpenedSlot);
        connect(&task_manager_, &TaskManager::expertModeChangedSignal, widget_.get(),
                &DatabaseOpenTaskWidget::expertModeChangedSlot);
    }

    return widget_.get();
}

void DatabaseOpenTask::deleteWidget() { widget_.reset(nullptr); }

void DatabaseOpenTask::generateSubConfigurable(const std::string& class_id,
                                               const std::string& instance_id)
{
    throw std::runtime_error("DatabaseOpenTask: generateSubConfigurable: unknown class_id " +
                             class_id);
}

bool DatabaseOpenTask::checkPrerequisites()
{
    return !ATSDB::instance().interface().ready();  // only usable if not already connected
}

bool DatabaseOpenTask::isRecommended()
{
    if (!checkPrerequisites())
        return false;

    return true;
}

bool DatabaseOpenTask::isRequired()
{
    if (!checkPrerequisites())
        return false;

    return true;
}

void DatabaseOpenTask::databaseOpenedSlot()
{
    loginf << "DatabaseOpenTask: databaseOpenedSlot";
    done_ = true;

    task_manager_.appendSuccess(
        "DatabaseOpenTask: database '" + ATSDB::instance().interface().connection().type() + ":" +
        ATSDB::instance().interface().connection().identifier() + "' opened");

    emit doneSignal(name_);
}
