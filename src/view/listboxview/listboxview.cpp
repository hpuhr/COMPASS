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

#include "listboxview.h"

#include <QApplication>

#include "compass.h"
//#include "dbcontent/dbcontentmanager.h"
#include "listboxviewconfigwidget.h"
#include "listboxviewdatasource.h"
#include "listboxviewdatawidget.h"
#include "listboxviewwidget.h"
#include "logger.h"
#include "latexvisitor.h"

const std::string ParamCurrentSetName  = "current_set_name";
const std::string ParamShowSelected    = "show_only_selected";
const std::string ParamUsePresentation = "use_presentation";
const std::string SubConfigDataSource  = "ListBoxViewDataSource";
const std::string SubConfigViewWidget  = "ListBoxViewWidget";

const std::string ListBoxView::DefaultSetName = "Default";

/**
*/
ListBoxView::Settings::Settings()
:   current_set_name  (DefaultSetName)
,   show_only_selected(false)
,   use_presentation  (true)
{
}

/**
*/
ListBoxView::ListBoxView(const std::string& class_id, 
                         const std::string& instance_id,
                         ViewContainer* w, 
                         ViewManager& view_manager)
:   View(class_id, instance_id, w, view_manager)
{
    registerParameter(ParamCurrentSetName, &settings_.current_set_name, Settings().current_set_name);
    registerParameter(ParamShowSelected, &settings_.show_only_selected, Settings().show_only_selected);
    registerParameter(ParamUsePresentation, &settings_.use_presentation, Settings().use_presentation);
}

/**
*/
ListBoxView::~ListBoxView()
{
    if (data_source_)
    {
        delete data_source_;
        data_source_ = nullptr;
    }

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }
}

/**
*/
bool ListBoxView::init_impl()
{
    createSubConfigurables();

    assert(data_source_);

//    DBContentManager& object_man = COMPASS::instance().dbContentManager();
//    connect(&object_man, &DBContentManager::loadingDoneSignal,
//            this, &ListBoxView::allLoadingDoneSlot);
//    connect(&object_man, &DBContentManager::loadingDoneSignal,
//            widget_->getDataWidget(), &ListBoxViewDataWidget::loadingDoneSlot);

//    connect(data_source_, &ListBoxViewDataSource::loadingStartedSignal, widget_->getDataWidget(),
//            &ListBoxViewDataWidget::loadingStartedSlot);
//    connect(data_source_, &ListBoxViewDataSource::updateDataSignal, widget_->getDataWidget(),
//            &ListBoxViewDataWidget::updateDataSlot);

    connect(widget_->getViewConfigWidget(), &ListBoxViewConfigWidget::exportSignal,
            widget_->getViewDataWidget(), &ListBoxViewDataWidget::exportDataSlot);
    connect(widget_->getViewDataWidget(), &ListBoxViewDataWidget::exportDoneSignal,
            widget_->getViewConfigWidget(), &ListBoxViewConfigWidget::exportDoneSlot);

//    connect(data_source_, &ListBoxViewDataSource::loadingStartedSignal, widget_->configWidget(),
//            &ListBoxViewConfigWidget::loadingStartedSlot);

    connect(this, &ListBoxView::showOnlySelectedSignal,
            widget_->getViewDataWidget(), &ListBoxViewDataWidget::showOnlySelectedSlot);
    connect(this, &ListBoxView::usePresentationSignal,
            widget_->getViewDataWidget(), &ListBoxViewDataWidget::usePresentationSlot);

    widget_->getViewDataWidget()->showOnlySelectedSlot(settings_.show_only_selected);
    widget_->getViewDataWidget()->usePresentationSlot(settings_.use_presentation);

    return true;
}

