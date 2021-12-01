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

#include <QMainWindow>

#include "global.h"

class QPushButton;
class QTabWidget;
class QCheckBox;
class QMenu;
class QPushButton;
class QAction;

class DBSelectionWidget;
class DBSchemaManagerWidget;
class DBObjectManagerWidget;
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

    void importAsterixRecordingSlot();

    void configureMetaVariablesSlot();

    void quitRequestedSlot();
    void showAddViewMenuSlot();

  public:
    MainWindow();
    virtual ~MainWindow();

    void disableConfigurationSaving();
    void showEvaluationTab();
    void showViewPointsTab();

protected:
    bool started_ {false};
    QTabWidget* tab_widget_{nullptr};

    QPushButton* add_view_button_{nullptr};

    bool save_configuration_{true};

    // menu

    // file menu
    QAction* new_db_action_ {nullptr};
    QAction* open_existng_action_ {nullptr};
    QMenu* open_recent_menu_ {nullptr};
    QAction* close_db_action_ {nullptr};

    // import menu
    QMenu* import_menu_ {nullptr};

    void createMenus ();
    void updateMenus();

    /// @brief Called when application closes
    void closeEvent(QCloseEvent* event);

    void shutdown();
};

//}
#endif /* MAINWINDOW_H_ */
