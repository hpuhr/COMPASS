/*
 * OSGViewWidget.h
 *
 *  Created on: Nov 11, 2012
 *      Author: sk
 */

#ifndef OSGVIEWWIDGET_H_
#define OSGVIEWWIDGET_H_

#include "viewwidget.h"
#include "eventprocessor.h"

class OSGView;
class QTabWidget;
class OSGViewDataWidget;
class OSGViewConfigWidget;

/**
 * @brief Used for textual data display in a OSGView.
 *
 * Consists of a OSGViewDataWidget for data view and a OSGViewConfigWidget for configuration and
 * starting the loading process.
 */
class OSGViewWidget : public ViewWidget
{
public:
    /// @brief Constructor
    OSGViewWidget( const std::string& class_id, const std::string& instance_id, Configurable* config_parent, OSGView* view, QWidget* parent=nullptr );
    /// @brief Destructor
    virtual ~OSGViewWidget();

    /// @brief Does nothing.
    virtual void updateView();

    /// @brief Toggles visibility of the config widget
    void toggleConfigWidget();

    /// @brief Returns the config widget
    OSGViewConfigWidget* configWidget();

    /// @brief Returns the basis view
    OSGView* getView() { return (OSGView*)view_; }
    /// @brief Returns the data widget
    OSGViewDataWidget *getDataWidget () { return data_widget_; }

protected:
    /// Data widget with data display
    OSGViewDataWidget *data_widget_;
    /// Config widget with configuration elements
    OSGViewConfigWidget* config_widget_;
};

#endif /* OSGVIEWWIDGET_H_ */
