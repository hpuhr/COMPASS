#include "ffteditwidget.h"
#include "fftmanager.h"
#include "configurationfft.h"
#include "dbfft.h"
#include "fftsconfigurationdialog.h"
#include "logger.h"
#include "textfielddoublevalidator.h"

#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QScrollArea>
#include <QFormLayout>

using namespace std;

FFTEditWidget::FFTEditWidget(FFTManager& ds_man, FFTsConfigurationDialog& dialog)
    : ds_man_(ds_man), dialog_(dialog)
{
    setMaximumWidth(400);

    QScrollArea* scroll_area = new QScrollArea();
    scroll_area->setWidgetResizable(true);

    QWidget* main_widget = new QWidget();

    QVBoxLayout* main_layout = new QVBoxLayout();
    main_layout->setSizeConstraint(QLayout::SetMinimumSize);

    QGridLayout* properties_layout_ = new QGridLayout();

    unsigned int row = 0;

    // "Name", "Short Name", "DSType", "SAC", "SIC"

    //name_edit_
    properties_layout_->addWidget(new QLabel("Name"), row, 0);

    name_edit_ = new QLineEdit();
    name_edit_->setReadOnly(true);
    //connect(name_edit_, &QLineEdit::textEdited, this, &FFTEditWidget::nameEditedSlot);
    properties_layout_->addWidget(name_edit_, row, 1);
    row++;

    // position_widget_

    position_widget_ = new QWidget();
    position_widget_->setContentsMargins(0, 0, 0, 0);

    QGridLayout* position_layout = new QGridLayout();
    //position_layout->setMargin(0);

    position_layout->addWidget(new QLabel("Latitude"), 0, 0);

    latitude_edit_ = new QLineEdit();
    latitude_edit_->setValidator(new TextFieldDoubleValidator(-90, 90, 12));
    connect(latitude_edit_, &QLineEdit::textEdited, this, &FFTEditWidget::latitudeEditedSlot);
    position_layout->addWidget(latitude_edit_, 0, 1);

    position_layout->addWidget(new QLabel("Longitude"), 1, 0);

    longitude_edit_ = new QLineEdit();
    longitude_edit_->setValidator(new TextFieldDoubleValidator(-180, 180, 12));
    connect(longitude_edit_, &QLineEdit::textEdited, this, &FFTEditWidget::longitudeEditedSlot);
    position_layout->addWidget(longitude_edit_, 1, 1);

    position_layout->addWidget(new QLabel("Altitude"), 2, 0);

    altitude_edit_ = new QLineEdit();
    altitude_edit_->setValidator(new TextFieldDoubleValidator(-10000, 10000, 12));
    connect(altitude_edit_, &QLineEdit::textEdited, this, &FFTEditWidget::altitudeEditedSlot);
    position_layout->addWidget(altitude_edit_, 2, 1);

    position_widget_->setLayout(position_layout);

    main_layout->addWidget(position_widget_);

    main_layout->addStretch();

    delete_button_ = new QPushButton("Delete");
    delete_button_->setToolTip("Deletes the data source in configuration");
    connect(delete_button_, &QPushButton::clicked, this, &FFTEditWidget::deleteSlot);
    main_layout->addWidget(delete_button_);

    updateContent();

    main_widget->setLayout(main_layout);
    scroll_area->setWidget(main_widget);

    QVBoxLayout* top_lay = new QVBoxLayout();
    top_lay->setSizeConstraint(QLayout::SetMinimumSize);
    top_lay->addWidget(scroll_area);

    setLayout(top_lay);
}

void FFTEditWidget::showFFT(const std::string& name)
{
    has_current_fft_ = true;
    current_name_ = name;
    current_fft_in_db_ = ds_man_.hasDBFFT(current_name_);

    loginf << "FFTEditWidget: showID: name " << name << " in db " << current_fft_in_db_;

    assert (ds_man_.hasConfigFFT(current_name_));

    updateContent();
}

void FFTEditWidget::clear()
{
    loginf << "FFTEditWidget: clear";

    has_current_fft_ = false;
    current_name_ = "";
    current_fft_in_db_ = false;

    updateContent();
}


//void FFTEditWidget::nameEditedSlot(const QString& value)
//{
//    string text = value.toStdString();

//    loginf << "FFTEditWidget: nameEditedSlot: '" << text << "'";

//    if (!text.size())
//    {
//        QMessageBox m_warning(QMessageBox::Warning, "Invalid Name",
//                              "Empty names are not permitted. Please set another name.",
//                              QMessageBox::Ok);

//        m_warning.exec();
//        return;
//    }

//    assert (has_current_fft_);

//    if (current_fft_in_db_)
//    {
//        assert (ds_man_.hasDBFFT(current_name_));
//        ds_man_.dbFFT(current_name_).name(text);
//    }

//    assert (ds_man_.hasConfigFFT(current_name_));
//    ds_man_.configFFT(current_name_).name(text);

//    dialog_.updateFFT(current_name_);
//}

//void FFTEditWidget::shortNameEditedSlot(const QString& value)
//{
//    string text = value.toStdString();

//    loginf << "FFTEditWidget: shortNameEditedSlot: '" << text << "'";

//    assert (has_current_fft_);

//    if (current_fft_in_db_)
//    {
//        assert (ds_man_.hasDBFFT(current_name_));
//        ds_man_.dbFFT(current_name_).shortName(text);
//    }