/**
*/
void ListBoxView::generateSubConfigurable(const std::string& class_id,
                                          const std::string& instance_id)
{
    logdbg << "ListBoxView: generateSubConfigurable: class_id " << class_id << " instance_id "
           << instance_id;
    if (class_id == SubConfigDataSource)
    {
        assert(!data_source_);
        data_source_ = new ListBoxViewDataSource(settings_.current_set_name, class_id, instance_id, this);

        //write current set back to local settings
        connect(data_source_, &ListBoxViewDataSource::currentSetChangedSignal, [ this ] { this->settings_.current_set_name = data_source_->currentSetName(); });

        //notify view that it needs to reload
        connect(data_source_, &ListBoxViewDataSource::reloadNeeded, [ this ] { widget_->notifyReloadNeeded(); });
    }
    else if (class_id == SubConfigViewWidget)
    {
        widget_ = new ListBoxViewWidget(class_id, instance_id, this, this, central_widget_);
        setWidget(widget_);
    }
    else
    {
        throw std::runtime_error("ListBoxView: generateSubConfigurable: unknown class_id " +
                                 class_id);
    }
}

/**
*/
void ListBoxView::checkSubConfigurables()
{
    if (!data_source_)
    {
        generateSubConfigurable(SubConfigDataSource, SubConfigDataSource + "0");
    }

    if (!widget_)
    {
        generateSubConfigurable(SubConfigViewWidget, SubConfigViewWidget + "0");
    }
}

ListBoxViewDataWidget* ListBoxView::getDataWidget()
{
    assert (widget_);
    return widget_->getViewDataWidget();
}

dbContent::VariableSet ListBoxView::getSet(const std::string& dbcontent_name)
{
    assert(data_source_);

    return data_source_->getSet()->getExistingInDBFor(dbcontent_name);
}

// void ListBoxView::selectionChanged()
//{
//    //  assert (data_source_);
//    //  data_source_->updateSelection();
//}
// void ListBoxView::selectionToBeCleared()
//{

//}

bool ListBoxView::usePresentation() const 
{ 
    return settings_.use_presentation; 
}

void ListBoxView::usePresentation(bool use_presentation)
{
    settings_.use_presentation = use_presentation;

    emit usePresentationSignal(settings_.use_presentation);
}

bool ListBoxView::showOnlySelected() const 
{ 
    return settings_.show_only_selected; 
}

void ListBoxView::showOnlySelected(bool value)
{
    loginf << "ListBoxView: showOnlySelected: " << value;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    settings_.show_only_selected = value;

    emit showOnlySelectedSignal(value);

    QApplication::restoreOverrideCursor();
}

void ListBoxView::accept(LatexVisitor& v)
{
    v.visit(this);
}

void ListBoxView::updateSelection()
{
    loginf << "ListBoxView: updateSelection";
    assert(widget_);

    if (settings_.show_only_selected)
        widget_->getViewDataWidget()->updateToSelection();
    else
        widget_->getViewDataWidget()->resetModels();  // just updates the checkboxes
}

void ListBoxView::unshowViewPointSlot (const ViewableDataConfig* vp)
{
    loginf << "ListBoxView: unshowViewPoint";

    assert (vp);
    assert (data_source_);
    data_source_->unshowViewPoint(vp);
}

void ListBoxView::showViewPointSlot (const ViewableDataConfig* vp)
{
    loginf << "ListBoxView: showViewPoint";

    assert (vp);
    assert (data_source_);
    data_source_->showViewPoint(vp);
    assert (widget_);
}

View::ViewUpdate ListBoxView::onConfigurationChanged_impl(const std::vector<std::string>& changed_params)
{
    for (const auto& param : changed_params)
    {
        if (param == ParamShowSelected)
        {
            emit showOnlySelectedSignal(settings_.show_only_selected); 
        }
        else if (param == ParamUsePresentation)
        {
            emit usePresentationSignal(settings_.use_presentation);
        }
        else if (param == ParamCurrentSetName)
        {
            //update current set in data source
            getDataSource()->currentSetName(settings_.current_set_name, true);
        }
    }

    //return empty view update (not needed explicitely for listboxview)
    return {};
}
