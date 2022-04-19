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

#ifndef CREATEARTASASSOCIATIONSTASKWIDGET_H
#define CREATEARTASASSOCIATIONSTASKWIDGET_H

#include <taskwidget.h>

class CreateARTASAssociationsTask;

namespace dbContent
{
class VariableSelectionWidget;
class DBDataSourceComboBox;
}

class QPushButton;
class QLineEdit;
class QCheckBox;

class CreateARTASAssociationsTaskWidget : public TaskWidget
{
    Q_OBJECT

  public slots:
    void currentDataSourceChangedSlot();

    void endTrackTimeEditSlot(QString value);

    void associationTimePastEditSlot(QString value);
    void associationTimeFutureEditSlot(QString value);
    void missesAcceptableTimeEditSlot(QString value);
    void associationsDubiousDistantTimeEditSlot(QString value);
    void associationDubiousCloseTimePastEditSlot(QString value);
    void associationDubiousCloseTimeFutureEditSlot(QString value);

    void anyTrackFlagChangedSlot();

    void expertModeChangedSlot();

  public:
    CreateARTASAssociationsTaskWidget(CreateARTASAssociationsTask& task, QWidget* parent = 0,
                                      Qt::WindowFlags f = 0);

    virtual ~CreateARTASAssociationsTaskWidget();

    void update();

  protected:
    CreateARTASAssociationsTask& task_;

    dbContent::DBDataSourceComboBox* ds_combo_{nullptr};

    QLineEdit* end_track_time_edit_{nullptr};

    QLineEdit* association_time_past_edit_{nullptr};
    QLineEdit* association_time_future_edit_{nullptr};

    QLineEdit* misses_acceptable_time_edit_{nullptr};

    QLineEdit* associations_dubious_distant_time_edit_{nullptr};
    QLineEdit* association_dubious_close_time_past_edit_{nullptr};
    QLineEdit* association_dubious_close_time_future_edit_{nullptr};

    QCheckBox* ignore_track_end_associations_check_{nullptr};
    QCheckBox* mark_track_end_associations_dubious_check_{nullptr};
    QCheckBox* ignore_track_coasting_associations_check_{nullptr};
    QCheckBox* mark_track_coasting_associations_dubious_check_{nullptr};
};

#endif  // CREATEARTASASSOCIATIONSTASKWIDGET_H
