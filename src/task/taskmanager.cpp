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

#include "taskmanager.h"

#include "compass.h"
#include "createartasassociationstask.h"
#include "createartasassociationstaskwidget.h"
#include "databaseopentask.h"
#include "databaseopentaskwidget.h"
#include "dbcontent/dbcontentmanager.h"
//#include "dboeditdatasourceswidget.h"
//#include "jsonimporttask.h"
//#include "jsonimporttaskwidget.h"
#include "jsonparsingschema.h"
//#include "managedatasourcestask.h"
//#include "managedatasourcestaskwidget.h"
#include "managedbobjectstask.h"
#include "managedbobjectstaskwidget.h"
#include "managesectorstask.h"
#include "managesectorstaskwidget.h"
//#include "postprocesstask.h"
//#include "postprocesstaskwidget.h"
#include "radarplotpositioncalculatortask.h"
#include "radarplotpositioncalculatortaskwidget.h"
#include "taskmanagerlogwidget.h"
#include "taskmanagerwidget.h"
#include "dbinterface.h"
#include "files.h"
#include "viewpointsimporttask.h"
#include "viewpointsimporttaskwidget.h"
#include "gpstrailimporttask.h"
#include "gpstrailimporttaskwidget.h"
#include "createassociationstask.h"
#include "createassociationstaskwidget.h"
#include "viewmanager.h"
#include "viewpointsreportgenerator.h"
#include "viewpointsreportgeneratordialog.h"
#include "evaluationmanager.h"
#include "eval/results/report/pdfgenerator.h"
#include "eval/results/report/pdfgeneratordialog.h"
#include "mainwindow.h"

#include "asteriximporttask.h"
#include "asteriximporttaskwidget.h"

#include <cassert>

#include <QCoreApplication>
#include <QApplication>
#include <QMainWindow>
#include <QThread>

#include "boost/date_time/posix_time/posix_time.hpp"

using namespace Utils;

