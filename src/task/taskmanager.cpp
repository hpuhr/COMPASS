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
#include "jsonimporttask.h"
#include "managesectorstask.h"
#include "radarplotpositioncalculatortask.h"
#include "viewpointsimporttask.h"
#include "gpstrailimporttask.h"
#include "gpsimportcsvtask.h"
#include "reconstructortask.h"
#include "mainwindow.h"
#include "viewabledataconfig.h"
#include "viewmanager.h"

#include "asteriximporttask.h"
#include "asteriximporttaskwidget.h"

#include "taskresult.h"
#include "taskresultswidget.h"

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
    if (class_id == "ASTERIXImportTask")
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
        radar_plot_position_calculator_task_.reset(new RadarPlotPositionCalculatorTask(class_id, instance_id, *this));
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
    else if (class_id == "ReconstructorTask")
    {
        assert(!reconstruct_references_task_);
        reconstruct_references_task_.reset(new ReconstructorTask(class_id, instance_id, *this));
        assert(reconstruct_references_task_);
        addTask(class_id, reconstruct_references_task_.get());
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
}

void TaskManager::checkSubConfigurables()
{
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

    if (!reconstruct_references_task_)
    {
        generateSubConfigurable("ReconstructorTask", "ReconstructorTask0");
        assert(reconstruct_references_task_);
    }
}

std::map<std::string, Task*> TaskManager::tasks() const { return tasks_; }

void TaskManager::init()
{
    //init all tasks
    for (const auto& t : tasks_)
        if (t.second)
            t.second->initTask();

    //update features
    updateFeatures();
}

void TaskManager::shutdown()
{
    loginf << "TaskManager: shutdown";

    asterix_importer_task_->stop(); // stops if active
    asterix_importer_task_ = nullptr;

    view_points_import_task_ = nullptr;
    json_import_task_ = nullptr;
    gps_trail_import_task_ = nullptr;
    gps_import_csv_task_ = nullptr;
    manage_sectors_task_ = nullptr;
    radar_plot_position_calculator_task_ = nullptr;
    create_artas_associations_task_ = nullptr;
    reconstruct_references_task_ = nullptr;
}

void TaskManager::runTask(const std::string& task_name)
{
    loginf << "TaskManager: runTask: name " << task_name;

    assert(tasks_.count(task_name));
    assert(tasks_.at(task_name)->canRun());

    tasks_.at(task_name)->run();
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

ReconstructorTask& TaskManager::reconstructReferencesTask() const
{
    assert(reconstruct_references_task_);
    return *reconstruct_references_task_;
}

TaskResultsWidget* TaskManager::widget()
{
    if (!widget_)
        widget_.reset(new TaskResultsWidget(*this));

    assert(widget_);
    return widget_.get();
}

void TaskManager::beginTaskResultWriting(const std::string& name)
{
    if (widget_)
        widget_->setDisabled(true);

    assert (!current_report_);
    current_report_ = getOrCreateResult(name)->report();

    current_report_->clear();
}

ResultReport::Report& TaskManager::currentReport()
{
    assert (current_report_);
    return *current_report_;
}

void TaskManager::endTaskResultWriting()
{
    if (widget_)
        widget_->setDisabled(false);

    assert (current_report_);
    current_report_ = nullptr;

    emit taskResultsChangedSignal();
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

void TaskManager::updateFeatures()
{
    for (auto& t : tasks_)
        if (t.second)
            t.second->updateFeatures();
}

const std::map<unsigned int, std::shared_ptr<TaskResult>>& TaskManager::results() const
{
    return results_;
}

std::shared_ptr<TaskResult> TaskManager::result(unsigned int id) const // get existing result
{
    assert (results_.count(id));
    return results_.at(id);
}

std::shared_ptr<TaskResult> TaskManager::getOrCreateResult (const std::string& name) // get or create result
{
    auto it = std::find_if(results_.begin(), results_.end(),
                           [&name](const std::pair<const unsigned int, std::shared_ptr<TaskResult>>& pair) {
                               return pair.second && pair.second->name() == name;
                           });

    if (it != results_.end())
        return it->second;
    else // create
    {
        unsigned int new_id{0};

        if (results_.size())
            new_id = results_.rend()->first + 1;

        results_[new_id] = std::make_shared<TaskResult>(new_id, *this);
        results_.at(new_id)->name(name);

        return results_.at(new_id);
    }
}

ResultReport::Report& TaskManager::report(const std::string& name)
{
    return *getOrCreateResult(name)->report();
}

bool TaskManager::hasResult (const std::string& name) const
{
    auto it = std::find_if(results_.begin(), results_.end(),
                           [&name](const std::pair<const unsigned int, std::shared_ptr<TaskResult>>& pair) {
                               return pair.second && pair.second->name() == name;
                           });

    return it != results_.end();
}


void TaskManager::databaseOpenedSlot()
{

}

void TaskManager::databaseClosedSlot()
{

}

void TaskManager::setViewableDataConfig(const nlohmann::json::object_t& data)
{
    viewable_data_cfg_.reset(new ViewableDataConfig(data));

    COMPASS::instance().viewManager().setCurrentViewPoint(viewable_data_cfg_.get());
}
