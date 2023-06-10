#include "calculatereferencestaskdialog.h"
#include "calculatereferencestask.h"
#include "selectdatasourceswidget.h"
#include "util/stringconv.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QTabWidget>
#include <QScrollArea>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>

using namespace std;
using namespace Utils;

CalculateReferencesTaskDialog::CalculateReferencesTaskDialog(CalculateReferencesTask& task)
    : QDialog(), task_(task)
{
    setWindowTitle("Calculate References");
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    setModal(true);
    setMinimumSize(QSize(800, 600));

    createUI();

    updateSourcesWidgets();
    updateButtons();
}

void CalculateReferencesTaskDialog::createUI()
{
    QVBoxLayout* main_layout = new QVBoxLayout();
    setLayout(main_layout);

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(16);

    //tab widget
    QTabWidget* tab_widget = new QTabWidget;
    tab_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    main_layout->addWidget(tab_widget);

    //data sources settings tab
    QWidget* data_sources_settings_widget = new QWidget;
    createDataSourcesSettingsWidget(data_sources_settings_widget);
    tab_widget->addTab(data_sources_settings_widget, "Input Data Sources");

    QWidget* position_filter_widget = new QWidget;
    createPositionFilterSettingsWidget(position_filter_widget);
    tab_widget->addTab(position_filter_widget, "Position Data Filter");

    //kalman settings tab
    QWidget* kalman_settings_widget = new QWidget;
    createKalmanSettingsWidget(kalman_settings_widget);
    tab_widget->addTab(kalman_settings_widget, "Kalman Settings");

    //output settings tab
    QWidget* output_settings_widget = new QWidget;
    createOutputSettingsWidget(output_settings_widget);
    tab_widget->addTab(output_settings_widget, "Output");

    //read in values from task
    readOptions();

    //bottom buttons
    {
        QHBoxLayout* button_layout = new QHBoxLayout();

        cancel_button_ = new QPushButton("Close");
        connect(cancel_button_, &QPushButton::clicked, this, &CalculateReferencesTaskDialog::cancelClickedSlot);
        button_layout->addWidget(cancel_button_);

        button_layout->addStretch();

        run_button_ = new QPushButton("Run");
        connect(run_button_, &QPushButton::clicked, this, &CalculateReferencesTaskDialog::runClickedSlot);
        button_layout->addWidget(run_button_);

        main_layout->addLayout(button_layout);
    }
}

void CalculateReferencesTaskDialog::createDataSourcesSettingsWidget(QWidget* w)
{
    QWidget* content_widget = addScrollArea(w);

    QVBoxLayout* layout = new QVBoxLayout;
    content_widget->setLayout(layout);

    use_tracker_check_ = new QCheckBox ("Use Tracker Data");
    use_tracker_check_->setChecked(task_.useTrackerData());
    connect (use_tracker_check_, SIGNAL(toggled(bool)), this, SLOT(toggleTrackerSourcesSlot()));
    layout->addWidget(use_tracker_check_);

    tracker_sources_ = new SelectDataSourcesWidget("Tracker Sources", "Tracker");
    tracker_sources_->updateSelected(task_.trackerDataSources());
    connect(tracker_sources_, &SelectDataSourcesWidget::selectionChangedSignal,
            this, &CalculateReferencesTaskDialog::trackerSourcesChangedSlot);
    layout->addWidget(tracker_sources_);

    use_adsb_check_ = new QCheckBox ("Use ADS-B Data");
    use_adsb_check_->setChecked(task_.useADSBData());
    connect (use_adsb_check_, SIGNAL(toggled(bool)), this, SLOT(toggleADSBSourcesSlot()));
    layout->addWidget(use_adsb_check_);

    adsb_sources_ = new SelectDataSourcesWidget("ADS-B Sources", "ADSB");
    adsb_sources_->updateSelected(task_.adsbDataSources());
    connect(adsb_sources_, &SelectDataSourcesWidget::selectionChangedSignal,
            this, &CalculateReferencesTaskDialog::adsbSourcesChangedSlot);
    layout->addWidget(adsb_sources_);
}

