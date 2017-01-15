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
 * AirspaceSectorWidget.h
 *
 *  Created on: Nov 25, 2013
 *      Author: sk
 */

#ifndef AIRSPACESECTORWIDGET_H_
#define AIRSPACESECTORWIDGET_H_

#include <QWidget>


class AirspaceSector;
class QLineEdit;
class QTableWidget;
class QPushButton;
class QCheckBox;

class AirspaceSectorWidget : public QWidget
{
    Q_OBJECT
public slots:
    void showSector (AirspaceSector *sector);
    void deleteSector ();
    void setName ();
    void setHasOwnVolume ();
    void setHeightMin();
    void setHeightMax ();
    void setUseForChecking ();
    void addPoints ();
    void clearPoints ();
    void copyPoints ();

signals:
    void sectorChanged ();

public:
    AirspaceSectorWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~AirspaceSectorWidget();

    void clearContent ();
    void disableWidgets ();
    void enableWidgets ();

protected:
    AirspaceSector *current_;

    QLineEdit *name_edit_;
    QCheckBox *has_own_volume_;
    QLineEdit *height_min_edit_;
    QLineEdit *height_max_edit_;
    QCheckBox *used_for_checking_;

    QPushButton *delete_button_;
    QPushButton *clear_button_;
    QPushButton *add_button_;
    QPushButton *copy_button_;

    QTableWidget *table_;

    void createElements ();

    void updatePointsTable ();

};

#endif /* AIRSPACESECTORWIDGET_H_ */
