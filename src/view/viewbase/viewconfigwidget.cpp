
#include "viewconfigwidget.h"
#include "viewwidget.h"
#include "viewpresetwidget.h"
#include "viewmanager.h"
#include "compass.h"

#include <iostream>

#include <QTabWidget>
#include <QVBoxLayout>

/********************************************************************************************
 * ViewConfigWidget
 ********************************************************************************************/

/**
*/
ViewConfigWidget::ViewConfigWidget(ViewWidget* view_widget, QWidget* parent, Qt::WindowFlags f) 
:   QWidget     (parent, f)
,   view_widget_(view_widget)
{
    assert(view_widget_);

    setObjectName("configwidget");

    setMinimumWidth(MinWidth);
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
 * Reacts on config changes.
*/
void ViewConfigWidget::configChanged()
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

/********************************************************************************************
 * TabStyleViewConfigWidget
 ********************************************************************************************/

/**
*/
TabStyleViewConfigWidget::TabStyleViewConfigWidget(ViewWidget* view_widget, QWidget* parent, Qt::WindowFlags f) 
:   ViewConfigWidget(view_widget, parent, f)
{
    //create main layout
    main_layout_ = new QVBoxLayout;
    main_layout_->setMargin(1);
    main_layout_->setContentsMargins(1, 1, 1, 1);
    setLayout(main_layout_);

    // //create view preset widget
    // if (COMPASS::instance().viewManager().viewPresetsEnabled())
    // {
    //     auto preset_widget = new ViewPresetWidget(getWidget()->getView(), this);
        
    //     main_layout_->addWidget(preset_widget);
    // }

    //create tab widget
    tab_widget_ = new QTabWidget(this);
    //tab_widget_->setStyleSheet("QTabBar::tab { height: "+ QString::number(TabHeight) + "px; }");
    main_layout_->addWidget(tab_widget_);
}
