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
 * AirspaceSectorTreeWidget.h
 *
 *  Created on: Nov 25, 2013
 *      Author: sk
 */

#ifndef AIRSPACESECTORTREEWIDGET_H_
#define AIRSPACESECTORTREEWIDGET_H_

#include <QTreeWidget>

class AirspaceSector;

class AirspaceSectorTreeWidget: public QTreeWidget
{
    Q_OBJECT
public slots:
    void updateLayerListSlot ();
//    void updateSlot();
//    void deleteSlot();
//    void addSlot();
//    void addFileSlot();
//    void stackSlot();
//    void deltaSlot( bool checked );
    void itemClickedSlot( QTreeWidgetItem* item, int column );
//    void itemChangedSlot( QTreeWidgetItem* item, int column );
//
protected:
//    virtual void contextMenuEvent( QContextMenuEvent * event );
    virtual void dropEvent( QDropEvent* event );

signals:
    void showSector (AirspaceSector *sector);

public:
    AirspaceSectorTreeWidget(QWidget* parent = 0);
    virtual ~AirspaceSectorTreeWidget();

protected:
//    virtual void contextMenuEvent( QContextMenuEvent * event );
//    virtual void dropEvent( QDropEvent* event );

    void buildRecursive( AirspaceSector* parent_sector, QTreeWidgetItem* parent_item);
//    void checkRecursive( QTreeWidgetItem* parent_item, bool check );

//    DOLayerManager* manager_;
//    GeographicView* view_;
//    DOLayer* sel_;
    std::map<QTreeWidgetItem*, AirspaceSector*> items_;
};

#endif /* AIRSPACESECTORTREEWIDGET_H_ */
