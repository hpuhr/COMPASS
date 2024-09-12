
#include "simplereferencecalculatorwidget.h"

#include "reconstructorbase.h"

#include "compass.h"

#include <QFormLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLabel>

#include <limits>

/**
*/
SimpleReferenceCalculatorWidget::SimpleReferenceCalculatorWidget(ReconstructorBase& reconstructor)
:   reconstructor_(reconstructor)
{
    auto* settings = &reconstructor_.referenceCalculatorSettings();

    auto layout = new QFormLayout;
    setLayout(layout);

    auto boldify = [&] (QLabel* l)
    {
        auto f = l->font();
        f.setBold(true);
        l->setFont(f);
        return l;
    };

    auto addHeader = [ & ] (const QString& name)
    {
        auto l = boldify(new QLabel(name));
        layout->addRow(l);
    };

    auto createProcessNoiseBox = [ = ] (double* settings_value)
    {
        auto box = new QDoubleSpinBox;
        box->setDecimals(3);
        box->setMinimum(0.0);
        box->setMaximum(std::numeric_limits<double>::max());
        QObject::connect(box, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [ = ] (double v) { *settings_value = v; });
        
        return box;
    };

    auto addProcessNoise = [ & ] (const QString& name,
                                  QDoubleSpinBox** Q_static_box,
                                  QDoubleSpinBox** Q_ground_box,
                                  QDoubleSpinBox** Q_air_box,
                                  QDoubleSpinBox** Q_unknown_box,
                                  ReferenceCalculatorSettings::ProcessNoise& Q_std)
    {
        QHBoxLayout* layout_h = new QHBoxLayout;

        *Q_static_box  = createProcessNoiseBox(&Q_std.Q_std_static);
        *Q_ground_box  = createProcessNoiseBox(&Q_std.Q_std_ground);
        *Q_air_box     = createProcessNoiseBox(&Q_std.Q_std_air);
        *Q_unknown_box = createProcessNoiseBox(&Q_std.Q_std_unknown);

        layout_h->addWidget(new QLabel("Static"));
        layout_h->addWidget(*Q_static_box);

        layout_h->addWidget(new QLabel("Ground"));
        layout_h->addWidget(*Q_ground_box);

        layout_h->addWidget(new QLabel("Air"));
        layout_h->addWidget(*Q_air_box);

        layout_h->addWidget(new QLabel("Unknown"));
        layout_h->addWidget(*Q_unknown_box);

        layout->addRow(name, layout_h);
    };

    bool is_appimage = COMPASS::instance().isAppImage();

    rec_type_combo_ = new QComboBox;
    rec_type_combo_->addItem("Uniform Motion", QVariant(kalman::UMKalman2D));
    rec_type_combo_->setEnabled(false);
    connect(rec_type_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
        [ = ] (int idx) { settings->kalman_type_assoc = (kalman::KalmanType)rec_type_combo_->currentData().toInt(); });
    layout->addRow("Kalman Type Association", rec_type_combo_);

    rec_type_combo_final_ = new QComboBox;
    rec_type_combo_final_->addItem("Uniform Motion", QVariant(kalman::UMKalman2D));
    rec_type_combo_final_->addItem("IMM", QVariant(kalman::IMMKalman2D));
    connect(rec_type_combo_final_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
        [ = ] (int idx) { settings->kalman_type_final = (kalman::KalmanType)rec_type_combo_final_->currentData().toInt(); });
    layout->addRow("Kalman Type Final", rec_type_combo_final_);

    addHeader("Map Projection");

    repr_distance_box_ = new QDoubleSpinBox;
    repr_distance_box_->setDecimals(3);
    repr_distance_box_->setMinimum(1.0);
    repr_distance_box_->setMaximum(std::numeric_limits<double>::max());
    connect(repr_distance_box_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [ = ] (double v) { settings->max_proj_distance_cart = v; });
    layout->addRow("Maximum projection distance [m]", repr_distance_box_);

    addHeader("Default Uncertainties");

    dynamic_Q_box_ = new QCheckBox("");
    connect(dynamic_Q_box_, &QCheckBox::toggled, 
        [ = ] (bool ok) 
        { 
            settings->dynamic_process_noise = ok; 
            this->updateEnabledStates();
        });
    layout->addRow("Dynamic Process Noise", dynamic_Q_box_);

    addProcessNoise("Process Stddev [m]", 
                    &Q_std_static_edit_,
                    &Q_std_ground_edit_,
                    &Q_std_air_edit_,
                    &Q_std_unknown_edit_,
                    settings->Q_std);

    addHeader("Chain Generation");

    min_chain_size_box_ = new QSpinBox;
    min_chain_size_box_->setMinimum(1);
    min_chain_size_box_->setMaximum(std::numeric_limits<int>::max());
    connect(min_chain_size_box_, QOverload<int>::of(&QSpinBox::valueChanged), [ = ] (int v) { settings->min_chain_size = v; });
    layout->addRow("Minimum Chain Size", min_chain_size_box_);

    if (!is_appimage)
    {
        min_dt_box_ = new QDoubleSpinBox;
        min_dt_box_->setDecimals(8);
        min_dt_box_->setMinimum(0.0);
        min_dt_box_->setMaximum(std::numeric_limits<double>::max());
        connect(min_dt_box_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [ = ] (double v) { settings->min_dt = v; });
        layout->addRow("Minimum Time Step [s]", min_dt_box_);
    }

    max_dt_box_ = new QDoubleSpinBox;
    max_dt_box_->setDecimals(3);
    max_dt_box_->setMinimum(0.0);
    max_dt_box_->setMaximum(std::numeric_limits<double>::max());
    connect(max_dt_box_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [ = ] (double v) { settings->max_dt = v; });
    layout->addRow("Maximum Time Step [s]", max_dt_box_);

    if (!is_appimage)
    {
        max_distance_box_ = new QDoubleSpinBox;
        max_distance_box_->setDecimals(3);
        max_distance_box_->setMinimum(0.0);
        max_distance_box_->setMaximum(std::numeric_limits<double>::max());
        connect(max_distance_box_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [ = ] (double v) { settings->max_distance = v; });
        layout->addRow("Maximum Distance [m]", max_distance_box_);
    }

    addHeader("Input Data Preprocessing");

    resample_systracks_box_ = new QCheckBox;
    connect(resample_systracks_box_, &QCheckBox::toggled, 
        [ = ] (bool ok) 
        { 
            settings->resample_systracks = ok; 
            this->updateEnabledStates();
        });
    layout->addRow("Resample SystemTracks", resample_systracks_box_);

    resample_systracks_dt_box_ = new QDoubleSpinBox;
    resample_systracks_dt_box_->setDecimals(3);
    resample_systracks_dt_box_->setMinimum(0.1);
    resample_systracks_dt_box_->setMaximum(std::numeric_limits<double>::max());
    connect(resample_systracks_dt_box_,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged), [ = ] (double v) { settings->resample_systracks_dt = v; });
    layout->addRow("Resample Interval [s]", resample_systracks_dt_box_);

    resample_systracks_maxdt_box_ = new QDoubleSpinBox;
    resample_systracks_maxdt_box_->setDecimals(3);
    resample_systracks_maxdt_box_->setMinimum(0.0);
    resample_systracks_maxdt_box_->setMaximum(std::numeric_limits<double>::max());
    connect(resample_systracks_maxdt_box_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [ = ] (double v) { settings->resample_systracks_max_dt = v; });
    layout->addRow("Maximum Time Step [s]", resample_systracks_maxdt_box_);

    addHeader("Result Generation");

    smooth_rts_box_ = new QCheckBox;
    connect(smooth_rts_box_, &QCheckBox::toggled, 
        [ = ] (bool ok) 
        { 
            settings->smooth_rts = ok; 
            this->updateEnabledStates();
        });
    layout->addRow("Smooth Results", smooth_rts_box_);

    smooth_scale_box_ = new QDoubleSpinBox;
    smooth_scale_box_->setDecimals(1);
    smooth_scale_box_->setMinimum(0.1);
    smooth_scale_box_->setMaximum(1.0);
    connect(smooth_scale_box_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [ = ] (double v) { settings->smooth_scale = v; });
    layout->addRow("Smooth Factor", smooth_scale_box_);

    resample_result_box_ = new QCheckBox;
    connect(resample_result_box_, &QCheckBox::toggled, 
        [ = ] (bool ok) 
        { 
            settings->resample_result = ok; 
            this->updateEnabledStates();
        });
    layout->addRow("Resample Results", resample_result_box_);

    resample_dt_box_ = new QDoubleSpinBox;
    resample_dt_box_->setDecimals(3);
    resample_dt_box_->setMinimum(0.01);
    resample_dt_box_->setMaximum(std::numeric_limits<double>::max());
    connect(resample_dt_box_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [ = ] (double v) { settings->resample_dt = v; });
    layout->addRow("Resample Interval [s]", resample_dt_box_);

    addProcessNoise("Resample Process Stddev [m]", 
                    &resample_Q_std_static_edit_,
                    &resample_Q_std_ground_edit_,
                    &resample_Q_std_air_edit_,
                    &resample_Q_std_unknown_edit_,
                    settings->resample_Q_std);

    update();
}

