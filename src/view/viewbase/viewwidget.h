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

#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include <memory>

#include <QWidget>

#include "configurable.h"

//class EventProcessor;
class View;
class ViewToolWidget;
class ViewDataWidget;
class ViewConfigWidget;
class ViewToolSwitcher;

class QSplitter;
class QLayout;

/**
@brief Base class for a views "display" component.

A view widget bases on a QWidget and provides a way to display
the views data, which is held in a model.

A view widget may hold an event processor, which is used to react in a specific
way to mouse and keyboard events. A new ViewWidget will most likely introduce a new
event processor.

Sometimes a ViewWidget is composed of more ViewWidgets, f.e. to be able to set a different
event processor for each part.
  */
class ViewWidget : public QWidget, public Configurable
{
    Q_OBJECT
public:
    ViewWidget(const std::string& class_id, const std::string& instance_id,
               Configurable* config_parent, View* view, QWidget* parent = nullptr);
    virtual ~ViewWidget();

    View* getView() { return view_; }

    void toggleConfigWidget();

    virtual ViewDataWidget* getViewDataWidget() { return data_widget_; }
    virtual ViewConfigWidget* getViewConfigWidget() { return config_widget_; }

protected:
    ViewToolWidget* getViewToolWidget() { return tool_widget_; }
    ViewToolSwitcher* getViewToolSwitcher() { return tool_switcher_.get(); }

    void setDataWidget(ViewDataWidget* w);
    void setConfigWidget(ViewConfigWidget* w);

    void createStandardLayout();

    /// The view the widget is part of
    View* view_;

private:
    void connectWidgets();

    QSplitter*      main_splitter_           = nullptr;
    QWidget*        data_widget_container_   = nullptr;
    QWidget*        config_widget_container_ = nullptr;

    ViewToolWidget*   tool_widget_   = nullptr;
    ViewDataWidget*   data_widget_   = nullptr;
    ViewConfigWidget* config_widget_ = nullptr;

    std::unique_ptr<ViewToolSwitcher> tool_switcher_;
};

#endif  // VIEWWIDGET_H