void CalculateReferencesTaskDialog::createPositionFilterSettingsWidget(QWidget* w)
{
    QWidget* content_widget = addScrollArea(w);

    QGridLayout* layout = new QGridLayout;
    content_widget->setLayout(layout);

    auto boldify = [&] (QLabel* l)
    {
        auto f = l->font();
        f.setBold(true);
        l->setFont(f);
        return l;
    };

    int row = 0;

    auto addRow = [&] (const QString& name, QWidget* w)
    {
        QLabel* label = new QLabel(name);
        label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        layout->addWidget(label, row, 0);
        if (w) layout->addWidget(w, row, 1);
        ++row;
    };

    auto addOptionalRow = [&] (QCheckBox* check_box, QWidget* w)
    {
        check_box->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        layout->addWidget(check_box, row, 0);
        if (w) layout->addWidget(w, row, 1);
        ++row;
    };

    auto addHeader = [&] (const QString& name)
    {
        QLabel* label = boldify(new QLabel(name));
        layout->addWidget(label, row++, 0);
    };

    filter_position_usage_check_ = new QCheckBox("Filter Position Data");
    addOptionalRow(filter_position_usage_check_, nullptr);

    addHeader("Tracker Position Data Usage");

    tracker_only_confirmed_positions_check_ = new QCheckBox("Only Use Confirmed");
    addOptionalRow(tracker_only_confirmed_positions_check_, nullptr);

    tracker_only_noncoasting_positions_check_ = new QCheckBox("Only Use Non-Coasting");
    addOptionalRow(tracker_only_noncoasting_positions_check_, nullptr);

    tracker_only_report_detection_positions_check_ = new QCheckBox("Only Use Detected Report");
    addOptionalRow(tracker_only_report_detection_positions_check_, nullptr);

    tracker_only_report_detection_nonpsronly_positions_check_ = new QCheckBox("Only Use Non-Single PSR-only Detections");
    addOptionalRow(tracker_only_report_detection_nonpsronly_positions_check_, nullptr);

    tracker_only_high_accuracy_postions_check_ = new QCheckBox("Only Use High Accuracy");
    addOptionalRow(tracker_only_high_accuracy_postions_check_, nullptr);

    tracker_minimum_accuracy_box_ = new QDoubleSpinBox;
    tracker_minimum_accuracy_box_->setMinimum(0.0);
    tracker_minimum_accuracy_box_->setMaximum(DBL_MAX);
    addRow("Minimum Position Stddev", tracker_minimum_accuracy_box_);

    addHeader("ADS-B Position Data Usage");

    adsb_only_v12_positions_check_ = new QCheckBox("Only Use MOPS V1 / V2");
    addOptionalRow(adsb_only_v12_positions_check_, nullptr);


    adsb_only_high_nucp_nic_positions_check_ = new QCheckBox("Only Use High NUCp / NIC");
    addOptionalRow(adsb_only_high_nucp_nic_positions_check_, nullptr);

    adsb_minimum_nucp_nic_box_ = new QSpinBox;
    adsb_minimum_nucp_nic_box_->setMinimum(0);
    adsb_minimum_nucp_nic_box_->setMaximum(20);
    addRow("Minimum Position NUCp / NIC", adsb_minimum_nucp_nic_box_);

    adsb_only_high_nacp_positions_check_ = new QCheckBox("Only Use High NACp");
    addOptionalRow(adsb_only_high_nacp_positions_check_, nullptr);

    adsb_minimum_nacp_box_ = new QSpinBox;
    adsb_minimum_nacp_box_->setMinimum(0);
    adsb_minimum_nacp_box_->setMaximum(20);
    addRow("Minimum Position NACp", adsb_minimum_nacp_box_);

    adsb_only_high_sil_positionss_check_ = new QCheckBox("Only Use High SIL");
    addOptionalRow(adsb_only_high_sil_positionss_check_, nullptr);

    adsb_minimum_sil_box_ = new QSpinBox;
    adsb_minimum_sil_box_->setMinimum(0);
    adsb_minimum_sil_box_->setMaximum(5);
    addRow("Minimum Position SIL", adsb_minimum_sil_box_);

}

