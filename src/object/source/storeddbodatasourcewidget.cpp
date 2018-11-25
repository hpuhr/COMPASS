#include "storeddbodatasourcewidget.h"
#include "dbobject.h"
#include "logger.h"
#include "files.h"
#include "invalidqlineedit.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

using namespace Utils;

StoredDBODataSourceWidget::StoredDBODataSourceWidget(StoredDBODataSource& data_source, bool add_headers,
                                                     QWidget* parent, Qt::WindowFlags f)
    : QWidget (parent, f), data_source_(&data_source)
{
    loginf << "StoredDBODataSourceWidget: ctor: " << data_source.name();

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

    name_edit_ = new QLineEdit ();
    connect(name_edit_, &QLineEdit::textEdited, this, &StoredDBODataSourceWidget::changedNameColumnSlot);
    main_layout->addWidget (name_edit_, row, col++);

    short_name_edit_ = new InvalidQLineEdit ();
    connect(short_name_edit_, &InvalidQLineEdit::textEdited,
            this, &StoredDBODataSourceWidget::changedShortNameColumnSlot);
    main_layout->addWidget (short_name_edit_, row, col++);

    sac_edit_ = new InvalidQLineEdit ();
    connect(sac_edit_, &InvalidQLineEdit::textEdited, this, &StoredDBODataSourceWidget::changedSacColumnSlot);
    main_layout->addWidget (sac_edit_, row, col++);

    sic_edit_ = new InvalidQLineEdit ();
    connect(sic_edit_, &InvalidQLineEdit::textEdited, this, &StoredDBODataSourceWidget::changedSicColumnSlot);
    main_layout->addWidget (sic_edit_, row, col++);

    latitude_edit_ = new InvalidQLineEdit ();
    connect(latitude_edit_, &InvalidQLineEdit::textEdited,
            this, &StoredDBODataSourceWidget::changedLatitudeColumnSlot);
    main_layout->addWidget (latitude_edit_, row, col++);

    longitude_edit_ = new InvalidQLineEdit ();
    connect(longitude_edit_, &InvalidQLineEdit::textEdited,
            this, &StoredDBODataSourceWidget::changedLongitudeColumnSlot);
    main_layout->addWidget (longitude_edit_, row, col++);

    altitude_edit_ = new InvalidQLineEdit ();
    connect(altitude_edit_, &InvalidQLineEdit::textEdited,
            this, &StoredDBODataSourceWidget::changedAltitudeColumnSlot);
    main_layout->addWidget (altitude_edit_, row, col++);

    QIcon del_icon(Files::getIconFilepath("delete.png").c_str());

    QPushButton* del_button = new QPushButton ();
    del_button->setIcon(del_icon);
    del_button->setFixedSize (UI_ICON_SIZE);
    del_button->setFlat(UI_ICON_BUTTON_FLAT);
    connect(del_button, &QPushButton::clicked, this, &StoredDBODataSourceWidget::deleteSlot);
    main_layout->addWidget (del_button, row, col++);

    update();

    setLayout(main_layout);
}

void StoredDBODataSourceWidget::setDataSource (StoredDBODataSource& data_source)
{
    data_source_ = &data_source;

    update();
}

void StoredDBODataSourceWidget::update ()
{
    updateIdSlot();
    updateNameColumnSlot ();
    updateShortNameColumnSlot ();
    updateSacColumnSlot ();
    updateSicColumnSlot ();
    updateLatitudeColumnSlot ();
    updateLongitudeColumnSlot ();
    updateAltitudeColumnSlot ();
}

void StoredDBODataSourceWidget::changedNameColumnSlot (const QString& value)
{
    data_source_->name(value.toStdString());
}

void StoredDBODataSourceWidget::changedShortNameColumnSlot (const QString& value_str)
{
    data_source_->shortName(value_str.toStdString());
    assert (short_name_edit_);
    short_name_edit_->setValid(true);
}

void StoredDBODataSourceWidget::changedSacColumnSlot (const QString& value_str)
{
    bool ok;
    unsigned char sac = value_str.toUShort(&ok);

    assert (sac_edit_);
    sac_edit_->setValid(ok);

    if (!ok)
        logwrn << "StoredDBODataSourceWidget: changedSacColumnSlot: conversion failed";
    else
        data_source_->sac(sac);
}
void StoredDBODataSourceWidget::changedSicColumnSlot (const QString& value_str)
{
    bool ok;
    unsigned char sic = value_str.toUShort(&ok);

    assert (sic_edit_);
    sic_edit_->setValid(ok);

    if (!ok)
        logwrn << "StoredDBODataSourceWidget: changedSicColumnSlot: conversion failed";
    else
        data_source_->sic(sic);
}
void StoredDBODataSourceWidget::changedLatitudeColumnSlot (const QString& value_str)
{
    bool ok;
    double value = value_str.toDouble(&ok);

    assert (latitude_edit_);
    latitude_edit_->setValid(ok);

    if (!ok)
        logwrn << "StoredDBODataSourceWidget: changedLatitudeColumnSlot: conversion failed";
    else
        data_source_->latitude(value);
}
void StoredDBODataSourceWidget::changedLongitudeColumnSlot (const QString& value_str)
{
    bool ok;
    double value = value_str.toDouble(&ok);

    assert (longitude_edit_);
    longitude_edit_->setValid(ok);

    if (!ok)
        logwrn << "StoredDBODataSourceWidget: changedLongitudeColumnSlot: conversion failed";
    else
        data_source_->longitude(value);
}
void StoredDBODataSourceWidget::changedAltitudeColumnSlot (const QString& value_str)
{
    bool ok;
    double value = value_str.toDouble(&ok);

    assert (altitude_edit_);
    altitude_edit_->setValid(ok);

    if (!ok)
        logwrn << "StoredDBODataSourceWidget: changedAltitudeColumnSlot: conversion failed";
    else
        data_source_->altitude(value);
}

void StoredDBODataSourceWidget::updateIdSlot()
{
    assert (id_edit_);
    id_edit_->setText(QString::number(data_source_->id()));
}

void StoredDBODataSourceWidget::updateShortNameColumnSlot ()
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
void StoredDBODataSourceWidget::updateNameColumnSlot ()
{
    assert (name_edit_);
    name_edit_->setText(data_source_->name().c_str());
}
void StoredDBODataSourceWidget::updateSacColumnSlot ()
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
void StoredDBODataSourceWidget::updateSicColumnSlot ()
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
void StoredDBODataSourceWidget::updateLatitudeColumnSlot ()
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
void StoredDBODataSourceWidget::updateLongitudeColumnSlot ()
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
void StoredDBODataSourceWidget::updateAltitudeColumnSlot ()
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

void StoredDBODataSourceWidget::deleteSlot ()
{
    assert (data_source_);
    data_source_->object().deleteStoredDataSource(data_source_->id());
}

