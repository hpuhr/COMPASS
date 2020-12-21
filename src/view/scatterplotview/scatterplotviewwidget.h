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

#ifndef SCATTERPLOTVIEWWIDGET_H_
#define SCATTERPLOTVIEWWIDGET_H_

#include "eventprocessor.h"
#include "viewwidget.h"

class ScatterPlotView;
class ScatterPlotViewDataWidget;
class ScatterPlotViewConfigWidget;

class QSplitter;
class QTabWidget;

class ScatterPlotViewWidget : public ViewWidget
{
  public:
    /// @brief Constructor
    ScatterPlotViewWidget(const std::string& class_id, const std::string& instance_id,
                      Configurable* config_parent, ScatterPlotView* view, QWidget* parent = NULL);
    /// @brief Destructor
    virtual ~ScatterPlotViewWidget();

    virtual void updateView();

    /// @brief Toggles visibility of the config widget
    void toggleConfigWidget();

    /// @brief Returns the config widget
    ScatterPlotViewConfigWidget* configWidget();

    /// @brief Returns the basis view
    ScatterPlotView* getView() { return (ScatterPlotView*)view_; }
    /// @brief Returns the data widget
    ScatterPlotViewDataWidget* getDataWidget() { return data_widget_; }

  protected:
    QSplitter* main_splitter_{nullptr};
    /// Data widget with data display
    ScatterPlotViewDataWidget* data_widget_{nullptr};
    /// Config widget with configuration elements
    ScatterPlotViewConfigWidget* config_widget_{nullptr};
};

#endif /* SCATTERPLOTVIEWWIDGET_H_ */
