/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "createassociationstaskwidget.h"
#include "createassociationstask.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace std;

CreateAssociationsTaskWidget::CreateAssociationsTaskWidget(
        CreateAssociationsTask& task, QWidget* parent, Qt::WindowFlags f)
    : TaskWidget(parent, f), task_(task)
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

    ++row;
    layout->addWidget(new QLabel("Associate Non-Mode S Data"), row, 0);

    associate_non_mode_s_check_ = new QCheckBox ();
    connect(associate_non_mode_s_check_, &QCheckBox::clicked,
            this, &CreateAssociationsTaskWidget::toggleAssociateNonModeSSlot);
    layout->addWidget(associate_non_mode_s_check_, row, 1);

    ++row;
    layout->addWidget(new QLabel("Clean Dubious UTNs"), row, 0);

    clean_dubious_utns_check_ = new QCheckBox ();
    connect(clean_dubious_utns_check_, &QCheckBox::clicked,
            this, &CreateAssociationsTaskWidget::toggleCleanDubiousUtnsSlot);
    layout->addWidget(clean_dubious_utns_check_, row, 1);

    ++row;
    layout->addWidget(new QLabel("Mark Dubious UTNs Unused"), row, 0);

    mark_dubious_utns_unused_check_ = new QCheckBox ();
    connect(mark_dubious_utns_unused_check_, &QCheckBox::clicked,
            this, &CreateAssociationsTaskWidget::toggleMarkDubiousUtnsUnusedSlot);
    layout->addWidget(mark_dubious_utns_unused_check_, row, 1);

    ++row;
    layout->addWidget(new QLabel("Comment Dubious UTNs"), row, 0);

    comment_dubious_utns_check_ = new QCheckBox ();
    connect(comment_dubious_utns_check_, &QCheckBox::clicked,
            this, &CreateAssociationsTaskWidget::toggleCommentDubiousUtnsSlot);
    layout->addWidget(comment_dubious_utns_check_, row, 1);

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
            this, &CreateAssociationsTaskWidget::maxTimeDiffTrackerEditedSlot);
    layout->addWidget(max_time_diff_tracker_edit_, row, 1);

    //    QLineEdit* max_distance_quit_tracker_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Maximum Quit Distance [m]"), row, 0);

    max_distance_quit_tracker_edit_ = new QLineEdit();
    connect(max_distance_quit_tracker_edit_, &QLineEdit::textEdited,
            this, &CreateAssociationsTaskWidget::maxDistanceQuitTrackerEditedSlot);
    layout->addWidget(max_distance_quit_tracker_edit_, row, 1);

    //    QLineEdit* max_distance_dubious_tracker_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Maximum Dubious Distance [m]"), row, 0);

    max_distance_dubious_tracker_edit_ = new QLineEdit();
    connect(max_distance_dubious_tracker_edit_, &QLineEdit::textEdited,
            this, &CreateAssociationsTaskWidget::maxDistanceDubiousTrackerEditedSlot);
    layout->addWidget(max_distance_dubious_tracker_edit_, row, 1);

    //    QLineEdit* max_positions_dubious_tracker_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Maximum Dubious Positions [1]"), row, 0);

    max_positions_dubious_tracker_edit_ = new QLineEdit();
    connect(max_positions_dubious_tracker_edit_, &QLineEdit::textEdited,
            this, &CreateAssociationsTaskWidget::maxPositionsDubiousTrackerEditedSlot);
    layout->addWidget(max_positions_dubious_tracker_edit_, row, 1);

    //    QLineEdit* max_distance_acceptable_tracker_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Maximum Acceptable Distance [m]"), row, 0);

    max_distance_acceptable_tracker_edit_ = new QLineEdit();
    connect(max_distance_acceptable_tracker_edit_, &QLineEdit::textEdited,
            this, &CreateAssociationsTaskWidget::maxDistanceAcceptableTrackerEditedSlot);
    layout->addWidget(max_distance_acceptable_tracker_edit_, row, 1);

    //    QLineEdit* max_altitude_diff_tracker_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Maximum Altitude Difference [ft]"), row, 0);

    max_altitude_diff_tracker_edit_ = new QLineEdit();
    connect(max_altitude_diff_tracker_edit_, &QLineEdit::textEdited,
            this, &CreateAssociationsTaskWidget::maxAltitudeDiffTrackerEditedSlot);
    layout->addWidget(max_altitude_diff_tracker_edit_, row, 1);

    //    QLineEdit* min_updates_tracker_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Minimum Updates [1]"), row, 0);

    min_updates_tracker_edit_ = new QLineEdit();
    connect(min_updates_tracker_edit_, &QLineEdit::textEdited,
            this, &CreateAssociationsTaskWidget::minUpdatesTrackerEditedSlot);
    layout->addWidget(min_updates_tracker_edit_, row, 1);

    //    QLineEdit* prob_min_time_overlap_tracker_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Minimum Time Overlap Probability [0-1]"), row, 0);

    prob_min_time_overlap_tracker_edit_ = new QLineEdit();
    connect(prob_min_time_overlap_tracker_edit_, &QLineEdit::textEdited,
            this, &CreateAssociationsTaskWidget::probMinTimeOverlapTrackerEditedSlot);
    layout->addWidget(prob_min_time_overlap_tracker_edit_, row, 1);

    //    QLineEdit* max_speed_tracker_kts_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Max Speed [kts]"), row, 0);

    max_speed_tracker_kts_edit_ = new QLineEdit();
    connect(max_speed_tracker_kts_edit_, &QLineEdit::textEdited,
            this, &CreateAssociationsTaskWidget::maxSpeedTrackerKtsEditedSlot);
    layout->addWidget(max_speed_tracker_kts_edit_, row, 1);

    // QLineEdit* cont_max_time_diff_tracker_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Maximum Continuation Time Difference [s]"), row, 0);

    cont_max_time_diff_tracker_edit_ = new QLineEdit();
    connect(cont_max_time_diff_tracker_edit_, &QLineEdit::textEdited,
            this, &CreateAssociationsTaskWidget::contMaxTimeDiffTrackerEditedSlot);
    layout->addWidget(cont_max_time_diff_tracker_edit_, row, 1);

    // QLineEdit* cont_max_distance_acceptable_tracker_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Maximum Acceptable Continuation Distance [m]"), row, 0);

    cont_max_distance_acceptable_tracker_edit_ = new QLineEdit();
    connect(cont_max_distance_acceptable_tracker_edit_, &QLineEdit::textEdited,
            this, &CreateAssociationsTaskWidget::contMaxDistanceAcceptableTrackerEditedSlot);
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
            this, &CreateAssociationsTaskWidget::maxTimeDiffSensorEditedSlot);
    layout->addWidget(max_time_diff_sensor_edit_, row, 1);

    //    QLineEdit* max_distance_acceptable_sensor_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Maximum Acceptable Distance [m]"), row, 0);

    max_distance_acceptable_sensor_edit_ = new QLineEdit();
    connect(max_distance_acceptable_sensor_edit_, &QLineEdit::textEdited,
            this, &CreateAssociationsTaskWidget::maxDistanceAcceptableSensorEditedSlot);
    layout->addWidget(max_distance_acceptable_sensor_edit_, row, 1);


    //    QLineEdit* max_altitude_diff_sensor_edit_{nullptr};
    ++row;

    layout->addWidget(new QLabel("Maximum Altitude Difference [ft]"), row, 0);

    max_altitude_diff_sensor_edit_ = new QLineEdit();
    connect(max_altitude_diff_sensor_edit_, &QLineEdit::textEdited,
            this, &CreateAssociationsTaskWidget::maxAltitudeDiffSensorEditedSlot);
    layout->addWidget(max_altitude_diff_sensor_edit_, row, 1);

    update();

    setContentsMargins(0, 0, 0, 0);

    main_layout->addLayout(layout);
    main_layout->addStretch();

    expertModeChangedSlot();

    setLayout(main_layout);
}

