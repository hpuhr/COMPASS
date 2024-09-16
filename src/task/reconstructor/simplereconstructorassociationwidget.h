#pragma once

#include <QWidget>

class SimpleReconstructor;
class SimpleReconstructorWidget;

class QLineEdit;
class QCheckBox;

class SimpleReconstructorAssociationWidget : public QWidget
{
    Q_OBJECT

  public slots:
//    void toggleAssociateNonModeSSlot();
//    void toggleCleanDubiousUtnsSlot();
//    void toggleMarkDubiousUtnsUnusedSlot();
//    void toggleCommentDubiousUtnsSlot();

    void maxTimeDiffEditedSlot (const QString& text);

    //void maxTimeDiffSensorEditedSlot (const QString& text);
    void maxDistanceQuitEditedSlot (const QString& text);
    void maxDistanceDubiousEditedSlot (const QString& text);
    //void maxPositionsDubiousEditedSlot (const QString& text);
    void maxDistanceAcceptableEditedSlot (const QString& text);
    //void maxDistanceAcceptableSensorEditedSlot (const QString& text);
    void maxAltitudeDiffEditedSlot (const QString& text);
    //void maxAltitudeDiffSensorEditedSlot (const QString& text);
    void probMinTimeOverlapEditedSlot (const QString& text);

    void minUpdatesEditedSlot (const QString& text);
    //void maxSpeedTrackerKtsEditedSlot (const QString& text);

    //void contMaxTimeDiffTrackerEditedSlot (const QString& text);
    //void contMaxDistanceAcceptableTrackerEditedSlot (const QString& text);

  signals:

  public:
    explicit SimpleReconstructorAssociationWidget(
        SimpleReconstructor& reconstructor, SimpleReconstructorWidget& parent);
    virtual ~SimpleReconstructorAssociationWidget();

    void updateValues();

  private:

    SimpleReconstructor& reconstructor_;

    SimpleReconstructorWidget& parent_;

//    QCheckBox* associate_non_mode_s_check_{nullptr};
//    QCheckBox* clean_dubious_utns_check_{nullptr};
//    QCheckBox* mark_dubious_utns_unused_check_{nullptr};
//    QCheckBox* comment_dubious_utns_check_{nullptr};

            // tracker
    QLineEdit* max_time_diff_edit_{nullptr};

    QLineEdit* max_distance_quit_edit_{nullptr};
    QLineEdit* max_distance_dubious_edit_{nullptr};
    QLineEdit* max_positions_dubious_edit_{nullptr};
    QLineEdit* max_distance_acceptable_edit_{nullptr};
    QLineEdit* max_altitude_diff_edit_{nullptr};
    QLineEdit* min_updates_edit_{nullptr};

    QLineEdit* prob_min_time_overlap_edit_{nullptr};
    //QLineEdit* max_speed_tracker_kts_edit_{nullptr};

    //QLineEdit* cont_max_time_diff_tracker_edit_{nullptr};
    //QLineEdit* cont_max_distance_acceptable_tracker_edit_{nullptr};

            // sensor
//    QLineEdit* max_time_diff_sensor_edit_{nullptr};
//    QLineEdit* max_distance_acceptable_sensor_edit_{nullptr};
//    QLineEdit* max_altitude_diff_sensor_edit_{nullptr};
};
