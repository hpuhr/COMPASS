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

#include "dbodatasourcewidget.h"
#include "invalidqlineedit.h"
#include "logger.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

DBODataSourceWidget::DBODataSourceWidget(DBODataSource& data_source, bool add_headers,
                                         QWidget* parent, Qt::WindowFlags f)
    : QWidget (parent, f), data_source_(&data_source)
{
    logdbg << "DBODataSourceWidget: ctor: " << data_source.name();

    QGridLayout* main_layout = new QGridLayout ();

    unsigned int row = 0;
    unsigned int col = 0;

    if (add_headers)
    {
        main_layout->addWidget (new QLabel ("ID"), row, col++);
        main_layout->addWidget (new QLabel ("Name"), row, col++);
        main_layout->addWidget (new QLabel ("Short Name"), row, col++);
        main_layout->addWidget (new QLabel ("SAC"), row, col++);
        main_layout->addWidget (new QLabel ("SIC"), row, col++);
        main_layout->addWidget (new QLabel ("Latitude"), row, col++);
        main_layout->addWidget (new QLabel ("Longitude"), row, col++);
        main_layout->addWidget (new QLabel ("Altitude"), row, col++);
        ++row;
        col = 0;
    }

    id_edit_ = new QLineEdit ();
    id_edit_->setReadOnly(true);
    main_layout->addWidget (id_edit_, row, col++);

    name_edit_ = new InvalidQLineEdit ();
    connect(name_edit_, &QLineEdit::textEdited, this, &DBODataSourceWidget::changedNameColumnSlot);
    main_layout->addWidget (name_edit_, row, col++);

    short_name_edit_ = new InvalidQLineEdit ();
    connect(short_name_edit_, &QLineEdit::textEdited,
            this, &DBODataSourceWidget::changedShortNameColumnSlot);
    main_layout->addWidget (short_name_edit_, row, col++);

    sac_edit_ = new InvalidQLineEdit ();
    connect(sac_edit_, &InvalidQLineEdit::textEdited, this, &DBODataSourceWidget::changedSacColumnSlot);
    main_layout->addWidget (sac_edit_, row, col++);


    sic_edit_ = new InvalidQLineEdit ();
    connect(sic_edit_, &InvalidQLineEdit::textEdited, this, &DBODataSourceWidget::changedSicColumnSlot);
    main_layout->addWidget (sic_edit_, row, col++);

    latitude_edit_ = new InvalidQLineEdit ();
    connect(latitude_edit_, &InvalidQLineEdit::textEdited,
            this, &DBODataSourceWidget::changedLatitudeColumnSlot);
    main_layout->addWidget (latitude_edit_, row, col++);


    longitude_edit_ = new InvalidQLineEdit ();
    connect(longitude_edit_, &InvalidQLineEdit::textEdited,
            this, &DBODataSourceWidget::changedLongitudeColumnSlot);
    main_layout->addWidget (longitude_edit_, row, col++);


    altitude_edit_ = new InvalidQLineEdit ();
    connect(altitude_edit_, &InvalidQLineEdit::textEdited,
            this, &DBODataSourceWidget::changedAltitudeColumnSlot);
    main_layout->addWidget (altitude_edit_, row, col++);

    update();

    setLayout(main_layout);
}

void DBODataSourceWidget::setDataSource (DBODataSource& data_source)
{
    data_source_ = &data_source;
    update();
}

void DBODataSourceWidget::update ()
{
    updateIdSlot();
    updateShortNameColumnSlot ();
    updateNameColumnSlot ();
    updateSacColumnSlot ();
    updateSicColumnSlot ();
    updateLatitudeColumnSlot ();
    updateLongitudeColumnSlot ();
    updateAltitudeColumnSlot ();
}

