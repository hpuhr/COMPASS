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

#include "scatterplotview.h"

#include "compass.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/variable/metavariable.h"
//#include "scatterplotviewconfigwidget.h"
#include "scatterplotviewdatasource.h"
#include "scatterplotviewdatawidget.h"
#include "scatterplotviewwidget.h"
#include "logger.h"
#include "latexvisitor.h"

#include <QApplication>

using namespace std;
using namespace dbContent;

ScatterPlotView::ScatterPlotView(const std::string& class_id, const std::string& instance_id,
                             ViewContainer* w, ViewManager& view_manager)
    : View(class_id, instance_id, w, view_manager)
{
    registerParameter("data_var_x_dbo", &data_var_x_dbo_, META_OBJECT_NAME);
    registerParameter("data_var_x_name", &data_var_x_name_, DBContent::meta_var_longitude_.name());

    registerParameter("data_var_y_dbo", &data_var_y_dbo_, META_OBJECT_NAME);
    registerParameter("data_var_y_name", &data_var_y_name_, DBContent::meta_var_latitude_.name());

    // create sub done in init
}

ScatterPlotView::~ScatterPlotView()
{
    loginf << "ScatterPlotView: dtor";

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

    loginf << "ScatterPlotView: dtor: done";
}

bool ScatterPlotView::init_impl()
{
    createSubConfigurables();

    assert(data_source_);

//    connect(data_source_, &ScatterPlotViewDataSource::loadingStartedSignal, widget_->getDataWidget(),
//            &ScatterPlotViewDataWidget::loadingStartedSlot);
//    connect(data_source_, &ScatterPlotViewDataSource::updateDataSignal, widget_->getDataWidget(),
//            &ScatterPlotViewDataWidget::updateDataSlot);

//    connect(widget_->configWidget(), &ScatterPlotViewConfigWidget::exportSignal,
//            widget_->getDataWidget(), &ScatterPlotViewDataWidget::exportDataSlot);
//    connect(widget_->getDataWidget(), &ScatterPlotViewDataWidget::exportDoneSignal,
//            widget_->configWidget(), &ScatterPlotViewConfigWidget::exportDoneSlot);

//    connect(data_source_, &ScatterPlotViewDataSource::loadingStartedSignal, widget_->configWidget(),
//            &ScatterPlotViewConfigWidget::loadingStartedSlot);

    //    connect(this, &ScatterPlotView::showOnlySelectedSignal, widget_->getDataWidget(),
    //            &ScatterPlotViewDataWidget::showOnlySelectedSlot);
    //    connect(this, &ScatterPlotView::usePresentationSignal, widget_->getDataWidget(),
    //            &ScatterPlotViewDataWidget::usePresentationSlot);
    //    connect(this, &ScatterPlotView::showAssociationsSignal, widget_->getDataWidget(),
    //            &ScatterPlotViewDataWidget::showAssociationsSlot);

    //    widget_->getDataWidget()->showOnlySelectedSlot(show_only_selected_);
    //    widget_->getDataWidget()->usePresentationSlot(use_presentation_);
    //    widget_->getDataWidget()->showAssociationsSlot(show_associations_);

    return true;
}

