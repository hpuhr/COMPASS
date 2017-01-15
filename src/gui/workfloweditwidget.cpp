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


#include "WorkflowEditWidget.h"
#include "Workflow.h"
#include "ComputationElement.h"
#include "Transformation.h"
#include "BufferFilter.h"
#include "FilterEditDialog.h"
#include "ComputationEditDialog.h"
#include "TransformationEditDialog.h"
#include "WorkflowSettingsEditDialog.h"

#include <stdexcept>

#include <QIcon>
#include <QFont>
#include <QContextMenuEvent>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QVariant>
#include <QInputDialog>

Q_DECLARE_METATYPE( QTreeWidgetItem* )


/*********************************************************************************************
WorkflowEditWidgetItem
**********************************************************************************************/

/**
Constructor.
@param type Item type.
  */
WorkflowEditWidgetItem::WorkflowEditWidgetItem( QTreeWidgetItem::ItemType type )
:   QTreeWidgetItem( type )
{
}

/**
Destructor.
  */
WorkflowEditWidgetItem::~WorkflowEditWidgetItem()
{
}

/*********************************************************************************************
WorkflowTreeItem
**********************************************************************************************/

/**
Constructor.
@param workflow Workflow to be represented by the item.
  */
WorkflowTreeItem::WorkflowTreeItem( Workflow* workflow )
:   WorkflowEditWidgetItem( (QTreeWidgetItem::ItemType)WorkflowEditWidgetItem::TypeWorkflow ),
    workflow_( workflow )
{
    assert( workflow_ );
    setText( 0, QString::fromStdString( workflow_->name() ) );
    setFont( 0, QFont( "" , 10 , QFont::Bold ) );
    setIcon( 0, QIcon( "Data/icons/workflow.png" ) );
}

/**
Destructor.
  */
WorkflowTreeItem::~WorkflowTreeItem()
{
}

/*********************************************************************************************
ComputationElementTreeItem
**********************************************************************************************/

/**
Constructor.
@param element Computation element to be represented by the item.
@param wf The workflow the element is part of.
  */
ComputationElementTreeItem::ComputationElementTreeItem( ComputationElement* element, Workflow* wf )
:   WorkflowEditWidgetItem( (QTreeWidgetItem::ItemType)WorkflowEditWidgetItem::TypeComputationElement ),
    element_( element ),
    wf_( wf )
{
    assert( element_ );
    assert( wf_ );

    setText( 0, QString::fromStdString( element_->name() ) );
    setFont( 0, QFont( "" , 10 , QFont::Normal ) );
    setIcon( 0, QIcon( "Data/icons/computation.png" ) );
}

/**
  */
ComputationElementTreeItem::~ComputationElementTreeItem()
{
}

/*********************************************************************************************
TransformationTreeItem
**********************************************************************************************/

/**
Constructor.
@param trafo Transformation to be represented by the item.
@param dbo_type DBO type that is assigned to the transformation.
@param elem ComputationElement the transformation is part of.
  */
TransformationTreeItem::TransformationTreeItem( Transformation* trafo, int dbo_type, ComputationElement* elem )
:   WorkflowEditWidgetItem( (QTreeWidgetItem::ItemType)WorkflowEditWidgetItem::TypeTransformation ),
    dbo_type_( dbo_type ),
    elem_( elem ),
    trafo_( trafo )
{
    assert( elem_ );
    assert( trafo_ );

    updateText();
    setIcon( 0, QIcon( "Data/icons/trafo.png" ) );
}

/**
Destructor.
  */
TransformationTreeItem::~TransformationTreeItem()
{
}

/**
Updates the items text.
  */
void TransformationTreeItem::updateText()
{
    QString type;
    switch( dbo_type_ )
    {
        case DBO_PLOTS:
            type = "(Plots)";
            break;
        case DBO_SYSTEM_TRACKS:
            type = "(SystemTracks)";
            break;
        case DBO_ADS_B:
            type = "(ADSB)";
            break;
        case DBO_MLAT:
            type = "(MLAT)";
            break;
        case DBO_REFERENCE_TRAJECTORIES:
            type = "(RefTraj)";
            break;
        case DBO_SENSOR_INFORMATION:
            type = "(Sensor)";
            break;
        default:
            type = "(Common)";
            break;
    }

    setText( 0, type + " " + QString::fromStdString( trafo_->getId() ) );
    setFont( 0, QFont( "" , 9 , QFont::Normal ) );
}

