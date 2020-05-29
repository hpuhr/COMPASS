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

#include "taskmanager.h"

#include "atsdb.h"
#include "createartasassociationstask.h"
#include "createartasassociationstaskwidget.h"
#include "databaseopentask.h"
#include "databaseopentaskwidget.h"
#include "dbobjectmanager.h"
#include "dboeditdatasourceswidget.h"
#include "jsonimporttask.h"
#include "jsonimporttaskwidget.h"
#include "jsonparsingschema.h"
#include "managedatasourcestask.h"
#include "managedatasourcestaskwidget.h"
#include "managedbobjectstask.h"
#include "managedbobjectstaskwidget.h"
#include "manageschematask.h"
#include "manageschemataskwidget.h"
#include "mysqldbimporttask.h"
#include "mysqldbimporttaskwidget.h"
#include "postprocesstask.h"
#include "postprocesstaskwidget.h"
#include "radarplotpositioncalculatortask.h"
#include "radarplotpositioncalculatortaskwidget.h"
#include "taskmanagerlogwidget.h"
#include "taskmanagerwidget.h"
#include "sqliteconnectionwidget.h"
#include "dbinterface.h"
#include "files.h"
#include "viewpointsimporttask.h"
#include "viewpointsimporttaskwidget.h"
#include "gpstrailimporttask.h"
#include "gpstrailimporttaskwidget.h"

#if USE_JASTERIX
#include "asteriximporttask.h"
#include "asteriximporttaskwidget.h"
#endif

#include <cassert>

#include <QCoreApplication>
#include <QThread>

using namespace Utils;

TaskManager::TaskManager(const std::string& class_id, const std::string& instance_id, ATSDB* atsdb)
    : Configurable(class_id, instance_id, atsdb, "task.json")
{
    registerParameter("expert_mode", &expert_mode_, false);

    createSubConfigurables();

    task_list_ = {"DatabaseOpenTask", "ManageSchemaTask",
                  "ManageDBObjectsTask", "ViewPointsImportTask"};  // defines order of tasks

#if USE_JASTERIX
    task_list_.push_back("ASTERIXImportTask");
#endif

    task_list_.insert(task_list_.end(), {"JSONImportTask", "MySQLDBImportTask", "GPSTrailImportTask",
                                         "ManageDataSourcesTask", "RadarPlotPositionCalculatorTask",
                                         "PostProcessTask", "CreateARTASAssociationsTask"});

    for (auto& task_it : task_list_)  // check that all tasks in list exist
        assert(tasks_.count(task_it));
}

TaskManager::~TaskManager() {}

