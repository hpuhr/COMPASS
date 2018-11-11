#include "storeddbodatasourcewidget.h"

#include "logger.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

StoredDBODataSourceWidget::StoredDBODataSourceWidget(StoredDBODataSource& data_source, QWidget* parent,
                                                     Qt::WindowFlags f)
    : QWidget (parent, f), data_source_(data_source)
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

void StoredDBODataSourceWidget::update ()
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

void StoredDBODataSourceWidget::changedIdSlot ()
{
    bool ok;
    unsigned int id = id_edit_->text().toUInt(&ok);

    if (!ok)
    {
        logwrn << "StoredDBODataSourceWidget: changedIdSlot: conversion failed";
        updateIdSlot();
    }
    else
        data_source_.id(id);
}

void StoredDBODataSourceWidget::changedShortNameColumnSlot ()
{
    data_source_.shortName(short_name_edit_->text().toStdString());
}
void StoredDBODataSourceWidget::changedNameColumnSlot ()
{
    data_source_.name(name_edit_->text().toStdString());

}
void StoredDBODataSourceWidget::changedSacColumnSlot ()
{
    bool ok;
    unsigned char sac = sac_edit_->text().toUShort(&ok);

    if (!ok)
    {
        logwrn << "StoredDBODataSourceWidget: changedSacColumnSlot: conversion failed";
        updateSacColumnSlot();
    }
    else
        data_source_.sac(sac);
}
void StoredDBODataSourceWidget::changedSicColumnSlot ()
{
    bool ok;
    unsigned char sic = sic_edit_->text().toUShort(&ok);

    if (!ok)
    {
        logwrn << "StoredDBODataSourceWidget: changedSicColumnSlot: conversion failed";
        updateSicColumnSlot();
    }
    else
        data_source_.sic(sic);
}
void StoredDBODataSourceWidget::changedLatitudeColumnSlot ()
{
    bool ok;
    double value = latitude_edit_->text().toDouble(&ok);

    if (!ok)
    {
        logwrn << "StoredDBODataSourceWidget: changedLatitudeColumnSlot: conversion failed";
        updateLatitudeColumnSlot();
    }
    else
        data_source_.latitude(value);
}
void StoredDBODataSourceWidget::changedLongitudeColumnSlot ()
{
    bool ok;
    double value = longitude_edit_->text().toDouble(&ok);

    if (!ok)
    {
        logwrn << "StoredDBODataSourceWidget: changedLongitudeColumnSlot: conversion failed";
        updateLongitudeColumnSlot();
    }
    else
        data_source_.longitude(value);
}
void StoredDBODataSourceWidget::changedAltitudeColumnSlot ()
{
    bool ok;
    double value = altitude_edit_->text().toDouble(&ok);

    if (!ok)
    {
        logwrn << "StoredDBODataSourceWidget: changedAltitudeColumnSlot: conversion failed";
        updateAltitudeColumnSlot();
    }
    else
        data_source_.altitude(value);
}

void StoredDBODataSourceWidget::updateIdSlot()
{
    assert (id_edit_);
    id_edit_->setText({data_source_.id()});
}
void StoredDBODataSourceWidget::updateShortNameColumnSlot ()
{
    assert (short_name_edit_);
    short_name_edit_->setText(data_source_.shortName().c_str());
}
void StoredDBODataSourceWidget::updateNameColumnSlot ()
{
    assert (name_edit_);
    name_edit_->setText(data_source_.name().c_str());
}
void StoredDBODataSourceWidget::updateSacColumnSlot ()
{
    assert (sac_edit_);
    sac_edit_->setText({data_source_.sac()});
}
void StoredDBODataSourceWidget::updateSicColumnSlot ()
{
    assert (sic_edit_);
    sic_edit_->setText(data_source_.shortName().c_str());
}
void StoredDBODataSourceWidget::updateLatitudeColumnSlot ()
{
    assert (latitude_edit_);
    latitude_edit_->setText(data_source_.shortName().c_str());
}
void StoredDBODataSourceWidget::updateLongitudeColumnSlot ()
{
    assert (longitude_edit_);
    longitude_edit_->setText(data_source_.shortName().c_str());
}
void StoredDBODataSourceWidget::updateAltitudeColumnSlot ()
{
    assert (altitude_edit_);
    altitude_edit_->setText(data_source_.shortName().c_str());
}