/*********************************************************************************************
BufferFilterTreeItem
**********************************************************************************************/

/**
Constructor.
@param filter Buffer filter to be represented by the item.
  */
BufferFilterTreeItem::BufferFilterTreeItem( BufferFilter* filter )
:   WorkflowEditWidgetItem( (QTreeWidgetItem::ItemType)WorkflowEditWidgetItem::TypeFilter ),
    filter_( filter )
{
    assert( filter_ );
    setText( 0, "InputFilter" );
    setFont( 0, QFont( "" , 9 , QFont::Normal ) );
    setIcon( 0, QIcon( "Data/icons/filter.png" ) );
}

/**
Destructor.
  */
BufferFilterTreeItem::~BufferFilterTreeItem()
{
}

/*********************************************************************************************
WorkflowEditWidget
**********************************************************************************************/

/**
Constructor.
@param parent Parent widget.
  */
WorkflowEditWidget::WorkflowEditWidget( QWidget* parent )
:   QTreeWidget( parent )
{
}

/**
Destructor.
  */
WorkflowEditWidget::~WorkflowEditWidget()
{
}

/**
Adds a new workflow to the widget.
@param wf Workflow to be added.
  */
void WorkflowEditWidget::addWorkflow( Workflow* wf )
{
    workflows_.push_back( wf );
}

/**
Inserts a new workflow into the widget.
@param wf Workflow to be inserted.
@param idx Index the workflow shall be inserted at.
  */
void WorkflowEditWidget::insertWorkflow( Workflow* wf, int idx )
{
    if( idx < 0 || idx >= (int)workflows_.size() )
        throw std::runtime_error( "WorkflowEditWidget: insertWorkflow: Index out of range." );
    workflows_.insert( workflows_.begin()+idx, wf );
}

/**
Removes a workflow from the widget.
@param idx Index of the workflow to be removed.
  */
void WorkflowEditWidget::removeWorkflow( int idx )
{
    if( idx < 0 || idx >= (int)workflows_.size() )
        throw std::runtime_error( "WorkflowEditWidget: insertWorkflow: Index out of range." );
    workflows_.erase( workflows_.begin()+idx );
}

/**
Returns the index position of the given workflow in the tree.
@return Index of the given workflow, returns -1 if not found.
  */
int WorkflowEditWidget::findWorkflow( Workflow* wf ) const
{
    int i, n = (int)workflows_.size();
    for( i=0; i<n; ++i )
    {
        if( workflows_[ i ] == wf )
            return i;
    }
    return -1;
}

/**
Returns the number of workflows added to the widget.
@return Number of added workflows.
  */
int WorkflowEditWidget::numberOfWorkflows() const
{
    return workflows_.size();
}

/**
Updates the tree and displays the added workflows.
  */
void WorkflowEditWidget::updateTree()
{
    clear();
    comp_items_.clear();

    //for each workflow...
    int i, n = (int)workflows_.size();
    for( i=0; i<n; ++i )
    {
        WorkflowTreeItem* item = new WorkflowTreeItem( workflows_[ i ] );
        addTopLevelItem( item );

        //...add the computations recursively
        addComputationsRecursive( item, workflows_[ i ]->getRoot(), workflows_[ i ] );
    }

    //expand tree
    expandAll();
}

/**
Recursive method to build the trees entries.
@param parent Parent tree widget item.
@param elem Computation to be added to the tree.
@param wf Workflow that is currently added.
  */
