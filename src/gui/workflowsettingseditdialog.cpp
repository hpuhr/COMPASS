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


#include "WorkflowSettingsEditDialog.h"
#include "Workflow.h"

#include <QMessageBox>


/**
Constructor.
@param wf Workflow to edit.
@param parent Parent widget.
  */
WorkflowSettingsEditDialog::WorkflowSettingsEditDialog( Workflow* wf, QWidget* parent )
:   QDialog( parent ),
    wf_( wf )
{
    setupUi( this );

    assert( wf_ );

    init();
}

/**
Destructor.
  */
WorkflowSettingsEditDialog::~WorkflowSettingsEditDialog()
{
}

/**
Accepts the changes made by the dialog.
  */
void WorkflowSettingsEditDialog::okSlot()
{
    if( name_edit_->text().isEmpty() )
    {
        QMessageBox::warning( this, "Warning", "Please provide a valid workflow name." );
        return;
    }

    wf_->setName( name_edit_->text().toStdString() );
    accept();
}

/**
Cancels the changes made by the dialog.
  */
void WorkflowSettingsEditDialog::cancelSlot()
{
    reject();
}

/**
Inits the dialog.
  */
void WorkflowSettingsEditDialog::init()
{
    name_edit_->setText( QString::fromStdString( wf_->name() ) );

    connect( ok_button_, SIGNAL(pressed()), this, SLOT(okSlot()) );
    connect( cancel_button_, SIGNAL(pressed()), this, SLOT(cancelSlot()) );
}