void ScatterPlotView::generateSubConfigurable(const std::string& class_id,
                                            const std::string& instance_id)
{
    logdbg << "ScatterPlotView: generateSubConfigurable: class_id " << class_id << " instance_id "
           << instance_id;
    if (class_id == "ScatterPlotViewDataSource")
    {
        assert(!data_source_);
        data_source_ = new ScatterPlotViewDataSource(class_id, instance_id, this);
    }
    else if (class_id == "ScatterPlotViewWidget")
    {
        widget_ = new ScatterPlotViewWidget(class_id, instance_id, this, this, central_widget_);
        setWidget(widget_);
    }
    else
        throw std::runtime_error("ScatterPlotView: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

void ScatterPlotView::checkSubConfigurables()
{
    if (!data_source_)
    {
        generateSubConfigurable("ScatterPlotViewDataSource", "ScatterPlotViewDataSource0");
    }

    if (!widget_)
    {
        generateSubConfigurable("ScatterPlotViewWidget", "ScatterPlotViewWidget0");
    }
}

ScatterPlotViewDataWidget* ScatterPlotView::getDataWidget()
{
    assert (widget_);
    return widget_->getViewDataWidget();
}

VariableSet ScatterPlotView::getSet(const std::string& dbcontent_name)
{
    loginf << "ScatterPlotView: getSet";

    assert(data_source_);

    VariableSet set = data_source_->getSet()->getExistingInDBFor(dbcontent_name);

    if (hasDataVarX())
    {
        if (isDataVarXMeta())
        {
            MetaVariable& meta_var = metaDataVarX();

            if (meta_var.existsIn(dbcontent_name) && !set.hasVariable(meta_var.getFor(dbcontent_name)))
            {
                loginf << "ScatterPlotView: getSet: adding x var " << meta_var.getFor(dbcontent_name).name();
                set.add(meta_var.getFor(dbcontent_name));
            }
        }
        else
        {
            if (dataVarX().dbContentName() == dbcontent_name && !set.hasVariable(dataVarX()))
            {
                loginf << "ScatterPlotView: getSet: adding x var " << dataVarX().name();
                set.add(dataVarX());
            }
        }
    }

    if (hasDataVarY())
    {
        if (isDataVarYMeta())
        {
            MetaVariable& meta_var = metaDataVarY();

            if (meta_var.existsIn(dbcontent_name) && !set.hasVariable(meta_var.getFor(dbcontent_name)))
            {
                loginf << "ScatterPlotView: getSet: adding y var " << meta_var.getFor(dbcontent_name).name();
                set.add(meta_var.getFor(dbcontent_name));
            }
        }
        else
        {
            if (dataVarY().dbContentName() == dbcontent_name && !set.hasVariable(dataVarY()))
            {
                loginf << "ScatterPlotView: getSet: adding y var " << dataVarY().name();
                set.add(dataVarY());
            }
        }
    }

    return set;
}

void ScatterPlotView::accept(LatexVisitor& v)
{
    v.visit(this);
}

bool ScatterPlotView::hasDataVarX ()
{
    if (!data_var_x_dbo_.size() || !data_var_x_name_.size())
        return false;

    if (data_var_x_dbo_ == META_OBJECT_NAME)
        return COMPASS::instance().dbContentManager().existsMetaVariable(data_var_x_name_);
    else
        return COMPASS::instance().dbContentManager().dbContent(data_var_x_dbo_).hasVariable(data_var_x_name_);
}

bool ScatterPlotView::isDataVarXMeta ()
{
    return data_var_x_dbo_ == META_OBJECT_NAME;
}

Variable& ScatterPlotView::dataVarX()
{
    assert (hasDataVarX());
    assert (!isDataVarXMeta());
    assert (COMPASS::instance().dbContentManager().dbContent(data_var_x_dbo_).hasVariable(data_var_x_name_));

    return COMPASS::instance().dbContentManager().dbContent(data_var_x_dbo_).variable(data_var_x_name_);
}

void ScatterPlotView::dataVarX (Variable& var)
{
    data_var_x_dbo_ = var.dbContentName();
    data_var_x_name_ = var.name();
    assert (hasDataVarX());
    assert (!isDataVarXMeta());

    assert (widget_);
    widget_->getViewDataWidget()->redrawData(true);
    widget_->updateComponents();
}

MetaVariable& ScatterPlotView::metaDataVarX()
{
    assert (hasDataVarX());
    assert (isDataVarXMeta());

    return COMPASS::instance().dbContentManager().metaVariable(data_var_x_name_);
}

void ScatterPlotView::metaDataVarX (MetaVariable& var)
{
    data_var_x_dbo_ = META_OBJECT_NAME;
    data_var_x_name_ = var.name();
    assert (hasDataVarX());
    assert (isDataVarXMeta());

    assert (widget_);
    widget_->getViewDataWidget()->redrawData(true);
    widget_->updateComponents();
}


std::string ScatterPlotView::dataVarXDBO() const
{
    return data_var_x_dbo_;
}

std::string ScatterPlotView::dataVarXName() const
{
    return data_var_x_name_;
}


bool ScatterPlotView::hasDataVarY ()
{
    if (!data_var_y_dbo_.size() || !data_var_y_name_.size())
        return false;

    if (data_var_y_dbo_ == META_OBJECT_NAME)
        return COMPASS::instance().dbContentManager().existsMetaVariable(data_var_y_name_);
    else
        return COMPASS::instance().dbContentManager().dbContent(data_var_y_dbo_).hasVariable(data_var_y_name_);
}

bool ScatterPlotView::isDataVarYMeta ()
{
    return data_var_y_dbo_ == META_OBJECT_NAME;
}

Variable& ScatterPlotView::dataVarY()
{
    assert (hasDataVarY());
    assert (!isDataVarYMeta());
    assert (COMPASS::instance().dbContentManager().dbContent(data_var_y_dbo_).hasVariable(data_var_y_name_));

    return COMPASS::instance().dbContentManager().dbContent(data_var_y_dbo_).variable(data_var_y_name_);
}

void ScatterPlotView::dataVarY (Variable& var)
{
    data_var_y_dbo_ = var.dbContentName();
    data_var_y_name_ = var.name();
    assert (hasDataVarY());
    assert (!isDataVarYMeta());

    assert (widget_);
    widget_->getViewDataWidget()->redrawData(true);
    widget_->updateComponents();
}

MetaVariable& ScatterPlotView::metaDataVarY()
{
    assert (hasDataVarY());
    assert (isDataVarYMeta());

    return COMPASS::instance().dbContentManager().metaVariable(data_var_y_name_);
}

void ScatterPlotView::metaDataVarY (MetaVariable& var)
{
    data_var_y_dbo_ = META_OBJECT_NAME;
    data_var_y_name_ = var.name();
    assert (hasDataVarY());
    assert (isDataVarYMeta());

    assert (widget_);
    widget_->getViewDataWidget()->redrawData(true);
    widget_->updateComponents();
}


std::string ScatterPlotView::dataVarYDBO() const
{
    return data_var_y_dbo_;
}

std::string ScatterPlotView::dataVarYName() const
{
    return data_var_y_name_;
}


void ScatterPlotView::updateSelection()
{
    loginf << "ScatterPlotView: updateSelection";
    assert(widget_);

    widget_->getViewDataWidget()->redrawData(true);

    //    if (show_only_selected_)
    //        widget_->getDataWidget()->updateToSelection();
    //    else
    //        widget_->getDataWidget()->resetModels();  // just updates the checkboxes
}

void ScatterPlotView::unshowViewPointSlot (const ViewableDataConfig* vp)
{
    loginf << "ScatterPlotView: unshowViewPoint";

    assert (vp);
    assert (data_source_);
    data_source_->unshowViewPoint(vp);
}

void ScatterPlotView::showViewPointSlot (const ViewableDataConfig* vp)
{
    loginf << "ScatterPlotView: showViewPoint";

    assert (vp);
    assert (data_source_);
    data_source_->showViewPoint(vp);
    assert (widget_);
}