void CalculateReferencesTaskDialog::createKalmanSettingsWidget(QWidget* w)
{
    QWidget* content_widget = addScrollArea(w);

    QGridLayout* layout = new QGridLayout;
    content_widget->setLayout(layout);

    auto boldify = [&] (QLabel* l)
    {
        auto f = l->font();
        f.setBold(true);
        l->setFont(f);
        return l;
    };

    int row = 0;

    auto addRow = [&] (const QString& name, QWidget* w)
    {
        QLabel* label = new QLabel(name);
        label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        layout->addWidget(label, row, 0);
        if (w) layout->addWidget(w, row, 1);
        ++row;
    };

    auto addOptionalRow = [&] (QCheckBox* check_box, QWidget* w)
    {
        check_box->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        layout->addWidget(check_box, row, 0);
        if (w) layout->addWidget(w, row, 1);
        ++row;
    };

    auto addHeader = [&] (const QString& name)
    {
        QLabel* label = boldify(new QLabel(name));
        layout->addWidget(label, row++, 0);
    };

    rec_type_box_ = new QComboBox;
    rec_type_box_->addItem("UMKalman2D");
#if USE_EXPERIMENTAL_SOURCE
    rec_type_box_->addItem("AMKalman2D");
#endif
    addRow("Reconstructor", rec_type_box_);

    //uncertainty section
    addHeader("Default Uncertainties");

    R_std_box_ = new QDoubleSpinBox;
    R_std_box_->setMinimum(0.0);
    R_std_box_->setMaximum(DBL_MAX);
    addRow("Measurement Stddev", R_std_box_);

    R_std_high_box_ = new QDoubleSpinBox;
    R_std_high_box_->setMinimum(0.0);
    R_std_high_box_->setMaximum(DBL_MAX);
    addRow("Measurement Stddev (high)", R_std_high_box_);

    Q_std_box_ = new QDoubleSpinBox;
    Q_std_box_->setMinimum(0.0);
    Q_std_box_->setMaximum(DBL_MAX);
    addRow("Process Stddev", Q_std_box_);

    P_std_box_ = new QDoubleSpinBox;
    P_std_box_->setMinimum(0.0);
    P_std_box_->setMaximum(DBL_MAX);
    addRow("System Stddev", P_std_box_);

    P_std_high_box_ = new QDoubleSpinBox;
    P_std_high_box_->setMinimum(0.0);
    P_std_high_box_->setMaximum(DBL_MAX);
    addRow("System Stddev (high)", P_std_high_box_);

    //chain section
    addHeader("Chain Generation");

    min_dt_box_ = new QDoubleSpinBox;
    min_dt_box_->setMinimum(0.0);
    min_dt_box_->setMaximum(DBL_MAX);
    min_dt_box_->setSuffix(" s");
    addRow("Minimum Time Step", min_dt_box_);

    max_dt_box_ = new QDoubleSpinBox;
    max_dt_box_->setMinimum(0.0);
    max_dt_box_->setMaximum(DBL_MAX);
    max_dt_box_->setSuffix(" s");
    addRow("Maximum Time Step", max_dt_box_);

    min_chain_size_box_ = new QSpinBox;
    min_chain_size_box_->setMinimum(1);
    min_chain_size_box_->setMaximum(INT_MAX);
    addRow("Minimum Chain Size", min_chain_size_box_);

    //additional option section
    addHeader("Additional Options");

    use_vel_mm_box_ = new QCheckBox("Use Velocity Measurements");
    addOptionalRow(use_vel_mm_box_, nullptr);

    smooth_box_ = new QCheckBox("Smooth Results");
    addOptionalRow(smooth_box_, nullptr);

    resample_systracks_box_    = new QCheckBox("Resample System Tracks");
    resample_systracks_dt_box_ = new QDoubleSpinBox;
    resample_systracks_dt_box_->setMinimum(1.0);
    resample_systracks_dt_box_->setMaximum(DBL_MAX);
    resample_systracks_dt_box_->setSuffix(" s");
    addOptionalRow(resample_systracks_box_, resample_systracks_dt_box_);

    resample_result_box_    = new QCheckBox("Resample Result");
    resample_result_dt_box_ = new QDoubleSpinBox;
    resample_result_dt_box_->setMinimum(1.0);
    resample_result_dt_box_->setMaximum(DBL_MAX);
    resample_result_dt_box_->setSuffix(" s");
    addOptionalRow(resample_result_box_, resample_result_dt_box_);

    verbose_box_ = new QCheckBox("Verbose");
    addOptionalRow(verbose_box_, nullptr);

    python_comp_box_ = new QCheckBox("Python Compatibility Mode");
    addOptionalRow(python_comp_box_, nullptr);

}

void CalculateReferencesTaskDialog::createOutputSettingsWidget(QWidget* w)
{
    QWidget* content_widget = addScrollArea(w);

    QVBoxLayout* out_layout = new QVBoxLayout();

    QGridLayout* layout = new QGridLayout;
    out_layout->addLayout(layout);
    out_layout->addStretch();

    content_widget->setLayout(out_layout);

    int row = 0;

    auto boldify = [&] (QLabel* l)
    {
        auto f = l->font();
        f.setBold(true);
        l->setFont(f);
        return l;
    };

    auto addRow = [&] (const QString& name, QWidget* w)
    {
        QLabel* label = new QLabel(name);
        label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        layout->addWidget(label, row, 0);
        if (w) layout->addWidget(w, row, 1);
        ++row;
    };

    auto addHeader = [&] (const QString& name)
    {
        QLabel* label = boldify(new QLabel(name));
        layout->addWidget(label, row++, 0);
    };

    addHeader("Output Data Source");

    ds_name_edit_ = new QLineEdit();
    addRow("Name", ds_name_edit_);

    ds_sac_box_ = new QSpinBox;
    ds_sac_box_->setMinimum(0);
    ds_sac_box_->setMaximum(255);
    addRow("SAC", ds_sac_box_);

    ds_sic_box_ = new QSpinBox;
    ds_sic_box_->setMinimum(0);
    ds_sic_box_->setMaximum(255);
    addRow("SIC", ds_sic_box_);

    layout->addWidget(new QLabel(), row, 0);

    ds_line_box_ = new QComboBox();
    ds_line_box_->addItems({"1", "2", "3", "4"});

    addRow("Line ID", ds_line_box_);
}

