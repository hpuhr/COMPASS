#pragma once

#include <QWidget>

class SimpleReconstructor;
class SimpleReconstructorWidget;

class QLineEdit;
class QCheckBox;
class QSpinBox;

class SimpleReconstructorAssociationWidget : public QWidget
{
    Q_OBJECT

public slots:

    void maxTimeDiffEditedSlot (const QString& text);
    void maxTimeDiffTrackerEditedSlot (int value);

    void maxDistanceNotOKEditedSlot (const QString& text);
    void maxDistanceDubiousEditedSlot (const QString& text);
    void maxDistanceAcceptableEditedSlot (const QString& text);
    void maxAltitudeDiffEditedSlot (const QString& text);
    void probMinTimeOverlapEditedSlot (const QString& text);

    void minUpdatesEditedSlot (const QString& text);

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

    // tracker
    QLineEdit* max_time_diff_edit_{nullptr};
    QSpinBox* max_time_diff_tracker_edit_{nullptr};

    QLineEdit* max_distance_notok_edit_{nullptr};
    QLineEdit* max_distance_dubious_edit_{nullptr};
    QLineEdit* max_positions_dubious_edit_{nullptr};
    QLineEdit* max_distance_acceptable_edit_{nullptr};
    QLineEdit* max_altitude_diff_edit_{nullptr};

    QCheckBox* do_track_number_disassociate_using_distance_box_ {nullptr};
    QSpinBox* tn_disassoc_distance_factor_edit_{nullptr};

    QLineEdit* min_updates_edit_{nullptr};

    QLineEdit* prob_min_time_overlap_edit_{nullptr};
};
