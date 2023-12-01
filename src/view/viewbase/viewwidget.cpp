/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "viewwidget.h"
#include "view.h"
#include "viewtoolwidget.h"
#include "viewdatawidget.h"
#include "viewconfigwidget.h"
#include "viewtoolswitcher.h"
#include "viewloadstatewidget.h"
#include "files.h"
#include "compass.h"
#include "ui_test_common.h"
#include "dbcontentmanager.h"
#include "dbcontent.h"
#include "viewmanager.h"

#include <QVBoxLayout>
#include <QSplitter>
#include <QSettings>
#include <QPainter>

/**
@brief Constructor.
@param class_id Configurable class id.
@param instance_id Configurable instance id.
@param config_parent Configurable parent.
@param view The view the view widget is part of.
@param parent The widgets parent.
*/
ViewWidget::ViewWidget(const std::string& class_id, 
                       const std::string& instance_id,
                       Configurable* config_parent, 
                       View* view, 
                       QWidget* parent)
    : QWidget     (parent),
      Configurable(class_id, instance_id, config_parent),
      view_       (view)
{
    //generate and set a nice object name which can be used to identify the view widget in the object hierarchy
    UI_TEST_OBJ_NAME(this, QString::fromStdString(view->getName()))

    setContentsMargins(0, 0, 0, 0);

    tool_switcher_.reset(new ViewToolSwitcher);

    createStandardLayout();
}

/**
@brief Destructor.
*/
ViewWidget::~ViewWidget()
{
    if (main_splitter_)
    {
        QSettings settings("COMPASS", instanceId().c_str());
        settings.setValue("mainSplitterSizes", main_splitter_->saveState());
    }
}

/**
 * Creates the standard view layout.
 */
void ViewWidget::createStandardLayout()
{
    //create main layout
    QVBoxLayout* main_layout = new QVBoxLayout;
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->setSpacing(0);
    main_layout->setMargin(0);

    setLayout(main_layout);

    //create tool widget
    {
        tool_widget_ = new ViewToolWidget(this, tool_switcher_.get(), this);
        tool_widget_->setContentsMargins(0, 0, 0, 0);

        if (COMPASS::instance().viewManager().viewPresetsEnabled())
            tool_widget_->addPresetSelection();

        main_layout->addWidget(tool_widget_);
    }

    //central layout
    QHBoxLayout* central_layout = new QHBoxLayout;
    central_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->addLayout(central_layout);
    
    //create lower widget container
    {
        lower_widget_container_ = new QWidget;
        lower_widget_container_->setVisible(false);
        lower_widget_container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        main_layout->addWidget(lower_widget_container_);
    }

    //main splitter (containing a left and a right widget)
    main_splitter_ = new QSplitter;
    main_splitter_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    main_splitter_->setOrientation(Qt::Horizontal);
    main_splitter_->setContentsMargins(0, 0, 0, 0);

    QSettings settings("COMPASS", instanceId().c_str());

    const int DataWidgetStretch   = 5;
    const int ConfigWidgetStretch = 1;

    QWidget* left_widget = new QWidget;
    left_widget->setContentsMargins(0, 0, 0, 0);

    right_widget_ = new QWidget;
    right_widget_->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* left_layout = new QVBoxLayout;
    left_layout->setContentsMargins(0, 0, 0, 0);
    left_layout->setMargin(0);

    QVBoxLayout* right_layout = new QVBoxLayout;
    right_layout->setContentsMargins(0, 0, 0, 0);
    right_layout->setMargin(0);

    left_widget->setLayout(left_layout);
    right_widget_->setLayout(right_layout);

    main_splitter_->addWidget(left_widget);
    main_splitter_->addWidget(right_widget_);

    //create data widget container in left widget
    {
        QSizePolicy size_policy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        size_policy.setHorizontalStretch(DataWidgetStretch);

        data_widget_container_ = new QWidget;
        data_widget_container_->setSizePolicy(size_policy);
        data_widget_container_->setContentsMargins(0, 0, 0, 0);

        left_layout->addWidget(data_widget_container_);
    }

    //create config widget container in right widget
    {
        config_widget_container_ = new QWidget;

        QSizePolicy size_policy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        size_policy.setHorizontalStretch(ConfigWidgetStretch);

        config_widget_container_->setSizePolicy(size_policy);
        config_widget_container_->setContentsMargins(0, 0, 0, 0);

        right_layout->addWidget(config_widget_container_);
    }

    //create load state widget in right widget
    {
        state_widget_ = new ViewLoadStateWidget(this, right_widget_);

        right_layout->addWidget(state_widget_);
    }

    //add main splitter to central layout and restore state from config
    main_splitter_->restoreState(settings.value("mainSplitterSizes").toByteArray());
    central_layout->addWidget(main_splitter_);

    setFocusPolicy(Qt::StrongFocus);

    //deactivate collapsing of children
    main_splitter_->setChildrenCollapsible(false); 
    for (int i = 0; i < main_splitter_->count(); ++i)
        main_splitter_->setCollapsible(i, false);
}

