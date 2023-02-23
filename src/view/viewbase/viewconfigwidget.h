
#pragma once

#include "appmode.h"

#include <QWidget>

class ViewConfigWidget : public QWidget
{
public:
    ViewConfigWidget(QWidget* parent = nullptr, Qt::WindowFlags f = 0) : QWidget(parent, f) {}
    virtual ~ViewConfigWidget() = default;

    void onDisplayChange()
    {
        onDisplayChange_impl();
    }

    virtual void loadingStarted();
    virtual void loadingDone();
    virtual void redrawStarted();
    virtual void redrawDone();
    virtual void appModeSwitch(AppMode app_mode) {}

protected:
    virtual void onDisplayChange_impl() {}
};
