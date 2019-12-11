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

#include "atsdb.h"
#include "taskmanager.h"
#include "taskmanagerwidget.h"
#include "databaseopentask.h"
#include "manageschematask.h"
#include "managedbobjectstask.h"
#include "jsonimportertask.h"
#include "jsonimportertaskwidget.h"
#include "radarplotpositioncalculatortask.h"
#include "radarplotpositioncalculatortaskwidget.h"
#include "createartasassociationstask.h"
#include "createartasassociationstaskwidget.h"

#if USE_JASTERIX
#include "asteriximportertask.h"
#include "asteriximportertaskwidget.h"
#endif

#include <cassert>

TaskManager::TaskManager(const std::string &class_id, const std::string &instance_id, ATSDB *atsdb)
: Configurable (class_id, instance_id, atsdb, "task.json")
{
    createSubConfigurables();

    task_list_ = {"DatabaseOpenTask", "ManageSchemaTask", "ManageDBObjectsTask"}; // defines order of tasks

    for (auto& task_it : task_list_) // check that all tasks in list exist
        assert (tasks_.count(task_it));
}

TaskManager::~TaskManager()
{
    assert (!json_importer_task_);
    assert (!radar_plot_position_calculator_task_);

#if USE_JASTERIX
    assert (!asterix_importer_task_);
#endif
}

JSONImporterTask* TaskManager::getJSONImporterTask()
{
    assert (json_importer_task_);
    return json_importer_task_;
}

RadarPlotPositionCalculatorTask* TaskManager::getRadarPlotPositionCalculatorTask()
{
    assert (radar_plot_position_calculator_task_);
    return radar_plot_position_calculator_task_;
}

CreateARTASAssociationsTask* TaskManager::getCreateARTASAssociationsTask()
{
    assert (create_artas_associations_task_);
    return create_artas_associations_task_;
}

#if USE_JASTERIX
ASTERIXImporterTask* TaskManager::getASTERIXImporterTask()
{
    assert (asterix_importer_task_);
    return asterix_importer_task_;
}
#endif

void TaskManager::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    if (class_id.compare ("DatabaseOpenTask") == 0)
    {
        assert (!database_open_task_);
        database_open_task_.reset (new DatabaseOpenTask (class_id, instance_id, *this));
        assert (database_open_task_);

        assert (!tasks_.count(class_id));
        tasks_[class_id] = database_open_task_.get();
    }
    else if (class_id.compare ("ManageSchemaTask") == 0)
    {
        assert (!manage_schema_task_);
        manage_schema_task_.reset (new ManageSchemaTask (class_id, instance_id, *this));
        assert (manage_schema_task_);

        assert (!tasks_.count(class_id));
        tasks_[class_id] = manage_schema_task_.get();
    }
    else if (class_id.compare ("ManageDBObjectsTask") == 0)
    {
        assert (!manage_dbobjects_task_);
        manage_dbobjects_task_.reset (new ManageDBObjectsTask (class_id, instance_id, *this));
        assert (manage_dbobjects_task_);

        assert (!tasks_.count(class_id));
        tasks_[class_id] = manage_dbobjects_task_.get();
    }
    else if (class_id.compare ("JSONImporterTask") == 0)
    {
        assert (!json_importer_task_);
        json_importer_task_ = new JSONImporterTask (class_id, instance_id, this);
        assert (json_importer_task_);
    }
    else if (class_id.compare ("RadarPlotPositionCalculatorTask") == 0)
    {
        assert (!radar_plot_position_calculator_task_);
        radar_plot_position_calculator_task_ = new RadarPlotPositionCalculatorTask (class_id, instance_id, this);
        assert (radar_plot_position_calculator_task_);
    }
    else if (class_id.compare ("CreateARTASAssociationsTask") == 0)
    {
        assert (!create_artas_associations_task_);
        create_artas_associations_task_ = new CreateARTASAssociationsTask (class_id, instance_id, this);
        assert (create_artas_associations_task_);
    }
