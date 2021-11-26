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

//#include "JobOrderer.h"

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

/**
 * @brief Main window which embeds all other components
 *
 * When started, allows management of database connection and schema. When database is opened,
 * a stack widget is used to display the main widget with further components.
 *
 * Also handles shutdown behavior using the closeEvent() function.
 */
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

    //void startSlot();

    // void keyPressEvent ( QKeyEvent * event );

    void quitRequestedSlot();
    void showAddViewMenuSlot();

  public:
    /// @brief Constructor
    MainWindow();
    /// @brief Destructor
    virtual ~MainWindow();

    void disableConfigurationSaving();
    void showEvaluationTab();
    void showViewPointsTab();

protected:
    bool started_ {false};
    /// Widget stack for startup to usage switch
    QTabWidget* tab_widget_{nullptr};

    MainLoadWidget* management_widget_{nullptr};

    QPushButton* add_view_button_{nullptr};

    bool save_configuration_{true};

    QAction* new_db_action_ {nullptr};
    QAction* open_existng_action_ {nullptr};
    QMenu* open_recent_menu_ {nullptr};
    QAction* close_db_action_ {nullptr};

    void createMenus ();
    void updateMenus();

    /// @brief Called when application closes
    void closeEvent(QCloseEvent* event);

    void shutdown();
};

//}
#endif /* MAINWINDOW_H_ */
