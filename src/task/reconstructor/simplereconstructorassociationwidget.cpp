#include "simplereconstructorassociationwidget.h"
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

SimpleReconstructorAssociationWidget::SimpleReconstructorAssociationWidget(
    SimpleReconstructor& reconstructor, SimpleReconstructorWidget& parent)
    : reconstructor_(reconstructor), parent_(parent)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* main_layout = new QVBoxLayout();


    int row = 0;

    QGridLayout* layout = new QGridLayout();

            // common
    QLabel* common_label = new QLabel("Common Parameters");
    common_label->setFont(font_bold);
    layout->addWidget(common_label, row, 0);

    ++row;
    layout->addWidget(new QLabel("Associate Non-Mode S Data"), row, 0);

    associate_non_mode_s_check_ = new QCheckBox ();
    connect(associate_non_mode_s_check_, &QCheckBox::clicked,
            this, &SimpleReconstructorAssociationWidget::toggleAssociateNonModeSSlot);
    layout->addWidget(associate_non_mode_s_check_, row, 1);

//    ++row;
//    layout->addWidget(new QLabel("Clean Dubious UTNs"), row, 0);

//    clean_dubious_utns_check_ = new QCheckBox ();
//    connect(clean_dubious_utns_check_, &QCheckBox::clicked,
//            this, &SimpleReconstructorAssociationWidget::toggleCleanDubiousUtnsSlot);
//    layout->addWidget(clean_dubious_utns_check_, row, 1);

//    ++row;
//    layout->addWidget(new QLabel("Mark Dubious UTNs Unused"), row, 0);

//    mark_dubious_utns_unused_check_ = new QCheckBox ();
//    connect(mark_dubious_utns_unused_check_, &QCheckBox::clicked,
//            this, &SimpleReconstructorAssociationWidget::toggleMarkDubiousUtnsUnusedSlot);
//    layout->addWidget(mark_dubious_utns_unused_check_, row, 1);

//    ++row;
//    layout->addWidget(new QLabel("Comment Dubious UTNs"), row, 0);

//    comment_dubious_utns_check_ = new QCheckBox ();
//    connect(comment_dubious_utns_check_, &QCheckBox::clicked,
//            this, &SimpleReconstructorAssociationWidget::toggleCommentDubiousUtnsSlot);
//    layout->addWidget(comment_dubious_utns_check_, row, 1);

            // tracker
    ++row;
    QLabel* tracker_label = new QLabel("Track/Track Association Parameters");
    tracker_label->setFont(font_bold);
    layout->addWidget(tracker_label, row, 0);


            //    QLineEdit* max_time_diff_tracker_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Maximum Comparison Time Difference [s]"), row, 0);

    max_time_diff_tracker_edit_ = new QLineEdit();
    connect(max_time_diff_tracker_edit_, &QLineEdit::textEdited,
            this, &SimpleReconstructorAssociationWidget::maxTimeDiffEditedSlot);
    layout->addWidget(max_time_diff_tracker_edit_, row, 1);

            //    QLineEdit* max_distance_quit_tracker_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Maximum Quit Distance [m]"), row, 0);

    max_distance_quit_tracker_edit_ = new QLineEdit();
    connect(max_distance_quit_tracker_edit_, &QLineEdit::textEdited,
            this, &SimpleReconstructorAssociationWidget::maxDistanceQuitTrackerEditedSlot);
    layout->addWidget(max_distance_quit_tracker_edit_, row, 1);

            //    QLineEdit* max_distance_dubious_tracker_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Maximum Dubious Distance [m]"), row, 0);

    max_distance_dubious_tracker_edit_ = new QLineEdit();
    connect(max_distance_dubious_tracker_edit_, &QLineEdit::textEdited,
            this, &SimpleReconstructorAssociationWidget::maxDistanceDubiousTrackerEditedSlot);
    layout->addWidget(max_distance_dubious_tracker_edit_, row, 1);

            //    QLineEdit* max_positions_dubious_tracker_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Maximum Dubious Positions [1]"), row, 0);

    max_positions_dubious_tracker_edit_ = new QLineEdit();
    connect(max_positions_dubious_tracker_edit_, &QLineEdit::textEdited,
            this, &SimpleReconstructorAssociationWidget::maxPositionsDubiousTrackerEditedSlot);
    layout->addWidget(max_positions_dubious_tracker_edit_, row, 1);

            //    QLineEdit* max_distance_acceptable_tracker_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Maximum Acceptable Distance [m]"), row, 0);

    max_distance_acceptable_tracker_edit_ = new QLineEdit();
    connect(max_distance_acceptable_tracker_edit_, &QLineEdit::textEdited,
            this, &SimpleReconstructorAssociationWidget::maxDistanceAcceptableTrackerEditedSlot);
    layout->addWidget(max_distance_acceptable_tracker_edit_, row, 1);

            //    QLineEdit* max_altitude_diff_tracker_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Maximum Altitude Difference [ft]"), row, 0);

    max_altitude_diff_tracker_edit_ = new QLineEdit();
    connect(max_altitude_diff_tracker_edit_, &QLineEdit::textEdited,
            this, &SimpleReconstructorAssociationWidget::maxAltitudeDiffTrackerEditedSlot);
    layout->addWidget(max_altitude_diff_tracker_edit_, row, 1);

            //    QLineEdit* min_updates_tracker_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Minimum Updates [1]"), row, 0);

    min_updates_tracker_edit_ = new QLineEdit();
    connect(min_updates_tracker_edit_, &QLineEdit::textEdited,
            this, &SimpleReconstructorAssociationWidget::minUpdatesTrackerEditedSlot);
    layout->addWidget(min_updates_tracker_edit_, row, 1);

            //    QLineEdit* prob_min_time_overlap_tracker_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Minimum Time Overlap Probability [0-1]"), row, 0);

    prob_min_time_overlap_tracker_edit_ = new QLineEdit();
    connect(prob_min_time_overlap_tracker_edit_, &QLineEdit::textEdited,
            this, &SimpleReconstructorAssociationWidget::probMinTimeOverlapTrackerEditedSlot);
    layout->addWidget(prob_min_time_overlap_tracker_edit_, row, 1);

            //    QLineEdit* max_speed_tracker_kts_edit_{nullptr};
    ++row;