#if USE_JASTERIX
    else if (class_id.compare ("ASTERIXImporterTask") == 0)
    {
        assert (!asterix_importer_task_);
        asterix_importer_task_ = new ASTERIXImporterTask (class_id, instance_id, this);
        assert (asterix_importer_task_);
    }
#endif
    else
        throw std::runtime_error ("TaskManager: generateSubConfigurable: unknown class_id "+class_id );
}

void TaskManager::checkSubConfigurables ()
{
    if (!database_open_task_)
    {
        generateSubConfigurable("DatabaseOpenTask", "DatabaseOpenTask0");
        assert (database_open_task_);
    }

    if (!manage_schema_task_)
    {
        generateSubConfigurable("ManageSchemaTask", "ManageSchemaTask0");
        assert (manage_schema_task_);
    }

    if (!manage_dbobjects_task_)
    {
        generateSubConfigurable("ManageDBObjectsTask", "ManageDBObjectsTask0");
        assert (manage_dbobjects_task_);
    }

    if (!json_importer_task_)
    {
        json_importer_task_ = new JSONImporterTask ("JSONImporterTask", "JSONImporterTask", this);
        assert (json_importer_task_);
    }

    if (!radar_plot_position_calculator_task_)
    {
        radar_plot_position_calculator_task_ = new RadarPlotPositionCalculatorTask (
                    "RadarPlotPositionCalculatorTask", "RadarPlotPositionCalculatorTask0", this);
        assert (radar_plot_position_calculator_task_);
    }

    if (!create_artas_associations_task_)
    {
        create_artas_associations_task_ = new CreateARTASAssociationsTask (
                    "CreateARTASAssociationsTask", "CreateARTASAssociationsTask0", this);
        assert (create_artas_associations_task_);
    }

#if USE_JASTERIX
    if (!asterix_importer_task_)
    {
        assert (!asterix_importer_task_);
        asterix_importer_task_ = new ASTERIXImporterTask ("ASTERIXImporterTask", "ASTERIXImporterTask0", this);
        assert (asterix_importer_task_);
    }
#endif
}

std::map<std::string, Task *> TaskManager::tasks() const
{
    return tasks_;
}

std::vector<std::string> TaskManager::taskList() const
{
    return task_list_;
}

void TaskManager::disable ()
{
    loginf << "TaskManager: disable";

    if (json_importer_task_ && json_importer_task_->hasOpenWidget())
        json_importer_task_->widget()->close();

    if (radar_plot_position_calculator_task_ && radar_plot_position_calculator_task_->hasOpenWidget())
        radar_plot_position_calculator_task_->widget()->close();

    if (create_artas_associations_task_ && create_artas_associations_task_->hasOpenWidget())
        create_artas_associations_task_->widget()->close();

#if USE_JASTERIX
    if (asterix_importer_task_ && asterix_importer_task_->hasOpenWidget())
        asterix_importer_task_->widget()->close();
#endif
}

void TaskManager::shutdown ()
{
    loginf << "TaskManager: shutdown";
    // TODO waiting for tasks?

    if (database_open_task_)
        database_open_task_.reset(nullptr);

    if (json_importer_task_)
    {
        delete json_importer_task_;
        json_importer_task_ = nullptr;
    }

    if (radar_plot_position_calculator_task_)
    {
        delete radar_plot_position_calculator_task_;
        radar_plot_position_calculator_task_ = nullptr;
    }

    if (create_artas_associations_task_)
    {
        delete create_artas_associations_task_;
        create_artas_associations_task_ = nullptr;
    }

#if USE_JASTERIX
    if (asterix_importer_task_)
    {
        delete asterix_importer_task_;
        asterix_importer_task_ = nullptr;
    }
#endif

    widget_ = nullptr;
}

TaskManagerWidget* TaskManager::widget()
{
    if (!widget_)
    {
        widget_.reset(new TaskManagerWidget(*this));
    }

    return widget_.get();
}
