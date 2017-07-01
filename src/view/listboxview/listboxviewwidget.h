/*
 * ListBoxViewWidget.h
 *
 *  Created on: Nov 11, 2012
 *      Author: sk
 */

#ifndef LISTBOXVIEWWIDGET_H_
#define LISTBOXVIEWWIDGET_H_

#include "viewwidget.h"
#include "eventprocessor.h"

class ListBoxView;
class QTabWidget;
class ListBoxViewDataWidget;
class ListBoxViewConfigWidget;

/**
 * @brief Used for textual data display in a ListBoxView.
 *
 * Consists of a ListBoxViewDataWidget for data view and a ListBoxViewConfigWidget for configuration and
 * starting the loading process.
 */
class ListBoxViewWidget : public ViewWidget
{
public:
    /// @brief Constructor
    ListBoxViewWidget( const std::string& class_id, const std::string& instance_id, Configurable* config_parent, ListBoxView* view, QWidget* parent=NULL );
    /// @brief Destructor
    virtual ~ListBoxViewWidget();

    /// @brief Does nothing.
    virtual void updateView();

    /// @brief Toggles visibility of the config widget
    void toggleConfigWidget();

    /// @brief Returns the config widget
    ListBoxViewConfigWidget* configWidget();

    /// @brief Returns the basis view
    ListBoxView* getView() { return (ListBoxView*)view_; }
    /// @brief Returns the data widget
    ListBoxViewDataWidget *getDataWidget () { return data_widget_; }

protected:
    /// Data widget with data display
    ListBoxViewDataWidget *data_widget_;
    /// Config widget with configuration elements
    ListBoxViewConfigWidget* config_widget_;
    /// Tab widget for enclosed BufferTableWidgets
    QTabWidget* tab_widget_;
};

#endif /* LISTBOXVIEWWIDGET_H_ */
