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
 * DBSelectionWidget.h
 *
 *  Created on: Aug 19, 2012
 *      Author: sk
 */

#ifndef DBSELECTIONWIDGET_H_
#define DBSELECTIONWIDGET_H_

#include <QFrame>
#include <QComboBox>
#include "Configurable.h"
#include "ATSDB.h"

class QTextEdit;
class QRadioButton;
class QLineEdit;
class QPushButton;
class DBConnectionInfo;


class DatabaseNameComboBox : public QComboBox
{
    Q_OBJECT
public:
    /// @brief Constructor.
    DatabaseNameComboBox (QWidget *parent = 0)
    : QComboBox (parent)
    {
    }

    /// @brief Destructor.
    virtual ~DatabaseNameComboBox () { }

    void loadDatabaseNames ()
    {
        std::vector<std::string> names = ATSDB::getInstance().getDatabaseNames();
        std::vector<std::string>::iterator it;

        for (it = names.begin(); it != names.end(); it++)
        {
            addItem((*it).c_str());
        }
    }

    /// @brief Returns the currently selected data source
    std::string getDatabaseName ()
    {
        return currentText().toStdString();
    }

    bool hasDatabaseName (std::string name)
    {
        int index = findText(name.c_str());
        return index >= 0;
    }

    /// @brief Sets the current data source
    void setDatabaseName (std::string name)
    {
        int index = findText(name.c_str());
        assert (index >= 0);
        setCurrentIndex(index);
    }
};

/**
 * @brief Widget for choosing a database system and parameters
 */
class DBSelectionWidget : public QFrame, public Configurable
{
    Q_OBJECT

public slots:
    /// @brief Starts the open file dialog
    //void selectFile();
    /// @brief Sets database system based on radio buttons
    void selectDBType();
    /// @brief Sets MySQL parameters
    void updateMySQLConnectInfo ();
    void updateMySQLDatabaseInfo ();

    void connectDB ();
    void openDB ();

signals:
    void databaseOpened ();

public:
    /// @brief Constructor
    DBSelectionWidget(std::string class_id, std::string instance_id, Configurable *parent);
    /// @brief Destructor
    virtual ~DBSelectionWidget();

    /// @brief Returns if a database system was set
    bool hasDefinedDatabase ();
    /// @brief Returns a description of a database system and parameters
    DBConnectionInfo *getConnectionInfo ();

    /// @brief Sets the database system
    void setDBType (std::string value);
    /// @brief Sets the filename for SQLite3 databases
    //void setDBFilename (std::string value);
    /// @brief Sets the server address for MySQL databases
    void setDBServer (std::string value);
    /// @brief Sets the database name for MySQL databases
    void setDBName (std::string value);
    /// @brief Sets the port for MySQL databases
    void setDBPort (std::string value);
    /// @brief Sets the database user for MySQL databases
    void setDBUser (std::string value);
    /// @brief Sets the database user password for MySQL databases
    void setDBPassword (std::string value);
    /// @brief Sets no password for MySQL databases
    void setDBNoPassword ();

protected:
    /// SQLite3 selection radio button
    //QRadioButton *file_radio_;
    /// MySQL++ selection radio button
    QRadioButton *mysqlpp_radio_;
    /// MySQL Connector selection radio button
    QRadioButton *mysqlcon_radio_;
    /// Database type, 0 undefined, 1 sqlite file, 2 mysqlpp, 3 mysqlcon
    unsigned int db_type_selection_;

    /// Filename edit field
    QTextEdit *filename_edit_;
    /// Filename
    std::string filename_;

    /// MySQL ip address edit field
    QLineEdit *mysql_db_ip_edit_;
    /// MySQL port edit field
    QLineEdit *mysql_db_port_edit_;
    /// MySQL username edit field
    QLineEdit *mysql_db_username_edit_;
    /// MySQL password edit field
    QLineEdit *mysql_db_password_edit_;

    /// Open connection button
    QPushButton *connect_button_;

    /// MySQL database name edit field
    //QLineEdit *mysql_db_name_edit_;
    DatabaseNameComboBox *mysql_db_name_box_;

    QPushButton *open_button_;

    /// MySQL database name
    std::string mysql_db_name_;
    /// MySQL ip address
    std::string mysql_db_ip_;
    /// MySQL port
    std::string mysql_db_port_;
    /// MySQL username
    std::string mysql_db_username_;
    /// MySQL password
    std::string mysql_db_password_;

    /// @brief Creates GUI elements
    void createElements ();

protected:
    virtual void checkSubConfigurables ();
};

#endif /* DBSELECTIONWIDGET_H_ */
