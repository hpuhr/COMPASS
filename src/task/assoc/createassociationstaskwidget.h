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

#ifndef CREATEASSOCIATIONSTASKWIDGET_H
#define CREATEASSOCIATIONSTASKWIDGET_H

#include <taskwidget.h>

class CreateAssociationsTask;

class QLineEdit;
class QCheckBox;

class CreateAssociationsTaskWidget : public TaskWidget
{
    Q_OBJECT

public slots:
    void expertModeChangedSlot();

    void toggleAssociateNonModeSSlot();
    void toggleCleanDubiousUtnsSlot();
    void toggleMarkDubiousUtnsUnusedSlot();
    void toggleCommentDubiousUtnsSlot();

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
    void maxSpeedTrackerKtsEditedSlot (const QString& text);

    void contMaxTimeDiffTrackerEditedSlot (const QString& text);
    void contMaxDistanceAcceptableTrackerEditedSlot (const QString& text);

public:
    CreateAssociationsTaskWidget(CreateAssociationsTask& task, QWidget* parent = 0,
                                 Qt::WindowFlags f = 0);
    virtual ~CreateAssociationsTaskWidget();

    void update();

protected:
    CreateAssociationsTask& task_;

    QCheckBox* associate_non_mode_s_check_{nullptr};
    QCheckBox* clean_dubious_utns_check_{nullptr};
    QCheckBox* mark_dubious_utns_unused_check_{nullptr};
    QCheckBox* comment_dubious_utns_check_{nullptr};

    // tracker
    QLineEdit* max_time_diff_tracker_edit_{nullptr};

    QLineEdit* max_distance_quit_tracker_edit_{nullptr};
    QLineEdit* max_distance_dubious_tracker_edit_{nullptr};
    QLineEdit* max_positions_dubious_tracker_edit_{nullptr};
    QLineEdit* max_distance_acceptable_tracker_edit_{nullptr};
    QLineEdit* max_altitude_diff_tracker_edit_{nullptr};
    QLineEdit* min_updates_tracker_edit_{nullptr};

    QLineEdit* prob_min_time_overlap_tracker_edit_{nullptr};
    QLineEdit* max_speed_tracker_kts_edit_{nullptr};

    QLineEdit* cont_max_time_diff_tracker_edit_{nullptr};
    QLineEdit* cont_max_distance_acceptable_tracker_edit_{nullptr};

    // sensor
    QLineEdit* max_time_diff_sensor_edit_{nullptr};
    QLineEdit* max_distance_acceptable_sensor_edit_{nullptr};
    QLineEdit* max_altitude_diff_sensor_edit_{nullptr};
};

#endif // CREATEASSOCIATIONSTASKWIDGET_H