CreateAssociationsTaskWidget::~CreateAssociationsTaskWidget() {}

void CreateAssociationsTaskWidget::update()
{
    //    QCheckBox* associate_non_mode_s_check_{nullptr};
    assert (associate_non_mode_s_check_);
    associate_non_mode_s_check_->setChecked(task_.associateNonModeS());

    //    QCheckBox* clean_dubious_utns_check_{nullptr};
    assert (clean_dubious_utns_check_);
    clean_dubious_utns_check_->setChecked(task_.cleanDubiousUtns());

    //    QCheckBox* mark_dubious_utns_unused_check_{nullptr};
    assert (mark_dubious_utns_unused_check_);
    mark_dubious_utns_unused_check_->setChecked(task_.markDubiousUtnsUnused());

    //    QCheckBox* comment_dubious_utns_check_{nullptr};
    assert (comment_dubious_utns_check_);
    comment_dubious_utns_check_->setChecked(task_.commentDubiousUtns());


    // tracker
    //    QLineEdit* max_time_diff_tracker_edit_{nullptr};
    assert (max_time_diff_tracker_edit_);
    max_time_diff_tracker_edit_->setText(QString::number(task_.maxTimeDiffTracker()));

    //    QLineEdit* max_distance_quit_tracker_edit_{nullptr};
    assert (max_distance_quit_tracker_edit_);
    max_distance_quit_tracker_edit_->setText(QString::number(task_.maxDistanceQuitTracker()));

    //    QLineEdit* max_distance_dubious_tracker_edit_{nullptr};
    assert (max_distance_dubious_tracker_edit_);
    max_distance_dubious_tracker_edit_->setText(QString::number(task_.maxDistanceDubiousTracker()));

    //    QLineEdit* max_positions_dubious_tracker_edit_{nullptr};
    assert (max_positions_dubious_tracker_edit_);
    max_positions_dubious_tracker_edit_->setText(QString::number(task_.maxPositionsDubiousTracker()));

    //    QLineEdit* max_distance_acceptable_tracker_edit_{nullptr};
    assert (max_distance_acceptable_tracker_edit_);
    max_distance_acceptable_tracker_edit_->setText(QString::number(task_.maxDistanceAcceptableTracker()));

    //    QLineEdit* max_altitude_diff_tracker_edit_{nullptr};
    assert (max_altitude_diff_tracker_edit_);
    max_altitude_diff_tracker_edit_->setText(QString::number(task_.maxAltitudeDiffTracker()));

    //    QLineEdit* min_updates_tracker_edit_{nullptr};
    assert (min_updates_tracker_edit_);
    min_updates_tracker_edit_->setText(QString::number(task_.minUpdatesTracker()));

    //    QLineEdit* prob_min_time_overlap_tracker_edit_{nullptr};
    assert (prob_min_time_overlap_tracker_edit_);
    prob_min_time_overlap_tracker_edit_->setText(QString::number(task_.probMinTimeOverlapTracker()));

    //    QLineEdit* max_speed_tracker_kts_edit_{nullptr};
    assert (max_speed_tracker_kts_edit_);
    max_speed_tracker_kts_edit_->setText(QString::number(task_.maxSpeedTrackerKts()));

    //    QLineEdit* cont_max_time_diff_tracker_edit_{nullptr};
    assert (cont_max_time_diff_tracker_edit_);
    cont_max_time_diff_tracker_edit_->setText(QString::number(task_.contMaxTimeDiffTracker()));

    //    QLineEdit* cont_max_distance_acceptable_tracker_edit_{nullptr};
    assert (cont_max_distance_acceptable_tracker_edit_);
    cont_max_distance_acceptable_tracker_edit_->setText(QString::number(task_.contMaxDistanceAcceptableTracker()));


    // sensor
    //    QLineEdit* max_time_diff_sensor_edit_{nullptr};
    assert (max_time_diff_sensor_edit_);
    max_time_diff_sensor_edit_->setText(QString::number(task_.maxTimeDiffSensor()));

    //    QLineEdit* max_distance_acceptable_sensor_edit_{nullptr};
    assert (max_distance_acceptable_sensor_edit_);
    max_distance_acceptable_sensor_edit_->setText(QString::number(task_.maxDistanceAcceptableSensor()));

    //    QLineEdit* max_altitude_diff_sensor_edit_{nullptr};
    assert (max_altitude_diff_sensor_edit_);
    max_altitude_diff_sensor_edit_->setText(QString::number(task_.maxAltitudeDiffSensor()));

}

