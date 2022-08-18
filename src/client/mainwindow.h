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

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include "global.h"
#include "appmode.h"

#include <QMainWindow>

class QLabel;
class QPushButton;
class QTabWidget;
class QCheckBox;
class QMenu;
class QPushButton;
class QAction;

class DBSelectionWidget;
class DBSchemaManagerWidget;
class DBContentManagerWidget;
class MainLoadWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public slots:

    void newDBSlot();
    void openExistingDBSlot();
    void openRecentDBSlot();
    void clearExistingDBsSlot();
    void closeDBSlot();

    void saveConfigSlot();

    void quitWOConfigSlot();
    void quitSlot();

    void loadButtonSlot();
    void loadingDoneSlot();

    void livePauseResumeSlot();
    void liveStopSlot();

    void configureDataSourcesSlot();
    void configureMetaVariablesSlot();
    void configureSectorsSlot();

    void importAsterixRecordingSlot();
    void importRecentAsterixRecordingSlot();
    void clearImportRecentAsterixRecordingsSlot();
    void importAsterixFromNetworkSlot();

    void importGPSTrailSlot();

    void importViewPointsSlot();

    void calculateRadarPlotPositionsSlot();
    void calculateAssociationsARTASSlot();
    void calculateAssociationsSlot();

    void quitRequestedSlot();
    void showAddViewMenuSlot();

    void appModeSwitchSlot (AppMode app_mode);

private slots:
    void runTestCodeSlot();

public:
    MainWindow();
    virtual ~MainWindow();

    void disableConfigurationSaving();
    void showEvaluationTab();
    void showViewPointsTab();

    void createAndOpenNewSqlite3DB(const std::string& filename);
    void openSqlite3DB(const std::string& filename);

    void importDataSourcesFile(const std::string& filename);
    void importViewPointsFile(const std::string& filename);

    void importASTERIXFile(const std::string& filename);
    void importASTERIXFromNetwork();
    void importASTERIXFromNetworkTimeOffset(float value);
    float importASTERIXFromNetworkTimeOffset();
    int importAsterixNetworkMaxLines() const;
    void importAsterixNetworkMaxLines(int value);
    //    void asterixFraming(const std::string& asterix_framing);
    //    void asterixDecoderConfig(const std::string& asterix_decoder_cfg);
    //    bool asterixOptionsSet() const;
    //    void setAsterixOptions();

    void importGPSTrailFile(const std::string& filename);
    void importSectorsFile(const std::string& filename);

    void calculateRadarPlotPositions(bool value);
    void associateData(bool value);

    void loadData(bool value);

    void exportViewPointsReportFile(const std::string& filename);
    void exportEvalReportFile(const std::string& filename);

    void evaluateRunFilter(bool value);
    void evaluate(bool evaluate);

    void quit(bool value);
    bool quitNeeded();

    bool automaticTasksDefined() const;
    void performAutomaticTasks ();

    void updateMenus();
    void updateBottomWidget();

protected:
    QTabWidget* tab_widget_{nullptr};

    QPushButton* add_view_button_{nullptr};

    bool save_configuration_{true};

    // command line defined tasks
    bool automatic_tasks_defined_ {false};
    bool sqlite3_create_new_db_ {false};
    std::string sqlite3_create_new_db_filename_;

    bool sqlite3_open_db_ {false};
    std::string sqlite3_open_db_filename_;

    bool data_sources_import_file_ {false};
    std::string data_sources_import_filename_;

    bool view_points_import_file_ {false};
    std::string view_points_import_filename_;

    bool asterix_import_file_ {false};
    std::string asterix_import_filename_;
    bool asterix_import_network_ {false};
    float asterix_import_network_time_offset_ {0};
    int asterix_import_network_max_lines_ {-1};

    bool gps_trail_import_file_ {false};
    std::string gps_trail_import_filename_;

    bool sectors_import_file_ {false};
    std::string sectors_import_filename_;

    bool calculate_radar_plot_postions_ {false};
    bool associate_data_ {false};

    bool load_data_ {false};

    bool export_view_points_report_ {false};
    std::string export_view_points_report_filename_;

    bool evaluate_run_filter_ {false};
    bool evaluate_ {false};
    bool export_eval_report_ {false};
    std::string export_eval_report_filename_;

    bool quit_ {false};

    // menu

    // file menu
    QAction* new_db_action_ {nullptr};
    QAction* open_existing_db_action_ {nullptr};
    QMenu* open_recent_db_menu_ {nullptr};
    QAction* close_db_action_ {nullptr};

    // configure sectors
    QAction* sectors_action_ {nullptr};

    // import menu
    QMenu* import_menu_ {nullptr};

    QMenu* import_recent_asterix_menu_ {nullptr};

    // process menu
    QMenu* process_menu_ {nullptr};

    bool loading_{false};

    QLabel* db_label_{nullptr};
    QLabel* status_label_{nullptr};
    QPushButton* load_button_{nullptr};

    QPushButton* live_pause_resume_button_{nullptr};
    QPushButton* live_stop_button_{nullptr};

    void createMenus ();

    /// @brief Called when application closes
    void closeEvent(QCloseEvent* event);

    void shutdown();
};

//}
#endif /* MAINWINDOW_H_ */
