#include "datasourceeditwidget.h"
#include "datasourcemanager.h"
#include "configurationdatasource.h"
#include "dbdatasource.h"
#include "dstypeselectioncombobox.h"
#include "datasourcesconfigurationdialog.h"
#include "logger.h"
#include "textfielddoublevalidator.h"

#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>

using namespace std;

DataSourceEditWidget::DataSourceEditWidget(DataSourceManager& ds_man, DataSourcesConfigurationDialog& dialog)
    : ds_man_(ds_man), dialog_(dialog)
{
    setMaximumWidth(300);

    QVBoxLayout* main_layout = new QVBoxLayout();

    QGridLayout* properties_layout_ = new QGridLayout();

    unsigned int row = 0;

    // "Name", "Short Name", "DSType", "SAC", "SIC"

    //name_edit_
    properties_layout_->addWidget(new QLabel("Name"), row, 0);

    name_edit_ = new QLineEdit();
    connect(name_edit_, &QLineEdit::textEdited, this, &DataSourceEditWidget::nameEditedSlot);
    properties_layout_->addWidget(name_edit_, row, 1);
    row++;

    //short_name_edit_

    properties_layout_->addWidget(new QLabel("Short Name"), row, 0);

    short_name_edit_ = new QLineEdit();
    connect(short_name_edit_, &QLineEdit::textEdited, this, &DataSourceEditWidget::shortNameEditedSlot);
    properties_layout_->addWidget(short_name_edit_, row, 1);
    row++;

    // dstype_combo_

    properties_layout_->addWidget(new QLabel("DSType"), row, 0);

    dstype_combo_ = new DSTypeSelectionComboBox();
    connect(dstype_combo_, &DSTypeSelectionComboBox::changedTypeSignal, this, &DataSourceEditWidget::dsTypeEditedSlot);
    properties_layout_->addWidget(dstype_combo_, row, 1);
    row++;

    //QLabel* sac_label_{nullptr};

    properties_layout_->addWidget(new QLabel("SAC"), row, 0);

    sac_label_ = new QLabel();
    properties_layout_->addWidget(sac_label_, row, 1);
    row++;

    //QLabel* sic_label_{nullptr};

    properties_layout_->addWidget(new QLabel("SIC"), row, 0);

    sic_label_ = new QLabel();
    properties_layout_->addWidget(sic_label_, row, 1);
    row++;

    //QLabel* ds_id_label_{nullptr};

    properties_layout_->addWidget(new QLabel("DS ID"), row, 0);

    ds_id_label_ = new QLabel();
    properties_layout_->addWidget(ds_id_label_, row, 1);
    row++;

    main_layout->addLayout(properties_layout_);

    // position_widget_

    position_widget_ = new QWidget();
    position_widget_->setContentsMargins(0, 0, 0, 0);

    QGridLayout* position_layout = new QGridLayout();
    //position_layout->setMargin(0);

    position_layout->addWidget(new QLabel("Latitude"), 0, 0);

    latitude_edit_ = new QLineEdit();
    latitude_edit_->setValidator(new TextFieldDoubleValidator(-90, 90, 12));
    connect(latitude_edit_, &QLineEdit::textEdited, this, &DataSourceEditWidget::latitudeEditedSlot);
    position_layout->addWidget(latitude_edit_, 0, 1);

    position_layout->addWidget(new QLabel("Longitude"), 1, 0);

    longitude_edit_ = new QLineEdit();
    longitude_edit_->setValidator(new TextFieldDoubleValidator(-180, 180, 12));
    connect(longitude_edit_, &QLineEdit::textEdited, this, &DataSourceEditWidget::longitudeEditedSlot);
    position_layout->addWidget(longitude_edit_, 1, 1);

    position_layout->addWidget(new QLabel("Altitude"), 2, 0);

    altitude_edit_ = new QLineEdit();
    altitude_edit_->setValidator(new TextFieldDoubleValidator(-10000, 10000, 12));
    connect(altitude_edit_, &QLineEdit::textEdited, this, &DataSourceEditWidget::altitudeEditedSlot);
    position_layout->addWidget(altitude_edit_, 2, 1);

    position_widget_->setLayout(position_layout);

    main_layout->addWidget(position_widget_);

    // net widget

    net_widget_ = new QWidget();
    net_widget_->setContentsMargins(0, 0, 0, 0);

    QGridLayout* net_layout = new QGridLayout();

    net_layout->addWidget(new QLabel("Line1"), 0, 0);

    net_l1_edit_ = new QLineEdit();
    connect(net_l1_edit_, &QLineEdit::textEdited, this, &DataSourceEditWidget::net1EditedSlot);
    net_layout->addWidget(net_l1_edit_, 0, 1);

    net_layout->addWidget(new QLabel("Line2"), 1, 0);

    net_l2_edit_ = new QLineEdit();
    connect(net_l2_edit_, &QLineEdit::textEdited, this, &DataSourceEditWidget::net2EditedSlot);
    net_layout->addWidget(net_l2_edit_, 1, 1);

    net_layout->addWidget(new QLabel("Line3"), 2, 0);

    net_l3_edit_ = new QLineEdit();
    connect(net_l3_edit_, &QLineEdit::textEdited, this, &DataSourceEditWidget::net3EditedSlot);
    net_layout->addWidget(net_l3_edit_, 2, 1);

    net_layout->addWidget(new QLabel("Line4"), 3, 0);

    net_l4_edit_ = new QLineEdit();
    connect(net_l4_edit_, &QLineEdit::textEdited, this, &DataSourceEditWidget::net4EditedSlot);
    net_layout->addWidget(net_l4_edit_, 3, 1);

    net_widget_->setLayout(net_layout);

    main_layout->addWidget(net_widget_);

    main_layout->addStretch();

    delete_button_ = new QPushButton("Delete");
    delete_button_->setToolTip("Deletes the data source in configuration");
    connect(delete_button_, &QPushButton::clicked, this, &DataSourceEditWidget::deleteSlot);
    main_layout->addWidget(delete_button_);

    updateContent();

    setLayout(main_layout);
}

