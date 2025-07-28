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

#pragma once

#include "radarplotpositioncalculatortaskwidget.h"

#include <QDialog>

#include <memory>

class RadarPlotPositionCalculatorTask;

class RadarPlotPositionCalculatorTaskDialog : public QDialog
{
    Q_OBJECT

signals:
    void closeSignal();

public slots:
    void okClickedSlot();
    void cancelClickedSlot();

public:
    RadarPlotPositionCalculatorTaskDialog(RadarPlotPositionCalculatorTask& task);
    virtual ~RadarPlotPositionCalculatorTaskDialog();

    bool runWanted() const;

    void updateCanRun();

protected:
    RadarPlotPositionCalculatorTask& task_;

    std::unique_ptr<RadarPlotPositionCalculatorTaskWidget> widget_;

    QPushButton* ok_button_ {nullptr};

    bool run_wanted_ {false};
};
