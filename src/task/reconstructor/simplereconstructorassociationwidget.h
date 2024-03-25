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
    void toggleAssociateNonModeSSlot();
//    void toggleCleanDubiousUtnsSlot();
//    void toggleMarkDubiousUtnsUnusedSlot();
//    void toggleCommentDubiousUtnsSlot();

    void maxTimeDiffTrackerEditedSlot (const QString& text);

    void maxTimeDiffSensorEditedSlot (const QString& text);
    void maxDistanceQuitTrackerEditedSlot (const QString& text);
    void maxDistanceDubiousTrackerEditedSlot (const QString& text);
    void maxPositionsDubiousTrackerEditedSlot (const QString& text);
    void maxDistanceAcceptableTrackerEditedSlot (const QString& text);
    void maxDistanceAcceptableSensorEditedSlot (const QString& text);
    void maxAltitudeDiffTrackerEditedSlot (const QString& text);
    void maxAltitudeDiffSensorEditedSlot (const QString& text);
    void probMinTimeOverlapTrackerEditedSlot (const QString& text);

    void minUpdatesTrackerEditedSlot (const QString& text);
    //void maxSpeedTrackerKtsEditedSlot (const QString& text);

    void contMaxTimeDiffTrackerEditedSlot (const QString& text);
    void contMaxDistanceAcceptableTrackerEditedSlot (const QString& text);

  signals:

  public:
    explicit SimpleReconstructorAssociationWidget(
        SimpleReconstructor& reconstructor, SimpleReconstructorWidget& parent);
    virtual ~SimpleReconstructorAssociationWidget();

    void update();

  private:

    SimpleReconstructor& reconstructor_;

    SimpleReconstructorWidget& parent_;

    QCheckBox* associate_non_mode_s_check_{nullptr};
//    QCheckBox* clean_dubious_utns_check_{nullptr};
//    QCheckBox* mark_dubious_utns_unused_check_{nullptr};
//    QCheckBox* comment_dubious_utns_check_{nullptr};

            // tracker
    QLineEdit* max_time_diff_tracker_edit_{nullptr};

    QLineEdit* max_distance_quit_tracker_edit_{nullptr};
    QLineEdit* max_distance_dubious_tracker_edit_{nullptr};
    QLineEdit* max_positions_dubious_tracker_edit_{nullptr};
    QLineEdit* max_distance_acceptable_tracker_edit_{nullptr};
    QLineEdit* max_altitude_diff_tracker_edit_{nullptr};
    QLineEdit* min_updates_tracker_edit_{nullptr};

    QLineEdit* prob_min_time_overlap_tracker_edit_{nullptr};
    //QLineEdit* max_speed_tracker_kts_edit_{nullptr};

    QLineEdit* cont_max_time_diff_tracker_edit_{nullptr};
    QLineEdit* cont_max_distance_acceptable_tracker_edit_{nullptr};

            // sensor
    QLineEdit* max_time_diff_sensor_edit_{nullptr};
    QLineEdit* max_distance_acceptable_sensor_edit_{nullptr};
    QLineEdit* max_altitude_diff_sensor_edit_{nullptr};
};
