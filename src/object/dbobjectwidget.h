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
class DBSchemaSelectionComboBox;
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
    void addNewVariablesSlot ();
    /// @brief Adds a MetaDBTable
    void addMetaTableSlot();

    /// @brief Updates data sources grid
    void updateDataSourcesGridSlot ();
    /// @brief Adds a data source
    void addDataSourceSlot ();
    /// @brief Edits a DBOVariable
    void editDataSourceSlot();
    /// @brief Deletes a DBOVariable
    void deleteDataSourceSlot();

    /// @brief Changes DBO name
    void editNameSlot ();
    /// @brief Changes DBO info
    void editInfoSlot ();

    /// @brief Edits a DBOVariable
    //void editDBOVarSlot();
    void editDBOVariableNameSlot ();
    void editDBOVariableDescriptionSlot ();
    void editDBOVariableDBColumnSlot (const QString& text);
    /// @brief Deletes a DBOVariable
    void deleteDBOVarSlot();

    /// @brief Updates the DBOVariables grid
    void updateDBOVarsGridSlot ();
    /// @brief Updates the schema selection for adding all variables
    //void updateAllVarsSchemaSelectionSlot ();
    /// @brief Updates meta tables grid
    void updateMetaTablesGridSlot();

    void showLabelDefinitionWidgetSlot();

public:
    /// @brief Constructor
    DBObjectWidget(DBObject* object, DBSchemaManager& schema_manager, QWidget* parent=0, Qt::WindowFlags f=0);
    /// @brief Destructor
    virtual ~DBObjectWidget();

    void lock ();
    void unlock ();

private:
    /// @brief DBObject to be managed
    DBObject* object_ {nullptr};
    DBSchemaManager& schema_manager_;

    /// @brief DBO name
    QLineEdit* name_edit_ {nullptr};
    /// @brief DBO info
    QLineEdit* info_edit_ {nullptr};

    QPushButton* edit_label_button_ {nullptr};

    /// @brief Grid with all data sources
    QGridLayout* ds_grid_ {nullptr};
    /// @brief Container with data sources edit buttons
    //std::map <QPushButton*, DBODataSourceDefinition*> ds_grid_edit_buttons_;
    /// @brief Container with data sources delete buttons
    //std::map <QPushButton*, DBODataSourceDefinition*> ds_grid_delete_buttons_;

    QPushButton* new_ds_button_ {nullptr};

    /// @brief grid with all meta tables per schema
    QGridLayout* meta_table_grid_ {nullptr};

    QPushButton* new_meta_button_ {nullptr};

    /// @brief Grid with all DBOVariables
    QGridLayout* dbovars_grid_ {nullptr};

    /// @brief Container with DBOVariable edit buttons
    //std::map <QPushButton*, DBOVariable*> dbo_vars_grid_edit_buttons_;
    /// @brief Container with DBOVariable delete buttons
    //std::map <QPushButton*, DBOVariable*> dbo_vars_grid_delete_buttons_;

    /// @brief Add all variables schema box
    DBSchemaSelectionComboBox* all_schemas_box_ {nullptr};
    QPushButton* add_schema_button_ {nullptr};

    bool locked_ {false};
};

#endif /* DBOBJECTEDITWIDGET_H_ */