void CalculateReferencesTaskDialog::readOptions()
{
    const auto& s = task_.settings();

    // position filter

    filter_position_usage_check_->setChecked(s.filter_position_usage);
    tracker_only_confirmed_positions_check_->setChecked(s.tracker_only_confirmed_positions);
    tracker_only_noncoasting_positions_check_->setChecked(s.tracker_only_noncoasting_positions);
    tracker_only_report_detection_positions_check_->setChecked(s.tracker_only_report_detection_positions);
    tracker_only_report_detection_nonpsronly_positions_check_->setChecked(
                s.tracker_only_report_detection_nonpsronly_positions);
    tracker_only_high_accuracy_postions_check_->setChecked(s.tracker_only_high_accuracy_postions);
    tracker_minimum_accuracy_box_->setValue(s.tracker_minimum_accuracy);

    adsb_only_v12_positions_check_->setChecked(s.adsb_only_v12_positions);
    adsb_only_high_nucp_nic_positions_check_->setChecked(s.adsb_only_high_nucp_nic_positions);
    adsb_minimum_nucp_nic_box_->setValue(s.adsb_minimum_nucp_nic);
    adsb_only_high_nacp_positions_check_->setChecked(s.adsb_only_high_nacp_positions);
    adsb_minimum_nacp_box_->setValue(s.adsb_minimum_nacp);
    adsb_only_high_sil_positionss_check_->setChecked(s.adsb_only_high_sil_positions);
    adsb_minimum_sil_box_->setValue(s.adsb_minimum_sil);

    // kalman

    rec_type_box_->setCurrentIndex((int)s.rec_type);

    R_std_box_->setValue(s.R_std);
    R_std_high_box_->setValue(s.R_std_high);
    Q_std_box_->setValue(s.Q_std);
    P_std_box_->setValue(s.P_std);
    P_std_high_box_->setValue(s.P_std_high);

    min_dt_box_->setValue(s.min_dt);
    max_dt_box_->setValue(s.max_dt);
    min_chain_size_box_->setValue(s.min_chain_size);

    use_vel_mm_box_->setChecked(s.use_vel_mm);
    smooth_box_->setChecked(s.smooth_rts);

    resample_systracks_box_->setChecked(s.resample_systracks);
    resample_systracks_dt_box_->setValue(s.resample_systracks_dt);

    resample_result_box_->setChecked(s.resample_result);
    resample_result_dt_box_->setValue(s.resample_result_dt);

    verbose_box_->setChecked(s.verbose);
    python_comp_box_->setChecked(s.python_compatibility);

    // output
    ds_name_edit_->setText(s.ds_name.c_str());
    ds_sac_box_->setValue(s.ds_sac);
    ds_sic_box_->setValue(s.ds_sic);
    ds_line_box_->setCurrentText(String::lineStrFrom(s.ds_line).c_str());
}

