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

#ifndef HISTOGRAMVIEWWIDGET_H_
#define HISTOGRAMVIEWWIDGET_H_

#include "viewwidget.h"

class HistogramView;
class HistogramViewDataToolWidget;
class HistogramViewDataWidget;
class HistogramViewConfigWidget;

class QSplitter;
class QTabWidget;

class HistogramViewWidget : public ViewWidget
{
  public:
    /// @brief Constructor
    HistogramViewWidget(const std::string& class_id, const std::string& instance_id,
                      Configurable* config_parent, HistogramView* view, QWidget* parent = NULL);
    /// @brief Destructor
    virtual ~HistogramViewWidget();

    /// @brief Toggles visibility of the config widget
    void toggleConfigWidget();

    /// @brief Returns the config widget
    HistogramViewConfigWidget* configWidget();

    /// @brief Returns the basis view
    HistogramView* getView() { return (HistogramView*)view_; }
    /// @brief Returns the data widget
    HistogramViewDataWidget* getDataWidget() { return data_widget_; }

  protected:
    HistogramViewDataToolWidget* tool_widget_{nullptr};

    QSplitter* main_splitter_{nullptr};
    /// Data widget with data display
    HistogramViewDataWidget* data_widget_{nullptr};
    /// Config widget with configuration elements
    HistogramViewConfigWidget* config_widget_{nullptr};
};

#endif /* HISTOGRAMVIEWWIDGET_H_ */
