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

#ifndef DBVIEWWIDGET_H
#define DBVIEWWIDGET_H

#include "EventProcessor.h"
#include "ViewWidget.h"

class DBView;
class Configurable;
class DBViewWidget;

/**
@brief Event processor to trigger DBView specific events.

Triggers the workflow editor.
  */
class DBViewWidgetEventProcessor : public EventProcessor
{
  public:
    DBViewWidgetEventProcessor(DBViewWidget* widget);
    virtual ~DBViewWidgetEventProcessor();

    virtual bool keyPressEvent(QKeyEvent* e);
    virtual bool keyReleaseEvent(QKeyEvent* e);

  protected:
    /// View widget the event processor is part of.
    DBViewWidget* widget_;
    /// State of the contorl key
    bool key_ctrl_pressed_;
};

/**
@brief Base widget for database driven views.

The DBViewWidget has been introduced to somewhat force the use
of a DisplayObjectManager, which is needed in the DBViewModel for
the generators.

The widget also obtains a special event processor to trigger the workflow editor.

@todo As with the DBViewModel: Generators are not specifically database driven,
so this should maybe be reworked into a GeneratorViewWidget?
  */
class DBViewWidget : public ViewWidget
{
  public:
    DBViewWidget(const std::string& class_id, const std::string& instance_id,
                 Configurable* config_parent, DBView* view, QWidget* parent = NULL);
    virtual ~DBViewWidget();

    /// @brief Updates the display
    virtual void updateView() = 0;

    DBView* getView();
    DBViewWidgetEventProcessor* getEventProcessor();
    DisplayObjectManager* getDOManager();

  protected:
    /// Display object manager used in the widget
    DisplayObjectManager* do_manager_;
};

#endif  // DBVIEWWIDGET_H