void CreateAssociationsTaskWidget::expertModeChangedSlot()
{
}


void CreateAssociationsTaskWidget::toggleAssociateNonModeSSlot()
{
    assert (associate_non_mode_s_check_);
    task_.associateNonModeS(associate_non_mode_s_check_->checkState() == Qt::Checked);
}

void CreateAssociationsTaskWidget::toggleCleanDubiousUtnsSlot()
{
    assert (clean_dubious_utns_check_);
    task_.cleanDubiousUtns(clean_dubious_utns_check_->checkState() == Qt::Checked);
}

void CreateAssociationsTaskWidget::toggleMarkDubiousUtnsUnusedSlot()
{
    assert (mark_dubious_utns_unused_check_);
    task_.markDubiousUtnsUnused(mark_dubious_utns_unused_check_->checkState() == Qt::Checked);
}

void CreateAssociationsTaskWidget::toggleCommentDubiousUtnsSlot()
{
    assert (comment_dubious_utns_check_);
    task_.commentDubiousUtns(comment_dubious_utns_check_->checkState() == Qt::Checked);
}


void CreateAssociationsTaskWidget::maxTimeDiffTrackerEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "CreateAssociationsTaskWidget: maxTimeDiffTrackerEditedSlot: value '" << value_str << "'";

    bool ok;

    double value = text.toDouble(&ok);

    if (ok)
        task_.maxTimeDiffTracker(value);
    else
        logwrn << "CreateAssociationsTaskWidget: maxTimeDiffTrackerEditedSlot: unable to parse value '"
               << value_str << "'";
}

