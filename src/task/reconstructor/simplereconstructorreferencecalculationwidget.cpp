
#include "simplereconstructorreferencecalculationwidget.h"

#include "simplereconstructorwidget.h"
#include "simplereconstructor.h"

#include <QFormLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLabel>

/**
*/
SimpleReconstructorReferenceCalculationWidget::SimpleReconstructorReferenceCalculationWidget(SimpleReconstructor& reconstructor, 
                                                                                             SimpleReconstructorWidget& parent)
:   reconstructor_(reconstructor)
,   parent_       (parent       )
{
    auto layout = new QFormLayout;
    setLayout(layout);

    rec_type_combo_ = new QComboBox;
    rec_type_combo_->addItem("Uniform Motion"    , QVariant(SimpleReferenceCalculator::Settings::Rec_UMKalman2D));
    rec_type_combo_->addItem("Accelerated Motion", QVariant(SimpleReferenceCalculator::Settings::Rec_AMKalman2D));
    layout->addRow("Kalman Type", rec_type_combo_);

    Q_std_edit_ = new QDoubleSpinBox;
    layout->addRow("Process Stddev [m]", Q_std_edit_);

    repr_distance_box_ = new QDoubleSpinBox;
    layout->addRow("Maximum projection distance [m]", repr_distance_box_);

    min_chain_size_box_ = new QSpinBox;
    layout->addRow("Minimum Chain Size", min_chain_size_box_);

    min_dt_box_ = new QDoubleSpinBox;
    min_dt_box_->setVisible(false);
    layout->addRow("Minimum Time Step [s]", min_dt_box_);

    max_dt_box_ = new QDoubleSpinBox;
    layout->addRow("Maximum Time Step [s]", min_dt_box_);

    max_distance_box_ = new QDoubleSpinBox;
    max_distance_box_->setVisible(false);
    layout->addRow("Maximum Distance [m]", max_distance_box_);

    smooth_rts_box_ = new QCheckBox;
    layout->addRow("Smooth Results", smooth_rts_box_);

    resample_systracks_box_ = new QCheckBox;
    layout->addRow("Resample SystemTracks", resample_systracks_box_);

    resample_dt_box_ = new QDoubleSpinBox;
    layout->addRow("Resample Interval [s]", resample_systracks_dt_box_);

    resample_Q_std_box_ = new QDoubleSpinBox;
    layout->addRow("Maximum Time Step [s]", resample_systracks_maxdt_box_);

    resample_result_box_ = new QCheckBox;
    layout->addRow("Resample Results", resample_result_box_);

    resample_dt_box_ = new QDoubleSpinBox;
    layout->addRow("Resample Interval [s]", resample_dt_box_);

    resample_Q_std_box_ = new QDoubleSpinBox;
    layout->addRow("Resample Process Stddev [m]", resample_Q_std_box_);
}

/**
*/
SimpleReconstructorReferenceCalculationWidget::~SimpleReconstructorReferenceCalculationWidget() = default;

/**
*/
void SimpleReconstructorReferenceCalculationWidget::update()
{
    const auto& settings = reconstructor_.settings().ref_calc_settings_;
}
