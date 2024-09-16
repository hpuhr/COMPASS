#pragma once

#include <QWidget>

class ReconstructorBase;

class QComboBox;
class QDoubleSpinBox;
class QSpinBox;
class QCheckBox;

/**
*/
class SimpleReferenceCalculatorWidget : public QWidget
{
public:
    explicit SimpleReferenceCalculatorWidget(ReconstructorBase& reconstructor);
    virtual ~SimpleReferenceCalculatorWidget();

    void update();

private:
    void updateEnabledStates();

    ReconstructorBase& reconstructor_;

    QComboBox*      rec_type_combo_               = nullptr;
    QComboBox*      rec_type_combo_final_         = nullptr;
    QDoubleSpinBox* Q_std_static_edit_            = nullptr;
    QDoubleSpinBox* Q_std_ground_edit_            = nullptr;
    QDoubleSpinBox* Q_std_air_edit_               = nullptr;
    QDoubleSpinBox* Q_std_unknown_edit_           = nullptr;
    QCheckBox*      dynamic_Q_box_                = nullptr;
    QDoubleSpinBox* repr_distance_box_            = nullptr;
    QSpinBox*       min_chain_size_box_           = nullptr;
    QDoubleSpinBox* min_dt_box_                   = nullptr;
    QDoubleSpinBox* max_dt_box_                   = nullptr;
    QDoubleSpinBox* max_distance_box_             = nullptr;
    QCheckBox*      smooth_rts_box_               = nullptr;
    QDoubleSpinBox* smooth_scale_box_             = nullptr;
    QCheckBox*      resample_systracks_box_       = nullptr;
    QDoubleSpinBox* resample_systracks_dt_box_    = nullptr;
    QDoubleSpinBox* resample_systracks_maxdt_box_ = nullptr;
    QCheckBox*      resample_result_box_          = nullptr;
    QDoubleSpinBox* resample_Q_std_static_edit_   = nullptr;
    QDoubleSpinBox* resample_Q_std_ground_edit_   = nullptr;
    QDoubleSpinBox* resample_Q_std_air_edit_      = nullptr;
    QDoubleSpinBox* resample_Q_std_unknown_edit_  = nullptr;
    QDoubleSpinBox* resample_dt_box_              = nullptr;
};
