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

#include "eventprocessor.h"

/**
@brief Constructor.
*/
EventProcessor::EventProcessor()
    : mouse_left_pressed_(false), mouse_right_pressed_(false), mouse_middle_pressed_(false)
{
}

/**
@brief Destructor.
*/
EventProcessor::~EventProcessor() {}

/**
@brief Handles specific mouse press events.
@param e The event.
@return True if the event has been handled, false otherwise.
*/
bool EventProcessor::mousePressEvent(QMouseEvent* e)
{
    mouse_pos_ = e->pos();
    if (e->button() == Qt::RightButton)
        mouse_right_pressed_ = true;
    if (e->button() == Qt::LeftButton)
        mouse_left_pressed_ = true;
    if (e->button() == Qt::MidButton)
        mouse_middle_pressed_ = true;

    // not handled in base class
    return false;
}

/**
@brief Handles specific mouse release events.
@param e The event.
@return True if the event has been handled, false otherwise.
*/
bool EventProcessor::mouseReleaseEvent(QMouseEvent* e)
{
    mouse_pos_ = e->pos();
    if (e->button() == Qt::RightButton)
        mouse_right_pressed_ = false;
    if (e->button() == Qt::LeftButton)
        mouse_left_pressed_ = false;
    if (e->button() == Qt::MidButton)
        mouse_middle_pressed_ = false;

    // not handled in base class
    return false;
}

/**
@brief Handles specific mouse move events.
@param e The event.
@return True if the event has been handled, false otherwise.
*/
bool EventProcessor::mouseMoveEvent(QMouseEvent* e)
{
    mouse_pos_ = e->pos();

    // not handled in base class
    return false;
}

/**
@brief Handles specific mouse wheel events.
@param e The event.
@return True if the event has been handled, false otherwise.
*/
bool EventProcessor::wheelEvent(QWheelEvent* e)
{
    // not handled in base class
    return false;
}

/**
@brief Handles specific key press events.
@param e The event.
@return True if the event has been handled, false otherwise.
*/
bool EventProcessor::keyPressEvent(QKeyEvent* e)
{
    // not handled in base class
    return false;
}

/**
@brief Handles specific key release events.
@param e The event.
@return True if the event has been handled, false otherwise.
*/
bool EventProcessor::keyReleaseEvent(QKeyEvent* e)
{
    // not handled in base class
    return false;
}

/**
@brief Handles specific mouse double click events.
@param e The event.
@return True if the event has been handled, false otherwise.
*/
bool EventProcessor::mouseDoubleClickEvent(QMouseEvent* e)
{
    // not handled in base class
    return false;
}
