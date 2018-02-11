/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RADARPLOTPOSITIONCALCULATORTASKWIDGET_H_
#define RADARPLOTPOSITIONCALCULATORTASKWIDGET_H_

#include <QWidget>

class Buffer;
class QLabel;
class RadarPlotPositionCalculatorTask;
class DBObjectComboBox;
class DBOVariableSelectionWidget;

class RadarPlotPositionCalculatorTaskWidget : public QWidget
{
    Q_OBJECT

public slots:
    void dbObjectChangedSlot();
    void anyVariableChangedSlot();
    void calculateSlot ();

public:
    RadarPlotPositionCalculatorTaskWidget(RadarPlotPositionCalculatorTask& task, QWidget * parent = 0, Qt::WindowFlags f = 0);
    virtual ~RadarPlotPositionCalculatorTaskWidget();

    void update ();

protected:
    RadarPlotPositionCalculatorTask& task_;

    DBObjectComboBox* object_box_ {nullptr};
    DBOVariableSelectionWidget* key_box_ {nullptr};
    DBOVariableSelectionWidget* datasource_box_ {nullptr};
    DBOVariableSelectionWidget* range_box_ {nullptr};
    DBOVariableSelectionWidget* azimuth_box_ {nullptr};
    DBOVariableSelectionWidget* altitude_box_ {nullptr};

    DBOVariableSelectionWidget* latitude_box_ {nullptr};
    DBOVariableSelectionWidget* longitude_box_ {nullptr};

    void setDBOBject (const std::string& object_name);
};

#endif /* RADARPLOTPOSITIONCALCULATORTASKWIDGET_H_ */
