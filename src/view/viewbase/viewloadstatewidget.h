
#pragma once

#include "appmode.h"

#include <string>

#include <QWidget>
#include <QColor>

class ViewWidget;

class QLabel;
class QPushButton;

/**
*/
class ViewLoadStateWidget : public QWidget
{
public:
    enum class State
    {
        None = 0,
        NoData,
        Loading,
        Drawing,
        Loaded,
        ReloadRequired,
        RedrawRequired
    };

    ViewLoadStateWidget(ViewWidget* view_widget, QWidget* parent = nullptr);
    virtual ~ViewLoadStateWidget() = default;

    void setState(State state);
    void updateState();
    
    void loadingStarted();
    void loadingDone();
    void redrawStarted();
    void redrawDone();

    void appModeSwitch(AppMode app_mode);

private:
    static std::string messageFromState(State state);
    static QColor colorFromState(State state);
    static std::string buttonTextFromState(State state);

    void updateData();

    State        state_         = State::None;

    QLabel*      status_label_  = nullptr;
    QPushButton* reload_button_ = nullptr;

    ViewWidget*  view_widget_   = nullptr;
};
