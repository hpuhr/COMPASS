#include "probabilisticassociationwidget.h"
#include "probimmreconstructor.h"
#include "probimmreconstructorwidget.h"


#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>

ProbabilisticAssociationWidget::ProbabilisticAssociationWidget(
    ProbIMMReconstructor& reconstructor, ProbIMMReconstructorWidget& parent)
    : reconstructor_(reconstructor), parent_(parent)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* main_layout = new QVBoxLayout();


    QFormLayout* layout = new QFormLayout();

    max_time_diff_edit_ = new QSpinBox();
    max_time_diff_edit_->setRange(0, 300);
    connect(max_time_diff_edit_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ProbabilisticAssociationWidget::maxTimeDiffEditedSlot);

    layout->addRow(new QLabel("Maximum Comparison Time Difference [s]"), max_time_diff_edit_);


    max_time_diff_tracker_edit_ = new QSpinBox();
    max_time_diff_tracker_edit_->setRange(0, 300);
    connect(max_time_diff_tracker_edit_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ProbabilisticAssociationWidget::maxTimeDiffTrackerEditedSlot);

    layout->addRow(new QLabel("Maximum Track Time Difference [s]"), max_time_diff_tracker_edit_);


    max_altitude_diff_edit_ = new QSpinBox();
    max_altitude_diff_edit_->setRange(0, 3000);
    connect(max_altitude_diff_edit_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ProbabilisticAssociationWidget::maxAltitudeDiffEditedSlot);

    layout->addRow(new QLabel("Maximum Altitude Difference [ft]"), max_altitude_diff_edit_);

    max_mahalanobis_sec_verified_dist_edit_ = new QDoubleSpinBox();
    max_mahalanobis_sec_verified_dist_edit_->setRange(0, 1000);
    max_mahalanobis_sec_verified_dist_edit_->setDecimals(2);
    connect(max_mahalanobis_sec_verified_dist_edit_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ProbabilisticAssociationWidget::maxMahalanobisSecVerifiedDistEditedSlot);

    layout->addRow( new QLabel("Maximum Mahalanobis Distance for Secondary Matches [pos. sigma]"),
                   max_mahalanobis_sec_verified_dist_edit_);

    max_mahalanobis_sec_unknown_dist_edit_ = new QDoubleSpinBox();
    max_mahalanobis_sec_unknown_dist_edit_->setRange(0, 1000);
    max_mahalanobis_sec_unknown_dist_edit_->setDecimals(2);
    connect(max_mahalanobis_sec_unknown_dist_edit_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ProbabilisticAssociationWidget::maxMahalanobisSecUnknownDistEditedSlot);

    layout->addRow( new QLabel("Maximum Mahalanobis Distance for Unverified Matches [pos. sigma]"),
                   max_mahalanobis_sec_unknown_dist_edit_);

    max_tgt_est_std_dev_edit_ = new QDoubleSpinBox();
    max_tgt_est_std_dev_edit_->setRange(0, 10000);
    max_tgt_est_std_dev_edit_->setDecimals(2);
    connect(max_tgt_est_std_dev_edit_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ProbabilisticAssociationWidget::maxTgtEstStdDevEditedSlot);

    layout->addRow( new QLabel("Maximimum Tracking Std.Dev. for Position Matching [m]"),
                   max_tgt_est_std_dev_edit_);


//    QDoubleSpinBox* max_sum_est_std_dev_edit_{nullptr};

    max_sum_est_std_dev_edit_ = new QDoubleSpinBox();
    max_sum_est_std_dev_edit_->setRange(0, 10000);
    max_sum_est_std_dev_edit_->setDecimals(2);
    connect(max_sum_est_std_dev_edit_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ProbabilisticAssociationWidget::maxSumEstStdDevEditedSlot);

    layout->addRow( new QLabel("Maximimum Sum Std.Dev. for Position Matching [m]"),
                   max_sum_est_std_dev_edit_);

//    QDoubleSpinBox* min_sum_est_std_dev_edit_{nullptr};

    min_sum_est_std_dev_edit_ = new QDoubleSpinBox();
    min_sum_est_std_dev_edit_->setRange(0, 10000);
    min_sum_est_std_dev_edit_->setDecimals(2);
    connect(min_sum_est_std_dev_edit_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ProbabilisticAssociationWidget::minSumEstStdDevEditedSlot);

    layout->addRow( new QLabel("Minimum Sum Std.Dev. for Position Matching [m]"),
                   min_sum_est_std_dev_edit_);

    main_layout->addLayout(layout);

    main_layout->addStretch();

    updateValues();

    setLayout(main_layout);
}