void CreateAssociationsTaskWidget::maxTimeDiffSensorEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "CreateAssociationsTaskWidget: maxTimeDiffSensorEditedSlot: value '" << value_str << "'";

    bool ok;

    double value = text.toDouble(&ok);

    if (ok)
        task_.maxTimeDiffSensor(value);
    else
        logwrn << "CreateAssociationsTaskWidget: maxTimeDiffSensorEditedSlot: unable to parse value '"
               << value_str << "'";
}

void CreateAssociationsTaskWidget::maxDistanceQuitTrackerEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "CreateAssociationsTaskWidget: maxDistanceQuitTrackerEditedSlot: value '" << value_str << "'";

    bool ok;

    double value = text.toDouble(&ok);

    if (ok)
        task_.maxDistanceQuitTracker(value);
    else
        logwrn << "CreateAssociationsTaskWidget: maxDistanceQuitTrackerEditedSlot: unable to parse value '"
               << value_str << "'";
}

void CreateAssociationsTaskWidget::maxDistanceDubiousTrackerEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "CreateAssociationsTaskWidget: maxDistanceDubiousTrackerEditedSlot: value '" << value_str << "'";

    bool ok;

    double value = text.toDouble(&ok);

    if (ok)
        task_.maxDistanceDubiousTracker(value);
    else
        logwrn << "CreateAssociationsTaskWidget: maxDistanceDubiousTrackerEditedSlot: unable to parse value '"
               << value_str << "'";
}

void CreateAssociationsTaskWidget::maxPositionsDubiousTrackerEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "CreateAssociationsTaskWidget: maxPositionsDubiousTrackerEditedSlot: value '" << value_str << "'";

    bool ok;

    unsigned int value = text.toUInt(&ok);

    if (ok)
        task_.maxPositionsDubiousTracker(value);
    else
        logwrn << "CreateAssociationsTaskWidget: maxPositionsDubiousTrackerEditedSlot: unable to parse value '"
               << value_str << "'";
}

void CreateAssociationsTaskWidget::maxDistanceAcceptableTrackerEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "CreateAssociationsTaskWidget: maxDistanceAcceptableTrackerEditedSlot: value '" << value_str << "'";

    bool ok;

    double value = text.toDouble(&ok);

    if (ok)
        task_.maxDistanceAcceptableTracker(value);
    else
        logwrn << "CreateAssociationsTaskWidget: maxDistanceAcceptableTrackerEditedSlot: unable to parse value '"
               << value_str << "'";
}

