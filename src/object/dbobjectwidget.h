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
 * DBObjectEditWidget.h
 *
 *  Created on: Aug 27, 2012
 *      Author: sk
 */

#ifndef DBOBJECTEDITWIDGET_H_
#define DBOBJECTEDITWIDGET_H_

#include <QWidget>
#include <map>

class QLineEdit;
class QComboBox;
class QCheckBox;
class QGridLayout;
class QPushButton;
class QTextEdit;


class DBObject;
class DBODataSourceDefinition;
class DBOVariable;
class DBTableColumnComboBox;
class DBOVariableDataTypeComboBox;
class StringRepresentationComboBox;
class DBSchemaManager;

/**
 * @brief Edit widget for a DBObject
 */
class DBObjectWidget : public QWidget
{
    Q_OBJECT

signals:
    /// @brief Emitted if DBObject was changed
    void changedDBOSignal();

public slots:
    /// @brief Adds all new DBOVariables
    void addNewVariables ();
    /// @brief Adds a MetaDBTable
    void addMetaTable();

    /// @brief Updates data source schema selection
    void updateDSSchemaSelection();
    /// @brief Updates data sources grid
    void updateDataSourcesGrid ();
    /// @brief Adds a data source
    void addDataSource ();
    /// @brief Changes data source schema
    void changedDSSchema();
    /// @brief Updates data sources meta table dependents
    void changedDSMetaTable();
    /// @brief Edits a DBOVariable
    void editDataSource();
    /// @brief Deletes a DBOVariable
    void deleteDataSource();

    /// @brief Changes DBO name
    void editName ();
    /// @brief Changes DBO info
    void editInfo ();

    /// @brief Edits a DBOVariable
    void editDBOVar();
    /// @brief Deletes a DBOVariable
    void deleteDBOVar();

    /// @brief Updates the DBOVariables grid
    void updateDBOVarsGrid ();
    /// @brief Updates the schema selection for meta table
    void updateMetaSchemaSelection ();
    /// @brief Updates the meta table selection
    void updateMetaTableSelection ();
    /// @brief Updates the schema selection for adding all variables
    void updateAllVarsSchemaSelection ();
    /// @brief Updates meta tables grid
    void updateMetaTablesGrid();

public:
    /// @brief Constructor
    DBObjectWidget(DBObject *object, DBSchemaManager &schema_manager, QWidget * parent = 0, Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~DBObjectWidget();

private:
    /// @brief DBObject to be managed
    DBObject* object_;
    DBSchemaManager &schema_manager_;

    /// @brief DBO name
    QLineEdit* name_edit_;
    /// @brief DBO info
    QLineEdit* info_edit_;

    /// @brief Grid with all data sources
    QGridLayout* ds_grid_;
    /// @brief Container with data sources edit buttons
    std::map <QPushButton*, DBODataSourceDefinition*> ds_grid_edit_buttons_;
    /// @brief Container with data sources delete buttons
    std::map <QPushButton*, DBODataSourceDefinition*> ds_grid_delete_buttons_;


    /// @brief Add new data source schema selection
    QComboBox* ds_schema_box_;

    /// @brief grid with all meta tables per schema
    QGridLayout* meta_table_grid_;

    /// @brief Add meta table for schema schema selection
    QComboBox* new_meta_schema_box_;
    /// @brief Add meta table for schema meta table selection
    QComboBox* new_meta_box_;

    /// @brief Grid with all DBOVariables
    QGridLayout* dbovars_grid_;

    /// @brief Container with DBOVariable edit buttons
    std::map <QPushButton*, DBOVariable*> dbo_vars_grid_edit_buttons_;
    /// @brief Container with DBOVariable delete buttons
    std::map <QPushButton*, DBOVariable*> dbo_vars_grid_delete_buttons_;

    /// @brief New DBOVariable name edit field
    QLineEdit* new_var_name_edit_;

    /// @brief Add all variables schema box
    QComboBox* all_schemas_box_;
    /// @brief Add all variables button
    QPushButton* add_all_button_;

    /// @brief Updates a schema selection box
    void updateSchemaSelectionBox (QComboBox* box);
};

#endif /* DBOBJECTEDITWIDGET_H_ */
