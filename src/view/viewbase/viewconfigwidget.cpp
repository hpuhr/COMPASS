
#include "viewconfigwidget.h"
#include "viewwidget.h"

#include <iostream>

/**
*/
ViewConfigWidget::ViewConfigWidget(ViewWidget* view_widget, QWidget* parent, Qt::WindowFlags f) 
:   QWidget     (parent, f)
,   view_widget_(view_widget)
{
    assert(view_widget_);
}

/**
 * React on loading start.
*/
void ViewConfigWidget::loadingStarted()
{
    setDisabled(true);
}

/**
 * React on loading end.
*/
void ViewConfigWidget::loadingDone()
{
    setDisabled(false);
}

/**
 * React on redraw start.
*/
void ViewConfigWidget::redrawStarted()
{
    setDisabled(true);
}

/**
 * React on redraw end.
*/
void ViewConfigWidget::redrawDone()
{
    setDisabled(false);
}

/**
 * Reacts on switching the application mode.
 */
void ViewConfigWidget::appModeSwitch(AppMode app_mode) 
{
    //per default do nothing
}

/**
 * Reacts on changes in the display (e.g. if display information should be visualized in the config widget).
 * (Note: Only called if both config and data widget exist)
 */
void ViewConfigWidget::onDisplayChange()
{
    //invoke derived
    onDisplayChange_impl();
}