void CalculateReferencesTaskDialog::writeOptions()
{
    CalculateReferencesTaskSettings& s = task_.settings();

    // position filter

    s.filter_position_usage = filter_position_usage_check_->isChecked();

    s.tracker_only_confirmed_positions = tracker_only_confirmed_positions_check_->isChecked();
    s.tracker_only_noncoasting_positions = tracker_only_noncoasting_positions_check_->isChecked();
    s.tracker_only_report_detection_positions = tracker_only_report_detection_positions_check_->isChecked();
    s.tracker_only_report_detection_nonpsronly_positions  =
            tracker_only_report_detection_nonpsronly_positions_check_->isChecked();
    s.tracker_only_high_accuracy_postions = tracker_only_high_accuracy_postions_check_->isChecked();
    s.tracker_minimum_accuracy  = tracker_minimum_accuracy_box_->value();

    s.adsb_only_v12_positions = adsb_only_v12_positions_check_->isChecked();
    s.adsb_only_high_nucp_nic_positions = adsb_only_high_nucp_nic_positions_check_->isChecked();
    s.adsb_minimum_nucp_nic = adsb_minimum_nucp_nic_box_->value();
    s.adsb_only_high_nacp_positions = adsb_only_high_nacp_positions_check_->isChecked();
    s.adsb_minimum_nacp = adsb_minimum_nacp_box_->value();
    s.adsb_only_high_sil_positions = adsb_only_high_sil_positionss_check_->isChecked();
    s.adsb_minimum_sil = adsb_minimum_sil_box_->value();

    // kalman

    s.rec_type              = (CalculateReferencesTaskSettings::ReconstructorType)rec_type_box_->currentIndex();

    s.R_std                 = R_std_box_->value();
    s.R_std_high            = R_std_high_box_->value();
    s.Q_std                 = Q_std_box_->value();
    s.P_std                 = P_std_box_->value();
    s.P_std_high            = P_std_box_->value();

    s.min_dt                = min_dt_box_->value();
    s.max_dt                = max_dt_box_->value();
    s.min_chain_size        = min_chain_size_box_->value();

    s.use_vel_mm            = use_vel_mm_box_->isChecked();
    s.smooth_rts            = smooth_box_->isChecked();

    s.resample_systracks    = resample_systracks_box_->isChecked();
    s.resample_systracks_dt = resample_systracks_dt_box_->value();

    s.resample_result       = resample_result_box_->isChecked();
    s.resample_result_dt    = resample_result_dt_box_->value();

    s.verbose               = verbose_box_->isChecked();
    s.python_compatibility  = python_comp_box_->isChecked();

    // output

    s.ds_name = ds_name_edit_->text().toStdString();
    s.ds_sac = ds_sac_box_->value();
    s.ds_sic = ds_sic_box_->value();

    bool ok;
    unsigned int line_id = ds_line_box_->currentText().toUInt(&ok);
    assert (ok);
    assert (line_id > 0 && line_id <= 4);

    task_.settings().ds_line = line_id-1;
}

QWidget* CalculateReferencesTaskDialog::addScrollArea(QWidget* w) const
{
    QVBoxLayout* layout_outer = new QVBoxLayout;
    layout_outer->setMargin(0);
    layout_outer->setContentsMargins(0, 0, 0, 0);

    w->setLayout(layout_outer);

    QScrollArea* scroll_area = new QScrollArea;
    scroll_area->setWidgetResizable(true);

    QWidget* scroll_widget = new QWidget;

    QVBoxLayout* layout_inner = new QVBoxLayout;
    layout_inner->setMargin(0);
    layout_inner->setContentsMargins(0, 0, 0, 0);

    QWidget* content_widget = new QWidget;
    layout_inner->addWidget(content_widget);

    scroll_widget->setLayout(layout_inner);
    scroll_area->setWidget(scroll_widget);

    layout_outer->addWidget(scroll_area);

    return content_widget;
}

void CalculateReferencesTaskDialog::updateSourcesWidgets()
{
    assert (tracker_sources_);
    tracker_sources_->setEnabled(task_.useTrackerData());

    assert (adsb_sources_);
    adsb_sources_->setEnabled(task_.useADSBData());

}

void CalculateReferencesTaskDialog::updateButtons()
{
    assert (run_button_);

    run_button_->setDisabled(!task_.canRun());
}

void CalculateReferencesTaskDialog::runClickedSlot()
{
    //write selected values to task
    writeOptions();

    emit runSignal();
}

void CalculateReferencesTaskDialog::cancelClickedSlot()
{
    //write selected values to task
    writeOptions();

    emit cancelSignal();
}

void CalculateReferencesTaskDialog::toggleTrackerSourcesSlot()
{
    assert (use_tracker_check_);
    task_.useTrackerData(use_tracker_check_->isChecked());

    updateSourcesWidgets();
    updateButtons();
}

void CalculateReferencesTaskDialog::trackerSourcesChangedSlot(std::map<std::string, bool> selection)
{
    task_.trackerDataSources(selection);

    updateSourcesWidgets();
    updateButtons();
}

void CalculateReferencesTaskDialog::toggleADSBSourcesSlot()
{
    assert (use_adsb_check_);
    task_.useADSBData(use_adsb_check_->isChecked());

    updateSourcesWidgets();
    updateButtons();
}

void CalculateReferencesTaskDialog::adsbSourcesChangedSlot(std::map<std::string, bool> selection)
{
    task_.adsbDataSources(selection);

    updateSourcesWidgets();
    updateButtons();
}