void TaskManager::generateSubConfigurable(const std::string& class_id,
                                          const std::string& instance_id)
{
    if (class_id.compare("DatabaseOpenTask") == 0)
    {
        assert(!database_open_task_);
        database_open_task_.reset(new DatabaseOpenTask(class_id, instance_id, *this));
        assert(database_open_task_);
        addTask(class_id, database_open_task_.get());
    }
    else if (class_id.compare("ManageSchemaTask") == 0)
    {
        assert(!manage_schema_task_);
        manage_schema_task_.reset(new ManageSchemaTask(class_id, instance_id, *this));
        assert(manage_schema_task_);
        addTask(class_id, manage_schema_task_.get());
    }
    else if (class_id.compare("ManageDBObjectsTask") == 0)
    {
        assert(!manage_dbobjects_task_);
        manage_dbobjects_task_.reset(new ManageDBObjectsTask(class_id, instance_id, *this));
        assert(manage_dbobjects_task_);
        addTask(class_id, manage_dbobjects_task_.get());
    }
#if USE_JASTERIX
    else if (class_id.compare("ASTERIXImportTask") == 0)
    {
        assert(!asterix_importer_task_);
        asterix_importer_task_.reset(new ASTERIXImportTask(class_id, instance_id, *this));
        assert(asterix_importer_task_);
        addTask(class_id, asterix_importer_task_.get());
    }
#endif
    else if (class_id.compare("ViewPointsImportTask") == 0)
    {
        assert(!view_points_import_task_);
        view_points_import_task_.reset(new ViewPointsImportTask(class_id, instance_id, *this));
        assert(view_points_import_task_);
        addTask(class_id, view_points_import_task_.get());
    }
    else if (class_id.compare("JSONImportTask") == 0)
    {
        assert(!json_import_task_);
        json_import_task_.reset(new JSONImportTask(class_id, instance_id, *this));
        assert(json_import_task_);
        addTask(class_id, json_import_task_.get());
    }
    else if (class_id.compare("MySQLDBImportTask") == 0)
    {
        assert(!mysqldb_import_task_);
        mysqldb_import_task_.reset(new MySQLDBImportTask(class_id, instance_id, *this));
        assert(mysqldb_import_task_);
        addTask(class_id, mysqldb_import_task_.get());
    }
    else if (class_id.compare("GPSTrailImportTask") == 0)
    {
        assert(!gps_trail_import_task_);
        gps_trail_import_task_.reset(new GPSTrailImportTask(class_id, instance_id, *this));
        assert(gps_trail_import_task_);
        addTask(class_id, gps_trail_import_task_.get());
    }
    else if (class_id.compare("ManageDataSourcesTask") == 0)
    {
        assert(!manage_datasources_task_);
        manage_datasources_task_.reset(new ManageDataSourcesTask(class_id, instance_id, *this));
        assert(manage_datasources_task_);
        addTask(class_id, manage_datasources_task_.get());
    }
    else if (class_id.compare("RadarPlotPositionCalculatorTask") == 0)
    {
        assert(!radar_plot_position_calculator_task_);
        radar_plot_position_calculator_task_.reset(
            new RadarPlotPositionCalculatorTask(class_id, instance_id, *this));
        assert(radar_plot_position_calculator_task_);
        addTask(class_id, radar_plot_position_calculator_task_.get());
    }
    else if (class_id.compare("CreateARTASAssociationsTask") == 0)
    {
        assert(!create_artas_associations_task_);
        create_artas_associations_task_.reset(
            new CreateARTASAssociationsTask(class_id, instance_id, *this));
        assert(create_artas_associations_task_);
        addTask(class_id, create_artas_associations_task_.get());
    }
    else if (class_id.compare("PostProcessTask") == 0)
    {
        assert(!post_process_task_);
        post_process_task_.reset(new PostProcessTask(class_id, instance_id, *this));
        assert(post_process_task_);
        addTask(class_id, post_process_task_.get());
    }
    else
        throw std::runtime_error("TaskManager: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

void TaskManager::addTask(const std::string& class_id, Task* task)
{
    assert(task);
    assert(!tasks_.count(class_id));
    tasks_[class_id] = task;
    connect(task, &Task::statusChangedSignal, this, &TaskManager::taskStatusChangesSlot);
    connect(task, &Task::doneSignal, this, &TaskManager::taskDoneSlot);
}

void TaskManager::checkSubConfigurables()
{
    if (!database_open_task_)
    {
        generateSubConfigurable("DatabaseOpenTask", "DatabaseOpenTask0");
        assert(database_open_task_);
    }

    if (!manage_schema_task_)
    {
        generateSubConfigurable("ManageSchemaTask", "ManageSchemaTask0");
        assert(manage_schema_task_);
    }

    if (!manage_dbobjects_task_)
    {
        generateSubConfigurable("ManageDBObjectsTask", "ManageDBObjectsTask0");
        assert(manage_dbobjects_task_);
    }

#if USE_JASTERIX
    if (!asterix_importer_task_)
    {
        generateSubConfigurable("ASTERIXImportTask", "ASTERIXImportTask0");
        assert(asterix_importer_task_);
    }
#endif

    if (!view_points_import_task_)
    {
        generateSubConfigurable("ViewPointsImportTask", "ViewPointsImportTask0");
        assert(view_points_import_task_);
    }

    if (!json_import_task_)
    {
        generateSubConfigurable("JSONImportTask", "JSONImportTask0");
        assert(json_import_task_);
    }

    if (!mysqldb_import_task_)
    {
        generateSubConfigurable("MySQLDBImportTask", "MySQLDBImportTask0");
        assert(mysqldb_import_task_);
    }

    if (!gps_trail_import_task_)
    {
        generateSubConfigurable("GPSTrailImportTask", "GPSTrailImportTask0");
        assert(gps_trail_import_task_);
    }

    if (!manage_datasources_task_)
    {
        generateSubConfigurable("ManageDataSourcesTask", "ManageDataSourcesTask0");
        assert(manage_datasources_task_);
    }

    if (!radar_plot_position_calculator_task_)
    {
        generateSubConfigurable("RadarPlotPositionCalculatorTask",
                                "RadarPlotPositionCalculatorTask0");
        assert(radar_plot_position_calculator_task_);
    }

    if (!create_artas_associations_task_)
    {
        generateSubConfigurable("CreateARTASAssociationsTask", "CreateARTASAssociationsTask0");
        assert(create_artas_associations_task_);
    }

    if (!post_process_task_)
    {
        generateSubConfigurable("PostProcessTask", "PostProcessTask0");
        assert(post_process_task_);
    }
}

std::map<std::string, Task*> TaskManager::tasks() const { return tasks_; }

bool TaskManager::expertMode() const { return expert_mode_; }

void TaskManager::expertMode(bool value)
{
    expert_mode_ = value;

    if (widget_)
    {
        widget_->updateTaskStates();
        widget_->selectNextTask();
    }

    emit expertModeChangedSignal();
}

std::vector<std::string> TaskManager::taskList() const { return task_list_; }

void TaskManager::taskStatusChangesSlot(std::string task_name)
{
    loginf << "TaskManager: taskStatusChangesSlot: task " << task_name;

    if (widget_)
    {
        widget_->updateTaskStates();
        widget_->selectNextTask();
    }
}

void TaskManager::taskDoneSlot(std::string task_name)
{
    loginf << "TaskManager: taskDoneSlot: task " << task_name;

    if (widget_)
    {
        widget_->updateTaskStates();
        widget_->selectNextTask();
    }
}

void TaskManager::dbObjectsChangedSlot()
{
    loginf << "TaskManager: dbObjectsChangedSlot";

    if (widget_)
    {
        widget_->updateTaskStates();
        widget_->selectNextTask();
    }
}

void TaskManager::schemaChangedSlot()
{
    loginf << "TaskManager: schemaChangedSlot";

    if (widget_)
    {
        widget_->updateTaskStates();
        widget_->selectNextTask();
    }
}

void TaskManager::shutdown()
{
    loginf << "TaskManager: shutdown";

    database_open_task_ = nullptr;
    manage_schema_task_ = nullptr;
    manage_dbobjects_task_ = nullptr;

#if USE_JASTERIX
    asterix_importer_task_ = nullptr;
#endif

    view_points_import_task_ = nullptr;
    json_import_task_ = nullptr;
    mysqldb_import_task_ = nullptr;
    gps_trail_import_task_ = nullptr;
    manage_datasources_task_ = nullptr;
    radar_plot_position_calculator_task_ = nullptr;
    create_artas_associations_task_ = nullptr;
    post_process_task_ = nullptr;

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

void TaskManager::runTask(const std::string& task_name)
{
    loginf << "TaskManager: runTask: name " << task_name;

    assert(tasks_.count(task_name));
    assert(tasks_.at(task_name)->checkPrerequisites());

    tasks_.at(task_name)->run();
}

DatabaseOpenTask& TaskManager::databaseOpenTask() const
{
    assert(database_open_task_);
    return *database_open_task_;
}

ManageSchemaTask& TaskManager::manageSchemaTask() const
{
    assert(manage_schema_task_);
    return *manage_schema_task_;
}

ManageDataSourcesTask& TaskManager::manageDataSourcesTask() const
{
    assert(manage_datasources_task_);
    return *manage_datasources_task_;
}

#if USE_JASTERIX
ASTERIXImportTask& TaskManager::asterixImporterTask() const
{
    assert(asterix_importer_task_);
    return *asterix_importer_task_;
}
#endif

ViewPointsImportTask& TaskManager::viewPointsImportTask() const
{
    assert(view_points_import_task_);
    return *view_points_import_task_;
}

JSONImportTask& TaskManager::jsonImporterTask() const
{
    assert(json_import_task_);
    return *json_import_task_;
}

MySQLDBImportTask& TaskManager::mysqldbImportTask() const
{
    assert(mysqldb_import_task_);
    return *mysqldb_import_task_;
}

GPSTrailImportTask& TaskManager::gpsTrailImportTask() const
{
    assert(gps_trail_import_task_);
    return *gps_trail_import_task_;

}

RadarPlotPositionCalculatorTask& TaskManager::radarPlotPositionCalculatorTask() const
{
    assert(radar_plot_position_calculator_task_);
    return *radar_plot_position_calculator_task_;
}

CreateARTASAssociationsTask& TaskManager::createArtasAssociationsTask() const
{
    assert(manage_datasources_task_);
    return *create_artas_associations_task_;
}

PostProcessTask& TaskManager::postProcessTask() const
{
    assert(manage_datasources_task_);
    return *post_process_task_;
}

void TaskManager::createAndOpenNewSqlite3DB(const std::string& filename)
{
    loginf << "TaskManager: sqlite3CreateNewDB: filename '" << filename << "'";

    automatic_tasks_defined_ = true;
    sqlite3_create_new_db_ = true;
    sqlite3_create_new_db_filename_ = filename;
}

void TaskManager::openSqlite3DB(const std::string& filename)
{
    loginf << "TaskManager: sqlite3OpenDB: filename '" << filename << "'";

    automatic_tasks_defined_ = true;
    sqlite3_open_db_ = true;
    sqlite3_open_db_filename_ = filename;
}

#if USE_JASTERIX
void TaskManager::importASTERIXFile(const std::string& filename)
{
    loginf << "TaskManager: asterixImportFile: filename '" << filename << "'";

    automatic_tasks_defined_ = true;
    asterix_import_file_ = true;
    asterix_import_filename_ = filename;
}
#endif

void TaskManager::importGPSTrailFile(const std::string& filename)
{
    automatic_tasks_defined_ = true;
    gps_trail_import_file_ = true;
    gps_trail_import_filename_ = filename;
}

void TaskManager::importViewPointsFile(const std::string& filename)
{
    loginf << "TaskManager: importViewPointsFile: filename '" << filename << "'";

    automatic_tasks_defined_ = true;
    view_points_import_file_ = true;
    view_points_import_filename_ = filename;
}

void TaskManager::autoProcess(bool value)
{
    loginf << "TaskManager: autoProcess: value " << value;

    automatic_tasks_defined_ = true;
    auto_process_ = value;
}

void TaskManager::quitAfterAutoProcess(bool value)
{
    loginf << "TaskManager: autoQuitAfterProcess: value " << value;

    automatic_tasks_defined_ = true;
    quit_after_auto_process_ = value;
}

void TaskManager::startAfterAutoProcess(bool value)
{
    loginf << "TaskManager: startAfterAutoProcess: value " << value;

    automatic_tasks_defined_ = true;
    start_after_auto_process_ = value;
}

bool TaskManager::automaticTasksDefined() const
{
    return automatic_tasks_defined_;
}

void TaskManager::performAutomaticTasks ()
{
    loginf << "TaskManager: performAutomaticTasks";
    assert (automatic_tasks_defined_);

    if (!(sqlite3_create_new_db_ || sqlite3_open_db_))
    {
        logerr << "TaskManager: performAutomaticTasks: neither create nor open sqlite3 is set";
        return;
    }

    if (sqlite3_create_new_db_ && sqlite3_open_db_)
    {
        logerr << "TaskManager: performAutomaticTasks: both create and open sqlite3 are set";
        return;
    }

    database_open_task_->useConnection("SQLite Connection");
    SQLiteConnectionWidget* connection_widget =
        dynamic_cast<SQLiteConnectionWidget*>(ATSDB::instance().interface().connectionWidget());

    if (sqlite3_create_new_db_)
    {
        while (QCoreApplication::hasPendingEvents())
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        loginf << "TaskManager: performAutomaticTasks: creating and opening new sqlite3 database '"
               << sqlite3_create_new_db_filename_ << "'";

        if (Files::fileExists(sqlite3_create_new_db_filename_))
            Files::deleteFile(sqlite3_create_new_db_filename_);

        connection_widget->addFile(sqlite3_create_new_db_filename_);
        connection_widget->selectFile(sqlite3_create_new_db_filename_);
        connection_widget->openFileSlot();

        while (QCoreApplication::hasPendingEvents())
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        QThread::msleep(100);  // delay
    }
    else if (sqlite3_open_db_)
    {
        while (QCoreApplication::hasPendingEvents())
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        loginf << "TaskManager: performAutomaticTasks: opening existing sqlite3 database '"
               << sqlite3_open_db_filename_ << "'";

        if (!Files::fileExists(sqlite3_open_db_filename_))
        {
            logerr << "TaskManager: performAutomaticTasks: sqlite3 database '" << sqlite3_open_db_filename_
                   << "' does not exist";
            return;
        }

        connection_widget->addFile(sqlite3_open_db_filename_);
        connection_widget->selectFile(sqlite3_open_db_filename_);
        connection_widget->openFileSlot();

        while (QCoreApplication::hasPendingEvents())
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        QThread::msleep(100);  // delay
    }

    if (view_points_import_file_)
    {
        while (QCoreApplication::hasPendingEvents())
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        loginf << "TaskManager: performAutomaticTasks: importing view points file '"
               << view_points_import_filename_ << "'";

        if (!Files::fileExists(view_points_import_filename_))
        {
            logerr << "TaskManager: performAutomaticTasks: view points file '" << view_points_import_filename_
                   << "' does not exist";
            return;
        }

        widget_->setCurrentTask(*view_points_import_task_);
        if(widget_->getCurrentTaskName() != view_points_import_task_->name())
        {
            logerr << "TaskManager: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
                   << "' selected, aborting";
            return;
        }

        ViewPointsImportTaskWidget* view_points_import_task_widget =
            dynamic_cast<ViewPointsImportTaskWidget*>(view_points_import_task_->widget());
        assert(view_points_import_task_widget);

        view_points_import_task_widget->addFile(view_points_import_filename_);
        view_points_import_task_widget->selectFile(view_points_import_filename_);

        assert(view_points_import_task_->canImport());
        view_points_import_task_->showDoneSummary(false);

        view_points_import_task_widget->importSlot();

        while (QCoreApplication::hasPendingEvents() || !view_points_import_task_->finished())
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        QThread::msleep(100);  // delay
    }

    #if USE_JASTERIX
    if (asterix_import_file_)
    {
        while (QCoreApplication::hasPendingEvents())
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        loginf << "TaskManager: performAutomaticTasks: importing ASTERIX file '"
               << asterix_import_filename_ << "'";

        if (!Files::fileExists(asterix_import_filename_))
        {
            logerr << "TaskManager: performAutomaticTasks: ASTERIX file '" << asterix_import_filename_
                   << "' does not exist";
            return;
        }

        widget_->setCurrentTask(*asterix_importer_task_);
        if(widget_->getCurrentTaskName() != asterix_importer_task_->name())
        {
            logerr << "TaskManager: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
                   << "' selected, aborting";
            return;
        }

        ASTERIXImportTaskWidget* asterix_import_task_widget =
            dynamic_cast<ASTERIXImportTaskWidget*>(asterix_importer_task_->widget());
        assert(asterix_import_task_widget);

        asterix_import_task_widget->addFile(asterix_import_filename_);
        asterix_import_task_widget->selectFile(asterix_import_filename_);

        assert(asterix_importer_task_->canRun());
        asterix_importer_task_->showDoneSummary(false);

        //widget_->runCurrentTaskSlot();
        widget_->runTask(*asterix_importer_task_);

        while (QCoreApplication::hasPendingEvents() || !asterix_importer_task_->done())
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        QThread::msleep(100);  // delay
    }
    #endif

    if (gps_trail_import_file_)
    {
        while (QCoreApplication::hasPendingEvents())
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        loginf << "TaskManager: performAutomaticTasks: importing GPS trail file '"
               << gps_trail_import_filename_ << "'";

        if (!Files::fileExists(gps_trail_import_filename_))
        {
            logerr << "TaskManager: performAutomaticTasks: GPS trail file '" << gps_trail_import_filename_
                   << "' does not exist";
            return;
        }

        widget_->setCurrentTask(*gps_trail_import_task_);
        if(widget_->getCurrentTaskName() != gps_trail_import_task_->name())
        {
            logerr << "TaskManager: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
                   << "' selected, aborting";
            return;
        }

        GPSTrailImportTaskWidget* gps_import_task_widget =
            dynamic_cast<GPSTrailImportTaskWidget*>(gps_trail_import_task_->widget());
        assert(gps_import_task_widget);

        gps_import_task_widget->addFile(gps_trail_import_filename_);
        gps_import_task_widget->selectFile(gps_trail_import_filename_);

        assert(gps_trail_import_task_->canRun());
        gps_trail_import_task_->showDoneSummary(false);

        //widget_->runCurrentTaskSlot();
        widget_->runTask(*gps_trail_import_task_);

        while (QCoreApplication::hasPendingEvents() || !gps_trail_import_task_->done())
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        QThread::msleep(100);  // delay
    }

    if (auto_process_)
    {
        while (QCoreApplication::hasPendingEvents())
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        // calculate radar plot positions
        if (radar_plot_position_calculator_task_->isRecommended())
        {
            loginf << "TaskManager: performAutomaticTasks: starting radar plot position calculation task";

            widget_->setCurrentTask(*radar_plot_position_calculator_task_);
            if(widget_->getCurrentTaskName() != radar_plot_position_calculator_task_->name())
            {
                logerr << "TaskManager: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
                       << "' selected, aborting";
                return;
            }
            radar_plot_position_calculator_task_->showDoneSummary(false);

            //widget_->runCurrentTaskSlot();
            widget_->runTask(*radar_plot_position_calculator_task_);

            while (QCoreApplication::hasPendingEvents() || !radar_plot_position_calculator_task_->done())
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

            QThread::msleep(100);  // delay
        }

        // post-process
        while (QCoreApplication::hasPendingEvents())
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        loginf << "TaskManager: performAutomaticTasks: starting post-processing task";

        if (!post_process_task_->isRecommended())
        {

            logerr << "TaskManager: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
                   << "' selected, aborting";
            return;
        }

        assert(post_process_task_->isRecommended());
        assert(post_process_task_->isRequired());

        widget_->setCurrentTask(*post_process_task_);
        if(widget_->getCurrentTaskName() != post_process_task_->name())
            widget_->setCurrentTask(*post_process_task_);

        //widget_->runCurrentTaskSlot();
        widget_->runTask(*post_process_task_);

        while (QCoreApplication::hasPendingEvents() || !post_process_task_->done())
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        QThread::msleep(100);  // delay

        loginf << "TaskManager: performAutomaticTasks: post-processing task done";

        // assocs
        if (create_artas_associations_task_->isRecommended())
        {
            while (QCoreApplication::hasPendingEvents())
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

            loginf << "TaskManager: performAutomaticTasks: starting association task";

            widget_->setCurrentTask(*create_artas_associations_task_);
            if(widget_->getCurrentTaskName() != create_artas_associations_task_->name())
            {
                logerr << "TaskManager: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
                       << "' selected, aborting";
                return;
            }

            create_artas_associations_task_->showDoneSummary(false);

            //widget_->runCurrentTaskSlot();
            widget_->runTask(*create_artas_associations_task_);

            while (QCoreApplication::hasPendingEvents() || !create_artas_associations_task_->done())
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

            QThread::msleep(100);  // delay
        }
    }

    loginf << "TaskManager: performAutomaticTasks: done";

    if (quit_after_auto_process_)
    {
        loginf << "TaskManager: performAutomaticTasks: quit requested";
        emit quitRequestedSignal();
    }

    if (start_after_auto_process_)
    {
        loginf << "TaskManager: performAutomaticTasks: starting";

        while (QCoreApplication::hasPendingEvents())
            QCoreApplication::processEvents();

        if(widget_->isStartPossible())
            widget_->startSlot();
        else
            loginf << "TaskManager: performAutomaticTasks: start not possible";
    }
}
