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
#include <QScrollArea>

using namespace std;

DataSourceEditWidget::DataSourceEditWidget(DataSourceManager& ds_man, DataSourcesConfigurationDialog& dialog)
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

    // update interval

    properties_layout_->addWidget(new QLabel("Update Interval [s]"), row, 0);

    update_interval_edit_ = new QLineEdit();
    update_interval_edit_->setValidator(new TextFieldDoubleValidator(0, 90, 3));
    connect(update_interval_edit_, &QLineEdit::textEdited, this, &DataSourceEditWidget::updateIntervalEditedSlot);
    properties_layout_->addWidget(update_interval_edit_, row, 1);

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

    // radar ranges
    ranges_widget_ = new QWidget();
    ranges_widget_->setContentsMargins(0, 0, 0, 0);

    QGridLayout* ranges_layout = new QGridLayout();
    unsigned int row_cnt = 0;
    ranges_layout->addWidget(new QLabel("Radar Ranges [nm]"), row_cnt, 0, 1, 2);

    // psr
    ++row_cnt;
    ranges_layout->addWidget(new QLabel("PSR Minimum"), row_cnt, 0);

    psr_min_edit_ = new QLineEdit();
    psr_min_edit_->setValidator(new TextFieldDoubleValidator(0, 10000, 2));
    psr_min_edit_->setProperty("key", "primary_ir_min");
    connect(psr_min_edit_, &QLineEdit::textEdited, this, &DataSourceEditWidget::radarRangeEditedSlot);
    ranges_layout->addWidget(psr_min_edit_, row_cnt, 1);

    ++row_cnt;
    ranges_layout->addWidget(new QLabel("PSR Maximum"), row_cnt, 0);

    psr_max_edit_ = new QLineEdit();
    psr_max_edit_->setValidator(new TextFieldDoubleValidator(0, 10000, 2));
    psr_max_edit_->setProperty("key", "primary_ir_max");
    connect(psr_max_edit_, &QLineEdit::textEdited, this, &DataSourceEditWidget::radarRangeEditedSlot);
    ranges_layout->addWidget(psr_max_edit_, row_cnt, 1);

    // ssr
    ++row_cnt;
    ranges_layout->addWidget(new QLabel("SSR Minimum"), row_cnt, 0);

    ssr_min_edit_ = new QLineEdit();
    ssr_min_edit_->setValidator(new TextFieldDoubleValidator(0, 10000, 2));
    ssr_min_edit_->setProperty("key", "secondary_ir_min");
    connect(ssr_min_edit_, &QLineEdit::textEdited, this, &DataSourceEditWidget::radarRangeEditedSlot);
    ranges_layout->addWidget(ssr_min_edit_, row_cnt, 1);

    ++row_cnt;
    ranges_layout->addWidget(new QLabel("SSR Maximum"), row_cnt, 0);

    ssr_max_edit_ = new QLineEdit();
    ssr_max_edit_->setValidator(new TextFieldDoubleValidator(0, 10000, 2));
    ssr_max_edit_->setProperty("key", "secondary_ir_max");
    connect(ssr_max_edit_, &QLineEdit::textEdited, this, &DataSourceEditWidget::radarRangeEditedSlot);
    ranges_layout->addWidget(ssr_max_edit_, row_cnt, 1);

    // mode s
    ++row_cnt;
    ranges_layout->addWidget(new QLabel("Mode S Minimum"), row_cnt, 0);

    mode_s_min_edit_ = new QLineEdit();
    mode_s_min_edit_->setValidator(new TextFieldDoubleValidator(0, 10000, 2));
    mode_s_min_edit_->setProperty("key", "mode_s_ir_min");
    connect(mode_s_min_edit_, &QLineEdit::textEdited, this, &DataSourceEditWidget::radarRangeEditedSlot);
    ranges_layout->addWidget(mode_s_min_edit_, row_cnt, 1);

    ++row_cnt;
    ranges_layout->addWidget(new QLabel("Mode S Maximum"), row_cnt, 0);

    mode_s_max_edit_ = new QLineEdit();
    mode_s_max_edit_->setValidator(new TextFieldDoubleValidator(0, 10000, 2));
    mode_s_max_edit_->setProperty("key", "mode_s_ir_max");
    connect(mode_s_max_edit_, &QLineEdit::textEdited, this, &DataSourceEditWidget::radarRangeEditedSlot);
    ranges_layout->addWidget(mode_s_max_edit_, row_cnt, 1);

    ranges_widget_->setLayout(ranges_layout);
    //ranges_widget_->setMinimumHeight(300);

    main_layout->addWidget(ranges_widget_);

    add_ranges_button_ = new QPushButton("Add Radar Ranges");
    add_ranges_button_->setToolTip("Adds Radar ranges information");
    connect(add_ranges_button_, &QPushButton::clicked, this, &DataSourceEditWidget::addRadarRangesSlot);
    main_layout->addWidget(add_ranges_button_);

    // radar accuracies
    accuracies_widget_ = new QWidget();
    accuracies_widget_->setContentsMargins(0, 0, 0, 0);

    QGridLayout* accuracies_layout = new QGridLayout();
    row_cnt = 0;
    accuracies_layout->addWidget(new QLabel("Radar Accuracies"), row_cnt, 0, 1, 2);

    // psr
    ++row_cnt;
    accuracies_layout->addWidget(new QLabel("PSR Azimuth StdDev [deg]"), row_cnt, 0);

    acc_psr_azm_edit_ = new QLineEdit();
    acc_psr_azm_edit_->setValidator(new TextFieldDoubleValidator(-180, 180, 2));
    acc_psr_azm_edit_->setProperty("key", "primary_azimuth_stddev");
    connect(acc_psr_azm_edit_, &QLineEdit::textEdited, this, &DataSourceEditWidget::radarAccuraciesEditedSlot);
    accuracies_layout->addWidget(acc_psr_azm_edit_, row_cnt, 1);

    ++row_cnt;
    accuracies_layout->addWidget(new QLabel("PSR Range StdDev [m]"), row_cnt, 0);

    acc_psr_rng_edit_ = new QLineEdit();
    acc_psr_rng_edit_->setValidator(new TextFieldDoubleValidator(-50000, 50000, 2));
    acc_psr_rng_edit_->setProperty("key", "primary_range_stddev");
    connect(acc_psr_rng_edit_, &QLineEdit::textEdited, this, &DataSourceEditWidget::radarAccuraciesEditedSlot);
    accuracies_layout->addWidget(acc_psr_rng_edit_, row_cnt, 1);

    // ssr
    ++row_cnt;
    accuracies_layout->addWidget(new QLabel("SSR Azimuth StdDev [deg]"), row_cnt, 0);

    acc_ssr_azm_edit_ = new QLineEdit();
    acc_ssr_azm_edit_->setValidator(new TextFieldDoubleValidator(-180, 180, 2));
    acc_ssr_azm_edit_->setProperty("key", "secondary_azimuth_stddev");
    connect(acc_ssr_azm_edit_, &QLineEdit::textEdited, this, &DataSourceEditWidget::radarAccuraciesEditedSlot);
    accuracies_layout->addWidget(acc_ssr_azm_edit_, row_cnt, 1);

    ++row_cnt;
    accuracies_layout->addWidget(new QLabel("SSR Range StdDev [m]"), row_cnt, 0);

    acc_ssr_rng_edit_ = new QLineEdit();
    acc_ssr_rng_edit_->setValidator(new TextFieldDoubleValidator(-50000, 50000, 2));
    acc_ssr_rng_edit_->setProperty("key", "secondary_range_stddev");
    connect(acc_ssr_rng_edit_, &QLineEdit::textEdited, this, &DataSourceEditWidget::radarAccuraciesEditedSlot);
    accuracies_layout->addWidget(acc_ssr_rng_edit_, row_cnt, 1);

    // mode s
    ++row_cnt;
    accuracies_layout->addWidget(new QLabel("Mode S Azimuth StdDev [deg]"), row_cnt, 0);

    acc_mode_s_azm_edit_ = new QLineEdit();
    acc_mode_s_azm_edit_->setValidator(new TextFieldDoubleValidator(-180, 180, 2));
    acc_mode_s_azm_edit_->setProperty("key", "mode_s_azimuth_stddev");
    connect(acc_mode_s_azm_edit_, &QLineEdit::textEdited, this, &DataSourceEditWidget::radarAccuraciesEditedSlot);
    accuracies_layout->addWidget(acc_mode_s_azm_edit_, row_cnt, 1);

    ++row_cnt;
    accuracies_layout->addWidget(new QLabel("Mode S Range StdDev [m]"), row_cnt, 0);

    acc_mode_s_rng_edit_ = new QLineEdit();
    acc_mode_s_rng_edit_->setValidator(new TextFieldDoubleValidator(-50000, 50000, 2));
    acc_mode_s_rng_edit_->setProperty("key", "mode_s_range_stddev");
    connect(acc_mode_s_rng_edit_, &QLineEdit::textEdited, this, &DataSourceEditWidget::radarAccuraciesEditedSlot);
    accuracies_layout->addWidget(acc_mode_s_rng_edit_, row_cnt, 1);

    accuracies_widget_->setLayout(accuracies_layout);
    //accuracies_widget_->setMinimumHeight(300);

    main_layout->addWidget(accuracies_widget_);

    add_accuracies_button_ = new QPushButton("Add Radar Accuracies");
    add_accuracies_button_->setToolTip("Adds Radar accuracy information");
    connect(add_accuracies_button_, &QPushButton::clicked, this, &DataSourceEditWidget::addRadarAccuraciesSlot);
    main_layout->addWidget(add_accuracies_button_);

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
    //net_widget_->setMinimumHeight(300);

    main_layout->addWidget(net_widget_);

    add_lines_button_ = new QPushButton("Add Network Lines");
    add_lines_button_->setToolTip("Adds network lines to the data source");
    connect(add_lines_button_, &QPushButton::clicked, this, &DataSourceEditWidget::addNetLinesSlot);
    main_layout->addWidget(add_lines_button_);

    main_layout->addStretch();

    delete_button_ = new QPushButton("Delete");
    delete_button_->setToolTip("Deletes the data source in configuration");
    connect(delete_button_, &QPushButton::clicked, this, &DataSourceEditWidget::deleteSlot);
    main_layout->addWidget(delete_button_);

    updateContent();

    main_widget->setLayout(main_layout);
    scroll_area->setWidget(main_widget);

    QVBoxLayout* top_lay = new QVBoxLayout();
    top_lay->setSizeConstraint(QLayout::SetMinimumSize);
    top_lay->addWidget(scroll_area);

    setLayout(top_lay);
}

