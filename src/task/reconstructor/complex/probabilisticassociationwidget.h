#pragma once

#include <QWidget>

class ProbabilisticReconstructor;
class ProbabilisticReconstructorWidget;

class ProbabilisticAssociationWidget : public QWidget
{
    Q_OBJECT

  signals:

  public slots:

  public:
    explicit ProbabilisticAssociationWidget(QWidget *parent = nullptr);

    //    float max_time_diff_ {5}; // sec
    //    float track_max_time_diff_ {300.0}; // sec

    //    float max_altitude_diff_ {300.0};

    //    float max_mahalanobis_sec_verified_dist_ {10.0};
    //    float max_mahalanobis_sec_unknown_dist_ {5.0};
    //    float max_tgt_est_std_dev_  {2000.0};

};