void DBODataSourceWidget::changedShortNameColumnSlot ()
{
    std::string value = short_name_edit_->text().toStdString();
    short_name_edit_->setValid(value.size());

    data_source_->shortName(value);
    data_source_->updateInDatabase();

}
void DBODataSourceWidget::changedNameColumnSlot ()
{
    std::string value = name_edit_->text().toStdString();
    name_edit_->setValid(value.size());

    data_source_->name(value);
    data_source_->updateInDatabase();
}
void DBODataSourceWidget::changedSacColumnSlot ()
{
    bool ok;
    unsigned char sac = sac_edit_->text().toUShort(&ok);
    sac_edit_->setValid(ok);

    if (!ok)
    {
        logwrn << "DBODataSourceWidget: changedSacColumnSlot: conversion failed";
        updateSacColumnSlot();
    }
    else
    {
        data_source_->sac(sac);
        data_source_->updateInDatabase();
    }
}
void DBODataSourceWidget::changedSicColumnSlot ()
{
    bool ok;
    unsigned char sic = sic_edit_->text().toUShort(&ok);
    sic_edit_->setValid(ok);

    if (!ok)
    {
        logwrn << "DBODataSourceWidget: changedSicColumnSlot: conversion failed";
        updateSicColumnSlot();
    }
    else
    {
        data_source_->sic(sic);
        data_source_->updateInDatabase();
    }
}
void DBODataSourceWidget::changedLatitudeColumnSlot ()
{
    bool ok;
    double value = latitude_edit_->text().toDouble(&ok);
    latitude_edit_->setValid(ok);

    if (!ok)
    {
        logwrn << "DBODataSourceWidget: changedLatitudeColumnSlot: conversion failed";
        updateLatitudeColumnSlot();
    }
    else
    {
        data_source_->latitude(value);
        data_source_->updateInDatabase();
    }
}
void DBODataSourceWidget::changedLongitudeColumnSlot ()
{
    bool ok;
    double value = longitude_edit_->text().toDouble(&ok);
    longitude_edit_->setValid(ok);

    if (!ok)
    {
        logwrn << "DBODataSourceWidget: changedLongitudeColumnSlot: conversion failed";
        updateLongitudeColumnSlot();
    }
    else
    {
        data_source_->longitude(value);
        data_source_->updateInDatabase();
    }
}
void DBODataSourceWidget::changedAltitudeColumnSlot ()
{
    bool ok;
    double value = altitude_edit_->text().toDouble(&ok);
    altitude_edit_->setValid(ok);

    if (!ok)
    {
        logwrn << "DBODataSourceWidget: changedAltitudeColumnSlot: conversion failed";
        updateAltitudeColumnSlot();
    }
    else
    {
        data_source_->altitude(value);
        data_source_->updateInDatabase();
    }
}

void DBODataSourceWidget::updateIdSlot()
{
    assert (id_edit_);
    id_edit_->setText(QString::number(data_source_->id()));
}
void DBODataSourceWidget::updateShortNameColumnSlot ()
{
    assert (short_name_edit_);
    if (data_source_->hasShortName())
    {
        short_name_edit_->setText(data_source_->shortName().c_str());
        short_name_edit_->setValid(true);
    }
    else
        short_name_edit_->setValid(false);
}
void DBODataSourceWidget::updateNameColumnSlot ()
{
    assert (name_edit_);
    name_edit_->setText(data_source_->name().c_str());
}
void DBODataSourceWidget::updateSacColumnSlot ()
{
    assert (sac_edit_);
    if (data_source_->hasSac())
    {
        sac_edit_->setText(QString::number(data_source_->sac()));
        sac_edit_->setValid(true);
    }
    else
        sac_edit_->setValid(false);
}
void DBODataSourceWidget::updateSicColumnSlot ()
{
    assert (sic_edit_);
    if (data_source_->hasSic())
    {
        sic_edit_->setText(QString::number(data_source_->sic()));
        sic_edit_->setValid(true);
    }
    else
        sic_edit_->setValid(false);
}
void DBODataSourceWidget::updateLatitudeColumnSlot ()
{
    assert (latitude_edit_);
    if (data_source_->hasLatitude())
    {
        latitude_edit_->setText(QString::number(data_source_->latitude(), 'g', 12));
        latitude_edit_->setValid(true);
    }
    else
        latitude_edit_->setValid(false);
}
void DBODataSourceWidget::updateLongitudeColumnSlot ()
{
    assert (longitude_edit_);
    if (data_source_->hasLongitude())
    {
        longitude_edit_->setText(QString::number(data_source_->longitude(), 'g', 12));
        longitude_edit_->setValid(true);
    }
    else
        longitude_edit_->setValid(false);
}
void DBODataSourceWidget::updateAltitudeColumnSlot ()
{
    assert (altitude_edit_);
    if (data_source_->hasAltitude())
    {
        altitude_edit_->setText(QString::number(data_source_->altitude(), 'g', 12));
        altitude_edit_->setValid(true);
    }
    else
        altitude_edit_->setValid(false);
}