//    assert (ds_man_.hasConfigFFT(current_name_));
//    ds_man_.configFFT(current_name_).shortName(text);

//    dialog_.updateFFT(current_name_);
//}

//void FFTEditWidget::dsTypeEditedSlot(const QString& value)
//{
//    string text = value.toStdString();

//    loginf << "FFTEditWidget: dsTypeEditedSlot: '" << text << "'";

//    assert (has_current_fft_);

//    if (current_fft_in_db_)
//    {
//        assert (ds_man_.hasDBFFT(current_name_));
//        ds_man_.dbFFT(current_name_).dsType(text);
//    }

//    assert (ds_man_.hasConfigFFT(current_name_));
//    ds_man_.configFFT(current_name_).dsType(text);

//    dialog_.updateFFT(current_name_);

//    updateContent();
//}

//void FFTEditWidget::updateIntervalEditedSlot(const QString& value_str)
//{
//    string text = value_str.toStdString();

//    loginf << "FFTEditWidget: updateIntervalEditedSlot: '" << text << "'";

//    if (!value_str.size()) // remove if empty
//    {
//        if (current_fft_in_db_)
//        {
//            assert (ds_man_.hasDBFFT(current_name_));

//            if (ds_man_.dbFFT(current_name_).hasUpdateInterval())
//                ds_man_.dbFFT(current_name_).removeUpdateInterval();
//        }

//        assert (ds_man_.hasConfigFFT(current_name_));

//        if (ds_man_.configFFT(current_name_).hasUpdateInterval())
//            ds_man_.configFFT(current_name_).removeUpdateInterval();

//        return;
//    }

//    float value = value_str.toFloat();

//    if (current_fft_in_db_)
//    {
//        assert (ds_man_.hasDBFFT(current_name_));
//        ds_man_.dbFFT(current_name_).updateInterval(value);
//    }

//    assert (ds_man_.hasConfigFFT(current_name_));
//    ds_man_.configFFT(current_name_).updateInterval(value);
//}

void FFTEditWidget::latitudeEditedSlot(const QString& value_str)
{
    double value = value_str.toDouble();

    loginf << "FFTEditWidget: latitudeEditedSlot: '" << value << "'";

    if (current_fft_in_db_)
    {
        assert (ds_man_.hasDBFFT(current_name_));
        ds_man_.dbFFT(current_name_).latitude(value);
    }

    assert (ds_man_.hasConfigFFT(current_name_));
    ds_man_.configFFT(current_name_).latitude(value);
}

void FFTEditWidget::longitudeEditedSlot(const QString& value_str)
{
    double value = value_str.toDouble();

    loginf << "FFTEditWidget: longitudeEditedSlot: '" << value << "'";

    if (current_fft_in_db_)
    {
        assert (ds_man_.hasDBFFT(current_name_));
        ds_man_.dbFFT(current_name_).longitude(value);
    }

    assert (ds_man_.hasConfigFFT(current_name_));
    ds_man_.configFFT(current_name_).longitude(value);
}

void FFTEditWidget::altitudeEditedSlot(const QString& value_str)
{
    double value = value_str.toDouble();

    loginf << "FFTEditWidget: altitudeEditedSlot: '" << value << "'";

    if (current_fft_in_db_)
    {
        assert (ds_man_.hasDBFFT(current_name_));
        ds_man_.dbFFT(current_name_).altitude(value);
    }

    assert (ds_man_.hasConfigFFT(current_name_));
    ds_man_.configFFT(current_name_).altitude(value);
}


void FFTEditWidget::deleteSlot()
{
    loginf << "FFTEditWidget: deleteSlot";

    assert (has_current_fft_);
    //assert (!current_fft_in_db_);

    dialog_.beginResetModel();
    ds_man_.deleteFFT(current_name_);
    dialog_.endResetModel();

    clear();
}

void FFTEditWidget::updateContent()
{
    assert (name_edit_);
    assert (position_widget_);
    assert (delete_button_);

    if (!has_current_fft_)
    {
        name_edit_->setText("");
        name_edit_->setDisabled(true);

        position_widget_->setHidden(true);

        delete_button_->setHidden(true);
    }
    else
    {
        FFTBase* ds = nullptr;

        if (current_fft_in_db_) // db && cfg
        {
            assert (ds_man_.hasDBFFT(current_name_));
            assert (ds_man_.hasConfigFFT(current_name_));
            ds = dynamic_cast<FFTBase*>(&ds_man_.dbFFT(current_name_));
        }
        else
        {
            assert (ds_man_.hasConfigFFT(current_name_));
            ds = dynamic_cast<FFTBase*>(&ds_man_.configFFT(current_name_));
        }
        assert (ds);

        name_edit_->setText(ds->name().c_str());
        name_edit_->setDisabled(false);

        if (ds->hasPosition())
        {
            latitude_edit_->setText(QString::number(ds->latitude(), 'g', 12));
            longitude_edit_->setText(QString::number(ds->longitude(), 'g', 12));
            altitude_edit_->setText(QString::number(ds->altitude(), 'g', 12));
        }
        else
        {
            latitude_edit_->setText("0");
            longitude_edit_->setText("0");
            altitude_edit_->setText("0");
        }

        position_widget_->setHidden(false);

        delete_button_->setHidden(false);
    }
}