void CreateAssociationsTaskWidget::maxDistanceAcceptableSensorEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "CreateAssociationsTaskWidget: maxDistanceAcceptableSensorEditedSlot: value '" << value_str << "'";

    bool ok;

    double value = text.toDouble(&ok);

    if (ok)
        task_.maxDistanceAcceptableSensor(value);
    else
        logwrn << "CreateAssociationsTaskWidget: maxDistanceAcceptableSensorEditedSlot: unable to parse value '"
               << value_str << "'";
}

void CreateAssociationsTaskWidget::maxAltitudeDiffTrackerEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "CreateAssociationsTaskWidget: maxAltitudeDiffTrackerEditedSlot: value '" << value_str << "'";

    bool ok;

    double value = text.toDouble(&ok);

    if (ok)
        task_.maxAltitudeDiffTracker(value);
    else
        logwrn << "CreateAssociationsTaskWidget: maxAltitudeDiffTrackerEditedSlot: unable to parse value '"
               << value_str << "'";
}

void CreateAssociationsTaskWidget::maxAltitudeDiffSensorEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "CreateAssociationsTaskWidget: maxAltitudeDiffSensorEditedSlot: value '" << value_str << "'";

    bool ok;

    double value = text.toDouble(&ok);

    if (ok)
        task_.maxAltitudeDiffSensor(value);
    else
        logwrn << "CreateAssociationsTaskWidget: maxAltitudeDiffSensorEditedSlot: unable to parse value '"
               << value_str << "'";
}

void CreateAssociationsTaskWidget::probMinTimeOverlapTrackerEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "CreateAssociationsTaskWidget: probMinTimeOverlapTrackerEditedSlot: value '" << value_str << "'";

    bool ok;

    double value = text.toDouble(&ok);

    if (ok)
        task_.probMinTimeOverlapTracker(value);
    else
        logwrn << "CreateAssociationsTaskWidget: probMinTimeOverlapTrackerEditedSlot: unable to parse value '"
               << value_str << "'";
}

void CreateAssociationsTaskWidget::minUpdatesTrackerEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "CreateAssociationsTaskWidget: minUpdatesTrackerEditedSlot: value '" << value_str << "'";

    bool ok;

    unsigned int value = text.toUInt(&ok);

    if (ok)
        task_.minUpdatesTracker(value);
    else
        logwrn << "CreateAssociationsTaskWidget: minUpdatesTrackerEditedSlot: unable to parse value '"
               << value_str << "'";
}

void CreateAssociationsTaskWidget::maxSpeedTrackerKtsEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "CreateAssociationsTaskWidget: maxSpeedTrackerKtsEditedSlot: value '" << value_str << "'";

    bool ok;

    double value = text.toDouble(&ok);

    if (ok)
        task_.maxSpeedTrackerKts(value);
    else
        logwrn << "CreateAssociationsTaskWidget: maxSpeedTrackerKtsEditedSlot: unable to parse value '"
               << value_str << "'";
}

void CreateAssociationsTaskWidget::contMaxTimeDiffTrackerEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "CreateAssociationsTaskWidget: contMaxTimeDiffTrackerEditedSlot: value '" << value_str << "'";

    bool ok;

    double value = text.toDouble(&ok);

    if (ok)
        task_.contMaxTimeDiffTracker(value);
    else
        logwrn << "CreateAssociationsTaskWidget: contMaxTimeDiffTrackerEditedSlot: unable to parse value '"
               << value_str << "'";
}

void CreateAssociationsTaskWidget::contMaxDistanceAcceptableTrackerEditedSlot (const QString& text)
{
    string value_str = text.toStdString();

    loginf << "CreateAssociationsTaskWidget: contMaxDistanceAcceptableTrackerEditedSlot: value '" << value_str << "'";

    bool ok;

    double value = text.toDouble(&ok);

    if (ok)
        task_.contMaxDistanceAcceptableTracker(value);
    else
        logwrn << "CreateAssociationsTaskWidget: contMaxDistanceAcceptableTrackerEditedSlot: unable to parse value '"
               << value_str << "'";
}