/**
 * View initialization. Called by View on view creation.
*/
void ViewWidget::init()
{
    //init should only be called once
    if (isInit())
        throw std::runtime_error("ViewWidget::init(): Called twice");

    //check if all relevant widgets have been constructed
    assert(data_widget_);
    assert(config_widget_);
    assert(tool_widget_);
    assert(state_widget_);

    //add screenshot button
    tool_widget_->addScreenshotButton();

    //add toggle button for config widget
    tool_widget_->addConfigWidgetToggle();

    //call derived
    init_impl();

    init_ = true;

    //update view components after init is done
    updateComponents();
}

/**
 */
void ViewWidget::setDataWidget(ViewDataWidget* w)
{
    if (!w)
        throw std::runtime_error("ViewWidget::setDataWidget: Null pointer passed");
    if (!data_widget_container_)
        throw std::runtime_error("ViewWidget::setDataWidget: No container to add to");
    if (data_widget_)
        throw std::runtime_error("ViewWidget::setDataWidget: Already set");
    
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    data_widget_container_->setLayout(layout);

    layout->addWidget(w);

    data_widget_ = w;
    data_widget_->setToolSwitcher(tool_switcher_.get());

    connect(data_widget_, &ViewDataWidget::redrawStarted, this, &ViewWidget::redrawStarted);
    connect(data_widget_, &ViewDataWidget::redrawDone, this, &ViewWidget::redrawDone);

    //try to connect
    connectWidgets();
}

/**
 */
void ViewWidget::setConfigWidget(ViewConfigWidget* w)
{
    if (!w)
        throw std::runtime_error("ViewWidget::setConfigWidget: Null pointer passed");
    if (!config_widget_container_)
        throw std::runtime_error("ViewWidget::setConfigWidget: No container to add to");

    //remove old config widget?
    if (config_widget_)
    {
        config_widget_container_->layout()->removeWidget(config_widget_);

        //setLayout() can only be called on widgets without layout
        //the old layout can be removed by just deleting it
        delete config_widget_container_->layout();

        delete config_widget_;
        config_widget_ = nullptr;
    }

    //add new config widget
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    config_widget_container_->setLayout(layout);

    layout->addWidget(w);

    config_widget_ = w;

    //try to connect
    connectWidgets();
}

/**
 * Tries to connect data and config widget.
 * (Note: Could also be done in init() actually)
 */
void ViewWidget::connectWidgets()
{
    if (config_widget_ && data_widget_)
    {
        connect(data_widget_, &ViewDataWidget::displayChanged, config_widget_, &ViewConfigWidget::onDisplayChange);
    }
}

/**
 */
void ViewWidget::setLowerWidget(QWidget* w)
{
    if (!w)
        throw std::runtime_error("ViewWidget::setLowerWidget: Null pointer passed");
    if (!lower_widget_container_)
        throw std::runtime_error("ViewWidget::setLowerWidget: No container to add to");
    if (lower_widget_)
        throw std::runtime_error("ViewWidget::setLowerWidget: Already set");

    lower_widget_ = w;
    lower_widget_->setParent(lower_widget_container_);
    
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    lower_widget_container_->setLayout(layout);

    layout->addWidget(lower_widget_);

    lower_widget_container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    lower_widget_container_->setVisible(true);
}

/**
 */
void ViewWidget::toggleConfigWidget()
{
    assert(right_widget_);

    bool vis = right_widget_->isVisible();
    right_widget_->setVisible(!vis);
}

/**
 */
QIcon ViewWidget::getIcon(const std::string& fn)
{
    return QIcon(Utils::Files::getIconFilepath(fn).c_str());
}

/**
*/
void ViewWidget::loadingStarted()
{
    //propagate to subwidgets (note: order might be important)
    getViewDataWidget()->loadingStarted();
    getViewConfigWidget()->loadingStarted();
    getViewLoadStateWidget()->loadingStarted();
    getViewToolWidget()->loadingStarted();
}

/**
*/
void ViewWidget::loadingDone()
{
    //set back flag
    reload_needed_ = false;
    redraw_needed_ = false; //a reload should always result in a redraw anyway

    //propagate to subwidgets (note: order might be important)
    getViewDataWidget()->loadingDone();
    getViewConfigWidget()->loadingDone();
    getViewLoadStateWidget()->loadingDone();
    getViewToolWidget()->loadingDone();
}

/**
*/
void ViewWidget::redrawStarted()
{
    //propagate to subwidgets (note: order might be important)
    getViewConfigWidget()->redrawStarted();
    getViewLoadStateWidget()->redrawStarted();
    getViewToolWidget()->redrawStarted();
}