//    layout->addWidget(new QLabel("Max Speed [kts]"), row, 0);

//    max_speed_tracker_kts_edit_ = new QLineEdit();
//    connect(max_speed_tracker_kts_edit_, &QLineEdit::textEdited,
//            this, &SimpleReconstructorAssociationWidget::maxSpeedTrackerKtsEditedSlot);
//    layout->addWidget(max_speed_tracker_kts_edit_, row, 1);

//            // QLineEdit* cont_max_time_diff_tracker_edit_{nullptr};
//    ++row;

    layout->addWidget(new QLabel("Maximum Continuation Time Difference [s]"), row, 0);

    cont_max_time_diff_tracker_edit_ = new QLineEdit();
    connect(cont_max_time_diff_tracker_edit_, &QLineEdit::textEdited,
            this, &SimpleReconstructorAssociationWidget::contMaxTimeDiffTrackerEditedSlot);
    layout->addWidget(cont_max_time_diff_tracker_edit_, row, 1);

            // QLineEdit* cont_max_distance_acceptable_tracker_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Maximum Acceptable Continuation Distance [m]"), row, 0);

    cont_max_distance_acceptable_tracker_edit_ = new QLineEdit();
    connect(cont_max_distance_acceptable_tracker_edit_, &QLineEdit::textEdited,
            this, &SimpleReconstructorAssociationWidget::contMaxDistanceAcceptableTrackerEditedSlot);
    layout->addWidget(cont_max_distance_acceptable_tracker_edit_, row, 1);

            // sensor
    ++row;
    QLabel* sensor_label = new QLabel("Sensor/Track Association Parameters");
    sensor_label->setFont(font_bold);
    layout->addWidget(sensor_label, row, 0);

            //    QLineEdit* max_time_diff_sensor_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Maximum Comparison Time Difference [s]"), row, 0);

    max_time_diff_sensor_edit_ = new QLineEdit();
    connect(max_time_diff_sensor_edit_, &QLineEdit::textEdited,
            this, &SimpleReconstructorAssociationWidget::maxTimeDiffSensorEditedSlot);
    layout->addWidget(max_time_diff_sensor_edit_, row, 1);

            //    QLineEdit* max_distance_acceptable_sensor_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Maximum Acceptable Distance [m]"), row, 0);

    max_distance_acceptable_sensor_edit_ = new QLineEdit();
    connect(max_distance_acceptable_sensor_edit_, &QLineEdit::textEdited,
            this, &SimpleReconstructorAssociationWidget::maxDistanceAcceptableSensorEditedSlot);
    layout->addWidget(max_distance_acceptable_sensor_edit_, row, 1);


            //    QLineEdit* max_altitude_diff_sensor_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Maximum Altitude Difference [ft]"), row, 0);

    max_altitude_diff_sensor_edit_ = new QLineEdit();
    connect(max_altitude_diff_sensor_edit_, &QLineEdit::textEdited,
            this, &SimpleReconstructorAssociationWidget::maxAltitudeDiffSensorEditedSlot);
    layout->addWidget(max_altitude_diff_sensor_edit_, row, 1);

    main_layout->addLayout(layout);

    main_layout->addStretch();

            //expertModeChangedSlot();

    setLayout(main_layout);
}

