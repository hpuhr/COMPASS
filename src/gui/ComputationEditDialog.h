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


#ifndef COMPUTATIONEDITDIALOG_H
#define COMPUTATIONEDITDIALOG_H

#include "ui_ComputationEditDialogBase.h"

#include <QDialog>

class Computation;
class Transformation;
class Workflow;


/**
@brief Dialog to edit a Computation.

This dialog can be used to edit some properties of a Computation. The most important
thing to edit are the computations transformations, which can be added, deleted, moved
and edited here on a per-DBO-type basis.

One can also edit the name of the Computation and the computations input buffer filter.

The changes made are only applied when pressing the Ok-button, the exception beeing the
buffer filter, which has its own edit dialog. The changes made in this dialog will be
applied immediately.

@todo Provide a copy constructor for the BufferFilter class and do not directly edit the
computations filter in the filter edit dialog. Edit a temporary filter instead and assign
the result to the computation when accepting the dialog.
  */
class ComputationEditDialog : public QDialog, private Ui::ComputationEditDialogBase
{
    Q_OBJECT
public:
    typedef std::vector<Transformation*> Trafos;

    ComputationEditDialog( Workflow* wf, Computation* comp, QWidget* parent=NULL );
    virtual ~ComputationEditDialog();

protected slots:
    void addTrafoSlot();
    void trafoUpSlot();
    void trafoDownSlot();
    void editTrafoSlot();
    void removeTrafoSlot();
    void editFilterSlot();
    void okSlot();
    void cancelSlot();
    void dboTypeChangedSlot( int idx );

protected:
    void init();
    void showTrafos( int id );
    void updateList();

    Computation* comp_;
    Workflow* wf_;

    std::map<int,Trafos> trafos_;
};

#endif //COMPUTATIONEDITDIALOG_H
