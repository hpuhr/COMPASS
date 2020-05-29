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

#ifndef EVENTPROCESSOR_H
#define EVENTPROCESSOR_H

#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QWidget>

/**
@brief Base class for all event processors.

An event processor provides a way to handle keyboard and mouse events individually in a
ViewWidget. For this derive from this class and reimplement the provided event handling methods.

The methods return if they have handled the event. When reimplementing them in a derived
class, one should first call the base class versions and check if these methods handled the event.
If not they may handle the event themselves and should return if they did so afterwards.
  */
class EventProcessor : public QObject
{
    Q_OBJECT
  public:
    EventProcessor();
    virtual ~EventProcessor();

    virtual bool mousePressEvent(QMouseEvent* e);
    virtual bool mouseReleaseEvent(QMouseEvent* e);
    virtual bool mouseMoveEvent(QMouseEvent* e);
    virtual bool wheelEvent(QWheelEvent* e);
    virtual bool keyPressEvent(QKeyEvent* e);
    virtual bool keyReleaseEvent(QKeyEvent* e);
    virtual bool mouseDoubleClickEvent(QMouseEvent* e);

  protected:
    /// Stores the current mouse position
    QPoint mouse_pos_;
    /// Stores the left mouse buttons state
    bool mouse_left_pressed_;
    /// Stores the right mouse buttons state
    bool mouse_right_pressed_;
    /// Stores the middle mouse buttons state
    bool mouse_middle_pressed_;
};

#endif  // EVENTPROCESSOR_H
