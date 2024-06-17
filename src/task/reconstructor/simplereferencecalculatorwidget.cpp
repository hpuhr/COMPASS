
#include "simplereferencecalculatorwidget.h"

#include "reconstructorbase.h"

#include "compass.h"

#include <QFormLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLabel>

/**
*/
SimpleReferenceCalculatorWidget::SimpleReferenceCalculatorWidget(ReconstructorBase& reconstructor)
:   reconstructor_(reconstructor)
{
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

    auto* settings = &reconstructor_.referenceCalculatorSettings();

    bool is_appimage = COMPASS::instance().isAppImage();

    rec_type_combo_ = new QComboBox;
    rec_type_combo_->addItem("Uniform Motion"    , QVariant(kalman::UMKalman2D));
    rec_type_combo_->addItem("Accelerated Motion", QVariant(kalman::AMKalman2D));
    rec_type_combo_->setEnabled(false);
    connect(rec_type_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
        [ = ] (int idx) { settings->kalman_type = (kalman::KalmanType)rec_type_combo_->currentData().toInt(); });
    layout->addRow("Kalman Type", rec_type_combo_);

    addHeader("Map Projection");

    repr_distance_box_ = new QDoubleSpinBox;
    repr_distance_box_->setDecimals(3);
    repr_distance_box_->setMinimum(1.0);
    repr_distance_box_->setMaximum(DBL_MAX);
    connect(repr_distance_box_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [ = ] (double v) { settings->max_proj_distance_cart = v; });
    layout->addRow("Maximum projection distance [m]", repr_distance_box_);

    addHeader("Default Uncertainties");

    Q_std_edit_ = new QDoubleSpinBox;
    Q_std_edit_->setDecimals(3);
    Q_std_edit_->setMinimum(0.0);
    Q_std_edit_->setMaximum(DBL_MAX);
    connect(Q_std_edit_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [ = ] (double v) { settings->Q_std = v; });
    layout->addRow("Process Stddev [m]", Q_std_edit_);

    addHeader("Chain Generation");

    min_chain_size_box_ = new QSpinBox;
    min_chain_size_box_->setMinimum(1);
    min_chain_size_box_->setMaximum(INT_MAX);
    connect(min_chain_size_box_, QOverload<int>::of(&QSpinBox::valueChanged), [ = ] (int v) { settings->min_chain_size = v; });
    layout->addRow("Minimum Chain Size", min_chain_size_box_);

    if (!is_appimage)
    {
        min_dt_box_ = new QDoubleSpinBox;
        min_dt_box_->setDecimals(8);
        min_dt_box_->setMinimum(0.0);
        min_dt_box_->setMaximum(DBL_MAX);
        connect(min_dt_box_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [ = ] (double v) { settings->min_dt = v; });
        layout->addRow("Minimum Time Step [s]", min_dt_box_);
    }

    max_dt_box_ = new QDoubleSpinBox;
    max_dt_box_->setDecimals(3);
    max_dt_box_->setMinimum(0.0);
    max_dt_box_->setMaximum(DBL_MAX);
    connect(max_dt_box_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [ = ] (double v) { settings->max_dt = v; });
    layout->addRow("Maximum Time Step [s]", max_dt_box_);

    if (!is_appimage)
    {
        max_distance_box_ = new QDoubleSpinBox;
        max_distance_box_->setDecimals(3);
        max_distance_box_->setMinimum(0.0);
        max_distance_box_->setMaximum(DBL_MAX);
        connect(max_distance_box_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [ = ] (double v) { settings->max_distance = v; });
        layout->addRow("Maximum Distance [m]", max_distance_box_);
    }

    addHeader("Input Data Preprocessing");

    resample_systracks_box_ = new QCheckBox;
    connect(resample_systracks_box_, &QCheckBox::toggled, 
        [ = ] (bool ok) 
        { 
            settings->resample_systracks = ok; 

            resample_systracks_dt_box_->setEnabled(ok);
            resample_systracks_maxdt_box_->setEnabled(ok);
        });
    layout->addRow("Resample SystemTracks", resample_systracks_box_);

    resample_systracks_dt_box_ = new QDoubleSpinBox;
    resample_systracks_dt_box_->setDecimals(3);
    resample_systracks_dt_box_->setMinimum(0.1);
    resample_systracks_dt_box_->setMaximum(DBL_MAX);
    connect(resample_systracks_dt_box_,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged), [ = ] (double v) { settings->resample_systracks_dt = v; });
    layout->addRow("Resample Interval [s]", resample_systracks_dt_box_);

    resample_systracks_maxdt_box_ = new QDoubleSpinBox;
    resample_systracks_maxdt_box_->setDecimals(3);
    resample_systracks_maxdt_box_->setMinimum(0.0);
    resample_systracks_maxdt_box_->setMaximum(DBL_MAX);
    connect(resample_systracks_maxdt_box_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [ = ] (double v) { settings->resample_systracks_max_dt = v; });
    layout->addRow("Maximum Time Step [s]", resample_systracks_maxdt_box_);

    addHeader("Result Generation");

    smooth_rts_box_ = new QCheckBox;
    connect(smooth_rts_box_, &QCheckBox::toggled, [ = ] (bool ok) { settings->smooth_rts = ok; });
    layout->addRow("Smooth Results", smooth_rts_box_);

    resample_result_box_ = new QCheckBox;
    connect(resample_result_box_, &QCheckBox::toggled, 
        [ = ] (bool ok) 
        { 
            settings->resample_result = ok; 

            resample_dt_box_->setEnabled(ok);
            resample_Q_std_box_->setEnabled(ok);
        });
    layout->addRow("Resample Results", resample_result_box_);

    resample_dt_box_ = new QDoubleSpinBox;
    resample_dt_box_->setDecimals(3);
    resample_dt_box_->setMinimum(0.01);
    resample_dt_box_->setMaximum(DBL_MAX);
    connect(resample_dt_box_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [ = ] (double v) { settings->resample_dt = v; });
    layout->addRow("Resample Interval [s]", resample_dt_box_);

    resample_Q_std_box_ = new QDoubleSpinBox;
    resample_Q_std_box_->setDecimals(3);
    resample_Q_std_box_->setMinimum(0.0);
    resample_Q_std_box_->setMaximum(DBL_MAX);
    connect(resample_Q_std_box_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [ = ] (double v) { settings->resample_Q_std = v; });
    layout->addRow("Resample Process Stddev [m]", resample_Q_std_box_);

    update();
}

