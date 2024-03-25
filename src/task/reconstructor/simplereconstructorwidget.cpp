#include "simplereconstructorwidget.h"
#include "simplereconstructor.h"


#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace std;

SimpleReconstructorWidget::SimpleReconstructorWidget(SimpleReconstructor& reconstructor)
    : reconstructor_(reconstructor)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* main_layout = new QVBoxLayout();

    QGridLayout* layout = new QGridLayout();

    int row = 0;

            // common
    QLabel* common_label = new QLabel("Common Parameters");
    common_label->setFont(font_bold);
    layout->addWidget(common_label, row, 0);

//    ++row;
//    layout->addWidget(new QLabel("Associate Non-Mode S Data"), row, 0);

//    associate_non_mode_s_check_ = new QCheckBox ();
//    connect(associate_non_mode_s_check_, &QCheckBox::clicked,
//            this, &CreateAssociationsTaskWidget::toggleAssociateNonModeSSlot);
//    layout->addWidget(associate_non_mode_s_check_, row, 1);

//    ++row;
//    layout->addWidget(new QLabel("Clean Dubious UTNs"), row, 0);

//    clean_dubious_utns_check_ = new QCheckBox ();
//    connect(clean_dubious_utns_check_, &QCheckBox::clicked,
//            this, &CreateAssociationsTaskWidget::toggleCleanDubiousUtnsSlot);
//    layout->addWidget(clean_dubious_utns_check_, row, 1);

//    ++row;
//    layout->addWidget(new QLabel("Mark Dubious UTNs Unused"), row, 0);

//    mark_dubious_utns_unused_check_ = new QCheckBox ();
//    connect(mark_dubious_utns_unused_check_, &QCheckBox::clicked,
//            this, &CreateAssociationsTaskWidget::toggleMarkDubiousUtnsUnusedSlot);
//    layout->addWidget(mark_dubious_utns_unused_check_, row, 1);

//    ++row;
//    layout->addWidget(new QLabel("Comment Dubious UTNs"), row, 0);

//    comment_dubious_utns_check_ = new QCheckBox ();
//    connect(comment_dubious_utns_check_, &QCheckBox::clicked,
//            this, &CreateAssociationsTaskWidget::toggleCommentDubiousUtnsSlot);
//    layout->addWidget(comment_dubious_utns_check_, row, 1);

//            // tracker
//    ++row;
//    QLabel* tracker_label = new QLabel("Track/Track Association Parameters");
//    tracker_label->setFont(font_bold);
//    layout->addWidget(tracker_label, row, 0);


//            //    QLineEdit* max_time_diff_tracker_edit_{nullptr};
//    ++row;

//    layout->addWidget(new QLabel("Maximum Comparison Time Difference [s]"), row, 0);

//    max_time_diff_tracker_edit_ = new QLineEdit();
//    connect(max_time_diff_tracker_edit_, &QLineEdit::textEdited,
//            this, &CreateAssociationsTaskWidget::maxTimeDiffTrackerEditedSlot);
//    layout->addWidget(max_time_diff_tracker_edit_, row, 1);

//            //    QLineEdit* max_distance_quit_tracker_edit_{nullptr};
//    ++row;

//    layout->addWidget(new QLabel("Maximum Quit Distance [m]"), row, 0);

//    max_distance_quit_tracker_edit_ = new QLineEdit();
//    connect(max_distance_quit_tracker_edit_, &QLineEdit::textEdited,
//            this, &CreateAssociationsTaskWidget::maxDistanceQuitTrackerEditedSlot);
//    layout->addWidget(max_distance_quit_tracker_edit_, row, 1);

//            //    QLineEdit* max_distance_dubious_tracker_edit_{nullptr};
//    ++row;

//    layout->addWidget(new QLabel("Maximum Dubious Distance [m]"), row, 0);

//    max_distance_dubious_tracker_edit_ = new QLineEdit();
//    connect(max_distance_dubious_tracker_edit_, &QLineEdit::textEdited,
//            this, &CreateAssociationsTaskWidget::maxDistanceDubiousTrackerEditedSlot);
//    layout->addWidget(max_distance_dubious_tracker_edit_, row, 1);

//            //    QLineEdit* max_positions_dubious_tracker_edit_{nullptr};
//    ++row;

//    layout->addWidget(new QLabel("Maximum Dubious Positions [1]"), row, 0);

//    max_positions_dubious_tracker_edit_ = new QLineEdit();
//    connect(max_positions_dubious_tracker_edit_, &QLineEdit::textEdited,
//            this, &CreateAssociationsTaskWidget::maxPositionsDubiousTrackerEditedSlot);
//    layout->addWidget(max_positions_dubious_tracker_edit_, row, 1);

//            //    QLineEdit* max_distance_acceptable_tracker_edit_{nullptr};
//    ++row;

//    layout->addWidget(new QLabel("Maximum Acceptable Distance [m]"), row, 0);

//    max_distance_acceptable_tracker_edit_ = new QLineEdit();
//    connect(max_distance_acceptable_tracker_edit_, &QLineEdit::textEdited,
//            this, &CreateAssociationsTaskWidget::maxDistanceAcceptableTrackerEditedSlot);
//    layout->addWidget(max_distance_acceptable_tracker_edit_, row, 1);

//            //    QLineEdit* max_altitude_diff_tracker_edit_{nullptr};
//    ++row;

//    layout->addWidget(new QLabel("Maximum Altitude Difference [ft]"), row, 0);

