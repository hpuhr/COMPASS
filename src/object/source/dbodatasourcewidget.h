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

#ifndef DBODATASOURCEWIDGET_H
#define DBODATASOURCEWIDGET_H

#include "dbodatasource.h"

#include <QWidget>

class QLineEdit;
class QGridLayout;
class InvalidQLineEdit;

class DBODataSourceWidget : public QWidget
{
    Q_OBJECT

public slots:
    // slots for setting by QLineEdit
    //void changedIdSlot ();
    void changedShortNameColumnSlot ();
    void changedNameColumnSlot ();
    void changedSacColumnSlot ();
    void changedSicColumnSlot ();
    void changedLatitudeColumnSlot ();
    void changedLongitudeColumnSlot ();
    void changedAltitudeColumnSlot ();

    // slots for updating from ds
    void updateIdSlot();
    void updateShortNameColumnSlot ();
    void updateNameColumnSlot ();
    void updateSacColumnSlot ();
    void updateSicColumnSlot ();
    void updateLatitudeColumnSlot ();
    void updateLongitudeColumnSlot ();
    void updateAltitudeColumnSlot ();

public:
    DBODataSourceWidget(DBODataSource& data_source, bool add_headers=false, QWidget* parent=0, Qt::WindowFlags f=0);
    virtual ~DBODataSourceWidget() {}

    void setDataSource (DBODataSource& data_source);

    void update ();

private:
    DBODataSource* data_source_;

    QLineEdit* id_edit_{nullptr};
    QLineEdit* name_edit_{nullptr};
    InvalidQLineEdit* short_name_edit_{nullptr};
    InvalidQLineEdit* sac_edit_{nullptr};
    InvalidQLineEdit* sic_edit_{nullptr};
    InvalidQLineEdit* latitude_edit_{nullptr};
    InvalidQLineEdit* longitude_edit_{nullptr};
    InvalidQLineEdit* altitude_edit_{nullptr};
};

#endif // DBODATASOURCEWIDGET_H
