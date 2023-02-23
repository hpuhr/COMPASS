
#pragma once

#include "appmode.h"

#include <QWidget>

/**
 * Base class for view configuration widgets, which are used to configure a view.
*/
class ViewConfigWidget : public QWidget
{
public:
    ViewConfigWidget(QWidget* parent = nullptr, Qt::WindowFlags f = 0) : QWidget(parent, f) {}
    virtual ~ViewConfigWidget() = default;

    void onDisplayChange();

    virtual void loadingStarted();
    virtual void loadingDone();
    virtual void redrawStarted();
    virtual void redrawDone();
    virtual void appModeSwitch(AppMode app_mode);

protected:
    virtual void onDisplayChange_impl() {} 
};
