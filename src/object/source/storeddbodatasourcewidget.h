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

#ifndef STOREDDBODATASOURCEWIDGET_H
#define STOREDDBODATASOURCEWIDGET_H

#include "storeddbodatasource.h"

#include <QWidget>

class QLineEdit;
class InvalidQLineEdit;

class StoredDBODataSourceWidget : public QWidget
{
    Q_OBJECT

public slots:
    // slots for setting by QLineEdit
    //void changedIdSlot ();
    void changedShortNameColumnSlot (const QString& value_str);
    void changedNameColumnSlot (const QString& value_str);
    void changedSacColumnSlot (const QString& value_str);
    void changedSicColumnSlot (const QString& value_str);
    void changedLatitudeColumnSlot (const QString& value_str);
    void changedLongitudeColumnSlot (const QString& value_str);
    void changedAltitudeColumnSlot (const QString& value_str);

    // slots for updating from ds
    void updateIdSlot();
    void updateShortNameColumnSlot ();
    void updateNameColumnSlot ();
    void updateSacColumnSlot ();
    void updateSicColumnSlot ();
    void updateLatitudeColumnSlot ();
    void updateLongitudeColumnSlot ();
    void updateAltitudeColumnSlot ();

    void deleteSlot ();

public:
    StoredDBODataSourceWidget(StoredDBODataSource& data_source, bool add_headers=false,
                              QWidget* parent=0, Qt::WindowFlags f=0);
    virtual ~StoredDBODataSourceWidget() {}

    void setDataSource (StoredDBODataSource& data_source);

    void update ();

private:
    StoredDBODataSource* data_source_;

    QLineEdit* id_edit_{nullptr};
    QLineEdit* name_edit_{nullptr};
    InvalidQLineEdit* short_name_edit_{nullptr};
    InvalidQLineEdit* sac_edit_{nullptr};
    InvalidQLineEdit* sic_edit_{nullptr};
    InvalidQLineEdit* latitude_edit_{nullptr};
    InvalidQLineEdit* longitude_edit_{nullptr};
    InvalidQLineEdit* altitude_edit_{nullptr};
};

#endif // STOREDDBODATASOURCEWIDGET_H
