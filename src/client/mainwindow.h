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

/*
 * MainWindow.h
 *
 *  Created on: Aug 2, 2011
 *      Author: sk
 */

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QMainWindow>

#include "configurable.h"
//#include "JobOrderer.h"

class QPushButton;
//class DBConnectionInfo;
//class DBObjectWidget;
class DBSelectionWidget;
//class DBSchemaWidget;
class QStackedWidget;

//namespace ATSDB
//{
    //class MainWidget;

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

//signals:
//    /// @brief Emitted when database was opened
//    void openedDatabase();

private slots:
    /// @brief Called when database was opened
    void openedDB();
    /// @brief If database is open, switch to ManagementWidget
    void start ();

    /// @brief Handles key press events
    void keyPressEvent ( QKeyEvent * event );

public:
    /// @brief Constructor
    MainWindow();
    /// @brief Destructor
    virtual ~MainWindow();

//    /// @brief Sets database type
//    void setDBType (std::string value);
//    /// @brief Sets database server
//    void setDBServer (std::string value);
//    /// @brief Sets database name
//    void setDBName (std::string value);
//    /// @brief Sets database port number
//    void setDBPort (std::string value);
//    /// @brief Sets database username
//    void setDBUser (std::string value);
//    /// @brief Sets database password
//    void setDBPassword (std::string value);
//    /// @brief Sets database usage without password
//    void setDBNoPassword ();
//    /// @brief Sets database schema
//    void setDBSchema (std::string value);
//    /// @brief Triggers a autostart without user interaction (from console parameters)
//    void triggerAutoStart ();

protected:
    /// Widget stack for startup to usage switch
    QStackedWidget *widget_stack_;
    /// Database configuration widget
    QWidget *db_config_widget_;
    /// Central widget
    //MainWidget *main_widget_;

    /// Contains database type and parameter elements
    DBSelectionWidget *selection_widget_;
    /// Contains database schema configuration elements
    //DBSchemaWidget *schema_widget_;
    /// Contains DBObject configuration elements
    //DBObjectWidget *object_widget_;

    static const unsigned int info_height_=200;

    QPushButton *start_button_;

    bool db_opened_;

    unsigned int pos_x_;
    unsigned int pos_y_;
    unsigned int width_;
    unsigned int height_;
    unsigned int min_width_;
    unsigned int min_height_;
    bool native_menu_;

    /// @brief Creates File menu
    void createMenus();
    /// @brief Creates database configuration widget
    void createDBConfigWidget ();
    /// @brief Opens the database
    //void openDatabase (DBConnectionInfo *info);

    /// @brief Called when application closes
    void closeEvent(QCloseEvent *event);

    /// @brief Unlocks database schema edit elements
    void unlockSchemaGui();
};

//}
#endif /* MAINWINDOW_H_ */