TaskManager::TaskManager(const std::string& class_id, const std::string& instance_id, COMPASS* compass)
    : Configurable(class_id, instance_id, compass, "task.json")
{
    registerParameter("expert_mode", &expert_mode_, false);

    createSubConfigurables();

    task_list_ = {"DatabaseOpenTask",
                  "ManageDBObjectsTask", "ASTERIXImportTask",
                  "ViewPointsImportTask", "GPSTrailImportTask", // "JSONImportTask",
                  "ManageSectorsTask", // "ManageDataSourcesTask",
                  "RadarPlotPositionCalculatorTask",
                  "CreateAssociationsTask", "CreateARTASAssociationsTask"};  // defines order of tasks

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
    else if (class_id.compare("ManageDBObjectsTask") == 0)
    {
        assert(!manage_dbobjects_task_);
        manage_dbobjects_task_.reset(new ManageDBObjectsTask(class_id, instance_id, *this));
        assert(manage_dbobjects_task_);
        addTask(class_id, manage_dbobjects_task_.get());
    }
    else if (class_id.compare("ASTERIXImportTask") == 0)
    {
        assert(!asterix_importer_task_);
        asterix_importer_task_.reset(new ASTERIXImportTask(class_id, instance_id, *this));
        assert(asterix_importer_task_);
        addTask(class_id, asterix_importer_task_.get());
    }
    else if (class_id.compare("ViewPointsImportTask") == 0)
    {
        assert(!view_points_import_task_);
        view_points_import_task_.reset(new ViewPointsImportTask(class_id, instance_id, *this));
        assert(view_points_import_task_);
        addTask(class_id, view_points_import_task_.get());
    }
//    else if (class_id.compare("JSONImportTask") == 0)
//    {
//        assert(!json_import_task_);
//        json_import_task_.reset(new JSONImportTask(class_id, instance_id, *this));
//        assert(json_import_task_);
//        addTask(class_id, json_import_task_.get());
//    }
    else if (class_id.compare("GPSTrailImportTask") == 0)
    {
        assert(!gps_trail_import_task_);
        gps_trail_import_task_.reset(new GPSTrailImportTask(class_id, instance_id, *this));
        assert(gps_trail_import_task_);
        addTask(class_id, gps_trail_import_task_.get());
    }
//    else if (class_id.compare("ManageDataSourcesTask") == 0)
//    {
//        assert(!manage_datasources_task_);
//        manage_datasources_task_.reset(new ManageDataSourcesTask(class_id, instance_id, *this));
//        assert(manage_datasources_task_);
//        addTask(class_id, manage_datasources_task_.get());
//    }
    else if (class_id.compare("ManageSectorsTask") == 0)
    {
        assert(!manage_sectors_task_);
        manage_sectors_task_.reset(new ManageSectorsTask(class_id, instance_id, *this));
        assert(manage_sectors_task_);
        addTask(class_id, manage_sectors_task_.get());
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
    else if (class_id.compare("CreateAssociationsTask") == 0)
    {
        assert(!create_associations_task_);
        create_associations_task_.reset(
                    new CreateAssociationsTask(class_id, instance_id, *this));
        assert(create_associations_task_);
        addTask(class_id, create_associations_task_.get());
    }
//    else if (class_id.compare("PostProcessTask") == 0)
//    {
//        assert(!post_process_task_);
//        post_process_task_.reset(new PostProcessTask(class_id, instance_id, *this));
//        assert(post_process_task_);
//        addTask(class_id, post_process_task_.get());
//    }
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

    if (!manage_dbobjects_task_)
    {
        generateSubConfigurable("ManageDBObjectsTask", "ManageDBObjectsTask0");
        assert(manage_dbobjects_task_);
    }

    if (!asterix_importer_task_)
    {
        generateSubConfigurable("ASTERIXImportTask", "ASTERIXImportTask0");
        assert(asterix_importer_task_);
    }

    if (!view_points_import_task_)
    {
        generateSubConfigurable("ViewPointsImportTask", "ViewPointsImportTask0");
        assert(view_points_import_task_);
    }

//    if (!json_import_task_)
//    {
//        generateSubConfigurable("JSONImportTask", "JSONImportTask0");
//        assert(json_import_task_);
//    }

    if (!gps_trail_import_task_)
    {
        generateSubConfigurable("GPSTrailImportTask", "GPSTrailImportTask0");
        assert(gps_trail_import_task_);
    }

//    if (!manage_datasources_task_)
//    {
//        generateSubConfigurable("ManageDataSourcesTask", "ManageDataSourcesTask0");
//        assert(manage_datasources_task_);
//    }

    if (!manage_sectors_task_)
    {
        generateSubConfigurable("ManageSectorsTask", "ManageSectorsTask0");
        assert(manage_sectors_task_);
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

    if (!create_associations_task_)
    {
        generateSubConfigurable("CreateAssociationsTask", "CreateAssociationsTask0");
        assert(create_associations_task_);
    }

//    if (!post_process_task_)
//    {
//        generateSubConfigurable("PostProcessTask", "PostProcessTask0");
//        assert(post_process_task_);
//    }
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
    manage_dbobjects_task_ = nullptr;

    asterix_importer_task_->stop(); // stops if active
    asterix_importer_task_ = nullptr;

    view_points_import_task_ = nullptr;
    //json_import_task_ = nullptr;
    gps_trail_import_task_ = nullptr;
    //manage_datasources_task_ = nullptr;
    manage_sectors_task_ = nullptr;
    radar_plot_position_calculator_task_ = nullptr;
    create_artas_associations_task_ = nullptr;
    //post_process_task_ = nullptr;
    create_associations_task_ = nullptr;

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

//ManageDataSourcesTask& TaskManager::manageDataSourcesTask() const
//{
//    assert(manage_datasources_task_);
//    return *manage_datasources_task_;
//}

ManageSectorsTask& TaskManager::manageSectorsTask() const
{
    assert(manage_sectors_task_);
    return *manage_sectors_task_;
}

ASTERIXImportTask& TaskManager::asterixImporterTask() const
{
    assert(asterix_importer_task_);
    return *asterix_importer_task_;
}

ViewPointsImportTask& TaskManager::viewPointsImportTask() const
{
    assert(view_points_import_task_);
    return *view_points_import_task_;
}

//JSONImportTask& TaskManager::jsonImporterTask() const
//{
//    assert(json_import_task_);
//    return *json_import_task_;
//}

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
    assert(create_artas_associations_task_);
    return *create_artas_associations_task_;
}

//PostProcessTask& TaskManager::postProcessTask() const
//{
//    assert(post_process_task_);
//    return *post_process_task_;
//}

CreateAssociationsTask& TaskManager::createAssociationsTask() const
{
    assert(create_associations_task_);
    return *create_associations_task_;
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

void TaskManager::importASTERIXFile(const std::string& filename)
{
    loginf << "TaskManager: asterixImportFile: filename '" << filename << "'";

    automatic_tasks_defined_ = true;
    asterix_import_file_ = true;
    asterix_import_filename_ = filename;
}

bool TaskManager::asterixOptionsSet() const
{
    return set_asterix_framing_ || set_asterix_decoder_cfg_;
}

void TaskManager::setAsterixOptions()
{
    assert (asterixOptionsSet());

    if (set_asterix_framing_)
        asterixImporterTask().asterixFileFraming(asterix_framing_);

    if (set_asterix_decoder_cfg_)
        asterixImporterTask().asterixDecoderConfig(asterix_decoder_cfg_);
}

void TaskManager::asterixFraming(const std::string& asterix_framing)
{
    asterix_framing_ = asterix_framing;
    set_asterix_framing_ = true;
}

void TaskManager::asterixDecoderConfig(const std::string& asterix_decoder_cfg)
{
    asterix_decoder_cfg_ = asterix_decoder_cfg;
    set_asterix_decoder_cfg_ = true;
}

void TaskManager::importJSONFile(const std::string& filename, const std::string& schema)
{
    loginf << "TaskManager: importJSONFile: filename '" << filename << "' schema '" << schema << "'";

    assert (schema.size());

    automatic_tasks_defined_ = true;
    json_import_file_ = true;
    json_import_filename_ = filename;
    json_import_schema_ = schema;
}

void TaskManager::importGPSTrailFile(const std::string& filename)
{
    automatic_tasks_defined_ = true;
    gps_trail_import_file_ = true;
    gps_trail_import_filename_ = filename;
}

void TaskManager::importSectorsFile(const std::string& filename)
{
    automatic_tasks_defined_ = true;
    sectors_import_file_ = true;
    sectors_import_filename_ = filename;
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

void TaskManager::associateData(bool value)
{
    loginf << "TaskManager: associateData: value " << value;

    automatic_tasks_defined_ = true;
    associate_data_ = value;
}

void TaskManager::quit(bool value)
{
    loginf << "TaskManager: autoQuitAfterProcess: value " << value;

    automatic_tasks_defined_ = true;
    quit_ = value;
}

void TaskManager::start(bool value)
{
    loginf << "TaskManager: start: value " << value;

    automatic_tasks_defined_ = true;
    start_ = value;
}


void TaskManager::loadData(bool value)
{
    loginf << "TaskManager: loadData: value " << value;

    automatic_tasks_defined_ = true;
    load_data_ = value;
}

void TaskManager::exportViewPointsReportFile(const std::string& filename)
{
    loginf << "TaskManager: exportViewPointsReport: file '" << filename << "'";

    automatic_tasks_defined_ = true;
    export_view_points_report_ = true;
    export_view_points_report_filename_ = filename;
}

void TaskManager::exportEvalReportFile(const std::string& filename)
{
    automatic_tasks_defined_ = true;
    export_eval_report_ = true;
    export_eval_report_filename_ = filename;
}

bool TaskManager::automaticTasksDefined() const
{
    return automatic_tasks_defined_;
}

void TaskManager::performAutomaticTasks ()
{
//    loginf << "TaskManager: performAutomaticTasks";
//    assert (automatic_tasks_defined_);

//    if (!(sqlite3_create_new_db_ || sqlite3_open_db_))
//    {
//        logerr << "TaskManager: performAutomaticTasks: neither create nor open sqlite3 is set";
//        return;
//    }

//    if (sqlite3_create_new_db_ && sqlite3_open_db_)
//    {
//        logerr << "TaskManager: performAutomaticTasks: both create and open sqlite3 are set";
//        return;
//    }

//    SQLiteConnectionWidget* connection_widget =
//            dynamic_cast<SQLiteConnectionWidget*>(COMPASS::instance().interface().connectionWidget());

//    while (QCoreApplication::hasPendingEvents())
//        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

//    if (sqlite3_create_new_db_)
//    {
//        loginf << "TaskManager: performAutomaticTasks: creating and opening new sqlite3 database '"
//               << sqlite3_create_new_db_filename_ << "'";

//        if (Files::fileExists(sqlite3_create_new_db_filename_))
//            Files::deleteFile(sqlite3_create_new_db_filename_);

//        connection_widget->addFile(sqlite3_create_new_db_filename_);
//        connection_widget->selectFile(sqlite3_create_new_db_filename_);
//        connection_widget->openFileSlot();
//    }
//    else if (sqlite3_open_db_)
//    {
//        loginf << "TaskManager: performAutomaticTasks: opening existing sqlite3 database '"
//               << sqlite3_open_db_filename_ << "'";

//        if (!Files::fileExists(sqlite3_open_db_filename_))
//        {
//            logerr << "TaskManager: performAutomaticTasks: sqlite3 database '" << sqlite3_open_db_filename_
//                   << "' does not exist";
//            return;
//        }

//        connection_widget->addFile(sqlite3_open_db_filename_);
//        connection_widget->selectFile(sqlite3_open_db_filename_);
//        connection_widget->openFileSlot();
//    }

//    loginf << "TaskManager: performAutomaticTasks: database opened";

//    // do longer wait on startup for things to settle
//    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();

//    while ((boost::posix_time::microsec_clock::local_time()-start_time).total_milliseconds() < 50)
//    {
//        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
//        QThread::msleep(1);
//    }
//    // does not show widget
//    //QCoreApplication::processEvents();

//    // does cause application halt
//    //    while (QCoreApplication::hasPendingEvents())
//    //        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

//    loginf << "TaskManager: performAutomaticTasks: waiting done";

//    if (view_points_import_file_)
//    {
//        loginf << "TaskManager: performAutomaticTasks: importing view points file '"
//               << view_points_import_filename_ << "'";

//        if (!Files::fileExists(view_points_import_filename_))
//        {
//            logerr << "TaskManager: performAutomaticTasks: view points file '" << view_points_import_filename_
//                   << "' does not exist";
//            return;
//        }

//        widget_->setCurrentTask(*view_points_import_task_);
//        if(widget_->getCurrentTaskName() != view_points_import_task_->name())
//        {
//            logerr << "TaskManager: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
//                   << "' selected, aborting";
//            return;
//        }

//        ViewPointsImportTaskWidget* view_points_import_task_widget =
//                dynamic_cast<ViewPointsImportTaskWidget*>(view_points_import_task_->widget());
//        assert(view_points_import_task_widget);

//        view_points_import_task_widget->addFile(view_points_import_filename_);
//        view_points_import_task_widget->selectFile(view_points_import_filename_);

//        assert(view_points_import_task_->canImport());
//        view_points_import_task_->showDoneSummary(false);

//        view_points_import_task_widget->importSlot();

//        while (!view_points_import_task_->finished())
//        {
//            QCoreApplication::processEvents();
//            QThread::msleep(1);
//        }
//    }

//#if USE_JASTERIX
//    if (asterix_import_file_)
//    {
//        loginf << "TaskManager: performAutomaticTasks: importing ASTERIX file '"
//               << asterix_import_filename_ << "'";

//        if (!Files::fileExists(asterix_import_filename_))
//        {
//            logerr << "TaskManager: performAutomaticTasks: ASTERIX file '" << asterix_import_filename_
//                   << "' does not exist";
//            return;
//        }

//        widget_->setCurrentTask(*asterix_importer_task_);
//        if(widget_->getCurrentTaskName() != asterix_importer_task_->name())
//        {
//            logerr << "TaskManager: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
//                   << "' selected, aborting";
//            return;
//        }

//        ASTERIXImportTaskWidget* asterix_import_task_widget =
//                dynamic_cast<ASTERIXImportTaskWidget*>(asterix_importer_task_->widget());
//        assert(asterix_import_task_widget);

//        asterix_import_task_widget->addFile(asterix_import_filename_);
//        asterix_import_task_widget->selectFile(asterix_import_filename_);

//        assert(asterix_importer_task_->canRun());
//        asterix_importer_task_->showDoneSummary(false);

//        widget_->runTask(*asterix_importer_task_);

//        while (!asterix_importer_task_->done())
//        {
//            QCoreApplication::processEvents();
//            QThread::msleep(1);
//        }
//    }
//#endif

////    if (json_import_file_)
////    {
////        loginf << "TaskManager: performAutomaticTasks: importing JSON file '"
////               << json_import_filename_ << "'";

////#if USE_JASTERIX
////        if (!Files::fileExists(json_import_filename_))
////        {
////            logerr << "TaskManager: performAutomaticTasks: JSON file '" << asterix_import_filename_
////                   << "' does not exist";
////            return;
////        }
////#endif

////        if(!json_import_task_->hasSchema(json_import_schema_))
////        {
////            logerr << "TaskManager: performAutomaticTasks: JSON schema '" << json_import_schema_
////                   << "' does not exist";
////            return;
////        }

////        widget_->setCurrentTask(*json_import_task_);
////        if(widget_->getCurrentTaskName() != json_import_task_->name())
////        {
////            logerr << "TaskManager: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
////                   << "' selected, aborting";
////            return;
////        }

////        JSONImportTaskWidget* json_import_task_widget =
////                dynamic_cast<JSONImportTaskWidget*>(json_import_task_->widget());
////        assert(json_import_task_widget);

////        json_import_task_widget->addFile(json_import_filename_);
////        json_import_task_widget->selectFile(json_import_filename_);
////        json_import_task_widget->selectSchema(json_import_schema_);

////        assert(json_import_task_->canRun());
////        json_import_task_->showDoneSummary(false);

////        widget_->runTask(*json_import_task_);

////        while (!json_import_task_->done())
////        {
////            QCoreApplication::processEvents();
////            QThread::msleep(1);
////        }

////        loginf << "TaskManager: performAutomaticTasks: importing JSON file done";
////    }

//    if (gps_trail_import_file_)
//    {
//        loginf << "TaskManager: performAutomaticTasks: importing GPS trail file '"
//               << gps_trail_import_filename_ << "'";

//        if (!Files::fileExists(gps_trail_import_filename_))
//        {
//            logerr << "TaskManager: performAutomaticTasks: GPS trail file '" << gps_trail_import_filename_
//                   << "' does not exist";
//            return;
//        }

//        widget_->setCurrentTask(*gps_trail_import_task_);
//        if(widget_->getCurrentTaskName() != gps_trail_import_task_->name())
//        {
//            logerr << "TaskManager: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
//                   << "' selected, aborting";
//            return;
//        }

//        GPSTrailImportTaskWidget* gps_import_task_widget =
//                dynamic_cast<GPSTrailImportTaskWidget*>(gps_trail_import_task_->widget());
//        assert(gps_import_task_widget);

//        gps_import_task_widget->addFile(gps_trail_import_filename_);
//        gps_import_task_widget->selectFile(gps_trail_import_filename_);

//        assert(gps_trail_import_task_->canRun());
//        gps_trail_import_task_->showDoneSummary(false);

//        widget_->runTask(*gps_trail_import_task_);

//        while (!gps_trail_import_task_->done())
//        {
//            QCoreApplication::processEvents();
//            QThread::msleep(1);
//        }
//    }

//    if (sectors_import_file_)
//    {
//        loginf << "TaskManager: performAutomaticTasks: importing sectors file '"
//               << sectors_import_filename_ << "'";

//        if (!Files::fileExists(sectors_import_filename_))
//        {
//            logerr << "TaskManager: performAutomaticTasks: sectors file file '" << sectors_import_filename_
//                   << "' does not exist";
//            return;
//        }

//        widget_->setCurrentTask(*manage_sectors_task_);
//        if(widget_->getCurrentTaskName() != manage_sectors_task_->name())
//        {
//            logerr << "TaskManager: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
//                   << "' selected, aborting";
//            return;
//        }

//        ManageSectorsTaskWidget* manage_sectors_task_widget =
//                dynamic_cast<ManageSectorsTaskWidget*>(manage_sectors_task_->widget());
//        assert(manage_sectors_task_widget);

//        manage_sectors_task_->showDoneSummary(false);
//        manage_sectors_task_widget->importSectorsJSON(sectors_import_filename_);

//        //widget_->runTask(*manage_sectors_task_);

//        //        while (!manage_sectors_task_->done())
//        //        {
//        //            QCoreApplication::processEvents();
//        //            QThread::msleep(1);
//        //        }
//    }

//    start_time = boost::posix_time::microsec_clock::local_time();
//    while ((boost::posix_time::microsec_clock::local_time()-start_time).total_milliseconds() < 50)
//    {
//        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
//        QThread::msleep(1);
//    }

//    if (auto_process_)
//    {
//        // calculate radar plot positions
//        if (radar_plot_position_calculator_task_->isRecommended())
//        {
//            loginf << "TaskManager: performAutomaticTasks: starting radar plot position calculation task";

//            widget_->setCurrentTask(*radar_plot_position_calculator_task_);
//            if(widget_->getCurrentTaskName() != radar_plot_position_calculator_task_->name())
//            {
//                logerr << "TaskManager: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
//                       << "' selected, aborting";
//                return;
//            }
//            radar_plot_position_calculator_task_->showDoneSummary(false);

//            widget_->runTask(*radar_plot_position_calculator_task_);

//            while (!radar_plot_position_calculator_task_->done())
//            {
//                QCoreApplication::processEvents();
//                QThread::msleep(1);
//            }
//        }

//        // post-process
////        loginf << "TaskManager: performAutomaticTasks: starting post-processing task";

////        if (!post_process_task_->isRecommended())
////        {

////            logerr << "TaskManager: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
////                   << "' selected, aborting";
////            return;
////        }

////        assert(post_process_task_->isRecommended());
////        assert(post_process_task_->isRequired());

////        widget_->setCurrentTask(*post_process_task_);
////        if(widget_->getCurrentTaskName() != post_process_task_->name())
////            widget_->setCurrentTask(*post_process_task_);

////        widget_->runTask(*post_process_task_);

////        while (!post_process_task_->done())
////        {
////            QCoreApplication::processEvents();
////            QThread::msleep(1);
////        }

////        loginf << "TaskManager: performAutomaticTasks: post-processing task done";

//        // artas assocs
//        if (create_artas_associations_task_->isRecommended())
//        {
//            loginf << "TaskManager: performAutomaticTasks: starting association task";

//            widget_->setCurrentTask(*create_artas_associations_task_);
//            if(widget_->getCurrentTaskName() != create_artas_associations_task_->name())
//            {
//                logerr << "TaskManager: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
//                       << "' selected, aborting";
//                return;
//            }

//            create_artas_associations_task_->showDoneSummary(false);

//            widget_->runTask(*create_artas_associations_task_);

//            while (!create_artas_associations_task_->done())
//            {
//                QCoreApplication::processEvents();
//                QThread::msleep(1);
//            }
//        }
//    }

//    if (associate_data_)
//    {
//        if (create_associations_task_->canRun())
//        {
//            widget_->setCurrentTask(*create_associations_task_);
//            if(widget_->getCurrentTaskName() != create_associations_task_->name())
//            {
//                logerr << "TaskManager: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
//                       << "' selected, aborting";
//                return;
//            }

//            create_associations_task_->showDoneSummary(false);

//            widget_->runTask(*create_associations_task_);

//            while (!create_associations_task_->done())
//            {
//                QCoreApplication::processEvents();
//                QThread::msleep(1);
//            }
//        }
//        else
//            logerr << "TaskManager: performAutomaticTasks: associate data task can not be run";
//    }

//    loginf << "TaskManager: performAutomaticTasks: done with startup tasks";

//    bool started = false;

//    if (start_)
//    {
//        loginf << "TaskManager: performAutomaticTasks: starting";

//        if(widget_->isStartPossible())
//        {
//            widget_->startSlot();
//            QCoreApplication::processEvents();

//            started = true;
//        }
//        else
//            loginf << "TaskManager: performAutomaticTasks: start not possible";
//    }

//    if (load_data_)
//    {
//        if (!started)
//        {
//            logerr << "TaskManager: performAutomaticTasks: loading data not possible since not started";
//        }
//        else
//        {

//            loginf << "TaskManager: performAutomaticTasks: loading data";

//            DBObjectManager& obj_man = COMPASS::instance().objectManager();

//            obj_man.loadSlot();

//            while (obj_man.loadInProgress())
//            {
//                QCoreApplication::processEvents();
//                QThread::msleep(1);
//            }
//        }
//    }
//    else
//        loginf << "TaskManager: performAutomaticTasks: not loading data";

//    if (export_view_points_report_)
//    {
//        if (!started)
//        {
//            logerr << "TaskManager: performAutomaticTasks: exporting view points report not possible since not started";
//        }
//        else
//        {
//            loginf << "TaskManager: performAutomaticTasks: exporting view points report";

//            getMainWindow()->showViewPointsTab();

//            ViewPointsReportGenerator& gen = COMPASS::instance().viewManager().viewPointsGenerator();

//            ViewPointsReportGeneratorDialog& dialog = gen.dialog();
//            dialog.show();

//            QCoreApplication::processEvents();

//            gen.reportPathAndFilename(export_view_points_report_filename_);
//            gen.showDone(false);

//            gen.run();

//            while (gen.isRunning()) // not sure if needed here but what the hell
//            {
//                QCoreApplication::processEvents();
//                QThread::msleep(1);
//            }

//            gen.showDone(true);
//        }
//    }

//    if (evaluate_ || export_eval_report_)
//    {
//        if (!started)
//        {
//            logerr << "TaskManager: performAutomaticTasks: evaluation not possible since not started";
//        }
//        else
//        {
//            loginf << "TaskManager: performAutomaticTasks: running evaluation";

//            getMainWindow()->showEvaluationTab();

//            EvaluationManager& eval_man = COMPASS::instance().evaluationManager();

//            if (eval_man.canLoadData())
//            {
//                loginf << "TaskManager: performAutomaticTasks: loading evaluation data";

//                eval_man.loadData();

//                while (!eval_man.dataLoaded())
//                {
//                    QCoreApplication::processEvents();
//                    QThread::msleep(1);
//                }

//                assert (eval_man.dataLoaded());

//                if (eval_man.canEvaluate())
//                {
//                    loginf << "TaskManager: performAutomaticTasks: doing evaluation";

//                    eval_man.evaluate();

////                    while (!eval_man.evaluated())
////                    {
////                        QCoreApplication::processEvents();
////                        QThread::msleep(1);
////                    }

//                    assert (eval_man.evaluated());

//                    loginf << "TaskManager: performAutomaticTasks: evaluation done";

//                    if (export_eval_report_)
//                    {
//                        if (eval_man.canGenerateReport())
//                        {
//                            loginf << "TaskManager: performAutomaticTasks: generating report";

//                            EvaluationResultsReport::PDFGenerator& gen = eval_man.pdfGenerator();

//                            EvaluationResultsReport::PDFGeneratorDialog& dialog = gen.dialog();
//                            dialog.show();

//                            QCoreApplication::processEvents();

//                            gen.reportPathAndFilename(export_eval_report_filename_);
//                            gen.showDone(false);

//                            gen.run();

//                            while (gen.isRunning()) // not sure if needed here but what the hell
//                            {
//                                QCoreApplication::processEvents();
//                                QThread::msleep(1);
//                            }

//                            gen.showDone(true);

//                            loginf << "TaskManager: performAutomaticTasks: generating evaluation report done";
//                        }
//                        else
//                            logerr << "TaskManager: performAutomaticTasks: "
//                                      "exporting evaluation report not possible since report can't be generated";
//                    }
//                }
//                else
//                    logerr << "TaskManager: performAutomaticTasks: "
//                              "evaluation not possible since evaluation can not be made";
//            }
//            else
//                logerr << "TaskManager: performAutomaticTasks: "
//                          "evaluation not possible since no data can be loaded";
//        }
//    }

//    if (quit_)
//    {
//        loginf << "TaskManager: performAutomaticTasks: quit requested";

//        emit quitRequestedSignal();
//    }
//    else
//        loginf << "TaskManager: performAutomaticTasks: not quitting";
}

void TaskManager::evaluate(bool evaluate)
{
    automatic_tasks_defined_ = true;
    evaluate_ = evaluate;
}

MainWindow* TaskManager::getMainWindow()
{
    for(QWidget* pWidget : QApplication::topLevelWidgets())
    {
        QMainWindow* qt_main_window = qobject_cast<QMainWindow*>(pWidget);

        if (qt_main_window)
        {
            MainWindow* main_window = dynamic_cast<MainWindow*>(qt_main_window);
            assert (main_window);
            return main_window;
        }
    }
    return nullptr;
}


