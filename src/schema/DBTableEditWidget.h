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
 * DBTableEditWidget.h
 *
 *  Created on: Aug 21, 2012
 *      Author: sk
 */

#ifndef DBTABLEEDITWIDGET_H_
#define DBTABLEEDITWIDGET_H_

#include <QWidget>

#include "Configurable.h"

class DBTable;
class DBTableColumn;
class QLineEdit;
class QComboBox;
class QGridLayout;
class QScrollArea;
class QPushButton;

/**
 * @brief Edit widget for a DBTable
 */
class DBTableEditWidget : public QWidget
{
    Q_OBJECT

signals:
    /// @brief Emitted if the table was changed
    void changedTable();

public slots:
    /// @brief Deletes a column
    void deleteColumn ();
    /// @brief Sets the table parameters
    void updateParameters ();
    /// @brief Creates all columns that exist in database table
    void createColumnsFromDB ();
    void createNewColumnsFromDB ();
    /// @brief Sets a special null value
    void setSpecialNull ();

public:
    /// @brief Constructor
    DBTableEditWidget(DBTable *table, QWidget * parent = 0, Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~DBTableEditWidget();

protected:
    /// Represented table
    DBTable *table_;

    /// Table name edit field
    QLineEdit *name_edit_;
    /// Database table selection field
    QComboBox *db_name_box_;
    /// Table info edit field
    QLineEdit *info_edit_;
    /// Table key selection field
    QComboBox *key_box_;

    /// Grid with all table columns
    QGridLayout *column_grid_;

    /// Container with all column delete buttons
    std::map <QPushButton *, DBTableColumn *> column_grid_delete_buttons_;
    /// Container with all column special null edit fields
    std::map <QLineEdit *, DBTableColumn* > column_grid_special_nulls_;

    /// @brief Creates GUI elements
    void createElements ();
    /// @brief Updates the database table selection
    void updateTableSelection();
    /// @brief Updates the table key selection field
    void updateKeySelection();
    /// @brief Updates the table columns grid
    void updateColumnGrid ();
};

#endif /* DBTABLEEDITWIDGET_H_ */
