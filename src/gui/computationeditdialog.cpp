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


#include "ComputationEditDialog.h"
#include "Computation.h"
#include "TransformationFactory.h"
#include "DBObjectManager.h"
#include "DBObject.h"
#include "Workflow.h"
#include "TransformationEditDialog.h"
#include "FilterEditDialog.h"

#include <QMessageBox>


/**
@brief Constructor.
@param wf Workflow the edited computation is part of.
@param comp The computation to be edited.
@param parent Parent widget.
  */
ComputationEditDialog::ComputationEditDialog( Workflow* wf, Computation* comp, QWidget* parent )
:   QDialog( parent ),
    comp_( comp ),
    wf_( wf )
{
    setupUi( this );

    assert( comp_ );
    assert( wf_ );

    trafo_list_->setSelectionMode( QListWidget::SingleSelection );

    init();
}

/**
@brief Destructor.
  */
ComputationEditDialog::~ComputationEditDialog()
{
    //delete the temporary transformations
    unsigned int i, n;
    std::map<int,Trafos>::const_iterator it, itend = trafos_.end();
    for( it=trafos_.begin(); it!=itend; ++it )
    {
        const Trafos& trafos = it->second;
        n = trafos.size();
        for( i=0; i<n; ++i )
        {
            delete trafos[ i ];
        }
    }
}

/**
@brief Inits the dialog.
  */
void ComputationEditDialog::init()
{
    common_box_->setChecked( comp_->commonTransformationEnabled() );

    //fill transformation combo box
    {
        const TransformationFactory::Transformations& trafos = TransformationFactory::getInstance().getRegisteredTransformations();
        TransformationFactory::Transformations::const_iterator it, itend = trafos.end();
        for( it=trafos.begin(); it!=itend; ++it )
            trafo_combo_->addItem( QString::fromStdString( it->first ) );
    }

    //fill DBO type switch combo box
    {
        DB_OBJECT_TYPE type;
        std::map <DB_OBJECT_TYPE,DBObject*>& dobs = DBObjectManager::getInstance().getDBObjects();
        std::map <DB_OBJECT_TYPE,DBObject*>::iterator it, itend = dobs.end();
        for( it=dobs.begin(); it!=itend; ++it )
        {
            type = it->second->getType();

            //skip undefined type
            if( type == DBO_UNDEFINED )
                continue;

            dbo_combo_->addItem( QString::fromStdString( DB_OBJECT_TYPE_STRINGS[ type ] ), QVariant( (int)type ) );
        }

        dbo_combo_->addItem( "Common", QVariant( -1 ) );
    }

    //read transformations from the computation and clone them
    {
        unsigned int i, n;
        bool errors = false;
        std::map <DB_OBJECT_TYPE,DBObject*>& dobs = DBObjectManager::getInstance().getDBObjects();
        const Computation::TransformationMap& trafos = comp_->getTransformations();
        Computation::TransformationMap::const_iterator it, itend = trafos.end();
        for( it=trafos.begin(); it!=itend; ++it )
        {
            //DBO type not found, skip and show error later
            if( it->first != -1 && dobs.find( (DB_OBJECT_TYPE)it->first ) == dobs.end() )
            {
                errors = true;
                continue;
            }

            //clone for DBO type
            const Computation::Transformations& trvec = it->second;
            n = trvec.size();
            for( i=0; i<n; ++i )
            {
                trafos_[ it->first ].push_back( trvec[ i ]->getTransformation()->clone() );
            }
        }

        //DBO type related errors
        if( errors )
        {
            QString msg = "Transformations with non-present DBO type have been detected and skipped. By pressing ok these transformations will be deleted.";
            QMessageBox::warning( this, "Warning", msg );
        }
    }

    //update the transformation list widget
    updateList();

    name_edit_->setText( QString::fromStdString( comp_->name() ) );

    connect( trafo_add_button_, SIGNAL(pressed()), this, SLOT(addTrafoSlot()) );
    connect( trafo_edit_button_, SIGNAL(pressed()), this, SLOT(editTrafoSlot()) );
    connect( trafo_remove_button_, SIGNAL(pressed()), this, SLOT(removeTrafoSlot()) );
    connect( trafo_up_button_, SIGNAL(pressed()), this, SLOT(trafoUpSlot()) );
    connect( trafo_down_button_, SIGNAL(pressed()), this, SLOT(trafoDownSlot()) );
    connect( ok_button_, SIGNAL(pressed()), this, SLOT(okSlot()) );
    connect( cancel_button_, SIGNAL(pressed()), this, SLOT(cancelSlot()) );
    connect( dbo_combo_, SIGNAL(currentIndexChanged(int)), this, SLOT(dboTypeChangedSlot(int)) );
    connect( filter_button_, SIGNAL(pressed()), this, SLOT(editFilterSlot()) );
}

/**
@brief Adds a new transformation by using the transformation combos chosen id.
  */
void ComputationEditDialog::addTrafoSlot()
{
    int type = dbo_combo_->itemData( dbo_combo_->currentIndex() ).toInt();
    std::string id = trafo_combo_->currentText().toStdString();

    trafos_[ type ].push_back( TransformationFactory::getInstance().createTransformation( id ) );
    updateList();
}