/**
*/
void ViewWidget::redrawDone()
{
    //set back flag
    redraw_needed_ = false;

    //propagate to subwidgets (note: order might be important)
    getViewConfigWidget()->redrawDone();
    getViewLoadStateWidget()->redrawDone();
    getViewToolWidget()->redrawDone();
}

/**
*/
void ViewWidget::appModeSwitch(AppMode app_mode)
{
    //propagate to subwidgets (note: order might be important)
    getViewDataWidget()->appModeSwitch(app_mode);
    getViewConfigWidget()->appModeSwitch(app_mode);
    getViewLoadStateWidget()->appModeSwitch(app_mode);
    getViewToolWidget()->appModeSwitch(app_mode);
}

/**
*/
void ViewWidget::configChanged()
{
    //propagate to subwidgets (note: order might be important)
    getViewConfigWidget()->configChanged();
    getViewDataWidget()->configChanged();
    getViewToolWidget()->configChanged();
}

/**
 * Updates the tool widget's items.
 */
void ViewWidget::updateToolWidget()
{
    if (!tool_widget_)
        return;

    getViewToolWidget()->updateItems();
}

/**
 * Updates the widget load state.
 */
void ViewWidget::updateLoadState()
{
    if (!state_widget_)
        return;

    getViewLoadStateWidget()->updateState();
}

/**
 * Updates the view widget's individual components.
*/
void ViewWidget::updateComponents()
{
    updateToolWidget();
    updateLoadState();
}

/**
 * Manually notifies the widget that a redraw is needed and updates the load state widget accordingly.
 * (Note: Might trigger an immediate redraw in live running mode)
 */
void ViewWidget::notifyRedrawNeeded()
{
    if (COMPASS::instance().appMode() == AppMode::LiveRunning)
    {
        //in live mode just redraw
        getViewDataWidget()->redrawData(true, false);
        return;
    } 

    redraw_needed_ = true;
    updateLoadState();
}

/**
 * Manually notifies the widget that a reload is needed and updates the load state widget accordingly.
*/
void ViewWidget::notifyReloadNeeded()
{
    if (COMPASS::instance().appMode() == AppMode::LiveRunning)
    {
        //in live mode a view handles its reload internally in its data widget
        getViewDataWidget()->liveReload();
        return;
    }

    reload_needed_ = true;
    updateLoadState();
}

/**
 * Manually notifies the widget that a refresh is needed, determined if redraw or reload, and updates the load state widget accordingly.
*/
void ViewWidget::notifyRefreshNeeded()
{
    bool has_varset = isVariableSetLoaded();

    //std::cout << "has varset: " << has_varset << std::endl;

    if (has_varset)
        notifyRedrawNeeded();
    else
        notifyReloadNeeded();
}

/**
 * Checks if a reload is needed.
*/
bool ViewWidget::reloadNeeded() const
{
    assert(isInit());

    return (reload_needed_ || reloadNeeded_impl());
}

/**
 * Checks if a redraw is needed.
*/
bool ViewWidget::redrawNeeded() const
{
    assert(isInit());

    return (redraw_needed_ || redrawNeeded_impl());
}

/**
 * Returns a view-specific loaded state message.
*/
std::string ViewWidget::loadedMessage() const
{
    assert(isInit());

    return loadedMessage_impl();
}

/**
 * Returns view-specific information as json struct.
 */
nlohmann::json ViewWidget::viewInfo(const std::string& what) const
{
    nlohmann::json info;

    info[ "name"       ] = view_->getName();

    info[ "data"       ] = getViewDataWidget()->viewInfo(what);
    info[ "config"     ] = getViewConfigWidget()->viewInfo(what);
    info[ "load_state" ] = getViewLoadStateWidget()->viewInfo(what);
    info[ "toolbar"    ] = getViewToolWidget()->viewInfo(what);

    nlohmann::json info_additional = viewInfo_impl(what);
    if (!info_additional.is_null())
        info[ "additional" ] = info_additional;

    return info;
}

/**
 * Running the 'uiget' rtcommand on a view widget will yield view specific json information.
 */
boost::optional<QString> ViewWidget::uiGet(const QString& what) const
{
    std::string view_info = viewInfo(what.toStdString()).dump();
    return QString::fromStdString(view_info);
}

/**
*/
bool ViewWidget::isVariableSetLoaded() const
{
    assert(data_widget_);
    return data_widget_->isVariableSetLoaded();
}

/**
*/
QImage ViewWidget::renderContents()
{
    assert (data_widget_);  
    QImage data_img = data_widget_->renderData();

    QImage img(this->size(), QImage::Format_ARGB32);
    QPainter painter(&img);

    render(&painter);

    auto p0 = this->mapToGlobal(QPoint(0,0));
    auto p1 = data_widget_->mapToGlobal(QPoint(0,0));

    painter.drawImage(p1 - p0, data_img);

    return img;
}
