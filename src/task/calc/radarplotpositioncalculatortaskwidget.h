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

#ifndef RADARPLOTPOSITIONCALCULATORTASKWIDGET_H_
#define RADARPLOTPOSITIONCALCULATORTASKWIDGET_H_

#include <taskwidget.h>

class Buffer;
class QLabel;
class RadarPlotPositionCalculatorTask;
class DBContentComboBox;

namespace dbContent
{
class VariableSelectionWidget;
}

class QPushButton;

class RadarPlotPositionCalculatorTaskWidget : public TaskWidget
{
    Q_OBJECT

  public slots:
    void dbObjectChangedSlot();
    void keyVarChangedSlot();
    void datasourceVarChangedSlot();
    void rangeVarChangedSlot();
    void azimuthVarChangedSlot();
    void altitudeVarChangedSlot();
    void latitudeVarChangedSlot();
    void longitudeVarChangedSlot();

    void expertModeChangedSlot();

  public:
    RadarPlotPositionCalculatorTaskWidget(RadarPlotPositionCalculatorTask& task,
                                          QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~RadarPlotPositionCalculatorTaskWidget();

    void update();

  protected:
    RadarPlotPositionCalculatorTask& task_;

    DBContentComboBox* object_box_{nullptr};
    dbContent::VariableSelectionWidget* key_box_{nullptr};
    dbContent::VariableSelectionWidget* datasource_box_{nullptr};
    dbContent::VariableSelectionWidget* range_box_{nullptr};
    dbContent::VariableSelectionWidget* azimuth_box_{nullptr};
    dbContent::VariableSelectionWidget* altitude_box_{nullptr};

    dbContent::VariableSelectionWidget* latitude_box_{nullptr};
    dbContent::VariableSelectionWidget* longitude_box_{nullptr};

    void setDBOBject(const std::string& object_name);
};

#endif /* RADARPLOTPOSITIONCALCULATORTASKWIDGET_H_ */
