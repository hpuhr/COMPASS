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
#include "appmode.h"

//class EventProcessor;
class View;
class ViewToolWidget;
class ViewDataWidget;
class ViewConfigWidget;
class ViewToolSwitcher;
class ViewLoadStateWidget;

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

    void toggleConfigWidget();

    void updateToolWidget();
    void updateLoadState();

    void loadingStarted();
    void loadingDone();
    void redrawStarted();
    void redrawDone();
    void appModeSwitch(AppMode app_mode);

    void notifyReloadNeeded();
    void notifyRedrawNeeded();

    ViewDataWidget* getViewDataWidget() { return data_widget_; }
    const ViewDataWidget* getViewDataWidget() const { return data_widget_; }
    ViewConfigWidget* getViewConfigWidget() { return config_widget_; }
    const ViewConfigWidget* getViewConfigWidget() const { return config_widget_; }

    QWidget* getLowerWidget() { return lower_widget_; }
    const QWidget* getLowerWidget() const { return lower_widget_; }

    virtual std::string loadedMessage() const { return ""; }

    bool reloadNeeded() const;
    bool redrawNeeded() const;

    void init();

    static QIcon getIcon(const std::string& fn);

protected:
    ViewToolWidget* getViewToolWidget() { return tool_widget_; }
    const ViewToolWidget* getViewToolWidget() const { return tool_widget_; }
    ViewToolSwitcher* getViewToolSwitcher() { return tool_switcher_.get(); }
    const ViewToolSwitcher* getViewToolSwitcher() const { return tool_switcher_.get(); }
    ViewLoadStateWidget* getViewLoadStateWidget() { return state_widget_; }
    const ViewLoadStateWidget* getViewLoadStateWidget() const { return state_widget_; }

    virtual bool reloadNeeded_impl() const { return false; };
    virtual bool redrawNeeded_impl() const { return false; };

    void setDataWidget(ViewDataWidget* w);
    void setConfigWidget(ViewConfigWidget* w);
    void setLowerWidget(QWidget* w);

    View* getView() { return view_; }

private:
    void createStandardLayout();
    void connectWidgets();

    /// The view the widget is part of
    View* view_ = nullptr;

    QSplitter*      main_splitter_           = nullptr;
    QWidget*        data_widget_container_   = nullptr;
    QWidget*        config_widget_container_ = nullptr;
    QWidget*        lower_widget_container_  = nullptr;
    QWidget*        right_widget_            = nullptr;

    ViewToolWidget*      tool_widget_   = nullptr;
    ViewDataWidget*      data_widget_   = nullptr;
    ViewConfigWidget*    config_widget_ = nullptr;
    ViewLoadStateWidget* state_widget_  = nullptr;
    QWidget*             lower_widget_  = nullptr;

    std::unique_ptr<ViewToolSwitcher> tool_switcher_;

    bool redraw_needed_ = false;
    bool reload_needed_ = false;
};

#endif  // VIEWWIDGET_H
