
#include "viewwidget.h"
#include "eventprocessor.h"
#include "view.h"


/**
@brief Constructor.
@param class_id Configurable class id.
@param instance_id Configurable instance id.
@param config_parent Configurable parent.
@param view The view the view widget is part of.
@param parent The widgets parent.
*/
ViewWidget::ViewWidget( const std::string& class_id, const std::string& instance_id, Configurable* config_parent, View* view, QWidget *parent )
:   QWidget( parent ), Configurable( class_id, instance_id, config_parent ), event_processor_( nullptr ), view_( view )
{
    setAutoFillBackground(true);
}

/**
@brief Destructor.
*/
ViewWidget::~ViewWidget()
{
    if( event_processor_ )
        delete event_processor_;
}

/**
@brief Sets a new event processor.

This will delete the old event processor. Also, the provided event processor
will be deleted automatically on destruction.
@param ep New event processor.
*/
void ViewWidget::setEventProcessor( EventProcessor* ep )
{
    if( event_processor_ )
        delete event_processor_;
    event_processor_ = ep;
}

/**
@brief Reroutes a mouse press event to the event processor.
@param e The event.
*/
void ViewWidget::mousePressEvent( QMouseEvent* e )
{
    if( !event_processor_ || event_processor_->mousePressEvent( e ) == false )
        QWidget::mousePressEvent( e );
}

/**
@brief Reroutes a mouse wheel event to the event processor.
@param e The event.
*/
void ViewWidget::wheelEvent( QWheelEvent* e )
{
    if( !event_processor_ || event_processor_->wheelEvent( e ) == false )
        QWidget::wheelEvent( e );
}

/**
@brief Reroutes a mouse release event to the event processor.
@param e The event.
*/
void ViewWidget::mouseReleaseEvent( QMouseEvent* e )
{
    if( !event_processor_ || event_processor_->mouseReleaseEvent( e ) == false )
        QWidget::mouseReleaseEvent( e );
}

/**
@brief Reroutes a keyboard press event to the event processor.
@param e The event.
*/
void ViewWidget::keyPressEvent( QKeyEvent* e )
{
    if( !event_processor_ || event_processor_->keyPressEvent( e ) == false )
        QWidget::keyPressEvent( e );
}

/**
@brief Reroutes a keyboard release event to the event processor.
@param e The event.
*/
void ViewWidget::keyReleaseEvent( QKeyEvent* e )
{
    if( !event_processor_ || event_processor_->keyReleaseEvent( e ) == false )
        QWidget::keyReleaseEvent( e );
}

/**
@brief Reroutes a mouse move event to the event processor.
@param e The event.
*/
void ViewWidget::mouseMoveEvent( QMouseEvent* e )
{
    if( !event_processor_ || event_processor_->mouseMoveEvent( e ) == false )
        QWidget::mouseMoveEvent( e );
}

/**
@brief Reroutes a mouse double click event to the event processor.
@param e The event.
*/
void ViewWidget::mouseDoubleClickEvent( QMouseEvent* e )
{
    if( !event_processor_ || event_processor_->mouseDoubleClickEvent( e ) == false )
        QWidget::mouseDoubleClickEvent( e );
}