void DataSourceEditWidget::showID(unsigned int ds_id)
{
    loginf << "DataSourceEditWidget: showID: id " << ds_id;

    has_current_ds_ = true;
    current_ds_id_ = ds_id;
    current_ds_in_db_ = ds_man_.hasDBDataSource(current_ds_id_);
    assert (ds_man_.hasConfigDataSource(current_ds_id_));

    updateContent();
}

void DataSourceEditWidget::clear()
{
    loginf << "DataSourceEditWidget: clear";

    has_current_ds_ = false;
    current_ds_id_ = 0;
    current_ds_in_db_ = false;

    updateContent();
}


void DataSourceEditWidget::nameEditedSlot(const QString& value)
{
    string text = value.toStdString();

    loginf << "DataSourceEditWidget: nameEditedSlot: '" << text << "'";

    if (!text.size())
    {
        QMessageBox m_warning(QMessageBox::Warning, "Invalid Name",
                              "Empty names are not permitted. Please set another name.",
                              QMessageBox::Ok);

        m_warning.exec();
        return;
    }

    assert (has_current_ds_);

    if (current_ds_in_db_)
    {
        assert (ds_man_.hasDBDataSource(current_ds_id_));
        ds_man_.dbDataSource(current_ds_id_).name(text);
    }

    assert (ds_man_.hasConfigDataSource(current_ds_id_));
    ds_man_.configDataSource(current_ds_id_).name(text);

    dialog_.updateDataSource(current_ds_id_);
}

void DataSourceEditWidget::shortNameEditedSlot(const QString& value)
{
    string text = value.toStdString();

    loginf << "DataSourceEditWidget: shortNameEditedSlot: '" << text << "'";

    assert (has_current_ds_);

    if (current_ds_in_db_)
    {
        assert (ds_man_.hasDBDataSource(current_ds_id_));
        ds_man_.dbDataSource(current_ds_id_).shortName(text);
    }

    assert (ds_man_.hasConfigDataSource(current_ds_id_));
    ds_man_.configDataSource(current_ds_id_).shortName(text);

    dialog_.updateDataSource(current_ds_id_);
}

void DataSourceEditWidget::dsTypeEditedSlot(const QString& value)
{
    string text = value.toStdString();

    loginf << "DataSourceEditWidget: dsTypeEditedSlot: '" << text << "'";

    assert (has_current_ds_);

    if (current_ds_in_db_)
    {
        assert (ds_man_.hasDBDataSource(current_ds_id_));
        ds_man_.dbDataSource(current_ds_id_).dsType(text);
    }

    assert (ds_man_.hasConfigDataSource(current_ds_id_));
    ds_man_.configDataSource(current_ds_id_).dsType(text);

    dialog_.updateDataSource(current_ds_id_);
}

void DataSourceEditWidget::latitudeEditedSlot(const QString& value_str)
{
    double value = value_str.toDouble();

    loginf << "DataSourceEditWidget: latitudeEditedSlot: '" << value << "'";

    if (current_ds_in_db_)
    {
        assert (ds_man_.hasDBDataSource(current_ds_id_));
        ds_man_.dbDataSource(current_ds_id_).latitude(value);
    }

    assert (ds_man_.hasConfigDataSource(current_ds_id_));
    ds_man_.configDataSource(current_ds_id_).latitude(value);
}