/**
*/
SimpleReferenceCalculatorWidget::~SimpleReferenceCalculatorWidget() = default;

/**
*/
void SimpleReferenceCalculatorWidget::updateEnabledStates()
{
    bool dynamic_Q          = dynamic_Q_box_->isChecked();
    bool smooth             = smooth_rts_box_->isChecked();
    bool resample_res       = resample_result_box_->isChecked();
    bool resample_systracks = resample_systracks_box_->isChecked();

    Q_std_static_edit_->setEnabled(!dynamic_Q);
    Q_std_ground_edit_->setEnabled(dynamic_Q);
    Q_std_air_edit_->setEnabled(dynamic_Q);
    Q_std_unknown_edit_->setEnabled(dynamic_Q);

    smooth_scale_box_->setEnabled(smooth);

    resample_Q_std_static_edit_->setEnabled(resample_res && !dynamic_Q);
    resample_Q_std_ground_edit_->setEnabled(resample_res && dynamic_Q);
    resample_Q_std_air_edit_->setEnabled(resample_res && dynamic_Q);
    resample_Q_std_unknown_edit_->setEnabled(resample_res && dynamic_Q);
    resample_dt_box_->setEnabled(resample_res);

    resample_systracks_dt_box_->setEnabled(resample_systracks);
    resample_systracks_maxdt_box_->setEnabled(resample_systracks);
}

