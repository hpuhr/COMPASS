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
#include "dbobjectmanager.h"
#include "listboxviewconfigwidget.h"
#include "listboxviewdatasource.h"
#include "listboxviewdatawidget.h"
#include "listboxviewwidget.h"
#include "logger.h"
#include "viewselection.h"
#include "latexvisitor.h"

ListBoxView::ListBoxView(const std::string& class_id, const std::string& instance_id,
                         ViewContainer* w, ViewManager& view_manager)
    : View(class_id, instance_id, w, view_manager)
{
    registerParameter("show_only_selected", &show_only_selected_, false);
    registerParameter("use_presentation", &use_presentation_, true);
    registerParameter("overwrite_csv", &overwrite_csv_, true);
    registerParameter("show_associations", &show_associations_, false);

    can_show_associations_ = COMPASS::instance().objectManager().hasAssociations();

    if (!can_show_associations_)
        show_associations_ = false;
}

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

void ListBoxView::update(bool atOnce) {}

void ListBoxView::clearData() {}

bool ListBoxView::init()
{
    View::init();

    createSubConfigurables();

    assert(data_source_);

    DBObjectManager& object_man = COMPASS::instance().objectManager();
    connect(&object_man, &DBObjectManager::allLoadingDoneSignal, this, &ListBoxView::allLoadingDoneSlot);
    connect(&object_man, &DBObjectManager::allLoadingDoneSignal,
            widget_->getDataWidget(), &ListBoxViewDataWidget::loadingDoneSlot);

    connect(data_source_, &ListBoxViewDataSource::loadingStartedSignal, widget_->getDataWidget(),
            &ListBoxViewDataWidget::loadingStartedSlot);
    connect(data_source_, &ListBoxViewDataSource::updateDataSignal, widget_->getDataWidget(),
            &ListBoxViewDataWidget::updateDataSlot);

    connect(widget_->configWidget(), &ListBoxViewConfigWidget::exportSignal,
            widget_->getDataWidget(), &ListBoxViewDataWidget::exportDataSlot);
    connect(widget_->getDataWidget(), &ListBoxViewDataWidget::exportDoneSignal,
            widget_->configWidget(), &ListBoxViewConfigWidget::exportDoneSlot);

    connect(widget_->configWidget(), &ListBoxViewConfigWidget::reloadRequestedSignal,
            &COMPASS::instance().objectManager(), &DBObjectManager::loadSlot);
    connect(data_source_, &ListBoxViewDataSource::loadingStartedSignal, widget_->configWidget(),
            &ListBoxViewConfigWidget::loadingStartedSlot);

    connect(this, &ListBoxView::showOnlySelectedSignal, widget_->getDataWidget(),
            &ListBoxViewDataWidget::showOnlySelectedSlot);
    connect(this, &ListBoxView::usePresentationSignal, widget_->getDataWidget(),
            &ListBoxViewDataWidget::usePresentationSlot);
    connect(this, &ListBoxView::showAssociationsSignal, widget_->getDataWidget(),
            &ListBoxViewDataWidget::showAssociationsSlot);

    widget_->getDataWidget()->showOnlySelectedSlot(show_only_selected_);
    widget_->getDataWidget()->usePresentationSlot(use_presentation_);
    widget_->getDataWidget()->showAssociationsSlot(show_associations_);

    return true;
}

void ListBoxView::generateSubConfigurable(const std::string& class_id,
                                          const std::string& instance_id)
{
    logdbg << "ListBoxView: generateSubConfigurable: class_id " << class_id << " instance_id "
           << instance_id;
    if (class_id == "ListBoxViewDataSource")
    {
        assert(!data_source_);
        data_source_ = new ListBoxViewDataSource(class_id, instance_id, this);
    }
    else if (class_id == "ListBoxViewWidget")
    {
        widget_ = new ListBoxViewWidget(class_id, instance_id, this, this, central_widget_);
        setWidget(widget_);
    }
    else
        throw std::runtime_error("ListBoxView: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

void ListBoxView::checkSubConfigurables()
{
    if (!data_source_)
    {
        generateSubConfigurable("ListBoxViewDataSource", "ListBoxViewDataSource0");
    }

    if (!widget_)
    {
        generateSubConfigurable("ListBoxViewWidget", "ListBoxViewWidget0");
    }
}

ListBoxViewDataWidget* ListBoxView::getDataWidget()
{
    assert (widget_);
    return widget_->getDataWidget();
}

DBOVariableSet ListBoxView::getSet(const std::string& dbo_name)
{
    assert(data_source_);

    return data_source_->getSet()->getExistingInDBFor(dbo_name);
}

// void ListBoxView::selectionChanged()
//{
//    //  assert (data_source_);
//    //  data_source_->updateSelection();
//}
// void ListBoxView::selectionToBeCleared()
//{

//}

bool ListBoxView::usePresentation() const { return use_presentation_; }

void ListBoxView::usePresentation(bool use_presentation)
{
    use_presentation_ = use_presentation;

    emit usePresentationSignal(use_presentation_);
}

bool ListBoxView::overwriteCSV() const { return overwrite_csv_; }

void ListBoxView::overwriteCSV(bool overwrite_csv) { overwrite_csv_ = overwrite_csv; }

bool ListBoxView::showOnlySelected() const { return show_only_selected_; }

void ListBoxView::showOnlySelected(bool value)
{
    loginf << "ListBoxView: showOnlySelected: " << value;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    show_only_selected_ = value;

    emit showOnlySelectedSignal(value);

    QApplication::restoreOverrideCursor();
}

bool ListBoxView::showAssociations() const { return show_associations_; }

void ListBoxView::showAssociations(bool show_associations)
{
    loginf << "ListBoxView: showAssociations: " << show_associations;
    show_associations_ = show_associations;

    emit showAssociationsSignal(show_associations_);
}

void ListBoxView::accept(LatexVisitor& v)
{
    v.visit(this);
}

const std::map<std::string, std::unique_ptr<DBOVariableOrderedSet>>& ListBoxView::getSets()
{
    assert (data_source_);
    return data_source_->getSets();
}

bool ListBoxView::canShowAssociations() const { return can_show_associations_; }

void ListBoxView::updateSelection()
{
    loginf << "ListBoxView: updateSelection";
    assert(widget_);

    if (show_only_selected_)
        widget_->getDataWidget()->updateToSelection();
    else
        widget_->getDataWidget()->resetModels();  // just updates the checkboxes
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

void ListBoxView::allLoadingDoneSlot()
{
    loginf << "ListBoxView: allLoadingDoneSlot";
    assert(widget_);
    widget_->configWidget()->setStatus("", false);
    //widget_->getDataWidget()->selectFirstSelectedRow();
}

