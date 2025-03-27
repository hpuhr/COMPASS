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

#pragma once

#include <QObject>

#include "configurable.h"
#include "task.h"

class COMPASS;
class CreateARTASAssociationsTask;
class JSONImportTask;
class GPSTrailImportTask;
class GPSImportCSVTask;
class ASTERIXImportTask;
class ViewPointsImportTask;
class RadarPlotPositionCalculatorTask;
class ManageSectorsTask;
class ReconstructorTask;
class MainWindow;
class TaskResult;
class TaskResultsWidget;
class ViewableDataConfig;

class QMainWindow;

namespace ResultReport
{
class Report;
}

class TaskManager : public QObject, public Configurable
{
    Q_OBJECT

  signals:
    void startInspectionSignal();
    void expertModeChangedSignal();

    void quitRequestedSignal ();

    void taskResultsChangedSignal();

  public slots:
    void databaseOpenedSlot();
    void databaseClosedSlot();

  public:
    TaskManager(const std::string& class_id, const std::string& instance_id, COMPASS* compass);

    virtual ~TaskManager();

    virtual void generateSubConfigurable(const std::string& class_id, const std::string& instance_id) override;

    void init();
    void shutdown();

    std::map<std::string, Task*> tasks() const;

    void runTask(const std::string& task_name);

    void updateFeatures();

    ASTERIXImportTask& asterixImporterTask() const;
    ViewPointsImportTask& viewPointsImportTask() const;
    JSONImportTask& jsonImporterTask() const;
    GPSTrailImportTask& gpsTrailImportTask() const;
    GPSImportCSVTask& gpsImportCSVTask() const;
    ManageSectorsTask& manageSectorsTask() const;
    RadarPlotPositionCalculatorTask& radarPlotPositionCalculatorTask() const;
    CreateARTASAssociationsTask& createArtasAssociationsTask() const;
    ReconstructorTask& reconstructReferencesTask() const;

    TaskResultsWidget* widget();

    void beginTaskResultWriting(const std::string& name);
    ResultReport::Report& currentReport();
    void endTaskResultWriting(bool store);

    const std::map<unsigned int, std::shared_ptr<TaskResult>>& results() const;
    std::shared_ptr<TaskResult> result(unsigned int id) const; // get existing result
    std::shared_ptr<TaskResult> getOrCreateResult (const std::string& name); // get or create result
    ResultReport::Report& report(const std::string& name);
    bool hasResult (const std::string& name) const;

    void setViewableDataConfig(const nlohmann::json::object_t& data);

protected:
    // tasks
    std::unique_ptr<ASTERIXImportTask> asterix_importer_task_;
    std::unique_ptr<ViewPointsImportTask> view_points_import_task_;
    std::unique_ptr<JSONImportTask> json_import_task_;
    std::unique_ptr<GPSTrailImportTask> gps_trail_import_task_;
    std::unique_ptr<GPSImportCSVTask> gps_import_csv_task_;
    std::unique_ptr<ManageSectorsTask> manage_sectors_task_;
    std::unique_ptr<RadarPlotPositionCalculatorTask> radar_plot_position_calculator_task_;
    std::unique_ptr<CreateARTASAssociationsTask> create_artas_associations_task_;
    std::unique_ptr<ReconstructorTask> reconstruct_references_task_;

    virtual void checkSubConfigurables() override;

    std::map<std::string, Task*> tasks_;

    std::unique_ptr<TaskResultsWidget> widget_{nullptr};

    std::map<unsigned int, std::shared_ptr<TaskResult>> results_; // id -> result
    std::shared_ptr<TaskResult> current_result_;

    std::unique_ptr<ViewableDataConfig> viewable_data_cfg_;

    void addTask(const std::string& class_id, Task* task);
    MainWindow* getMainWindow();

    void loadResults();
};