void ProbabilisticAssociationWidget::updateValues()
{
    //    QSpinBox* max_time_diff_edit_{nullptr};

    assert (max_time_diff_edit_);
    max_time_diff_edit_->setValue(reconstructor_.settings().max_time_diff_);

            //    QSpinBox* max_time_diff_tracker_edit_{nullptr};

    assert (max_time_diff_tracker_edit_);
    max_time_diff_tracker_edit_->setValue(reconstructor_.settings().track_max_time_diff_);

            //    float max_time_diff_ {5}; // sec
            //    float track_max_time_diff_ {300.0}; // sec

            //    QSpinBox* max_altitude_diff_edit_{nullptr};

    assert (max_altitude_diff_edit_);
    max_altitude_diff_edit_->setValue(reconstructor_.settings().max_altitude_diff_);

            //    float max_altitude_diff_ {300.0};

            //    QDoubleSpinBox* max_mahalanobis_sec_verified_dist_edit_{nullptr};

    assert (max_mahalanobis_sec_verified_dist_edit_);
    max_mahalanobis_sec_verified_dist_edit_->setValue(reconstructor_.settings().max_mahalanobis_sec_verified_dist_);

            //    QDoubleSpinBox* max_mahalanobis_sec_unknown_dist_edit_{nullptr};

    assert (max_mahalanobis_sec_unknown_dist_edit_);
    max_mahalanobis_sec_unknown_dist_edit_->setValue(reconstructor_.settings().max_mahalanobis_sec_unknown_dist_);

    //    QDoubleSpinBox* max_tgt_est_std_dev_edit_{nullptr};

    assert (max_tgt_est_std_dev_edit_);
    max_tgt_est_std_dev_edit_->setValue(reconstructor_.settings().max_tgt_est_std_dev_);

            //    float max_mahalanobis_sec_verified_dist_ {10.0};
            //    float max_mahalanobis_sec_unknown_dist_ {5.0};
            //    float max_tgt_est_std_dev_  {2000.0};


//    QDoubleSpinBox* max_sum_est_std_dev_edit_{nullptr};

    assert (max_sum_est_std_dev_edit_);
    max_sum_est_std_dev_edit_->setValue(reconstructor_.settings().max_sum_est_std_dev_);

//    QDoubleSpinBox* min_sum_est_std_dev_edit_{nullptr};

    assert (min_sum_est_std_dev_edit_);
    min_sum_est_std_dev_edit_->setValue(reconstructor_.settings().min_sum_est_std_dev_);
}

void ProbabilisticAssociationWidget::maxTimeDiffEditedSlot (int value)
{
    loginf << "SimpleReconstructorAssociationWidget: maxTimeDiffEditedSlot: value '" << value << "'";

    reconstructor_.settings().max_time_diff_ = value;
}
void ProbabilisticAssociationWidget::maxTimeDiffTrackerEditedSlot (int value)
{
    loginf << "SimpleReconstructorAssociationWidget: maxTimeDiffTrackerEditedSlot: value '" << value << "'";

    reconstructor_.settings().track_max_time_diff_ = value;
}

void ProbabilisticAssociationWidget::maxAltitudeDiffEditedSlot (int value)
{
    loginf << "SimpleReconstructorAssociationWidget: maxAltitudeDiffEditedSlot: value '" << value << "'";

    reconstructor_.settings().max_altitude_diff_ = value;
}

void ProbabilisticAssociationWidget::maxMahalanobisSecVerifiedDistEditedSlot (double value)
{
    loginf << "SimpleReconstructorAssociationWidget: maxMahalanobisSecVerifiedDistEditedSlot: value '" << value << "'";

    reconstructor_.settings().max_mahalanobis_sec_verified_dist_ = value;
}
void ProbabilisticAssociationWidget::maxMahalanobisSecUnknownDistEditedSlot (double value)
{
    loginf << "SimpleReconstructorAssociationWidget: maxMahalanobisSecUnknownDistEditedSlot: value '" << value << "'";

    reconstructor_.settings().max_mahalanobis_sec_unknown_dist_ = value;
}

void ProbabilisticAssociationWidget::maxTgtEstStdDevEditedSlot (double value)
{
    loginf << "SimpleReconstructorAssociationWidget: maxTgtEstStdDevEditedSlot: value '" << value << "'";

    reconstructor_.settings().max_tgt_est_std_dev_ = value;
}

void ProbabilisticAssociationWidget::maxSumEstStdDevEditedSlot (double value)
{
    loginf << "SimpleReconstructorAssociationWidget: maxSumEstStdDevEditedSlot: value '" << value << "'";

    reconstructor_.settings().max_sum_est_std_dev_ = value;
}
void ProbabilisticAssociationWidget::minSumEstStdDevEditedSlot (double value)
{
    loginf << "SimpleReconstructorAssociationWidget: minSumEstStdDevEditedSlot: value '" << value << "'";

    reconstructor_.settings().min_sum_est_std_dev_ = value;
}
