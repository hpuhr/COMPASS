
#include "viewloadstatewidget.h"
#include "viewwidget.h"
#include "viewdatawidget.h"
#include "compass.h"
#include "dbcontentmanager.h"
#include "ui_test_common.h"

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

/**
*/
ViewLoadStateWidget::ViewLoadStateWidget(ViewWidget* view_widget, QWidget* parent)
:   QWidget     (parent     )
,   view_widget_(view_widget)
{
    assert (view_widget_);

    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);

    QFont font_status;
    font_status.setItalic(true);

    status_label_  = new QLabel("");
    status_label_->setFont(font_status);

    reload_button_ = new QPushButton("Reload");
    UI_TEST_OBJ_NAME(reload_button_, reload_button_->text())
    connect(reload_button_, &QPushButton::clicked, this, [=] () { this->updateData(); });

    layout->addWidget(status_label_);
    layout->addWidget(reload_button_);

    setState(State::NoData);
}

/**
*/
void ViewLoadStateWidget::updateData()
{
    if (view_widget_->getViewDataWidget()->hasData() && state_ == State::RedrawRequired)
        view_widget_->getViewDataWidget()->redrawData();
    else
        COMPASS::instance().dbContentManager().load();
}

/**
*/
void ViewLoadStateWidget::setState(State state)
{
    state_ = state;

    //get message and color from state
    auto   msg   = messageFromState(state);
    QColor color = colorFromState(state);

    //the view widget may provide us with a special load message
    if (state == State::Loaded && view_widget_)
        msg = view_widget_->loadedMessage();

    status_label_->setText(QString::fromStdString(msg));

    QPalette palette = status_label_->palette();
    palette.setColor(status_label_->foregroundRole(), color);
    status_label_->setPalette(palette);

    bool label_visible  = (state != State::None && !msg.empty() && reload_button_->isVisible());
    bool button_enabled = (state != State::Loading);

    status_label_->setVisible(label_visible);
    reload_button_->setEnabled(button_enabled);
}

/**
 * 
*/
void ViewLoadStateWidget::updateState()
{
    //not needed in live running
    if (COMPASS::instance().appMode() == AppMode::LiveRunning)
        return;

    //ask widget if either a reload or a redraw is needed
    bool needs_reload = view_widget_->reloadNeeded();
    bool needs_redraw = view_widget_->redrawNeeded();

    if (needs_reload)
        setState(State::ReloadRequired);
    else if (needs_redraw)
        setState(State::RedrawRequired);
    else
        setState(State::Loaded); //everything up-to-date
}

/**
 * Reacts on starting to load from database.
*/
void ViewLoadStateWidget::loadingStarted()
{
    //not needed in live running
    if (COMPASS::instance().appMode() == AppMode::LiveRunning)
        return;

    setState(State::Loading);
}

/**
 * Reacts on finishing to load from database.
*/
void ViewLoadStateWidget::loadingDone()
{
    //not needed in live running
    if (COMPASS::instance().appMode() == AppMode::LiveRunning)
        return;

    updateState();
}

/**
 * Reacts on starting to redraw data.
*/
void ViewLoadStateWidget::redrawStarted()
{
    //@TODO
}

/**
 * Reacts on finishing to redraw data.
*/
void ViewLoadStateWidget::redrawDone()
{
    updateState();
}

/**
*/
void ViewLoadStateWidget::appModeSwitch(AppMode app_mode)
{
    //hide ui in live running mode (no manual updates)
    reload_button_->setHidden(app_mode == AppMode::LiveRunning);
    status_label_->setHidden(app_mode == AppMode::LiveRunning);
}

/**
*/
std::string ViewLoadStateWidget::messageFromState(State state)
{
    switch(state)
    {
        case State::NoData:
            return "No Data Loaded";
        case State::Loading:
            return "Loading...";
        case State::None:
        case State::Loaded:
            return "";
        case State::ReloadRequired:
            return "Reload Required";
        case State::RedrawRequired:
            return "Redraw Required";
    }
    return "";
}

/**
*/
QColor ViewLoadStateWidget::colorFromState(State state)
{
    switch (state)
    {
        case State::None:
        case State::Loading:
        case State::Loaded:
        case State::NoData:
            return Qt::black;
        case State::ReloadRequired:
        case State::RedrawRequired:
            return Qt::red;
    }
    return Qt::black;
}