SimpleReconstructorAssociationWidget::~SimpleReconstructorAssociationWidget()
{

}

void SimpleReconstructorAssociationWidget::updateValues()
{
    //    QCheckBox* associate_non_mode_s_check_{nullptr};
    assert (associate_non_mode_s_check_);
    associate_non_mode_s_check_->setChecked(reconstructor_.settings().associate_non_mode_s_);

//            //    QCheckBox* clean_dubious_utns_check_{nullptr};
//    assert (clean_dubious_utns_check_);
//    clean_dubious_utns_check_->setChecked(reconstructor_.settings().cleanDubiousUtns());

//            //    QCheckBox* mark_dubious_utns_unused_check_{nullptr};
//    assert (mark_dubious_utns_unused_check_);
//    mark_dubious_utns_unused_check_->setChecked(reconstructor_.settings().markDubiousUtnsUnused());

//            //    QCheckBox* comment_dubious_utns_check_{nullptr};
//    assert (comment_dubious_utns_check_);
//    comment_dubious_utns_check_->setChecked(reconstructor_.settings().commentDubiousUtns());


            // tracker
            //    QLineEdit* max_time_diff_tracker_edit_{nullptr};
    assert (max_time_diff_tracker_edit_);
    max_time_diff_tracker_edit_->setText(QString::number(reconstructor_.settings().max_time_diff_));

            //    QLineEdit* max_distance_quit_tracker_edit_{nullptr};
    assert (max_distance_quit_tracker_edit_);
    max_distance_quit_tracker_edit_->setText(QString::number(reconstructor_.settings().max_distance_quit_tracker_));

            //    QLineEdit* max_distance_dubious_tracker_edit_{nullptr};
    assert (max_distance_dubious_tracker_edit_);
    max_distance_dubious_tracker_edit_->setText(QString::number(
        reconstructor_.settings().max_distance_dubious_tracker_));

            //    QLineEdit* max_positions_dubious_tracker_edit_{nullptr};
    assert (max_positions_dubious_tracker_edit_);
    max_positions_dubious_tracker_edit_->setText(
        QString::number(reconstructor_.settings().max_positions_dubious_tracker_));

            //    QLineEdit* max_distance_acceptable_tracker_edit_{nullptr};
    assert (max_distance_acceptable_tracker_edit_);
    max_distance_acceptable_tracker_edit_->setText(
        QString::number(reconstructor_.settings().max_distance_acceptable_tracker_));

            //    QLineEdit* max_altitude_diff_tracker_edit_{nullptr};
    assert (max_altitude_diff_tracker_edit_);
    max_altitude_diff_tracker_edit_->setText(QString::number(reconstructor_.settings().max_altitude_diff_tracker_));

            //    QLineEdit* min_updates_tracker_edit_{nullptr};
    assert (min_updates_tracker_edit_);
    min_updates_tracker_edit_->setText(QString::number(reconstructor_.settings().min_updates_tracker_));

            //    QLineEdit* prob_min_time_overlap_tracker_edit_{nullptr};
    assert (prob_min_time_overlap_tracker_edit_);
    prob_min_time_overlap_tracker_edit_->setText(
        QString::number(reconstructor_.settings().prob_min_time_overlap_tracker_));

            //    QLineEdit* max_speed_tracker_kts_edit_{nullptr};
//    assert (max_speed_tracker_kts_edit_);
//    max_speed_tracker_kts_edit_->setText(QString::number(reconstructor_.settings().max_speed_tracker_kts_));

            //    QLineEdit* cont_max_time_diff_tracker_edit_{nullptr};
    assert (cont_max_time_diff_tracker_edit_);
    cont_max_time_diff_tracker_edit_->setText(QString::number(reconstructor_.settings().cont_max_time_diff_tracker_));

            //    QLineEdit* cont_max_distance_acceptable_tracker_edit_{nullptr};
    assert (cont_max_distance_acceptable_tracker_edit_);
    cont_max_distance_acceptable_tracker_edit_->setText(
        QString::number(reconstructor_.settings().cont_max_distance_acceptable_tracker_));


            // sensor
            //    QLineEdit* max_time_diff_sensor_edit_{nullptr};
    assert (max_time_diff_sensor_edit_);
    max_time_diff_sensor_edit_->setText(QString::number(reconstructor_.settings().max_time_diff_sensor_));

            //    QLineEdit* max_distance_acceptable_sensor_edit_{nullptr};
    assert (max_distance_acceptable_sensor_edit_);
    max_distance_acceptable_sensor_edit_->setText(
        QString::number(reconstructor_.settings().max_distance_acceptable_sensor_));

            //    QLineEdit* max_altitude_diff_sensor_edit_{nullptr};
    assert (max_altitude_diff_sensor_edit_);
    max_altitude_diff_sensor_edit_->setText(QString::number(reconstructor_.settings().max_altitude_diff_sensor_));
}