void DataSourceEditWidget::longitudeEditedSlot(const QString& value_str)
{
    double value = value_str.toDouble();

    loginf << "DataSourceEditWidget: longitudeEditedSlot: '" << value << "'";

    if (current_ds_in_db_)
    {
        assert (ds_man_.hasDBDataSource(current_ds_id_));
        ds_man_.dbDataSource(current_ds_id_).longitude(value);
    }

    assert (ds_man_.hasConfigDataSource(current_ds_id_));
    ds_man_.configDataSource(current_ds_id_).longitude(value);
}

void DataSourceEditWidget::altitudeEditedSlot(const QString& value_str)
{
    double value = value_str.toDouble();

    loginf << "DataSourceEditWidget: altitudeEditedSlot: '" << value << "'";

    if (current_ds_in_db_)
    {
        assert (ds_man_.hasDBDataSource(current_ds_id_));
        ds_man_.dbDataSource(current_ds_id_).altitude(value);
    }

    assert (ds_man_.hasConfigDataSource(current_ds_id_));
    ds_man_.configDataSource(current_ds_id_).altitude(value);
}

void DataSourceEditWidget::net1EditedSlot(const QString& value_str)
{
    string value = value_str.toStdString();

    loginf << "DataSourceEditWidget: net1EditedSlot: '" << value << "'";

    assert (has_current_ds_);

    if (current_ds_in_db_)
    {
        assert (ds_man_.hasDBDataSource(current_ds_id_));
        ds_man_.dbDataSource(current_ds_id_).networkLine("L1", value);
    }

    assert (ds_man_.hasConfigDataSource(current_ds_id_));
    ds_man_.configDataSource(current_ds_id_).networkLine("L1", value);
}

void DataSourceEditWidget::net2EditedSlot(const QString& value_str)
{
    string value = value_str.toStdString();

    loginf << "DataSourceEditWidget: net2EditedSlot: '" << value << "'";

    assert (has_current_ds_);

    if (current_ds_in_db_)
    {
        assert (ds_man_.hasDBDataSource(current_ds_id_));
        ds_man_.dbDataSource(current_ds_id_).networkLine("L2", value);
    }

    assert (ds_man_.hasConfigDataSource(current_ds_id_));
    ds_man_.configDataSource(current_ds_id_).networkLine("L2", value);
}

void DataSourceEditWidget::net3EditedSlot(const QString& value_str)
{
    string value = value_str.toStdString();

    loginf << "DataSourceEditWidget: net3EditedSlot: '" << value << "'";

    assert (has_current_ds_);

    if (current_ds_in_db_)
    {
        assert (ds_man_.hasDBDataSource(current_ds_id_));
        ds_man_.dbDataSource(current_ds_id_).networkLine("L3", value);
    }

    assert (ds_man_.hasConfigDataSource(current_ds_id_));
    ds_man_.configDataSource(current_ds_id_).networkLine("L3", value);
}

void DataSourceEditWidget::net4EditedSlot(const QString& value_str)
{
    string value = value_str.toStdString();

    loginf << "DataSourceEditWidget: net4EditedSlot: '" << value << "'";

    assert (has_current_ds_);

    if (current_ds_in_db_)
    {
        assert (ds_man_.hasDBDataSource(current_ds_id_));
        ds_man_.dbDataSource(current_ds_id_).networkLine("L4", value);
    }

    assert (ds_man_.hasConfigDataSource(current_ds_id_));
    ds_man_.configDataSource(current_ds_id_).networkLine("L4", value);
}

void DataSourceEditWidget::deleteSlot()
{
    loginf << "DataSourceEditWidget: deleteSlot";

    assert (has_current_ds_);
    assert (!current_ds_in_db_);

    dialog_.beginResetModel();
    ds_man_.deleteConfigDataSource(current_ds_id_);
    dialog_.endResetModel();

    clear();
}