void WorkflowEditWidget::addComputationsRecursive( QTreeWidgetItem* parent, ComputationElement* elem, Workflow* wf )
{
    assert( parent );
    assert( elem );

    //add computation
    ComputationElementTreeItem* item = new ComputationElementTreeItem( elem, wf );
    parent->addChild( item );

    //used to associate views/generators in a derived class
    comp_items_[ item->getElement()->name() ] = item;

    //add filter item
    //BufferFilterTreeItem* filter_item = new BufferFilterTreeItem( elem->getFilter() );
    //item->addChild( filter_item );

    //add transformations
    unsigned int i, n;
    const Computation::TransformationMap& trafos = elem->getTransformations();
    Computation::TransformationMap::const_iterator it, itend = trafos.end();
    for( it=trafos.begin(); it!=itend; ++it )
    {
        const Computation::Transformations& type_trafos = it->second;
        n = type_trafos.size();
        for( i=0; i<n; ++i )
        {
            TransformationTreeItem* trafo_item = new TransformationTreeItem( type_trafos[ i ]->getTransformation(), it->first, elem );
            item->addChild( trafo_item );
        }
    }

    //add child computations
    n = elem->numChildren();
    for( i=0; i<n; ++i )
        addComputationsRecursive( item, elem->getChild( i ), wf );
}

/**
Shows a context menu, depending on the click position.
@param event Context menu event.
  */
void WorkflowEditWidget::contextMenuEvent ( QContextMenuEvent* event )
{
    //get the item
    WorkflowEditWidgetItem* item = (WorkflowEditWidgetItem*)(itemAt( event->pos() ));

    //no item clicked
    if( !item )
    {
        //currently not used
        //contextMenuMain();
        return;
    }

    //open correct context menu
    int type = item->type();
    switch( type )
    {
        case WorkflowEditWidgetItem::TypeWorkflow:
        {
            contextMenuWorkflow( item );
            break;
        }
        case WorkflowEditWidgetItem::TypeComputationElement:
        {
            contextMenuComputation( item );
            break;
        }
        case WorkflowEditWidgetItem::TypeTransformation:
        {
            contextMenuTransformation( item );
            break;
        }
        case WorkflowEditWidgetItem::TypeFilter:
        {
            contextMenuFilter( item );
            break;
        }
        default:
            break;
    }
}

/**
Shows the workflow context menu.
@param item The clicked workflow item.
  */
void WorkflowEditWidget::contextMenuWorkflow( QTreeWidgetItem* item )
{
    QVariant var = encode( item );

    QMenu menu;
    QAction* action_edit = menu.addAction( "Edit", this, SLOT(editWorkflowSlot()) );
    action_edit->setData( var );

    menu.exec( QCursor::pos() );
}

/**
Used to build the computation menu. Can be overrided in a derived class to add more menu items.
@param menu Menu to add the new entries to.
@param item_data The encoded tree item.
  */
void WorkflowEditWidget::getComputationMenu( QMenu& menu, const QVariant& item_data )
{
    QAction* action_edit = menu.addAction( "Edit", this, SLOT(editComputationSlot()) );
    QAction* action_child_b = menu.addAction( "Add Computation Before", this, SLOT(addParentComputationSlot()) );
    QAction* action_child = menu.addAction( "Add Computation After", this, SLOT(addChildComputationSlot()) );
    QAction* action_delete = menu.addAction( "Delete", this, SLOT(deleteComputationSlot()) );

    action_edit->setData( item_data );
    action_child_b->setData( item_data );
    action_child->setData( item_data );
    action_delete->setData( item_data );
}

/**
Shows the computation context menu.
@param item The clicked computation item.
  */
void WorkflowEditWidget::contextMenuComputation( QTreeWidgetItem* item )
{
    QVariant var = encode( item );
    QMenu menu;
    getComputationMenu( menu, var );
    menu.exec( QCursor::pos() );
}

/**
Shows the transformation context menu.
@param item The clicked transformation item.
  */
void WorkflowEditWidget::contextMenuTransformation( QTreeWidgetItem* item )
{
    QVariant var = encode( item );

    QMenu menu;
    QAction* action_edit = menu.addAction( "Edit", this, SLOT(editTransformationSlot()) );
    action_edit->setData( var );

    menu.exec( QCursor::pos() );
}

/**
Shows the buffer filter context menu.
@param item The clicked buffer filter item.
  */
void WorkflowEditWidget::contextMenuFilter( QTreeWidgetItem* item )
{
    QVariant var = encode( item );

    QMenu menu;
    QAction* action_edit = menu.addAction( "Edit", this, SLOT(editFilterSlot()) );
    action_edit->setData( var );

    menu.exec( QCursor::pos() );
}

