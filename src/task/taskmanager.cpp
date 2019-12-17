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
#include "taskmanagerlogwidget.h"
#include "dbobjectmanager.h"
#include "databaseopentask.h"
#include "databaseopentaskwidget.h"
#include "manageschematask.h"
#include "manageschemataskwidget.h"
#include "managedbobjectstask.h"
#include "managedbobjectstaskwidget.h"
#include "jsonimportertask.h"
#include "jsonimportertaskwidget.h"
#include "radarplotpositioncalculatortask.h"
#include "radarplotpositioncalculatortaskwidget.h"
#include "createartasassociationstask.h"
#include "createartasassociationstaskwidget.h"
#include "postprocesstask.h"
#include "postprocesstaskwidget.h"
#include "mysqldbimporttask.h"
#include "mysqldbimporttaskwidget.h"

#if USE_JASTERIX
#include "asteriximportertask.h"
#include "asteriximportertaskwidget.h"
#endif

#include <cassert>

TaskManager::TaskManager(const std::string &class_id, const std::string &instance_id, ATSDB *atsdb)
: Configurable (class_id, instance_id, atsdb, "task.json")
{
    registerParameter("expert_mode", &expert_mode_, false);

    createSubConfigurables();

    task_list_ = {"DatabaseOpenTask", "ManageSchemaTask", "ManageDBObjectsTask"}; // defines order of tasks

#if USE_JASTERIX
    task_list_.push_back("ASTERIXImportTask");
#endif

    task_list_.insert (task_list_.end(), {"JSONImportTask", "MySQLDBImportTask", "RadarPlotPositionCalculatorTask",
                                          "PostProcessTask", "CreateARTASAssociationsTask"});

    for (auto& task_it : task_list_) // check that all tasks in list exist
        assert (tasks_.count(task_it));

}

TaskManager::~TaskManager()
{
}

void TaskManager::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    if (class_id.compare ("DatabaseOpenTask") == 0)
    {
        assert (!database_open_task_);
        database_open_task_.reset (new DatabaseOpenTask (class_id, instance_id, *this));
        assert (database_open_task_);
        addTask (class_id, database_open_task_.get());
    }
    else if (class_id.compare ("ManageSchemaTask") == 0)
    {
        assert (!manage_schema_task_);
        manage_schema_task_.reset (new ManageSchemaTask (class_id, instance_id, *this));
        assert (manage_schema_task_);
        addTask (class_id, manage_schema_task_.get());
    }
    else if (class_id.compare ("ManageDBObjectsTask") == 0)
    {
        assert (!manage_dbobjects_task_);
        manage_dbobjects_task_.reset (new ManageDBObjectsTask (class_id, instance_id, *this));
        assert (manage_dbobjects_task_);
        addTask (class_id, manage_dbobjects_task_.get());
    }
#if USE_JASTERIX
    else if (class_id.compare ("ASTERIXImportTask") == 0)
    {
        assert (!asterix_importer_task_);
        asterix_importer_task_.reset(new ASTERIXImporterTask (class_id, instance_id, *this));
        assert (asterix_importer_task_);
        addTask (class_id, asterix_importer_task_.get());
    }
#endif
    else if (class_id.compare ("JSONImportTask") == 0)
    {
        assert (!json_importer_task_);
        json_importer_task_.reset(new JSONImporterTask (class_id, instance_id, *this));
        assert (json_importer_task_);
        addTask (class_id, json_importer_task_.get());
    }
    else if (class_id.compare ("MySQLDBImportTask") == 0)
    {
        assert (!mysqldb_import_task_);
        mysqldb_import_task_.reset(new MySQLDBImportTask (class_id, instance_id, *this));
        assert (mysqldb_import_task_);
        addTask (class_id, mysqldb_import_task_.get());
    }
    else if (class_id.compare ("RadarPlotPositionCalculatorTask") == 0)
    {
        assert (!radar_plot_position_calculator_task_);
        radar_plot_position_calculator_task_.reset(new RadarPlotPositionCalculatorTask (class_id, instance_id, *this));
        assert (radar_plot_position_calculator_task_);
        addTask (class_id, radar_plot_position_calculator_task_.get());
    }
    else if (class_id.compare ("CreateARTASAssociationsTask") == 0)
    {
        assert (!create_artas_associations_task_);
        create_artas_associations_task_.reset(new CreateARTASAssociationsTask (class_id, instance_id, *this));
        assert (create_artas_associations_task_);
        addTask (class_id, create_artas_associations_task_.get());
    }
    else if (class_id.compare ("PostProcessTask") == 0)
    {
        assert (!post_process_task_);
        post_process_task_.reset(new PostProcessTask (class_id, instance_id, *this));
        assert (post_process_task_);
        addTask (class_id, post_process_task_.get());
    }
    else
        throw std::runtime_error ("TaskManager: generateSubConfigurable: unknown class_id "+class_id );
}