/**
@brief Moves a transformation up in the list.
  */
void ComputationEditDialog::trafoUpSlot()
{
    if( trafo_list_->currentItem() == NULL )
        return;

    int type = dbo_combo_->itemData( dbo_combo_->currentIndex() ).toInt();
    int idx = trafo_list_->currentIndex().row();

    if( idx == 0 )
        return;

    Trafos& trafos = trafos_[ type ];

    Transformation* trafo = trafos[ idx ];
    trafos.erase( trafos.begin()+idx );
    trafos.insert( trafos.begin()+idx-1, trafo );

    updateList();

    trafo_list_->setCurrentRow( idx-1 );
}

/**
@brief Moves a transformation down in the list.
  */
void ComputationEditDialog::trafoDownSlot()
{
    if( trafo_list_->currentItem() == NULL )
        return;

    int type = dbo_combo_->itemData( dbo_combo_->currentIndex() ).toInt();
    int idx = trafo_list_->currentIndex().row();

    if( idx == trafo_list_->count()-1 )
        return;

    Trafos& trafos = trafos_[ type ];

    Transformation* trafo = trafos[ idx ];
    trafos.erase( trafos.begin()+idx );
    trafos.insert( trafos.begin()+idx+1, trafo );

    updateList();

    trafo_list_->setCurrentRow( idx+1 );
}

/**
@brief Edit the selected transformation.
  */
void ComputationEditDialog::editTrafoSlot()
{
    //nothing selected
    if( trafo_list_->currentItem() == NULL )
        return;

    int type = dbo_combo_->itemData( dbo_combo_->currentIndex() ).toInt();
    Transformation* trafo = trafos_[ type ].at( trafo_list_->currentIndex().row() );

    TransformationEditDialog dlg( trafo, true, this );
    dlg.exec();
}

/**
@brief Removes the selected transformation.
  */
void ComputationEditDialog::removeTrafoSlot()
{
    //nothing selected
    if( trafo_list_->currentItem() == NULL )
        return;

    int type = dbo_combo_->itemData( dbo_combo_->currentIndex() ).toInt();
    int idx = trafo_list_->currentIndex().row();

    Transformation* trafo = trafos_[ type ][ idx ];

    trafos_[ type ].erase( trafos_[ type ].begin()+idx );
    delete trafo;

    updateList();

    if( idx == trafo_list_->count() )
        --idx;

    trafo_list_->setCurrentRow( idx );
}

/**
@brief Shows the transformations for the given DBO type in the list.
@param id DBO type as integer number. If -1 the common transformations will be shown.
  */
void ComputationEditDialog::showTrafos( int id )
{
    trafo_list_->clear();

    if( trafos_.find( id ) == trafos_.end() || trafos_[ id ].empty() )
        return;

    const Trafos& trafos = trafos_[ id ];
    unsigned int i, n = trafos.size();
    for( i=0; i<n; ++i )
    {
        trafo_list_->addItem( QString::fromStdString( trafos[ i ]->getId() ) );
    }
}

/**
@brief Updates the transformation list.
  */
void ComputationEditDialog::updateList()
{
    //get current DBO type
    int type = dbo_combo_->itemData( dbo_combo_->currentIndex() ).toInt();

    //show transformations for type
    showTrafos( type );
}

/**
@brief Accepts the changes made in the dialog.
  */
void ComputationEditDialog::okSlot()
{
    //invalid name
    if( name_edit_->text().isEmpty() )
    {
        QMessageBox::warning( this, "Warning", "Please enter a valid computation name." );
        return;
    }

    //set new name
    wf_->renameComputation( comp_->name(), name_edit_->text().toStdString() );

    comp_->enableCommonTransformation( common_box_->isChecked() );

    //delete old transformations
    comp_->deleteTransformations();

    //add new transformations
    unsigned int i, n;
    std::map<int,Trafos>::const_iterator it, itend = trafos_.end();
    for( it=trafos_.begin(); it!=itend; ++it )
    {
        const Trafos& trafos = it->second;
        n = trafos.size();
        for( i=0; i<n; ++i )
        {
            if( it->first == -1 )
                comp_->addCommonTransformation( trafos[ i ] );
            else
                comp_->addTransformation( (DB_OBJECT_TYPE)it->first, trafos[ i ] );
        }
    }

    accept();
}

/**
@brief Cancels the changes made in the dialog.
  */
void ComputationEditDialog::cancelSlot()
{
    reject();
}

/**
@brief Triggered if the DBO type combo has changed.
  */
void ComputationEditDialog::dboTypeChangedSlot( int idx )
{
    updateList();
}

/**
@brief Opens an edit dialog for the computations input buffer filter.

Note that changes made to this dialog are immediately applied to the computation.
  */
void ComputationEditDialog::editFilterSlot()
{
    /// @todo Apply changes to a temporary filter
    FilterEditDialog dlg( comp_->getFilter(), this );
    dlg.exec();
}
