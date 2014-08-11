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
 * AirspaceSectorManagerWidget.h
 *
 *  Created on: Nov 25, 2013
 *      Author: sk
 */

#ifndef AIRSPACESECTORMANAGERWIDGET_H_
#define AIRSPACESECTORMANAGERWIDGET_H_

#include <QWidget>

class AirspaceSectorTreeWidget;
class AirspaceSectorWidget;

class AirspaceSectorManagerWidget : public QWidget
{
    Q_OBJECT
public slots:
    void addNewSector ();
    void addSectorsByShapeFile ();
    void addSectorsByACGXMLFile ();

public:
    AirspaceSectorManagerWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~AirspaceSectorManagerWidget();

protected:
    void createElements ();

    AirspaceSectorTreeWidget *yggdrasil_;
    AirspaceSectorWidget *sector_widget_;
};

#endif /* AIRSPACESECTORMANAGERWIDGET_H_ */