void DataSourceEditWidget::showID(unsigned int ds_id)
{
    has_current_ds_ = true;
    current_ds_id_ = ds_id;
    current_ds_in_db_ = ds_man_.hasDBDataSource(current_ds_id_);

    loginf << "DataSourceEditWidget: showID: id " << ds_id << " in db " << current_ds_in_db_;

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

    updateContent();
}

void DataSourceEditWidget::updateIntervalEditedSlot(const QString& value_str)
{
    string text = value_str.toStdString();

    loginf << "DataSourceEditWidget: updateIntervalEditedSlot: '" << text << "'";

    if (!value_str.size()) // remove if empty
    {
        if (current_ds_in_db_)
        {
            assert (ds_man_.hasDBDataSource(current_ds_id_));

            if (ds_man_.dbDataSource(current_ds_id_).hasUpdateInterval())
                ds_man_.dbDataSource(current_ds_id_).removeUpdateInterval();
        }

        assert (ds_man_.hasConfigDataSource(current_ds_id_));

        if (ds_man_.configDataSource(current_ds_id_).hasUpdateInterval())
            ds_man_.configDataSource(current_ds_id_).removeUpdateInterval();

        return;
    }

    float value = value_str.toFloat();

    if (current_ds_in_db_)
    {
        assert (ds_man_.hasDBDataSource(current_ds_id_));
        ds_man_.dbDataSource(current_ds_id_).updateInterval(value);
    }

    assert (ds_man_.hasConfigDataSource(current_ds_id_));
    ds_man_.configDataSource(current_ds_id_).updateInterval(value);
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

void DataSourceEditWidget::addRadarRangesSlot()
{
    loginf << "DataSourceEditWidget: addRadarRangesSlot";

    assert (has_current_ds_);

    if (current_ds_in_db_)
    {
        assert (ds_man_.hasDBDataSource(current_ds_id_));
        ds_man_.dbDataSource(current_ds_id_).addRadarRanges();
    }

    assert (ds_man_.hasConfigDataSource(current_ds_id_));
    ds_man_.configDataSource(current_ds_id_).addRadarRanges();

    updateContent();
}

void DataSourceEditWidget::radarRangeEditedSlot(const QString& value_str)
{
    QLineEdit* line_edit = dynamic_cast<QLineEdit*> (QObject::sender());
    assert (line_edit);

    string key = line_edit->property("key").toString().toStdString();

    if (!value_str.size() || value_str.toDouble() == 0)
    {
        // remove key
        loginf << "DataSourceEditWidget: radarRangeEditedSlot: removing key '" << key << "'";

        if (current_ds_in_db_)
        {
            assert (ds_man_.hasDBDataSource(current_ds_id_));
            ds_man_.dbDataSource(current_ds_id_).removeRadarRange(key);
        }

        assert (ds_man_.hasConfigDataSource(current_ds_id_));
        ds_man_.configDataSource(current_ds_id_).removeRadarRange(key);

        return;
    }

    double value = value_str.toDouble();

    loginf << "DataSourceEditWidget: radarRangeEditedSlot: key '" << key << "' value '" << value << "'";

    if (current_ds_in_db_)
    {
        assert (ds_man_.hasDBDataSource(current_ds_id_));
        ds_man_.dbDataSource(current_ds_id_).radarRange(key, value);
    }

    assert (ds_man_.hasConfigDataSource(current_ds_id_));
    ds_man_.configDataSource(current_ds_id_).radarRange(key, value);
}

void DataSourceEditWidget::addRadarAccuraciesSlot()
{
    loginf << "DataSourceEditWidget: addRadarAccuraciesSlot";

    assert (has_current_ds_);

    if (current_ds_in_db_)
    {
        assert (ds_man_.hasDBDataSource(current_ds_id_));
        ds_man_.dbDataSource(current_ds_id_).addRadarAccuracies();
    }

    assert (ds_man_.hasConfigDataSource(current_ds_id_));
    ds_man_.configDataSource(current_ds_id_).addRadarAccuracies();

    updateContent();
}

void DataSourceEditWidget::radarAccuraciesEditedSlot(const QString& value_str)
{
    double value = value_str.toDouble();

    QLineEdit* line_edit = dynamic_cast<QLineEdit*> (QObject::sender());
    assert (line_edit);

    string key = line_edit->property("key").toString().toStdString();

    loginf << "DataSourceEditWidget: radarAccuraciesEditedSlot: key '" << key << "' value '" << value << "'";

    if (current_ds_in_db_)
    {
        assert (ds_man_.hasDBDataSource(current_ds_id_));
        ds_man_.dbDataSource(current_ds_id_).radarAccuracy(key, value);
    }

    assert (ds_man_.hasConfigDataSource(current_ds_id_));
    ds_man_.configDataSource(current_ds_id_).radarAccuracy(key, value);
}

void DataSourceEditWidget::addNetLinesSlot()
{
    loginf << "DataSourceEditWidget: addNetLinesSlot";

    assert (has_current_ds_);

    if (current_ds_in_db_)
    {
        assert (ds_man_.hasDBDataSource(current_ds_id_));
        ds_man_.dbDataSource(current_ds_id_).addNetworkLines();
    }

    assert (ds_man_.hasConfigDataSource(current_ds_id_));
    ds_man_.configDataSource(current_ds_id_).addNetworkLines();

    updateContent();
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
    assert (add_ranges_button_);
    assert (ranges_widget_);
    assert (accuracies_widget_);
    assert (add_accuracies_button_);
    assert (add_lines_button_);
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

        update_interval_edit_->setText("");

        position_widget_->setHidden(true);

        ranges_widget_->setHidden(true);
        add_ranges_button_->setHidden(true);

        accuracies_widget_->setHidden(true);
        add_accuracies_button_->setHidden(true);

        add_lines_button_->setHidden(true);
        net_widget_->setHidden(true);

        delete_button_->setHidden(true);
    }
    else
    {
        dbContent::DataSourceBase* ds = nullptr;

        if (current_ds_in_db_) // db && cfg
        {
            assert (ds_man_.hasDBDataSource(current_ds_id_));
            assert (ds_man_.hasConfigDataSource(current_ds_id_));
            ds = dynamic_cast<dbContent::DataSourceBase*>(&ds_man_.dbDataSource(current_ds_id_));
        }
        else
        {
            assert (ds_man_.hasConfigDataSource(current_ds_id_));
            ds = dynamic_cast<dbContent::DataSourceBase*>(&ds_man_.configDataSource(current_ds_id_));
        }
        assert (ds);

        name_edit_->setText(ds->name().c_str());
        name_edit_->setDisabled(false);

        if (ds->hasShortName())
            short_name_edit_->setText(ds->shortName().c_str());
        else
            short_name_edit_->setText("");

        short_name_edit_->setDisabled(false);

        dstype_combo_->setType(ds->dsType());
        dstype_combo_->setDisabled(false);

        sac_label_->setText(QString::number(ds->sac()));
        sic_label_->setText(QString::number(ds->sic()));
        ds_id_label_->setText(QString::number(ds->id()));

        if (ds->hasUpdateInterval())
            update_interval_edit_->setText(QString::number(ds->updateInterval()));
        else
            update_interval_edit_->setText("");

        loginf << "DataSourceEditWidget: updateContent: ds_type " << ds->dsType()
               << " has pos " << ds->hasPosition();

        // position
        if (ds->dsType() == "Radar" || ds->hasPosition())
        {
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
        }
        else
            position_widget_->setHidden(true);

        // ranges
        if (ds->dsType() == "Radar")
        {
            if (ds->hasRadarRanges())
            {
                ranges_widget_->setHidden(false);
                add_ranges_button_->setHidden(true);

                std::map<std::string, double> ranges = ds->radarRanges();

                // psr
                if (ranges.count("primary_ir_min"))
                    psr_min_edit_->setText(QString::number(ranges.at("primary_ir_min")));
                else
                    psr_min_edit_->setText("");

                if (ranges.count("primary_ir_max"))
                    psr_max_edit_->setText(QString::number(ranges.at("primary_ir_max")));
                else
                    psr_max_edit_->setText("");

                // ssr
                if (ranges.count("secondary_ir_min"))
                    ssr_min_edit_->setText(QString::number(ranges.at("secondary_ir_min")));
                else
                    ssr_min_edit_->setText("");

                if (ranges.count("secondary_ir_max"))
                    ssr_max_edit_->setText(QString::number(ranges.at("secondary_ir_max")));
                else
                    ssr_max_edit_->setText("");

                // mode s
                if (ranges.count("mode_s_ir_min"))
                    mode_s_min_edit_->setText(QString::number(ranges.at("mode_s_ir_min")));
                else
                    mode_s_min_edit_->setText("");

                if (ranges.count("mode_s_ir_max"))
                    mode_s_max_edit_->setText(QString::number(ranges.at("mode_s_ir_max")));
                else
                    mode_s_max_edit_->setText("");
            }
            else
            {
                ranges_widget_->setHidden(true);
                add_ranges_button_->setHidden(false);
            }

            if (ds->hasRadarAccuracies())
            {
                accuracies_widget_->setHidden(false);
                add_accuracies_button_->setHidden(true);

                std::map<std::string, double> ranges = ds->radarAccuracies();

                // psr
                if (ranges.count("primary_azimuth_stddev"))
                    acc_psr_azm_edit_->setText(QString::number(ranges.at("primary_azimuth_stddev")));
                else
                    acc_psr_azm_edit_->setText("");

                if (ranges.count("primary_range_stddev"))
                    acc_psr_rng_edit_->setText(QString::number(ranges.at("primary_range_stddev")));
                else
                    acc_psr_rng_edit_->setText("");

                // ssr
                if (ranges.count("secondary_azimuth_stddev"))
                    acc_ssr_azm_edit_->setText(QString::number(ranges.at("secondary_azimuth_stddev")));
                else
                    acc_ssr_azm_edit_->setText("");

                if (ranges.count("secondary_range_stddev"))
                    acc_ssr_rng_edit_->setText(QString::number(ranges.at("secondary_range_stddev")));
                else
                    acc_ssr_rng_edit_->setText("");

                // mode s
                if (ranges.count("mode_s_azimuth_stddev"))
                    acc_mode_s_azm_edit_->setText(QString::number(ranges.at("mode_s_azimuth_stddev")));
                else
                    acc_mode_s_azm_edit_->setText("");

                if (ranges.count("mode_s_range_stddev"))
                    acc_mode_s_rng_edit_->setText(QString::number(ranges.at("mode_s_range_stddev")));
                else
                    acc_mode_s_rng_edit_->setText("");
            }
            else
            {
                accuracies_widget_->setHidden(true);
                add_accuracies_button_->setHidden(false);
            }
        }
        else
        {
            ranges_widget_->setHidden(true);
            add_ranges_button_->setHidden(true);

            accuracies_widget_->setHidden(true);
            add_accuracies_button_->setHidden(true);
        }

        loginf << "UGA " << ds->name() << " lines " << ds->hasNetworkLines();

        // lines
        if (ds->hasNetworkLines())
        {
            add_lines_button_->setHidden(true);
            net_widget_->setHidden(false);

            std::map<std::string, std::pair<std::string, unsigned int>> lines = ds->networkLines();

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
        {
            add_lines_button_->setHidden(false);
            net_widget_->setHidden(true);
        }

        delete_button_->setHidden(true);

    }
}