/**
Shows the main context menu. Not used at the moment.
  */
void WorkflowEditWidget::contextMenuMain()
{
}

/**
Encodes a tree widget item into a variant.
@param item Tree item to encode.
@return Encoded item.
  */
QVariant WorkflowEditWidget::encode( QTreeWidgetItem* item ) const
{
    return QVariant::fromValue<QTreeWidgetItem*>( item );
}

/**
Retrieves a tree widget item from a variant.
@param var Variant to decode.
@return Decoded item.
  */
QTreeWidgetItem* WorkflowEditWidget::decode( const QVariant& var ) const
{
    return var.value<QTreeWidgetItem*>();
}

/**
Starts an edit dialog for the clicked workflow.
  */
void WorkflowEditWidget::editWorkflowSlot()
{
    QAction* action = (QAction*)(QObject::sender());
    WorkflowTreeItem* item = (WorkflowTreeItem*)decode( action->data() );

    WorkflowSettingsEditDialog dlg( item->getWorkflow(), this );
    dlg.exec();

    updateTree();
}

/**
Starts an edit dialog for the clicked computation.
  */
void WorkflowEditWidget::editComputationSlot()
{
    QAction* action = (QAction*)(QObject::sender());
    ComputationElementTreeItem* item = (ComputationElementTreeItem*)decode( action->data() );

    ComputationEditDialog dlg( item->getWorkflow(), item->getElement(), this );
    dlg.exec();

    updateTree();
}

/**
Adds a new computation between the clicked computation item and its parent.
  */
void WorkflowEditWidget::addParentComputationSlot()
{
    QAction* action = (QAction*)(QObject::sender());
    ComputationElementTreeItem* item = (ComputationElementTreeItem*)decode( action->data() );
    Workflow* wf = item->getWorkflow();
    ComputationElement* elem = item->getElement();

    if( !elem->parent() )
    {
        QMessageBox::warning( this, "Warning", "Cannot add parent to root computation." );
        return;
    }

    QString name = QInputDialog::getText( this, "New Computation", "Computation Name:" );
    if( wf->hasComputation( name.toStdString() ) )
    {
        QMessageBox::warning( this, "Warning", "Computation name already present." );
        return;
    }

    wf->addComputationBefore( elem, name.toStdString() );
    updateTree();
}

/**
Adds a new computation as the child of the clicked computation item.
  */
void WorkflowEditWidget::addChildComputationSlot()
{
    QAction* action = (QAction*)(QObject::sender());
    ComputationElementTreeItem* item = (ComputationElementTreeItem*)decode( action->data() );
    Workflow* wf = item->getWorkflow();
    ComputationElement* elem = item->getElement();

    QString name = QInputDialog::getText( this, "New Computation", "Computation Name:" );
    if( wf->hasComputation( name.toStdString() ) )
    {
        QMessageBox::warning( this, "Warning", "Computation name already present." );
        return;
    }

    wf->addComputation( elem, name.toStdString() );
    updateTree();
}

/**
Deletes the clicked computation item.
  */
void WorkflowEditWidget::deleteComputationSlot()
{
    QAction* action = (QAction*)(QObject::sender());
    ComputationElementTreeItem* item = (ComputationElementTreeItem*)decode( action->data() );
    Workflow* wf = item->getWorkflow();
    ComputationElement* elem = item->getElement();

    wf->deleteComputation( elem->name() );
    updateTree();
}

/**
Starts an edit dialog for the clicked transformation.
  */
void WorkflowEditWidget::editTransformationSlot()
{
    QAction* action = (QAction*)(QObject::sender());
    TransformationTreeItem* item = (TransformationTreeItem*)decode( action->data() );

    TransformationEditDialog dlg( item->getTransformation(), this );
    dlg.exec();
}

/**
Starts an edit dialog for the clicked buffer filter.
  */
void WorkflowEditWidget::editFilterSlot()
{
    QAction* action = (QAction*)(QObject::sender());
    BufferFilterTreeItem* item = (BufferFilterTreeItem*)decode( action->data() );

    FilterEditDialog dlg( item->getBufferFilter(), this );
    dlg.exec();
}
