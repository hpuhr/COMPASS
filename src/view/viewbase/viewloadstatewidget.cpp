
#include "viewloadstatewidget.h"
#include "viewwidget.h"
#include "viewdatawidget.h"
#include "view.h"
#include "compass.h"
#include "viewmanager.h"
#include "dbcontentmanager.h"
#include "ui_test_common.h"

#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QHBoxLayout>

/**
*/
ViewLoadStateWidget::ViewLoadStateWidget(ViewWidget* view_widget, QWidget* parent)
:   QWidget     (parent     )
,   view_widget_(view_widget)
{
    assert (view_widget_);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->setMargin(DefaultMargin);
    setLayout(layout);

    QFont font_status;
    font_status.setItalic(true);

    auto_reload_button_ = new QToolButton;
    auto_reload_button_->setCheckable(true);
    auto_reload_button_->setToolTip("Enable/disable auto-reload");
    auto_reload_button_->setIcon(QIcon(Utils::Files::getIconFilepath("refresh.png").c_str()));

    connect(auto_reload_button_, &QPushButton::toggled, this, &ViewLoadStateWidget::setAutoReload);

    refresh_button_ = new QPushButton;
    refresh_button_->setToolTip("Refresh view");
    refresh_button_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    refresh_button_->setIcon(QIcon(Utils::Files::getIconFilepath("refresh.png").c_str()));

    UI_TEST_OBJ_NAME(refresh_button_, refresh_button_->text())
    connect(refresh_button_, &QPushButton::pressed, this, [=] () { this->updateData(); });

    status_label_ = new QLabel("");
    status_label_->setFont(font_status);

    layout->addWidget(auto_reload_button_);
    layout->addWidget(refresh_button_);
    layout->addWidget(status_label_);

    setState(State::NoData);
}

/**
 * Triggered by the reload button. Updates the data depending on the action needed by the current state.
*/
void ViewLoadStateWidget::updateData()
{
    if (view_widget_->getViewDataWidget()->hasData() && (state_ == State::RedrawRequired || state_ == State::ReloadRequired))
        view_widget_->getView()->updateView();
    else
        COMPASS::instance().dbContentManager().load();
}

/**
 * Sets the widget to a certain state.
*/
void ViewLoadStateWidget::setState(State state)
{
    state_ = state;

    //get message and color from state
    auto   msg   = messageFromState(state);
    QColor color = colorFromState(state);

    //the view widget may provide us with a special load message
    if (state == State::Loaded && view_widget_)
    {
        auto load_message = view_widget_->loadedMessage();
        if (!load_message.empty())
            msg += " - " + load_message;
    }

    status_label_->setText(QString::fromStdString(msg));

    QPalette palette = status_label_->palette();
    palette.setColor(status_label_->foregroundRole(), color);
    status_label_->setPalette(palette);

    bool button_enabled = (state == State::NoData ||
                           state == State::RedrawRequired ||
                           state == State::ReloadRequired);

    //refresh_button_->setToolTip(QString::fromStdString(buttonTextFromState(state_)));
    refresh_button_->setEnabled(button_enabled);

    auto_reload_button_->blockSignals(true);
    auto_reload_button_->setChecked(COMPASS::instance().viewManager().automaticReloadEnabled());
    auto_reload_button_->blockSignals(false);
}

/**
 * Updates the current state by querying the view widget for information.
*/
void ViewLoadStateWidget::updateState()
{
    //not needed in live running
    if (COMPASS::instance().appMode() == AppMode::LiveRunning)
        return;

    //return if we are in loading/drawing state (will be set back after those operations have finished)
    if (state_ == State::Loading ||
        state_ == State::Drawing)
        return;

    //ask widget for special states
    bool needs_reload =  COMPASS::instance().viewManager().reloadNeeded();
    bool needs_redraw =  view_widget_->getView()->updateNeeded();
    bool needs_data   = !view_widget_->getViewDataWidget()->hasData();

    //std::cout << "needs reload: " << needs_reload << std::endl;
    //std::cout << "needs redraw: " << needs_redraw << std::endl;
    //std::cout << "needs data:   " << needs_data   << std::endl;

    if (needs_data)
        setState(State::NoData);
    else if (needs_reload)
        setState(State::ReloadRequired);
    else if (needs_redraw)
        setState(State::RedrawRequired);
    else
        setState(State::Loaded); //everything up-to-date
}

/**
 * Enables/disables auto-reload.
 */
void ViewLoadStateWidget::setAutoReload(bool enabled)
{
    COMPASS::instance().viewManager().enableAutomaticReload(enabled);
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

    setState(State::None); //reset state
    updateState();         //determine new state
}

/**
 * Reacts on starting to redraw data.
*/
void ViewLoadStateWidget::redrawStarted()
{
    //not needed in live running
    if (COMPASS::instance().appMode() == AppMode::LiveRunning)
        return;

    setState(State::Drawing);
}

/**
 * Reacts on finishing to redraw data.
*/
void ViewLoadStateWidget::redrawDone()
{
    //not needed in live running
    if (COMPASS::instance().appMode() == AppMode::LiveRunning)
        return;

    setState(State::None); //reset state
    updateState();         //determine new state
}

/**
 * Reacts on switching the application mode.
*/
void ViewLoadStateWidget::appModeSwitch(AppMode app_mode)
{
    //hide ui in live running mode (no manual updates)
    refresh_button_->setHidden(app_mode == AppMode::LiveRunning);
    status_label_->setHidden(app_mode == AppMode::LiveRunning);
}

/**
 * Returns a displayable message given a state.
*/
std::string ViewLoadStateWidget::messageFromState(State state)
{
    switch(state)
    {
        case State::NoData:
            return "No Data Loaded";
        case State::Loading:
            return "Refreshing...";
        case State::Drawing:
            return "Refreshing...";
        case State::None:
        case State::Loaded:
            return "Up-to-date";
        case State::ReloadRequired:
            return "Refresh Required";
        case State::RedrawRequired:
            return "Refresh Required";
    }
    return "";
}

/**
 * Returns a button label given a state.
*/
std::string ViewLoadStateWidget::buttonTextFromState(State state)
{
    //label for redraw states
    if (state == State::RedrawRequired || 
        state == State::Drawing)
        return "Redraw";
    
    //reload label in all other cases
    return "Reload";
}

/**
 * Returns a label color given a state.
*/
QColor ViewLoadStateWidget::colorFromState(State state)
{
    switch (state)
    {
        case State::None:
        case State::Loading:
        case State::Drawing:
        case State::Loaded:
        case State::NoData:
            return Qt::black;
        case State::ReloadRequired:
        case State::RedrawRequired:
            return Qt::red;
    }
    return Qt::black;
}
