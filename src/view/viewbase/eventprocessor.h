
#ifndef EVENTPROCESSOR_H
#define EVENTPROCESSOR_H

#include <QMouseEvent>
#include <QKeyEvent>
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

    virtual bool mousePressEvent(QMouseEvent *e);
    virtual bool mouseReleaseEvent(QMouseEvent *e);
    virtual bool mouseMoveEvent(QMouseEvent *e);
    virtual bool wheelEvent(QWheelEvent * e);
    virtual bool keyPressEvent(QKeyEvent *e);
    virtual bool keyReleaseEvent(QKeyEvent *e);
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

#endif //EVENTPROCESSOR_H
