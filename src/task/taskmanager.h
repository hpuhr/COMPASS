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

#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QObject>

#include "configurable.h"
#include "global.h"
#include "singleton.h"
#include "task.h"

class COMPASS;
class DatabaseOpenTask;
class ManageDBContentTask;
class CreateARTASAssociationsTask;
class JSONImportTask;
class GPSTrailImportTask;
class ASTERIXImportTask;
class ViewPointsImportTask;
class RadarPlotPositionCalculatorTask;
//class PostProcessTask;
//class TaskManagerWidget;
//class ManageDataSourcesTask;
class ManageSectorsTask;
class CreateAssociationsTask;
class MainWindow;



class QMainWindow;

class TaskManager : public QObject, public Configurable
{
    Q_OBJECT

  signals:
    void startInspectionSignal();
    void expertModeChangedSignal();

    void quitRequestedSignal ();

  public slots:
//    void taskStatusChangesSlot(std::string task_name);
//    void taskDoneSlot(std::string task_name);

    //void dbObjectsChangedSlot();
    //void schemaChangedSlot();

  public:
    TaskManager(const std::string& class_id, const std::string& instance_id, COMPASS* compass);

    virtual ~TaskManager();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    // void deleteWidgets ();
    void shutdown();

    //TaskManagerWidget* widget();  // owned here

    std::vector<std::string> taskList() const;
    std::map<std::string, Task*> tasks() const;

//    bool expertMode() const;
//    void expertMode(bool value);

//    void appendSuccess(const std::string& text);
//    void appendInfo(const std::string& text);
//    void appendWarning(const std::string& text);
//    void appendError(const std::string& text);

    void runTask(const std::string& task_name);

    DatabaseOpenTask& databaseOpenTask() const;
    //ManageDataSourcesTask& manageDataSourcesTask() const;
    ASTERIXImportTask& asterixImporterTask() const;
    ViewPointsImportTask& viewPointsImportTask() const;
    JSONImportTask& jsonImporterTask() const;
    GPSTrailImportTask& gpsTrailImportTask() const;
    //ManageDataSourcesTask& manageDatasourcesTask() const;
    ManageSectorsTask& manageSectorsTask() const;
    RadarPlotPositionCalculatorTask& radarPlotPositionCalculatorTask() const;
    CreateARTASAssociationsTask& createArtasAssociationsTask() const;
    //PostProcessTask& postProcessTask() const;
    CreateAssociationsTask& createAssociationsTask() const;

//    void createAndOpenNewSqlite3DB(const std::string& filename);
//    void openSqlite3DB(const std::string& filename);

//    void importViewPointsFile(const std::string& filename);

//    void importASTERIXFile(const std::string& filename);
//    void asterixFraming(const std::string& asterix_framing);
//    void asterixDecoderConfig(const std::string& asterix_decoder_cfg);
//    bool asterixOptionsSet() const;
//    void setAsterixOptions();

//    void importJSONFile(const std::string& filename, const std::string& schema);
//    void importGPSTrailFile(const std::string& filename);
//    void importSectorsFile(const std::string& filename);

//    void autoProcess(bool value);

//    void associateData(bool value);

//    void quit(bool value);
//    //void start(bool value);

//    void loadData(bool value);
//    void exportViewPointsReportFile(const std::string& filename);
//    void exportEvalReportFile(const std::string& filename);

//    bool automaticTasksDefined() const;
//    void performAutomaticTasks ();

//    void evaluate(bool evaluate);


protected:
//    bool expert_mode_{false};

    // command line defined tasks
//    bool automatic_tasks_defined_ {false};
//    bool sqlite3_create_new_db_ {false};
//    std::string sqlite3_create_new_db_filename_;

//    bool sqlite3_open_db_ {false};
//    std::string sqlite3_open_db_filename_;

//    bool view_points_import_file_ {false};
//    std::string view_points_import_filename_;

//    bool asterix_import_file_ {false};
//    std::string asterix_import_filename_;
//    bool set_asterix_framing_ {false};
//    std::string asterix_framing_;
//    bool set_asterix_decoder_cfg_ {false};
//    std::string asterix_decoder_cfg_;

//    bool json_import_file_ {false};
//    std::string json_import_filename_;
//    std::string json_import_schema_;

//    bool gps_trail_import_file_ {false};
//    std::string gps_trail_import_filename_;

//    bool sectors_import_file_ {false};
//    std::string sectors_import_filename_;

//    bool auto_process_ {false};

//    bool associate_data_ {false};

//    //bool start_ {false};
//    bool load_data_ {false};

//    bool export_view_points_report_ {false};
//    std::string export_view_points_report_filename_;

//    bool evaluate_ {false};
//    bool export_eval_report_ {false};
//    std::string export_eval_report_filename_;

//    bool quit_ {false};

    // tasks
    std::unique_ptr<DatabaseOpenTask> database_open_task_;
    std::unique_ptr<ManageDBContentTask> manage_dbobjects_task_;
    std::unique_ptr<ASTERIXImportTask> asterix_importer_task_;
    std::unique_ptr<ViewPointsImportTask> view_points_import_task_;
    std::unique_ptr<JSONImportTask> json_import_task_;
    std::unique_ptr<GPSTrailImportTask> gps_trail_import_task_;
    //std::unique_ptr<ManageDataSourcesTask> manage_datasources_task_;
    std::unique_ptr<ManageSectorsTask> manage_sectors_task_;
    std::unique_ptr<RadarPlotPositionCalculatorTask> radar_plot_position_calculator_task_;
    //std::unique_ptr<PostProcessTask> post_process_task_;
    std::unique_ptr<CreateARTASAssociationsTask> create_artas_associations_task_;
    std::unique_ptr<CreateAssociationsTask> create_associations_task_;

// std::unique_ptr<TaskManagerWidget> widget_;

    virtual void checkSubConfigurables();

    std::vector<std::string> task_list_;
    std::map<std::string, Task*> tasks_;

    void addTask(const std::string& class_id, Task* task);
    MainWindow* getMainWindow();
};

#endif  // TASKMANAGER_H
