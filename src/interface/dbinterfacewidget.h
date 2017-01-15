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

#ifndef DBINTERFACEWIDGET_H_
#define DBINTERFACEWIDGET_H_

#include <QFrame>
#include <QComboBox>

class QTextEdit;
class QRadioButton;
class QLineEdit;
class QPushButton;
class DBConnectionInfo;
class DBInterface;

/**
 * @brief Widget for choosing a database system and parameters
 */
class DBInterfaceWidget : public QFrame
{
    Q_OBJECT

public slots:
    /// @brief Starts the open file dialog
    //void selectFile();
    /// @brief Sets database system based on radio buttons
    void selectDBType();

signals:
    void databaseOpened ();

public:
    /// @brief Constructor
    explicit DBInterfaceWidget(DBInterface &interface, QWidget* parent = 0, Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~DBInterfaceWidget();

protected:
    DBInterface &interface_;
    /// SQLite3 selection radio button
    //QRadioButton *file_radio_;
    /// MySQL++ selection radio button
    QRadioButton *mysqlpp_radio_;
    /// MySQL Connector selection radio button
    //QRadioButton *mysqlcon_radio_;
    /// Database type, 0 undefined, 1 sqlite file, 2 mysqlpp, 3 mysqlcon
    unsigned int db_type_selection_;

    /// Filename edit field
    //QTextEdit *filename_edit_;
    /// Filename
    //std::string filename_;

    /// @brief Creates GUI elements
    void createElements ();
};

#endif /* DBINTERFACEWIDGET_H_ */
