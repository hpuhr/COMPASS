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


#ifndef WORKFLOWEDITDIALOG_H
#define WORKFLOWEDITDIALOG_H

#include <QDialog>

class WorkflowEditWidget;
class Workflow;


/**
@brief Encapsulates a WorkflowEditWidget into a dialog.

Encapsulates a WorkflowEditWidget into a dialog.
  */
class WorkflowEditDialog : public QDialog
{
public:
    /// @brief Constructor
    WorkflowEditDialog( QWidget* parent=NULL );
    /// @brief Destructor
    virtual ~WorkflowEditDialog();

    /// @brief Adds a new workflow
    void addWorkflow( Workflow* wf );

private:
    /// Edit widget
    WorkflowEditWidget* wf_edit_;
};

#endif //WORKFLOWEDITDIALOG_H
