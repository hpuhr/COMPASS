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


#ifndef WORKFLOWSETTINGSEDITDIALOG_H
#define WORKFLOWSETTINGSEDITDIALOG_H

#include "ui_WorkflowSettingsEditDialogBase.h"

class Workflow;


/**
@brief Edits a workflows settings.

Used to edit a workflows settings, like its name f.e.
  */
class WorkflowSettingsEditDialog : public QDialog, private Ui::WorkflowSettingsEditDialogBase
{
    Q_OBJECT
public:
    /// @brief Constructor
    WorkflowSettingsEditDialog( Workflow* wf, QWidget* parent=NULL );
    /// @brief Destructor
    virtual ~WorkflowSettingsEditDialog();

protected slots:
    /// @brief Accepts the changes
    void okSlot();
    /// @brief Cancels the changes
    void cancelSlot();

protected:
    /// @brief Inits the dialog
    void init();

    /// Workflow to edit
    Workflow* wf_;
};

#endif //WORKFLOWSETTINGSEDITDIALOG_H
