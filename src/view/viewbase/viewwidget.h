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

#include <QWidget>

#include "configurable.h"

//class EventProcessor;
class View;

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

    /// @brief Updates the display (only the display, the OGRE render window e.g.)
//    virtual void updateView() = 0;

//    void setEventProcessor(EventProcessor* ep);
//    /// @brief Returns the view widget's event processor
//    EventProcessor* getEventProcessor() { return event_processor_; }

//    virtual void mousePressEvent(QMouseEvent* e);
//    virtual void mouseReleaseEvent(QMouseEvent* e);
//    virtual void mouseMoveEvent(QMouseEvent* e);
//    virtual void wheelEvent(QWheelEvent* e);
//    virtual void keyPressEvent(QKeyEvent* e);
//    virtual void keyReleaseEvent(QKeyEvent* e);
//    virtual void mouseDoubleClickEvent(QMouseEvent* e);

    View* getView() { return view_; }

  signals:
    //void itemsSelected(ViewSelectionEntries& entries);

  protected:
    /// The widget's event processor
    //EventProcessor* event_processor_;
    /// The view the widget is part of
    View* view_;
};

#endif  // VIEWWIDGET_H