/**
*/
SimpleReferenceCalculatorWidget::~SimpleReferenceCalculatorWidget() = default;

/**
*/
void SimpleReferenceCalculatorWidget::update()
{
    const auto& settings = reconstructor_.referenceCalculatorSettings();

    if (rec_type_combo_) rec_type_combo_->setCurrentIndex(rec_type_combo_->findData(QVariant(settings.kalman_type)));

    if (Q_std_edit_) Q_std_edit_->setValue(settings.Q_std);

    if (repr_distance_box_) repr_distance_box_->setValue(settings.max_proj_distance_cart);

    if (min_chain_size_box_) min_chain_size_box_->setValue(settings.min_chain_size);
    if (min_dt_box_) min_dt_box_->setValue(settings.min_dt);
    if (max_dt_box_) max_dt_box_->setValue(settings.max_dt);
    if (max_distance_box_) max_distance_box_->setValue(settings.max_distance);

    if (smooth_rts_box_) smooth_rts_box_->setChecked(settings.smooth_rts);

    if (resample_systracks_box_) resample_systracks_box_->setChecked(settings.resample_systracks);
    if (resample_systracks_dt_box_) resample_systracks_dt_box_->setValue(settings.resample_systracks_dt);
    if (resample_systracks_maxdt_box_) resample_systracks_maxdt_box_->setValue(settings.resample_systracks_max_dt);

    if (resample_result_box_) resample_result_box_->setChecked(settings.resample_result);
    if (resample_dt_box_) resample_dt_box_->setValue(settings.resample_dt);
    if (resample_Q_std_box_) resample_Q_std_box_->setValue(settings.resample_Q_std);
}
