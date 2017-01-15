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


#ifndef TRANSFORMATIONVARIABLEEDITDIALOG_H
#define TRANSFORMATIONVARIABLEEDITDIALOG_H

#include "TransformationVariable.h"

#include <QDialog>

#include "ui_TransformationVariableEditDialogBase.h"

class QComboBox;


/**
@brief Used to display a Property entry in a TransformationVariable

This class is used to display a Property entry in a TransformationVariable.
The widgets are displayed inside of a TransformationVariableEditDialog.
  */
class PropertyWidget : public QWidget
{
    Q_OBJECT
public:
    /// @brief Constructor
    PropertyWidget( QWidget* parent=NULL );
    /// @brief Destructor
    ~PropertyWidget();

    /// @brief Returns the widgets data type combo
    QComboBox* getDataTypeCombo();
    /// @brief Returns the displayed Property
    Property getProperty() const;
    /// @brief Returns the displayed DBO type
    DB_OBJECT_TYPE getDBOType() const;
    /// @brief Returns the stored DBOVariable
    DBOVariable* getDBOVariable();

    /// @brief Sets the entries DBO type
    void setDBOType( DB_OBJECT_TYPE dbo_type );
    /// @brief Sets the entries data type
    void setDataType( PROPERTY_DATA_TYPE dtype );
    /// @brief Sets the entries name
    void setPropertyName( const QString& name );
    /// @brief Sets an associated DBOVariable
    void setDBOVariable( DBOVariable* var );

signals:
    /// @brief Sends a delete request
    void deleteMe();

private:
    QLineEdit* name_edit_;
    QLineEdit* dbo_edit_;
    QComboBox* dtype_combo_;
    QPushButton* delete_button_;

    /// Entries DBO type
    DB_OBJECT_TYPE dbo_type_;
    /// Stored DBOVariable
    DBOVariable* var_;
};

/**
@brief Dialog to edit a TransformationVariable.

This dialog can be used to edit the Property realizations of a transformation variable.

The data type of the variable can be edited here. Be careful what you do!
If a data type is chosen, the data types of the added properties will be greyed out, since
the variables data type will be used anyway. If 'NoType' is chosen, data type checking will
be disabled in the variable, and the entries data types will be reenabled. In this case
they may be edited independently.

A default Property can be provided. For this enable the respective checkbox and enter a valid name.
It will only be possible to assign a default property name, if a valid data type is chosen in the
variables check box. Those two are then set as the default Property in the edited variable.

It is possible to add Property entries in two ways. The first is to provide a DBOVariable through
the select widget and then click on the Add-button next to it.
The other one is to provide a name, DBO type and data type manually and click the other Add-button.

Widgets that already exist for a specific DBO type will just be updated with the new information.
When added, the displayed data may also be changed by editing the widgets directly.
Every entry can further be deleted by clicking on the respective Remove-button.

The stored variable is only changed at the moment the user presses the Ok-button and if all provided
data is valid. Clicking on the Cancel-button will discard all changes.
  */
class TransformationVariableEditDialog  : public QDialog, private Ui::TransformationVariableEditDialogBase
{
    Q_OBJECT
public:
    /// @brief Constructor
    TransformationVariableEditDialog( TransformationVariable* variable, QWidget* parent=NULL );
    /// @brief Destructor
    virtual ~TransformationVariableEditDialog();

protected slots:
    /// @brief Adds a Property from the specified DBO variable
    void addPropertyDBOSlot();
    /// @brief Adds a Property from the provided manual information
    void addPropertyManualSlot();
    /// @brief Deletes the clicked property entry
    void deletePropertySlot();
    /// @brief Triggered if the variables data type changes
    void dataTypeChangedSlot( int idx );
    /// @brief Accepts the changes
    void okSlot();
    /// @brief Cancels the changes
    void cancelSlot();
    /// @brief Triggered when the default id has been enabled/disabled
    void defaultIDEnabledSlot( bool enabled );

protected:
    /// @brief Inits the dialog
    void init();
    /// @brief Fills a data type combo box
    void fillDataTypeCombo( QComboBox* combo, bool add_notype=false );
    /// @brief Fills a DBO type combo box
    void fillDBOTypeCombo( QComboBox* combo );
    /// @brief Sets the variables data type using the given property data type
    void setDataType( int dtype );
    /// @brief Changes the data type combos of all Property widgets according to the variables data type combo
    void checkDataTypeCombos();

    /// The variable to edit
    TransformationVariable* variable_;
    /// A temporary variable
    TransformationVariable variable_tmp_;
    /// The Property widgets
    std::map<DB_OBJECT_TYPE,PropertyWidget*> widgets_;
};

#endif //TRANSFORMATIONVARIABLEEDITDIALOG_H