void TaskManager::addTask (const std::string& class_id, Task* task)
{
    assert (task);
    assert (!tasks_.count(class_id));
    tasks_[class_id] = task;
    connect (task, &Task::statusChangedSignal, this, &TaskManager::taskStatusChangesSlot);
    connect (task, &Task::doneSignal, this, &TaskManager::taskDoneSlot);
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

#if USE_JASTERIX
    if (!asterix_importer_task_)
    {
        generateSubConfigurable("ASTERIXImportTask", "ASTERIXImportTask0");
        assert (asterix_importer_task_);
    }
#endif

    if (!json_importer_task_)
    {
        generateSubConfigurable("JSONImportTask", "JSONImportTask0");
        assert (json_importer_task_);
    }

    if (!mysqldb_import_task_)
    {
        generateSubConfigurable("MySQLDBImportTask", "MySQLDBImportTask0");
        assert (mysqldb_import_task_);
    }

    if (!radar_plot_position_calculator_task_)
    {
        generateSubConfigurable("RadarPlotPositionCalculatorTask", "RadarPlotPositionCalculatorTask0");
        assert (radar_plot_position_calculator_task_);
    }

    if (!create_artas_associations_task_)
    {
        generateSubConfigurable("CreateARTASAssociationsTask", "CreateARTASAssociationsTask0");
        assert (create_artas_associations_task_);
    }

    if (!post_process_task_)
    {
        generateSubConfigurable("PostProcessTask", "PostProcessTask0");
        assert (post_process_task_);
    }
}

std::map<std::string, Task *> TaskManager::tasks() const
{
    return tasks_;
}

bool TaskManager::expertMode() const
{
    return expert_mode_;
}

void TaskManager::expertMode(bool value)
{
    expert_mode_ = value;

    if (widget_)
    {
        widget_->updateTaskStates();
        widget_->selectNextTask();
    }
}

std::vector<std::string> TaskManager::taskList() const
{
    return task_list_;
}

void TaskManager::taskStatusChangesSlot (std::string task_name)
{
    loginf << "TaskManager: taskStatusChangesSlot: task " << task_name;

    if (widget_)
    {
        widget_->updateTaskStates();
        widget_->selectNextTask();
    }
}

void TaskManager::taskDoneSlot (std::string task_name)
{
    loginf << "TaskManager: taskDoneSlot: task " << task_name;

    if (widget_)
    {
        widget_->updateTaskStates();
        widget_->selectNextTask();
    }
}

void TaskManager::dbObjectsChangedSlot ()
{
    loginf << "TaskManager: dbObjectsChangedSlot";

    if (widget_)
    {
        widget_->updateTaskStates();
        widget_->selectNextTask();
    }
}

void TaskManager::schemaChangedSlot ()
{
    loginf << "TaskManager: schemaChangedSlot";

    if (widget_)
    {
        widget_->updateTaskStates();
        widget_->selectNextTask();
    }
}

//void TaskManager::disable ()
//{
//    loginf << "TaskManager: disable";

//    if (json_importer_task_ && json_importer_task_->hasOpenWidget())
//        json_importer_task_->widget()->close();

//    if (radar_plot_position_calculator_task_ && radar_plot_position_calculator_task_->hasOpenWidget())
//        radar_plot_position_calculator_task_->widget()->close();

//    if (create_artas_associations_task_ && create_artas_associations_task_->hasOpenWidget())
//        create_artas_associations_task_->widget()->close();

//#if USE_JASTERIX
//    if (asterix_importer_task_ && asterix_importer_task_->hasOpenWidget())
//        asterix_importer_task_->widget()->close();
//#endif
//}

//void TaskManager::deleteWidgets ()
//{
//    loginf << "TaskManager: deleteWidgets";

//    if (database_open_task_)
//        database_open_task_->deleteWidget();

//    if (manage_schema_task_)
//        manage_schema_task_->deleteWidget();

//    if (manage_dbobjects_task_)
//        manage_dbobjects_task_->deleteWidget();

//#if USE_JASTERIX
//    if (asterix_importer_task_)
//        asterix_importer_task_->deleteWidget();
//#endif

//    if (json_importer_task_)
//        json_importer_task_->deleteWidget();

//    if (radar_plot_position_calculator_task_)
//        radar_plot_position_calculator_task_->deleteWidget();

//    if (create_artas_associations_task_)
//        create_artas_associations_task_->deleteWidget();

//    if (post_process_task_)
//        post_process_task_->deleteWidget();
//}

void TaskManager::shutdown ()
{
    loginf << "TaskManager: shutdown";

    if (database_open_task_)
        database_open_task_ = nullptr;

    if (manage_schema_task_)
        manage_schema_task_.reset(nullptr);

    if (manage_dbobjects_task_)
        manage_dbobjects_task_.reset(nullptr);

#if USE_JASTERIX
    if (asterix_importer_task_)
        asterix_importer_task_.reset(nullptr);
#endif

    if (json_importer_task_)
        json_importer_task_.reset(nullptr);

    if (mysqldb_import_task_)
        mysqldb_import_task_.reset(nullptr);

    if (radar_plot_position_calculator_task_)
        radar_plot_position_calculator_task_.reset(nullptr);

    if (create_artas_associations_task_)
        create_artas_associations_task_.reset(nullptr);

    if (post_process_task_)
        post_process_task_.reset(nullptr);

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

void TaskManager::appendSuccess(const std::string& text)
{
    if (widget_)
        widget_->logWidget()->appendSuccess(text);
}
void TaskManager::appendInfo(const std::string& text)
{
    if (widget_)
        widget_->logWidget()->appendInfo(text);
}
void TaskManager::appendWarning(const std::string& text)
{
    if (widget_)
        widget_->logWidget()->appendWarning(text);
}
void TaskManager::appendError(const std::string& text)
{
    if (widget_)
        widget_->logWidget()->appendError(text);
}

void TaskManager::runTask (const std::string& task_name)
{
    loginf << "TaskManager: runTask: name " << task_name;

    assert (tasks_.count(task_name));
    assert (tasks_.at(task_name)->checkPrerequisites());

    tasks_.at(task_name)->run();
}

