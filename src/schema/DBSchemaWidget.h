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
 * DBSchemaWidget.h
 *
 *  Created on: Aug 19, 2012
 *      Author: sk
 */

#ifndef DBSCHEMAWIDGET_H_
#define DBSCHEMAWIDGET_H_

#include <QFrame>
//#include "Configurable.h"

class QTextEdit;
class QLineEdit;
class QComboBox;
class QPushButton;
class DBSchemaEditWidget;

/**
 * @brief Widget for setting the current and adding a new DBSchema
 */
class DBSchemaWidget : public QFrame
{
    Q_OBJECT

protected slots:
    /// @brief Updates the current schema selection field
    void updateSchemaCombo();
    /// @brief Adds a new schema
    void addEmptySchema ();
    /// @brief Edits the current schema
    void editSchema ();
    /// @brief Selects the current schema
    void selectSchema (int index);
    /// @brief Called when the schema was renamed
    void renamed ();

public:
    /// @brief Constructor
    DBSchemaWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~DBSchemaWidget();

    /// @brief Sets the schema
    void setSchema (std::string schema);
    /// @brief Returns if a schema was selected
    bool hasSelectedSchema ();
    /// @brief Returns selected schema
    std::string getSelectedSchema ();

    /// @brief Unlocks editing functionality
    void unlock ();

protected:
    /// New schema name edit field
    QLineEdit *new_schema_name_edit_;
    /// Current schema selection field
    QComboBox *current_schema_select_;

    /// Current schema edit button
    QPushButton *edit_button_;
    /// Current schema edit widget
    DBSchemaEditWidget *edit_widget_;

    QPushButton *add_button_;

    /// @brief Creates GUI elements
    void createElements();
};

#endif /* DBSCHEMAWIDGET_H_ */