//    max_altitude_diff_tracker_edit_ = new QLineEdit();
//    connect(max_altitude_diff_tracker_edit_, &QLineEdit::textEdited,
//            this, &CreateAssociationsTaskWidget::maxAltitudeDiffTrackerEditedSlot);
//    layout->addWidget(max_altitude_diff_tracker_edit_, row, 1);

//            //    QLineEdit* min_updates_tracker_edit_{nullptr};
//    ++row;

//    layout->addWidget(new QLabel("Minimum Updates [1]"), row, 0);

//    min_updates_tracker_edit_ = new QLineEdit();
//    connect(min_updates_tracker_edit_, &QLineEdit::textEdited,
//            this, &CreateAssociationsTaskWidget::minUpdatesTrackerEditedSlot);
//    layout->addWidget(min_updates_tracker_edit_, row, 1);

//            //    QLineEdit* prob_min_time_overlap_tracker_edit_{nullptr};
//    ++row;

//    layout->addWidget(new QLabel("Minimum Time Overlap Probability [0-1]"), row, 0);

//    prob_min_time_overlap_tracker_edit_ = new QLineEdit();
//    connect(prob_min_time_overlap_tracker_edit_, &QLineEdit::textEdited,
//            this, &CreateAssociationsTaskWidget::probMinTimeOverlapTrackerEditedSlot);
//    layout->addWidget(prob_min_time_overlap_tracker_edit_, row, 1);

//            //    QLineEdit* max_speed_tracker_kts_edit_{nullptr};
//    ++row;

//    layout->addWidget(new QLabel("Max Speed [kts]"), row, 0);

//    max_speed_tracker_kts_edit_ = new QLineEdit();
//    connect(max_speed_tracker_kts_edit_, &QLineEdit::textEdited,
//            this, &CreateAssociationsTaskWidget::maxSpeedTrackerKtsEditedSlot);
//    layout->addWidget(max_speed_tracker_kts_edit_, row, 1);

//            // QLineEdit* cont_max_time_diff_tracker_edit_{nullptr};
//    ++row;

//    layout->addWidget(new QLabel("Maximum Continuation Time Difference [s]"), row, 0);

//    cont_max_time_diff_tracker_edit_ = new QLineEdit();
//    connect(cont_max_time_diff_tracker_edit_, &QLineEdit::textEdited,
//            this, &CreateAssociationsTaskWidget::contMaxTimeDiffTrackerEditedSlot);
//    layout->addWidget(cont_max_time_diff_tracker_edit_, row, 1);

//            // QLineEdit* cont_max_distance_acceptable_tracker_edit_{nullptr};
//    ++row;

//    layout->addWidget(new QLabel("Maximum Acceptable Continuation Distance [m]"), row, 0);

//    cont_max_distance_acceptable_tracker_edit_ = new QLineEdit();
//    connect(cont_max_distance_acceptable_tracker_edit_, &QLineEdit::textEdited,
//            this, &CreateAssociationsTaskWidget::contMaxDistanceAcceptableTrackerEditedSlot);
//    layout->addWidget(cont_max_distance_acceptable_tracker_edit_, row, 1);

//            // sensor
//    ++row;
//    QLabel* sensor_label = new QLabel("Sensor/Track Association Parameters");
//    sensor_label->setFont(font_bold);
//    layout->addWidget(sensor_label, row, 0);

//            //    QLineEdit* max_time_diff_sensor_edit_{nullptr};
//    ++row;

//    layout->addWidget(new QLabel("Maximum Comparison Time Difference [s]"), row, 0);

//    max_time_diff_sensor_edit_ = new QLineEdit();
//    connect(max_time_diff_sensor_edit_, &QLineEdit::textEdited,
//            this, &CreateAssociationsTaskWidget::maxTimeDiffSensorEditedSlot);
//    layout->addWidget(max_time_diff_sensor_edit_, row, 1);

//            //    QLineEdit* max_distance_acceptable_sensor_edit_{nullptr};
//    ++row;

//    layout->addWidget(new QLabel("Maximum Acceptable Distance [m]"), row, 0);

//    max_distance_acceptable_sensor_edit_ = new QLineEdit();
//    connect(max_distance_acceptable_sensor_edit_, &QLineEdit::textEdited,
//            this, &CreateAssociationsTaskWidget::maxDistanceAcceptableSensorEditedSlot);
//    layout->addWidget(max_distance_acceptable_sensor_edit_, row, 1);


//            //    QLineEdit* max_altitude_diff_sensor_edit_{nullptr};
//    ++row;

//    layout->addWidget(new QLabel("Maximum Altitude Difference [ft]"), row, 0);

//    max_altitude_diff_sensor_edit_ = new QLineEdit();
//    connect(max_altitude_diff_sensor_edit_, &QLineEdit::textEdited,
//            this, &CreateAssociationsTaskWidget::maxAltitudeDiffSensorEditedSlot);
//    layout->addWidget(max_altitude_diff_sensor_edit_, row, 1);

    update();

    setContentsMargins(0, 0, 0, 0);

    main_layout->addLayout(layout);
    main_layout->addStretch();

    //expertModeChangedSlot();

    setLayout(main_layout);
}

SimpleReconstructorWidget::~SimpleReconstructorWidget()
{

}

void SimpleReconstructorWidget::update()
{

}

void SimpleReconstructorWidget::updateSlot()
{
    update();
}