/**
*/
void SimpleReferenceCalculatorWidget::update()
{
    const auto& settings = reconstructor_.referenceCalculatorSettings();

    if (rec_type_combo_) rec_type_combo_->setCurrentIndex(rec_type_combo_->findData(QVariant(settings.kalman_type_assoc)));
    if (rec_type_combo_final_) rec_type_combo_final_->setCurrentIndex(rec_type_combo_final_->findData(QVariant(settings.kalman_type_final)));

    if (Q_std_static_edit_) Q_std_static_edit_->setValue(settings.Q_std.Q_std_static);
    if (Q_std_ground_edit_) Q_std_ground_edit_->setValue(settings.Q_std.Q_std_ground);
    if (Q_std_air_edit_) Q_std_air_edit_->setValue(settings.Q_std.Q_std_air);
    if (Q_std_unknown_edit_) Q_std_unknown_edit_->setValue(settings.Q_std.Q_std_unknown);
    if (dynamic_Q_box_) dynamic_Q_box_->setChecked(settings.dynamic_process_noise);

    if (repr_distance_box_) repr_distance_box_->setValue(settings.max_proj_distance_cart);

    if (min_chain_size_box_) min_chain_size_box_->setValue(settings.min_chain_size);
    if (min_dt_box_) min_dt_box_->setValue(settings.min_dt);
    if (max_dt_box_) max_dt_box_->setValue(settings.max_dt);
    if (max_distance_box_) max_distance_box_->setValue(settings.max_distance);

    if (smooth_rts_box_) smooth_rts_box_->setChecked(settings.smooth_rts);
    if (smooth_scale_box_) smooth_scale_box_->setValue(settings.smooth_scale);

    if (resample_systracks_box_) resample_systracks_box_->setChecked(settings.resample_systracks);
    if (resample_systracks_dt_box_) resample_systracks_dt_box_->setValue(settings.resample_systracks_dt);
    if (resample_systracks_maxdt_box_) resample_systracks_maxdt_box_->setValue(settings.resample_systracks_max_dt);

    if (resample_result_box_) resample_result_box_->setChecked(settings.resample_result);
    if (resample_dt_box_) resample_dt_box_->setValue(settings.resample_dt);
    if (resample_Q_std_static_edit_) resample_Q_std_static_edit_->setValue(settings.resample_Q_std.Q_std_static);
    if (resample_Q_std_ground_edit_) resample_Q_std_ground_edit_->setValue(settings.resample_Q_std.Q_std_ground);
    if (resample_Q_std_air_edit_) resample_Q_std_air_edit_->setValue(settings.resample_Q_std.Q_std_air);
    if (resample_Q_std_unknown_edit_) resample_Q_std_unknown_edit_->setValue(settings.resample_Q_std.Q_std_unknown); 

    updateEnabledStates();
}
