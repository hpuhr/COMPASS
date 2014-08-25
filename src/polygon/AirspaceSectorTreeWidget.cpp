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

/*
 * AirspaceSectorTreeWidget.cpp
 *
 *  Created on: Nov 25, 2013
 *      Author: sk
 */

#include "AirspaceSectorTreeWidget.h"
#include "AirspaceSectorManager.h"
#include "AirspaceSector.h"

#include <QVBoxLayout>
#include <QListWidget>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QContextMenuEvent>
#include <QToolButton>
#include <QLabel>
#include <QIcon>
#include <QTimer>
#include <QContextMenuEvent>
#include <QFileDialog>


AirspaceSectorTreeWidget::AirspaceSectorTreeWidget(QWidget* parent)
: QTreeWidget( parent )
{
    setMaximumWidth(400);

    this->setDropIndicatorShown( true );
    this->setDragEnabled( true );
    viewport()->setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setSelectionMode( QAbstractItemView::SingleSelection );

    connect( this, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(itemClickedSlot(QTreeWidgetItem*,int)) );

    updateLayerListSlot();
}

AirspaceSectorTreeWidget::~AirspaceSectorTreeWidget()
{

}

void AirspaceSectorTreeWidget::updateLayerListSlot()
{
    clear();
    items_.clear();

    const std::map <std::string, AirspaceSector*> &sectors = AirspaceSectorManager::getInstance().getSectors ();
    std::map <std::string, AirspaceSector*>::const_iterator it;

    for (it = sectors.begin(); it != sectors.end(); it++)
    {
        buildRecursive( it->second, 0);
    }

    //expand all entries
    //expandAll();
}

void AirspaceSectorTreeWidget::buildRecursive( AirspaceSector* parent_sector, QTreeWidgetItem* parent_item )
{
    if (parent_item == 0)
    {
        parent_item = new QTreeWidgetItem;
        //parent_item->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicatorWhenChildless);
        parent_item->setText( 0, parent_sector->getName().c_str());
        parent_item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled  );
        addTopLevelItem( parent_item );

        assert (items_.find (parent_item) == items_.end());

        items_[parent_item] = parent_sector;

    }

    const std::map<std::string, AirspaceSector *> &subsectors = parent_sector->getSubSectors();
    std::map<std::string, AirspaceSector *>::const_iterator it;

    for( it= subsectors.begin(); it != subsectors.end(); ++it )
    {
        AirspaceSector* sector = it->second;

        QTreeWidgetItem* item = new QTreeWidgetItem;
        //item->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicatorWhenChildless);
        item->setFlags( Qt::ItemIsSelectable  | Qt::ItemIsEnabled |
                        Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled );

        parent_item->addChild( item );

        item->setText (0, QString::fromStdString( sector->getName()));

        assert (items_.find (item) == items_.end());

        items_[item] = sector;

        buildRecursive (sector, item);
    }
}

void AirspaceSectorTreeWidget::itemClickedSlot( QTreeWidgetItem* item, int column )
{
    assert (items_.find (item) != items_.end());
    emit showSector (items_[item]);
}

void AirspaceSectorTreeWidget::dropEvent( QDropEvent* event )
{
    loginf << "AirspaceSectorTreeWidget: dropEvent";
    //get item
    QTreeWidgetItem* item = selectedItems().front();
    assert (items_.find( item ) != items_.end());

    AirspaceSector *sector = items_[item];

    QTreeWidgetItem* parent = item->parent();

    Configuration item_config = sector->getConfiguration();

    if( parent )
    {
        loginf << "AirspaceSectorTreeWidget: dropEvent: removing from parent "
        << parent->text(0).toStdString();
        assert (items_.find( parent ) != items_.end());
        AirspaceSector *parent_sector = items_[parent];
        parent_sector->removeSubSector(sector);
    }
    else
    {
        loginf << "AirspaceSectorTreeWidget: dropEvent: removing from parent root";
        AirspaceSectorManager::getInstance().removeSector(sector);
    }

    loginf << "AirspaceSectorTreeWidget: dropEvent: moving" << sector->getName().c_str();

    //do drop
    QTreeWidget::dropEvent( event );

    //parent after drop
    QTreeWidgetItem* parent_after = item->parent();

    if (parent == parent_after)
    {
        updateLayerListSlot ();
        return;
    }

    if( parent_after )
    {
        loginf << "AirspaceSectorTreeWidget: dropEvent: adding to parent "
        << parent_after->text(0).toStdString();

        assert (items_.find( parent_after ) != items_.end());

        AirspaceSector *parent_sector_after = items_[parent_after];
        Configuration &new_item_config = parent_sector_after->addNewSubConfiguration(item_config);
        parent_sector_after->generateSubConfigurable(new_item_config.getClassId(), new_item_config.getInstanceId());
    }
    else
    {
        loginf << "AirspaceSectorTreeWidget: dropEvent: adding to parent root";

        Configuration &new_item_config = AirspaceSectorManager::getInstance().addNewSubConfiguration(item_config);
        AirspaceSectorManager::getInstance().generateSubConfigurable(new_item_config.getClassId(), new_item_config.getInstanceId());
    }

    delete sector;

//    if( parent_after == invisibleRootItem() || parent_after == NULL )
//    {
//        //prevent drop to root tree layer
//        std::cout << "dropEvent(): No drop to root!" << std::endl;
//        updateLayerListSlot( false );
//        return;
//    }

//    bool is_layer_free  = !layer->isOrdered();
//    bool is_parent_top  = parent_after == item_root_ || parent_after == item_free_;
//    bool is_parent_free = parent_after == item_free_;
//    if( !is_parent_free )
//        is_parent_free = ( !is_parent_top && !items_[ parent_after ]->isOrdered() );
//    bool stack_changed = is_layer_free != is_parent_free;
//
//    std::cout << "layer free: " << is_layer_free << std::endl;
//    std::cout << "parent top: " << is_parent_top << std::endl;
//    std::cout << "parent free: " << is_parent_free << std::endl;
//    std::cout << "stack changed: " << stack_changed << std::endl;
//
//    layer->detachFromParent();
//    layer->setParent( NULL );
//    layer->setLayerOrdered( !is_parent_free );
//
//    DOLayer* parent_layer;
//    if( is_parent_top )
//        parent_layer = manager_->getRootLayer();
//    else
//        parent_layer = items_[ parent_after ];
//    int position_after = parent_after->indexOfChild( item );
//
//    manager_->addSubLayer( layer, parent_layer, position_after );
//
//    //update view
//    manager_->updateLayerHierarchySlot();
//    updateSlot();
    updateLayerListSlot ();
}
