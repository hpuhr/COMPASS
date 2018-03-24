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

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QMainWindow>

//#include "JobOrderer.h"

class QPushButton;
class DBSelectionWidget;
class DBSchemaManagerWidget;
class DBObjectManagerWidget;
class ManagementWidget;
class QTabWidget;
class QCheckBox;
class QMenu;

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

signals:
    void startedSignal ();

public slots:
    void postProcessingDoneSlot ();

private slots:
    /// @brief Called when database was opened
    void databaseOpenedSlot();
    /// @brief If database is open, switch to ManagementWidget
    void startSlot ();
    void addJSONImporterTaskSlot ();
    void addRadarPlotPositionCalculatorTaskSlot ();

    /// @brief Handles key press events
    void keyPressEvent ( QKeyEvent * event );

public:
    /// @brief Constructor
    MainWindow();
    /// @brief Destructor
    virtual ~MainWindow();

protected:
    /// Widget stack for startup to usage switch
    QTabWidget *tab_widget_;
    /// Database configuration widget
    QWidget *dbinterface_widget_;
    // Contains database schema configuration elements
    QWidget *dbschema_manager_widget_;
    QMenu *task_menu_ {nullptr};

    /// Contains DBObject configuration elements
    DBObjectManagerWidget *object_manager_widget_;

    ManagementWidget *management_widget_;

    QCheckBox *postprocess_check_{nullptr};
    QPushButton *start_button_{nullptr};

    /// @brief Creates File menu
    void createMenus();

    void initAfterStart ();

    /// @brief Called when application closes
    void closeEvent(QCloseEvent *event);
};

//}
#endif /* MAINWINDOW_H_ */
