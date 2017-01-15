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


#include "WorkflowEditDialog.h"
#include "WorkflowEditWidget.h"
#include "Workflow.h"

#include <QVBoxLayout>


/**
Constructor.
@param parent Parent widget.
  */
WorkflowEditDialog::WorkflowEditDialog( QWidget* parent )
:   QDialog( parent )
{
    QVBoxLayout* layout = new QVBoxLayout;
    setLayout( layout );

    wf_edit_ = new WorkflowEditWidget( this );
    layout->addWidget( wf_edit_ );

    setWindowTitle( "Workflow Edit" );
    setFixedSize( 800, 600 );
}

/**
Destructor.
  */
WorkflowEditDialog::~WorkflowEditDialog()
{
}

/**
Adds a new workflow.
@param wf Workflow to be added to the widget.
  */
void WorkflowEditDialog::addWorkflow( Workflow* wf )
{
    wf_edit_->addWorkflow( wf );
    wf_edit_->updateTree();
}