void SimpleReconstructorAssociationWidget::toggleAssociateNonModeSSlot()
{
    assert (associate_non_mode_s_check_);
    reconstructor_.settings().associate_non_mode_s_ =
        associate_non_mode_s_check_->checkState() == Qt::Checked;
}

//void SimpleReconstructorAssociationWidget::toggleCleanDubiousUtnsSlot()
//{
//    assert (clean_dubious_utns_check_);
//    reconstructor_.settings().cleanDubiousUtns(clean_dubious_utns_check_->checkState() == Qt::Checked);
//}

//void SimpleReconstructorAssociationWidget::toggleMarkDubiousUtnsUnusedSlot()
//{
//    assert (mark_dubious_utns_unused_check_);
//    reconstructor_.settings().markDubiousUtnsUnused(mark_dubious_utns_unused_check_->checkState() == Qt::Checked);
//}

//void SimpleReconstructorAssociationWidget::toggleCommentDubiousUtnsSlot()
//{
//    assert (comment_dubious_utns_check_);
//    reconstructor_.settings().commentDubiousUtns(comment_dubious_utns_check_->checkState() == Qt::Checked);
//}


void SimpleReconstructorAssociationWidget::maxTimeDiffEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "SimpleReconstructorAssociationWidget: maxTimeDiffEditedSlot: value '" << value_str << "'";

    bool ok;

    double value = text.toDouble(&ok);

    if (ok)
        reconstructor_.settings().max_time_diff_ = value;
    else
        logwrn << "SimpleReconstructorAssociationWidget: maxTimeDiffEditedSlot: unable to parse value '"
               << value_str << "'";
}

void SimpleReconstructorAssociationWidget::maxTimeDiffSensorEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "SimpleReconstructorAssociationWidget: maxTimeDiffSensorEditedSlot: value '" << value_str << "'";

    bool ok;

    double value = text.toDouble(&ok);

    if (ok)
        reconstructor_.settings().max_time_diff_sensor_ = value;
    else
        logwrn << "SimpleReconstructorAssociationWidget: maxTimeDiffSensorEditedSlot: unable to parse value '"
               << value_str << "'";
}

void SimpleReconstructorAssociationWidget::maxDistanceQuitTrackerEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "SimpleReconstructorAssociationWidget: maxDistanceQuitTrackerEditedSlot: value '" << value_str << "'";

    bool ok;

    double value = text.toDouble(&ok);

    if (ok)
        reconstructor_.settings().max_distance_quit_tracker_ = value;
    else
        logwrn << "SimpleReconstructorAssociationWidget: maxDistanceQuitTrackerEditedSlot: unable to parse value '"
               << value_str << "'";
}

void SimpleReconstructorAssociationWidget::maxDistanceDubiousTrackerEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "SimpleReconstructorAssociationWidget: maxDistanceDubiousTrackerEditedSlot: value '" << value_str << "'";

    bool ok;

    double value = text.toDouble(&ok);

    if (ok)
        reconstructor_.settings().max_distance_dubious_tracker_ = value;
    else
        logwrn << "SimpleReconstructorAssociationWidget: maxDistanceDubiousTrackerEditedSlot: unable to parse value '"
               << value_str << "'";
}

void SimpleReconstructorAssociationWidget::maxPositionsDubiousTrackerEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "SimpleReconstructorAssociationWidget: maxPositionsDubiousTrackerEditedSlot: value '" << value_str << "'";

    bool ok;

    unsigned int value = text.toUInt(&ok);

    if (ok)
        reconstructor_.settings().max_positions_dubious_tracker_ = value;
    else
        logwrn << "SimpleReconstructorAssociationWidget: maxPositionsDubiousTrackerEditedSlot: unable to parse value '"
               << value_str << "'";
}

void SimpleReconstructorAssociationWidget::maxDistanceAcceptableTrackerEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "SimpleReconstructorAssociationWidget: maxDistanceAcceptableTrackerEditedSlot: value '" << value_str << "'";

    bool ok;

    double value = text.toDouble(&ok);

    if (ok)
        reconstructor_.settings().max_distance_acceptable_tracker_ = value;
    else
        logwrn << "SimpleReconstructorAssociationWidget: maxDistanceAcceptableTrackerEditedSlot: unable to parse value '"
               << value_str << "'";
}

