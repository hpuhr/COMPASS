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


#include "TransformationEditDialog.h"
#include "TransformationVariableEditDialog.h"

#include <QMessageBox>


/*************************************************************************************************************
VariableItem
**************************************************************************************************************/

/**
Constructor.
@param var The transformation variable to be represented by the list item.
@param type The variable type.
  */
VariableItem::VariableItem( TransformationVariable* var, VariableType type )
:   QListWidgetItem( NULL, UserType ),
    var_( var ),
    type_( type )
{
    assert( var );

    QString type_str;
    if( type == VAR_IN )
        type_str = "(Input) ";
    else
        type_str = "(Output) ";

    std::string dtype_str = " [NoType]";
    if( var->dataTypeChecked() )
        dtype_str = " [" + PROPERTY_DATA_TYPE_STRINGS[ var->dataType() ] + "]";

    setText( type_str + QString::fromStdString( var->name() ) + QString::fromStdString( dtype_str ) );
}

/**
Destructor.
  */
VariableItem::~VariableItem()
{
}

/**
Returns the represented variable.
@return Represented transformation variable.
  */
TransformationVariable* VariableItem::getVariable()
{
    return var_;
}

/**
Returns the variable type.
@return Variable type.
  */
VariableItem::VariableType VariableItem::getType() const
{
    return type_;
}

/*************************************************************************************************************
TransformationEditDialog
**************************************************************************************************************/

/**
Constructor.
@param trafo Transformation to be edited.
@param with_extras Tells the dialog if transformation variables can be removed and added.
@param parent Parent widget.
  */
TransformationEditDialog::TransformationEditDialog( Transformation* trafo, bool with_extras, QWidget* parent )
:   QDialog( parent ),
    trafo_( trafo ),
    with_extras_( with_extras )
{
    setupUi( this );

    assert( trafo_ );

    init();
}

/**
Destructor.
  */
TransformationEditDialog::~TransformationEditDialog()
{
}

/**
Inits the dialog.
  */
void TransformationEditDialog::init()
{
    type_label_->setText( QString::fromStdString( trafo_->getId() ) );

    type_combo_->addItem( "Input" );
    type_combo_->addItem( "Output" );

    append_box_->setChecked( trafo_->isAppending() );

    //adding and removing variables only if enabled
    name_edit_->setEnabled( with_extras_ );
    type_combo_->setEnabled( with_extras_ );
    add_button_->setEnabled( with_extras_ );
    remove_button_->setEnabled( with_extras_ );

    //add input variable items
    TransformationVariable* var;
    unsigned int i, n = trafo_->numberInputVariables();
    for( i=0; i<n; ++i )
    {
        var = vars_in_.addVariable( trafo_->getInputVariable( i ) );
        variable_list_->addItem( new VariableItem( var, VariableItem::VAR_IN ) );
    }

    //add output variable items
    n = trafo_->numberOutputVariables();
    for( i=0; i<n; ++i )
    {
        var = vars_out_.addVariable( trafo_->getOutputVariable( i ) );
        variable_list_->addItem( new VariableItem( var, VariableItem::VAR_OUT ) );
    }

    //sort by name
    variable_list_->sortItems();

    connect( add_button_, SIGNAL(pressed()), this, SLOT(addVariableSlot()) );
    connect( remove_button_, SIGNAL(pressed()), this, SLOT(removeVariableSlot()) );
    connect( edit_button_, SIGNAL(pressed()), this, SLOT(editVariableSlot()) );
    connect( ok_button_, SIGNAL(pressed()), this, SLOT(okSlot()) );
    connect( cancel_button_, SIGNAL(pressed()), this, SLOT(cancelSlot()) );
}

/**
Adds a new transformation variable.
  */
void TransformationEditDialog::addVariableSlot()
{
    //no valid name
    if( name_edit_->text().isEmpty() )
    {
        QMessageBox::warning( this, "Warning", "Please provide a valid variable name." );
        return;
    }

    std::string name = name_edit_->text().toStdString();

    //add to input or output variables
    if( type_combo_->currentIndex() == VariableItem::VAR_IN )
    {
        if( vars_in_.exists( name ) )
        {
            QMessageBox::warning( this, "Warning", "Please provide a unique variable name." );
            return;
        }

        TransformationVariable* var = vars_in_.addVariable( name );
        variable_list_->addItem( new VariableItem( var, VariableItem::VAR_IN ) );
    }
    else
    {
        if( vars_out_.exists( name ) )
        {
            QMessageBox::warning( this, "Warning", "Please provide a unique variable name." );
            return;
        }

        TransformationVariable* var = vars_out_.addVariable( name );
        variable_list_->addItem( new VariableItem( var, VariableItem::VAR_OUT ) );
    }

    name_edit_->clear();
    variable_list_->sortItems();
}

/**
Removes the selected transformation variable.
  */
void TransformationEditDialog::removeVariableSlot()
{
    int row = variable_list_->currentRow();
    VariableItem* item = (VariableItem*)variable_list_->currentItem();

    if( item->getType() == VariableItem::VAR_IN )
        vars_in_.removeVariable( item->getVariable()->name() );
    else
        vars_out_.removeVariable( item->getVariable()->name() );

    delete variable_list_->takeItem( row );

    //set new active item
    if( row == variable_list_->count() )
        --row;
    variable_list_->setCurrentRow( row );
}

/**
Opens an edit dialog for the selected variable.
  */
void TransformationEditDialog::editVariableSlot()
{
    if( !variable_list_->currentItem() )
        return;
    VariableItem* item = (VariableItem*)variable_list_->currentItem();

    TransformationVariableEditDialog dlg( item->getVariable(), this );
    dlg.exec();
}

/**
Accepts the changes made by in the dialog.
  */
void TransformationEditDialog::okSlot()
{
    trafo_->setAppend( append_box_->isChecked() );

    //asign the stored variables
    trafo_->setInputVariables( vars_in_ );
    trafo_->setOutputVariables( vars_out_ );

    accept();
}

/**
Cancels the changes made by in the dialog.
  */
void TransformationEditDialog::cancelSlot()
{
    reject();
}
