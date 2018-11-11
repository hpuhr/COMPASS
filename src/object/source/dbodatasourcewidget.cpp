#include "dbodatasourcewidget.h"
#include "logger.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

DBODataSourceWidget::DBODataSourceWidget(DBODataSource& data_source, QWidget* parent, Qt::WindowFlags f)
    : QWidget (parent, f), data_source_(&data_source)
{
    QHBoxLayout* main_layout = new QHBoxLayout ();

    main_layout->addWidget (new QLabel ("ID"));
    id_edit_ = new QLineEdit ();
    main_layout->addWidget (id_edit_);

    main_layout->addWidget (new QLabel ("Short Name"));
    short_name_edit_ = new QLineEdit ();
    main_layout->addWidget (short_name_edit_);

    main_layout->addWidget (new QLabel ("Name"));
    name_edit_ = new QLineEdit ();
    main_layout->addWidget (name_edit_);

    main_layout->addWidget (new QLabel ("SAC"));
    sac_edit_ = new QLineEdit ();
    main_layout->addWidget (sac_edit_);

    main_layout->addWidget (new QLabel ("SIC"));
    sic_edit_ = new QLineEdit ();
    main_layout->addWidget (sic_edit_);

    main_layout->addWidget (new QLabel ("Latitude"));
    latitude_edit_ = new QLineEdit ();
    main_layout->addWidget (latitude_edit_);

    main_layout->addWidget (new QLabel ("Longitude"));
    longitude_edit_ = new QLineEdit ();
    main_layout->addWidget (longitude_edit_);

    main_layout->addWidget (new QLabel ("Altitude"));
    altitude_edit_ = new QLineEdit ();
    main_layout->addWidget (altitude_edit_);

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

void DBODataSourceWidget::changedIdSlot ()
{
    bool ok;
    unsigned int id = id_edit_->text().toUInt(&ok);

    if (!ok)
    {
        logwrn << "DBODataSourceWidget: changedIdSlot: conversion failed";
        updateIdSlot();
    }
    else
        data_source_->id(id);
}
void DBODataSourceWidget::changedShortNameColumnSlot ()
{
    data_source_->shortName(short_name_edit_->text().toStdString());
}
void DBODataSourceWidget::changedNameColumnSlot ()
{
    data_source_->name(name_edit_->text().toStdString());

}
void DBODataSourceWidget::changedSacColumnSlot ()
{
    bool ok;
    unsigned char sac = sac_edit_->text().toUShort(&ok);

    if (!ok)
    {
        logwrn << "DBODataSourceWidget: changedSacColumnSlot: conversion failed";
        updateSacColumnSlot();
    }
    else
        data_source_->sac(sac);
}
void DBODataSourceWidget::changedSicColumnSlot ()
{
    bool ok;
    unsigned char sic = sic_edit_->text().toUShort(&ok);

    if (!ok)
    {
        logwrn << "DBODataSourceWidget: changedSicColumnSlot: conversion failed";
        updateSicColumnSlot();
    }
    else
        data_source_->sic(sic);
}
void DBODataSourceWidget::changedLatitudeColumnSlot ()
{
    bool ok;
    double value = latitude_edit_->text().toDouble(&ok);

    if (!ok)
    {
        logwrn << "DBODataSourceWidget: changedLatitudeColumnSlot: conversion failed";
        updateLatitudeColumnSlot();
    }
    else
        data_source_->latitude(value);
}
void DBODataSourceWidget::changedLongitudeColumnSlot ()
{
    bool ok;
    double value = longitude_edit_->text().toDouble(&ok);

    if (!ok)
    {
        logwrn << "DBODataSourceWidget: changedLongitudeColumnSlot: conversion failed";
        updateLongitudeColumnSlot();
    }
    else
        data_source_->longitude(value);
}
void DBODataSourceWidget::changedAltitudeColumnSlot ()
{
    bool ok;
    double value = altitude_edit_->text().toDouble(&ok);

    if (!ok)
    {
        logwrn << "DBODataSourceWidget: changedAltitudeColumnSlot: conversion failed";
        updateAltitudeColumnSlot();
    }
    else
        data_source_->altitude(value);
}

void DBODataSourceWidget::updateIdSlot()
{
    assert (id_edit_);
    id_edit_->setText({data_source_->id()});
}
void DBODataSourceWidget::updateShortNameColumnSlot ()
{
    assert (short_name_edit_);
    short_name_edit_->setText(data_source_->shortName().c_str());
}
void DBODataSourceWidget::updateNameColumnSlot ()
{
    assert (name_edit_);
    name_edit_->setText(data_source_->name().c_str());
}
void DBODataSourceWidget::updateSacColumnSlot ()
{
    assert (sac_edit_);
    sac_edit_->setText({data_source_->sac()});
}
void DBODataSourceWidget::updateSicColumnSlot ()
{
    assert (sic_edit_);
    sic_edit_->setText(data_source_->shortName().c_str());
}
void DBODataSourceWidget::updateLatitudeColumnSlot ()
{
    assert (latitude_edit_);
    latitude_edit_->setText(data_source_->shortName().c_str());
}
void DBODataSourceWidget::updateLongitudeColumnSlot ()
{
    assert (longitude_edit_);
    longitude_edit_->setText(data_source_->shortName().c_str());
}
void DBODataSourceWidget::updateAltitudeColumnSlot ()
{
    assert (altitude_edit_);
    altitude_edit_->setText(data_source_->shortName().c_str());
}
