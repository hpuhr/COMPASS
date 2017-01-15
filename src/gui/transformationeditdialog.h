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


#ifndef TRANSFORMATIONEDITDIALOG_H
#define TRANSFORMATIONEDITDIALOG_H

#include "TransformationVariable.h"
#include "Transformation.h"

#include <QDialog>
#include <QListWidget>

#include "ui_TransformationEditDialogBase.h"


/**
@brief A list widget item for a transformation variable entry.

Used to display a transformation variable in a TransformationEditDialog's variable list.
  */
class VariableItem : public QListWidgetItem
{
public:
    /// The type of transformation variable, either input variable or output variable
    enum VariableType { VAR_IN=0, VAR_OUT };

    /// @brief Constructor
    VariableItem( TransformationVariable* var, VariableType type );
    /// @brief Destructor
    virtual ~VariableItem();

    /// @brief Returns the represented transformation variable
    TransformationVariable* getVariable();
    /// @brief Returns the variable type
    VariableType getType() const;

private:
    /// The represented transformation variable
    TransformationVariable* var_;
    /// The variable type
    VariableType type_;
};

/**
@brief Dialog to edit a Transformation.

This dialog can be used to edit a transformations internals, e.g. its transformation variables.
Existing variables can be edited in any case, by clicking on them and pressing the Edit-button.
Adding and deleting variables must be enebled by passing 'with_extras' as TRUE to the constructor.
Since transformations mostly rely on the variables they define internally, it is not advised to
activate this feature at the moment.

Hidden transformation variables cannot be edited by the dialog.
  */
class TransformationEditDialog : public QDialog, private Ui::TransformationEditDialogBase
{
    Q_OBJECT
public:
    /// @brief Constructor
    TransformationEditDialog( Transformation* trafo, bool with_extras, QWidget* parent=NULL );
    /// @brief Destructor
    virtual ~TransformationEditDialog();

protected slots:
    /// @brief Adds a new transformation variable
    void addVariableSlot();
    /// @brief Removes the selected transformation variable
    void removeVariableSlot();
    /// @brief Opens an edit dialog for the selected variable
    void editVariableSlot();
    /// @brief Accepts the changes made by in the dialog
    void okSlot();
    /// @brief Cancels the changes made by in the dialog
    void cancelSlot();

protected:
    /// @brief Inits the dialog
    void init();

    /// The transformation to edit
    Transformation* trafo_;
    /// Enables/disables extra GUI elements
    bool with_extras_;

    /// Temporary input variables
    TransformationVariables vars_in_;
    /// Temporary output variables
    TransformationVariables vars_out_;
};

#endif //TRANSFORMATIONEDITDIALOG_H
