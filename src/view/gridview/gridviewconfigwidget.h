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

#include "variableviewconfigwidget.h"

class GridViewWidget;
class GridView;

class QCheckBox;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QPushButton;
class QToolButton;
class QDialog;

/**
 * @brief Widget with configuration elements for a ScatterPlotView
 *
 */
class GridViewConfigWidget : public VariableViewConfigWidget
{
public:
    GridViewConfigWidget(GridViewWidget* view_widget, 
                         QWidget* parent = nullptr);
    virtual ~GridViewConfigWidget();

protected:
    virtual void viewInfoJSON_impl(nlohmann::json& info) const override;

    void attachExportMenu();

    void valueTypeChanged();
    void gridResolutionChanged();
    void colorStepsChanged();

    void updateConfig();

    void exportToGeographicView();
    void exportToGeoTiff();

    GridView* view_ = nullptr;

    QComboBox*   value_type_combo_    = nullptr;
    QSpinBox*    grid_resolution_box_ = nullptr;
    QSpinBox*    color_steps_box_     = nullptr;
    QPushButton* export_button_       = nullptr;
};
