#include "calculatereferencestaskdialog.h"
#include "calculatereferencestask.h"

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

CalculateReferencesTaskDialog::CalculateReferencesTaskDialog(CalculateReferencesTask& task)
    : QDialog(), task_(task)
{
    setWindowTitle("Calculate References");
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    setModal(true);
    setMinimumSize(QSize(800, 600));

    createUI();
    update();
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
    
    //settings tab
    QWidget* settings_widget = new QWidget;
    createSettingsWidget(settings_widget);
    tab_widget->addTab(settings_widget, "Settings");

    //task_widget_ = new CreateAssociationsTaskWidget(task_, this);
    //main_layout->addWidget(task_widget_);

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

void CalculateReferencesTaskDialog::createSettingsWidget(QWidget* w)
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

    //read in values from task
    readOptions();
}

void CalculateReferencesTaskDialog::readOptions()
{
    const auto& s = task_.settings();

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
}

void CalculateReferencesTaskDialog::writeOptions()
{
    CalculateReferencesTaskSettings& s = task_.settings();

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