void SimpleReconstructorAssociationWidget::maxDistanceAcceptableSensorEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "SimpleReconstructorAssociationWidget: maxDistanceAcceptableSensorEditedSlot: value '" << value_str << "'";

    bool ok;

    double value = text.toDouble(&ok);

    if (ok)
        reconstructor_.settings().max_distance_acceptable_sensor_ = value;
    else
        logwrn << "SimpleReconstructorAssociationWidget: maxDistanceAcceptableSensorEditedSlot: unable to parse value '"
               << value_str << "'";
}

void SimpleReconstructorAssociationWidget::maxAltitudeDiffTrackerEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "SimpleReconstructorAssociationWidget: maxAltitudeDiffTrackerEditedSlot: value '" << value_str << "'";

    bool ok;

    double value = text.toDouble(&ok);

    if (ok)
        reconstructor_.settings().max_altitude_diff_tracker_ = value;
    else
        logwrn << "SimpleReconstructorAssociationWidget: maxAltitudeDiffTrackerEditedSlot: unable to parse value '"
               << value_str << "'";
}

void SimpleReconstructorAssociationWidget::maxAltitudeDiffSensorEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "SimpleReconstructorAssociationWidget: maxAltitudeDiffSensorEditedSlot: value '" << value_str << "'";

    bool ok;

    double value = text.toDouble(&ok);

    if (ok)
        reconstructor_.settings().max_altitude_diff_sensor_ = value;
    else
        logwrn << "SimpleReconstructorAssociationWidget: maxAltitudeDiffSensorEditedSlot: unable to parse value '"
               << value_str << "'";
}

void SimpleReconstructorAssociationWidget::probMinTimeOverlapTrackerEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "SimpleReconstructorAssociationWidget: probMinTimeOverlapTrackerEditedSlot: value '" << value_str << "'";

    bool ok;

    double value = text.toDouble(&ok);

    if (ok)
        reconstructor_.settings().prob_min_time_overlap_tracker_ = value;
    else
        logwrn << "SimpleReconstructorAssociationWidget: probMinTimeOverlapTrackerEditedSlot: unable to parse value '"
               << value_str << "'";
}

void SimpleReconstructorAssociationWidget::minUpdatesTrackerEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "SimpleReconstructorAssociationWidget: minUpdatesTrackerEditedSlot: value '" << value_str << "'";

    bool ok;

    unsigned int value = text.toUInt(&ok);

    if (ok)
        reconstructor_.settings().min_updates_tracker_ = value;
    else
        logwrn << "SimpleReconstructorAssociationWidget: minUpdatesTrackerEditedSlot: unable to parse value '"
               << value_str << "'";
}

//void SimpleReconstructorAssociationWidget::maxSpeedTrackerKtsEditedSlot (const QString& text)
//{
//    string value_str = text.toStdString();

//    loginf << "SimpleReconstructorAssociationWidget: maxSpeedTrackerKtsEditedSlot: value '" << value_str << "'";

//    bool ok;

//    double value = text.toDouble(&ok);

//    if (ok)
//        reconstructor_.settings().maxSpeedTrackerKts(value);
//    else
//        logwrn << "SimpleReconstructorAssociationWidget: maxSpeedTrackerKtsEditedSlot: unable to parse value '"
//               << value_str << "'";
//}

void SimpleReconstructorAssociationWidget::contMaxTimeDiffTrackerEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "SimpleReconstructorAssociationWidget: contMaxTimeDiffTrackerEditedSlot: value '" << value_str << "'";

    bool ok;

    double value = text.toDouble(&ok);

    if (ok)
        reconstructor_.settings().cont_max_time_diff_tracker_ = value;
    else
        logwrn << "SimpleReconstructorAssociationWidget: contMaxTimeDiffTrackerEditedSlot: unable to parse value '"
               << value_str << "'";
}

void SimpleReconstructorAssociationWidget::contMaxDistanceAcceptableTrackerEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "SimpleReconstructorAssociationWidget: contMaxDistanceAcceptableTrackerEditedSlot: value '" << value_str << "'";

    bool ok;

    double value = text.toDouble(&ok);

    if (ok)
        reconstructor_.settings().cont_max_distance_acceptable_tracker_ = value;
    else
        logwrn << "SimpleReconstructorAssociationWidget: contMaxDistanceAcceptableTrackerEditedSlot: unable to parse value '"
               << value_str << "'";
}

