
#pragma once

#include "appmode.h"

#include <QWidget>

class ViewWidget;
class View;

/**
 * Base class for view configuration widgets, which are held in the configuration area of the ViewWidget.
 * Derive and reimplement as needed.
*/
class ViewConfigWidget : public QWidget
{
public:
    ViewConfigWidget(ViewWidget* view_widget, QWidget* parent = nullptr, Qt::WindowFlags f = 0);
    virtual ~ViewConfigWidget() = default;

    void onDisplayChange();

    virtual void loadingStarted();
    virtual void loadingDone();
    virtual void redrawStarted();
    virtual void redrawDone();
    virtual void appModeSwitch(AppMode app_mode);

protected:
    virtual void onDisplayChange_impl() {} 

    ViewWidget* getWidget() { return view_widget_; }

private:
    ViewWidget* view_widget_ = nullptr;
};
