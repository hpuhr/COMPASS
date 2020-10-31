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

#include "view.h"
#include "logger.h"
#include "viewcontainer.h"
#include "viewmanager.h"
#include "viewmodel.h"
#include "viewwidget.h"
#include "viewpoint.h"
#include "atsdb.h"

#include <QVBoxLayout>
#include <QWidget>

#include <cassert>

unsigned int View::cnt_ = 0;

/**
@brief Constructor.
@param class_id Configurable class id.
@param instance_id Configurable instance id.
@param w ViewContainerWidget the view is embedded in, configurable parent.
 */
View::View(const std::string& class_id, const std::string& instance_id, ViewContainer* container,
           ViewManager& view_manager)
    : Configurable(class_id, instance_id, container),
      view_manager_(view_manager),
      model_(nullptr),
      widget_(nullptr),
      container_(container)
{
    logdbg << "View: constructor";

    central_widget_ = new QWidget();
    //central_widget_->setAutoFillBackground(true);

    connect(this, &View::selectionChangedSignal, &view_manager_, &ViewManager::selectionChangedSlot);

    connect(&view_manager_, &ViewManager::selectionChangedSignal, this, &View::selectionChangedSlot);

    connect(&view_manager_, &ViewManager::unshowViewPointSignal, this, &View::unshowViewPointSlot);
    connect(&view_manager_, &ViewManager::showViewPointSignal, this, &View::showViewPointSlot);
}

/**
@brief Destructor.

Just deleting a view is totally feasible and will remove the view from its ViewContainerWidget.
 */
View::~View()
{
    // delete model
    if (model_)
        delete model_;

    // unregister from manager
    if (view_manager_.isRegistered(this))
        view_manager_.unregisterView(this);

    // remove view from container widget first, then delete it
    container_->removeView(this);
    delete central_widget_;
}

/**
@brief Inits the view.

Construct the widget and model here in derived views. Construct the widget first,
then the model, which will need the widget in it's constructor.
 */
bool View::init()
{
    logdbg << "View: init";

    // register in manager
    view_manager_.registerView(this);

    // add view to container widget
    container_->addView(this);

    return true;
}

/**
@brief Returns the views name.
@return The views name.
 */
const std::string& View::getName() const { return instanceId(); }

void View::showInTabWidget()
{
    assert (container_);
    container_->showView(getCentralWidget());
}

/**
@brief Returns a unique instance key.
@return Unique instance key.
 */
unsigned int View::getInstanceKey()
{
    ++cnt_;
    return cnt_;
}

/**
@brief Sets the view's model.

Should only be called once to set the model.
@param model The view's model.
 */
void View::setModel(ViewModel* model)
{
    if (model_)
        throw std::runtime_error("View: setModel: Model already set.");
    model_ = model;
}

/**
@brief Sets the view's widget.

Should only be called once to set the widget.
@param model The view's widget.
 */
void View::setWidget(ViewWidget* widget)
{
    if (model_)
        throw std::runtime_error("View: setWidget: Widget already set.");

    widget_ = widget;
    if (widget_)
        constructWidget();
}

/**
@brief Embeds the view's widget into the central widget.
 */
void View::constructWidget()
{
    assert(widget_);
    assert(central_widget_);

    QVBoxLayout* central_vlayout = new QVBoxLayout();
    QHBoxLayout* central_hlayout = new QHBoxLayout();
    central_widget_->setLayout(central_vlayout);
    widget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    central_hlayout->addWidget(widget_);
    central_vlayout->addLayout(central_hlayout);
}

/**
@brief Will delete the view and show the given error message.
@param err An error message.
 */
void View::viewShutdown(const std::string& err) { view_manager_.viewShutdown(this, err); }

void View::emitSelectionChange()
{
    //    assert (!selection_change_emitted_);
    //    selection_change_emitted_ = true;

    emit selectionChangedSignal();
}

void View::selectionChangedSlot()
{
    //    if (selection_change_emitted_)
    //        selection_change_emitted_ = false;
    //    else // only update if not self-emitted
    updateSelection();
}
