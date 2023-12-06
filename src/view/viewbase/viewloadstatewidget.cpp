
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
#include <QVBoxLayout>
#include <QCheckBox>

/**
*/
ViewLoadStateWidget::ViewLoadStateWidget(ViewWidget* view_widget, QWidget* parent)
:   QWidget     (parent     )
,   view_widget_(view_widget)
{
    assert (view_widget_);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setMargin(DefaultMargin);
    setLayout(layout);

    QHBoxLayout* layout_h = new QHBoxLayout;
    layout_h->setMargin(0);

    QHBoxLayout* layout_buttons = new QHBoxLayout;
    layout_buttons->setMargin(0);
    layout_buttons->setSpacing(0);

    layout->addLayout(layout_h);
    layout_h->addLayout(layout_buttons);
    
    QFont font_status;
    font_status.setItalic(true);

    auto_refresh_button_ = new QToolButton;
    auto_refresh_button_->setCheckable(true);
    auto_refresh_button_->setToolTip("Enable/disable auto-reload");
    auto_refresh_button_->setIcon(QIcon(Utils::Files::getIconFilepath("refresh.png").c_str()));

    refresh_button_ = new QPushButton;
    refresh_button_->setToolTip("Refresh view");
    refresh_button_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    refresh_button_->setIcon(QIcon(Utils::Files::getIconFilepath("refresh.png").c_str()));

    UI_TEST_OBJ_NAME(refresh_button_, refresh_button_->text())
    
    layout_buttons->addWidget(auto_refresh_button_);
    layout_buttons->addWidget(refresh_button_);

    status_label_ = new QLabel("");
    status_label_->setFont(font_status);
    layout_h->addWidget(status_label_);

    auto_refresh_box_ = new QCheckBox("Auto-refresh");
    
    layout->addWidget(auto_refresh_box_);

    auto_refresh_button_->setVisible(false);
    auto_refresh_box_->setVisible(true);

    setState(State::NoData);

    connect(refresh_button_, &QPushButton::pressed, this, [=] () { this->updateData(); });

    connect(auto_refresh_button_, &QPushButton::toggled, this, &ViewLoadStateWidget::setAutoReload);
    connect(auto_refresh_box_, &QPushButton::toggled, this, &ViewLoadStateWidget::setAutoReload);
}

/**
 * Triggered by the reload button. Updates the data depending on the action needed by the current state.
*/
void ViewLoadStateWidget::updateData()
{
    if (view_widget_->getViewDataWidget()->hasData() && (state_ == State::RedrawRequired || state_ == State::ReloadRequired))
        view_widget_->getView()->updateView(); //run view update
    else
        COMPASS::instance().dbContentManager().load(); //fallback: just reload
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

    //update refresh button activity
    bool button_enabled = (state == State::NoData ||
                           state == State::RedrawRequired ||
                           state == State::ReloadRequired);

    //refresh_button_->setToolTip(QString::fromStdString(buttonTextFromState(state_)));
    refresh_button_->setEnabled(button_enabled);

    //update auto-refresh button state
    auto_refresh_button_->blockSignals(true);
    auto_refresh_button_->setChecked(COMPASS::instance().viewManager().automaticReloadEnabled());
    auto_refresh_button_->blockSignals(false);

    auto_refresh_box_->blockSignals(true);
    auto_refresh_box_->setChecked(COMPASS::instance().viewManager().automaticReloadEnabled());
    auto_refresh_box_->blockSignals(false);
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
