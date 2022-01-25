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

private slots:

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

    void livePauseSlot();
    void liveStopSlot();

    void configureMetaVariablesSlot();

    void importAsterixRecordingSlot();
    void importRecentAsterixRecordingSlot();
    void importAsterixFromNetworkSlot();

    void calculateAssociationsARTASSlot();
    void calculateAssociationsSlot();

    void quitRequestedSlot();
    void showAddViewMenuSlot();

    void appModeSwitchSlot (AppMode app_mode);

public:
    MainWindow();
    virtual ~MainWindow();

    void disableConfigurationSaving();
    void showEvaluationTab();
    void showViewPointsTab();

    void createAndOpenNewSqlite3DB(const std::string& filename);
    void openSqlite3DB(const std::string& filename);

    void importASTERIXFile(const std::string& filename);
    void importASTERIXFromNetwork();
    void importASTERIXFromNetworkTimeOffset(float value);

    float importASTERIXFromNetworkTimeOffset();
    //    void asterixFraming(const std::string& asterix_framing);
    //    void asterixDecoderConfig(const std::string& asterix_decoder_cfg);
    //    bool asterixOptionsSet() const;
    //    void setAsterixOptions();

    void loadData(bool value);
    void quit(bool value);
    bool quitNeeded();

    bool automaticTasksDefined() const;
    void performAutomaticTasks ();

    void updateMenus();
    void updateBottomWidget();

protected:
    bool started_ {false};
    QTabWidget* tab_widget_{nullptr};

    QPushButton* add_view_button_{nullptr};

    bool save_configuration_{true};

    // command line defined tasks
    bool automatic_tasks_defined_ {false};
    bool sqlite3_create_new_db_ {false};
    std::string sqlite3_create_new_db_filename_;

    bool sqlite3_open_db_ {false};
    std::string sqlite3_open_db_filename_;

    bool asterix_import_file_ {false};
    std::string asterix_import_filename_;
    bool asterix_import_network_ {false};
    float asterix_import_network_time_offset_ {0};
    //    bool set_asterix_framing_ {false};
    //    std::string asterix_framing_;
    //    bool set_asterix_decoder_cfg_ {false};
    //    std::string asterix_decoder_cfg_;

    bool load_data_ {false};
    bool quit_ {false};

    // menu

    // file menu
    QAction* new_db_action_ {nullptr};
    QAction* open_existing_db_action_ {nullptr};
    QMenu* open_recent_db_menu_ {nullptr};
    QAction* close_db_action_ {nullptr};

    // import menu
    QMenu* import_menu_ {nullptr};

    QMenu* import_recent_asterix_menu_ {nullptr};

    // process menu
    QMenu* process_menu_ {nullptr};

    bool loading_{false};

    QLabel* db_label_{nullptr};
    QLabel* status_label_{nullptr};
    QPushButton* load_button_{nullptr};

    QPushButton* live_pause_button_{nullptr};
    QPushButton* live_stop_button_{nullptr};

    void createMenus ();

    /// @brief Called when application closes
    void closeEvent(QCloseEvent* event);

    void shutdown();
};

//}
#endif /* MAINWINDOW_H_ */
