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
#include "jsonimporttask.h"
#include "jsonimporttaskwidget.h"
#include "jsonparsingschema.h"
#include "managedbcontenttask.h"
#include "managedbcontenttaskwidget.h"
#include "managesectorstask.h"
#include "managesectorstaskwidget.h"
#include "radarplotpositioncalculatortask.h"
#include "radarplotpositioncalculatortaskwidget.h"
#include "dbinterface.h"
#include "files.h"
#include "viewpointsimporttask.h"
#include "viewpointsimporttaskwidget.h"
#include "gpstrailimporttask.h"
#include "gpstrailimporttaskwidget.h"
#include "gpsimportcsvtask.h"
#include "gpsimportcsvtaskwidget.h"
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
    createSubConfigurables();

    setObjectName("TaskManager");
}

TaskManager::~TaskManager() {}

void TaskManager::generateSubConfigurable(const std::string& class_id,
                                          const std::string& instance_id)
{
    if (class_id == "DatabaseOpenTask")
    {
        assert(!database_open_task_);
        database_open_task_.reset(new DatabaseOpenTask(class_id, instance_id, *this));
        assert(database_open_task_);
        addTask(class_id, database_open_task_.get());
    }
    else if (class_id == "ManageDBContentTask")
    {
        assert(!manage_dbobjects_task_);
        manage_dbobjects_task_.reset(new ManageDBContentTask(class_id, instance_id, *this));
        assert(manage_dbobjects_task_);
        addTask(class_id, manage_dbobjects_task_.get());
    }
    else if (class_id == "ASTERIXImportTask")
    {
        assert(!asterix_importer_task_);
        asterix_importer_task_.reset(new ASTERIXImportTask(class_id, instance_id, *this));
        assert(asterix_importer_task_);
        addTask(class_id, asterix_importer_task_.get());
    }
    else if (class_id == "ViewPointsImportTask")
    {
        assert(!view_points_import_task_);
        view_points_import_task_.reset(new ViewPointsImportTask(class_id, instance_id, *this));
        assert(view_points_import_task_);
        addTask(class_id, view_points_import_task_.get());
    }
    else if (class_id == "JSONImportTask")
    {
        assert(!json_import_task_);
        json_import_task_.reset(new JSONImportTask(class_id, instance_id, *this));
        assert(json_import_task_);
        addTask(class_id, json_import_task_.get());
    }
    else if (class_id == "GPSTrailImportTask")
    {
        assert(!gps_trail_import_task_);
        gps_trail_import_task_.reset(new GPSTrailImportTask(class_id, instance_id, *this));
        assert(gps_trail_import_task_);
        addTask(class_id, gps_trail_import_task_.get());
    }
    else if (class_id == "GPSImportCSVTask")
    {
        assert(!gps_import_csv_task_);
        gps_import_csv_task_.reset(new GPSImportCSVTask(class_id, instance_id, *this));
        assert(gps_import_csv_task_);
        addTask(class_id, gps_import_csv_task_.get());
    }
    else if (class_id == "ManageSectorsTask")
    {
        assert(!manage_sectors_task_);
        manage_sectors_task_.reset(new ManageSectorsTask(class_id, instance_id, *this));
        assert(manage_sectors_task_);
        addTask(class_id, manage_sectors_task_.get());
    }
    else if (class_id == "RadarPlotPositionCalculatorTask")
    {
        assert(!radar_plot_position_calculator_task_);
        radar_plot_position_calculator_task_.reset(
                    new RadarPlotPositionCalculatorTask(class_id, instance_id, *this));
        assert(radar_plot_position_calculator_task_);
        addTask(class_id, radar_plot_position_calculator_task_.get());
    }
    else if (class_id == "CreateARTASAssociationsTask")
    {
        assert(!create_artas_associations_task_);
        create_artas_associations_task_.reset(
                    new CreateARTASAssociationsTask(class_id, instance_id, *this));
        assert(create_artas_associations_task_);
        addTask(class_id, create_artas_associations_task_.get());
    }
    else if (class_id == "CreateAssociationsTask")
    {
        assert(!create_associations_task_);
        create_associations_task_.reset(
                    new CreateAssociationsTask(class_id, instance_id, *this));
        assert(create_associations_task_);
        addTask(class_id, create_associations_task_.get());
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
//    connect(task, &Task::statusChangedSignal, this, &TaskManager::taskStatusChangesSlot);
//    connect(task, &Task::doneSignal, this, &TaskManager::taskDoneSlot);
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
        generateSubConfigurable("ManageDBContentsTask", "ManageDBContentsTask0");
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

    if (!json_import_task_)
    {
        generateSubConfigurable("JSONImportTask", "JSONImportTask0");
        assert(json_import_task_);
    }

    if (!gps_trail_import_task_)
    {
        generateSubConfigurable("GPSTrailImportTask", "GPSTrailImportTask0");
        assert(gps_trail_import_task_);
    }

    if (!gps_import_csv_task_)
    {
        generateSubConfigurable("GPSImportCSVTask", "GPSImportCSVTask0");
        assert(gps_import_csv_task_);
    }

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

}

std::map<std::string, Task*> TaskManager::tasks() const { return tasks_; }

void TaskManager::shutdown()
{
    loginf << "TaskManager: shutdown";

    database_open_task_ = nullptr;
    manage_dbobjects_task_ = nullptr;

    asterix_importer_task_->stop(); // stops if active
    asterix_importer_task_ = nullptr;

    view_points_import_task_ = nullptr;
    json_import_task_ = nullptr;
    gps_trail_import_task_ = nullptr;
    gps_import_csv_task_ = nullptr;
    manage_sectors_task_ = nullptr;
    radar_plot_position_calculator_task_ = nullptr;
    create_artas_associations_task_ = nullptr;
    create_associations_task_ = nullptr;
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

JSONImportTask& TaskManager::jsonImporterTask() const
{
    assert(json_import_task_);
    return *json_import_task_;
}

GPSTrailImportTask& TaskManager::gpsTrailImportTask() const
{
    assert(gps_trail_import_task_);
    return *gps_trail_import_task_;
}

GPSImportCSVTask& TaskManager::gpsImportCSVTask() const
{
    assert(gps_import_csv_task_);
    return *gps_import_csv_task_;
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

CreateAssociationsTask& TaskManager::createAssociationsTask() const
{
    assert(create_associations_task_);
    return *create_associations_task_;
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