void DataSourceEditWidget::updateContent()
{
    assert (name_edit_);
    assert (short_name_edit_);
    assert (dstype_combo_);
    assert (sac_label_);
    assert (sic_label_);
    assert (ds_id_label_);
    assert (position_widget_);
    assert (net_widget_);
    assert (delete_button_);

    if (!has_current_ds_)
    {
        name_edit_->setText("");
        name_edit_->setDisabled(true);

        short_name_edit_->setText("");
        short_name_edit_->setDisabled(true);

        dstype_combo_->setType("");
        dstype_combo_->setDisabled(true);

        sac_label_->setText("");
        sic_label_->setText("");
        ds_id_label_->setText("");

        position_widget_->setHidden(true);
        net_widget_->setHidden(true);

        delete_button_->setHidden(true);
    }
    else
    {
        if (current_ds_in_db_) // db && cfg
        {
            assert (ds_man_.hasDBDataSource(current_ds_id_));

            dbContent::DBDataSource& ds = ds_man_.dbDataSource(current_ds_id_);
            assert (ds_man_.hasConfigDataSource(current_ds_id_));

            name_edit_->setText(ds.name().c_str());
            name_edit_->setDisabled(false);

            if (ds.hasShortName())
                short_name_edit_->setText(ds.shortName().c_str());
            else
                short_name_edit_->setText("");

            short_name_edit_->setDisabled(false);

            dstype_combo_->setType(ds.dsType());
            dstype_combo_->setDisabled(false);

            sac_label_->setText(QString::number(ds.sac()));
            sic_label_->setText(QString::number(ds.sic()));
            ds_id_label_->setText(QString::number(ds.id()));

            if (ds.dsType() == "Radar" || ds.hasPosition())
            {
                latitude_edit_->setText(QString::number(ds.latitude(), 'g', 12));
                longitude_edit_->setText(QString::number(ds.longitude(), 'g', 12));
                altitude_edit_->setText(QString::number(ds.altitude(), 'g', 12));

                position_widget_->setHidden(false);
            }
            else
                position_widget_->setHidden(true);

            if (ds.hasNetworkLines())
            {
                net_widget_->setHidden(false);

                std::map<std::string, std::pair<std::string, unsigned int>> lines = ds.networkLines();

                if (lines.count("L1"))
                    net_l1_edit_->setText((lines.at("L1").first+":"+to_string(lines.at("L1").second)).c_str());
                else
                    net_l1_edit_->setText("");

                if (lines.count("L2"))
                    net_l2_edit_->setText((lines.at("L2").first+":"+to_string(lines.at("L2").second)).c_str());
                else
                    net_l2_edit_->setText("");

                if (lines.count("L3"))
                    net_l3_edit_->setText((lines.at("L3").first+":"+to_string(lines.at("L3").second)).c_str());
                else
                    net_l3_edit_->setText("");

                if (lines.count("L4"))
                    net_l4_edit_->setText((lines.at("L4").first+":"+to_string(lines.at("L4").second)).c_str());
                else
                    net_l4_edit_->setText("");

            }
            else
                net_widget_->setHidden(true);

            delete_button_->setHidden(true);
        }
        else // cfg only
        {
            assert (ds_man_.hasConfigDataSource(current_ds_id_));

            dbContent::ConfigurationDataSource& ds = ds_man_.configDataSource(current_ds_id_);

            name_edit_->setText(ds.name().c_str());
            name_edit_->setDisabled(false);

            if (ds.hasShortName())
                short_name_edit_->setText(ds.shortName().c_str());
            else
                short_name_edit_->setText("");

            short_name_edit_->setDisabled(false);

            dstype_combo_->setType(ds.dsType());
            dstype_combo_->setDisabled(false);

            sac_label_->setText(QString::number(ds.sac()));
            sic_label_->setText(QString::number(ds.sic()));
            ds_id_label_->setText(QString::number(ds.id()));

            if (ds.dsType() == "Radar" || ds.hasPosition())
            {
                latitude_edit_->setText(QString::number(ds.latitude(), 'g', 12));
                longitude_edit_->setText(QString::number(ds.longitude(), 'g', 12));
                altitude_edit_->setText(QString::number(ds.altitude(), 'g', 12));

                position_widget_->setHidden(false);
            }
            else
                position_widget_->setHidden(true);

            if (ds.hasNetworkLines())
            {
                net_widget_->setHidden(false);

                std::map<std::string, std::pair<std::string, unsigned int>> lines = ds.networkLines();

                if (lines.count("L1"))
                    net_l1_edit_->setText((lines.at("L1").first+":"+to_string(lines.at("L1").second)).c_str());
                else
                    net_l1_edit_->setText("");

                if (lines.count("L2"))
                    net_l2_edit_->setText((lines.at("L2").first+":"+to_string(lines.at("L2").second)).c_str());
                else
                    net_l2_edit_->setText("");

                if (lines.count("L3"))
                    net_l3_edit_->setText((lines.at("L3").first+":"+to_string(lines.at("L3").second)).c_str());
                else
                    net_l3_edit_->setText("");

                if (lines.count("L4"))
                    net_l4_edit_->setText((lines.at("L4").first+":"+to_string(lines.at("L4").second)).c_str());
                else
                    net_l4_edit_->setText("");

            }
            else
                net_widget_->setHidden(true);

            delete_button_->setHidden(false);
        }
    }
}
