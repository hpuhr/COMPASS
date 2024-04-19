#pragma once

#include <QWidget>

class ProbIMMReconstructor;
class ProbIMMReconstructorWidget;

class QSpinBox;
class QDoubleSpinBox;

class ProbabilisticAssociationWidget : public QWidget
{
    Q_OBJECT

  signals:

  public slots:

    void maxTimeDiffEditedSlot (int value);
    void maxTimeDiffTrackerEditedSlot (int value);

    void maxAltitudeDiffEditedSlot (int value);

    void maxMahalanobisSecVerifiedDistEditedSlot (double value);
    void maxMahalanobisSecUnknownDistEditedSlot (double value);

    void maxTgtEstStdDevEditedSlot (double value);

  public:
    explicit ProbabilisticAssociationWidget(
        ProbIMMReconstructor& reconstructor, ProbIMMReconstructorWidget& parent);

    void updateValues();

  private:

    ProbIMMReconstructor& reconstructor_;

    ProbIMMReconstructorWidget& parent_;

    QSpinBox* max_time_diff_edit_{nullptr};
    QSpinBox* max_time_diff_tracker_edit_{nullptr};

            //    float max_time_diff_ {15}; // sec
            //    float track_max_time_diff_ {300.0}; // sec

    QSpinBox* max_altitude_diff_edit_{nullptr};

            //    float max_altitude_diff_ {300.0};

    QDoubleSpinBox* max_mahalanobis_sec_verified_dist_edit_{nullptr};
    QDoubleSpinBox* max_mahalanobis_sec_unknown_dist_edit_{nullptr};
    QDoubleSpinBox* max_tgt_est_std_dev_edit_{nullptr};

            //    float max_mahalanobis_sec_verified_dist_ {10.0};
            //    float max_mahalanobis_sec_unknown_dist_ {5.0};
            //    float max_tgt_est_std_dev_  {2000.0};

};


