#pragma once

#pragma once

#include <QWidget>

class ReconstructorBase;

class QComboBox;
class QSpinBox;
class QLineEdit;
class QCheckBox;

/**
*/
class ReconstructorMainWidget : public QWidget
{
public:
    explicit ReconstructorMainWidget(ReconstructorBase& reconstructor, QWidget *parent = nullptr);
    virtual ~ReconstructorMainWidget();

    void updateValues();

protected:
    ReconstructorBase& reconstructor_;

    QLineEdit* ds_name_edit_      = nullptr; 
    QSpinBox*  ds_sac_box_        = nullptr;
    QSpinBox*  ds_sic_box_        = nullptr;
    QComboBox* ds_line_combo_     = nullptr;

    QSpinBox*  slice_length_box_  = nullptr;
    QSpinBox*  slice_overlap_box_ = nullptr;

    QCheckBox* delete_refs_box_   = nullptr;
    QCheckBox* skip_reference_data_writing_box_   = nullptr;
};
