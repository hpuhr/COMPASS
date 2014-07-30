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
 * MetaDBObjectEditWidget.h
 *
 *  Created on: Aug 27, 2012
 *      Author: sk
 */

#ifndef MetaDBObjectEditWidget_H_
#define MetaDBObjectEditWidget_H_

#include <QWidget>
#include <map>

class DBObject;
class DBOVariable;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QGridLayout;
class QPushButton;
class DBOTypeComboBox;
class DBOVariableDataTypeComboBox;
class StringRepresentationComboBox;

/**
 * @brief Edit widget for a meta DBObject
 */
class MetaDBObjectEditWidget : public QWidget
{
    Q_OBJECT

signals:
    /// @brief Emitted when the DBObject was changed
    void changedDBO();

public slots:
    /// @brief Adds a variable
    void addVariable();
    /// @brief Deletes a variable
    void deleteVariable();
    /// @brief Sets the name
    void editName ();
    /// @brief Sets the info
    void editInfo ();
    /// @brief Updates the variables grid
    void updateDBOVarsGrid ();


public:
    /// @brief Constructor
    MetaDBObjectEditWidget(DBObject *object, QWidget * parent = 0, Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~MetaDBObjectEditWidget();

protected:
    /// Represented object
    DBObject *object_;

    /// Name edit field
    QLineEdit *name_edit_;
    /// Info edit field
    QLineEdit *info_edit_;
    /// Grid for variables
    QGridLayout *dbovars_grid_;

    /// Container for variable delete buttons
    std::map <QPushButton *, DBOVariable *> dbo_vars_grid_delete_buttons_;
    /// Container for variable data type selection fields
    std::map <DBOVariableDataTypeComboBox *, DBOVariable *> dbo_vars_grid_data_type_boxes_;
    /// Container for variable string representation selection fields
    std::map <StringRepresentationComboBox *, DBOVariable *> dbo_vars_grid_representation_boxes_;

    /// New variable name edit field
    QLineEdit *new_var_name_edit_;

    /// @brief Creates GUI elements
    void createElements ();
};

#endif /* MetaDBObjectEditWidget_H_ */
