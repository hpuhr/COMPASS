
#include "eventprocessor.h"


/**
@brief Constructor.
*/
EventProcessor::EventProcessor()
:   mouse_left_pressed_( false ), mouse_right_pressed_( false ), mouse_middle_pressed_( false )
{
}

/**
@brief Destructor.
*/
EventProcessor::~EventProcessor()
{
}

/**
@brief Handles specific mouse press events.
@param e The event.
@return True if the event has been handled, false otherwise.
*/
bool EventProcessor::mousePressEvent(QMouseEvent *e)
{
    mouse_pos_ = e->pos();
    if( e->button() == Qt::RightButton )
        mouse_right_pressed_ = true;
    if( e->button() == Qt::LeftButton )
        mouse_left_pressed_ = true;
    if( e->button() == Qt::MidButton )
        mouse_middle_pressed_ = true;

    //not handled in base class
    return false;
}

/**
@brief Handles specific mouse release events.
@param e The event.
@return True if the event has been handled, false otherwise.
*/
bool EventProcessor::mouseReleaseEvent(QMouseEvent *e)
{
    mouse_pos_ = e->pos();
    if( e->button() == Qt::RightButton )
        mouse_right_pressed_ = false;
    if( e->button() == Qt::LeftButton )
        mouse_left_pressed_ = false;
    if( e->button() == Qt::MidButton )
        mouse_middle_pressed_ = false;

    //not handled in base class
    return false;
}

/**
@brief Handles specific mouse move events.
@param e The event.
@return True if the event has been handled, false otherwise.
*/
bool EventProcessor::mouseMoveEvent(QMouseEvent *e)
{
    mouse_pos_ = e->pos();

    //not handled in base class
    return false;
}

/**
@brief Handles specific mouse wheel events.
@param e The event.
@return True if the event has been handled, false otherwise.
*/
bool EventProcessor::wheelEvent(QWheelEvent * e)
{
    //not handled in base class
    return false;
}

/**
@brief Handles specific key press events.
@param e The event.
@return True if the event has been handled, false otherwise.
*/
bool EventProcessor::keyPressEvent(QKeyEvent *e)
{
    //not handled in base class
    return false;
}

/**
@brief Handles specific key release events.
@param e The event.
@return True if the event has been handled, false otherwise.
*/
bool EventProcessor::keyReleaseEvent(QKeyEvent *e)
{
    //not handled in base class
    return false;
}

/**
@brief Handles specific mouse double click events.
@param e The event.
@return True if the event has been handled, false otherwise.
*/
bool EventProcessor::mouseDoubleClickEvent(QMouseEvent* e)
{
    //not handled in base class
    return false;
}
