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

const std::string ScatterPlotView::ParamDataVarXDBO        = "data_var_x_dbo";
const std::string ScatterPlotView::ParamDataVarXName       = "data_var_x_name";
const std::string ScatterPlotView::ParamDataVarYDBO        = "data_var_y_dbo";
const std::string ScatterPlotView::ParamDataVarYName       = "data_var_y_name";
const std::string ScatterPlotView::ParamUseConnectionLines = "use_connection_lines";

/**
*/
ScatterPlotView::Settings::Settings()
:   data_var_x_dbo (META_OBJECT_NAME)
,   data_var_x_name(DBContent::meta_var_longitude_.name())
,   data_var_y_dbo (META_OBJECT_NAME)
,   data_var_y_name(DBContent::meta_var_latitude_.name())
{
}

/**
*/
ScatterPlotView::ScatterPlotView(const std::string& class_id, 
                                 const std::string& instance_id,
                                 ViewContainer* w, 
                                 ViewManager& view_manager)
:   View(class_id, instance_id, w, view_manager)
{
    registerParameter(ParamDataVarXDBO, &settings_.data_var_x_dbo, Settings().data_var_x_dbo);
    registerParameter(ParamDataVarXName, &settings_.data_var_x_name, Settings().data_var_x_name);

    registerParameter(ParamDataVarYDBO, &settings_.data_var_y_dbo, Settings().data_var_y_dbo);
    registerParameter(ParamDataVarYName, &settings_.data_var_y_name, Settings().data_var_y_name);

    // create sub done in init
}

/**
*/
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

/**
*/
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

/**
*/
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

/**
*/
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

/**
*/
ScatterPlotViewDataWidget* ScatterPlotView::getDataWidget()
{
    assert (widget_);
    return widget_->getViewDataWidget();
}

/**
*/
VariableSet ScatterPlotView::getSet(const std::string& dbcontent_name)
{
    logdbg << "ScatterPlotView: getSet";

    assert(data_source_);

    VariableSet set = data_source_->getSet()->getFor(dbcontent_name);

    if (hasDataVarX())
    {
        if (isDataVarXMeta())
        {
            MetaVariable& meta_var = metaDataVarX();

            if (meta_var.existsIn(dbcontent_name) && !set.hasVariable(meta_var.getFor(dbcontent_name)))
            {
                logdbg << "ScatterPlotView: getSet: adding x var " << meta_var.getFor(dbcontent_name).name();
                set.add(meta_var.getFor(dbcontent_name));
            }
        }
        else
        {
            if (dataVarX().dbContentName() == dbcontent_name && !set.hasVariable(dataVarX()))
            {
                logdbg << "ScatterPlotView: getSet: adding x var " << dataVarX().name();
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

/**
*/
void ScatterPlotView::accept(LatexVisitor& v)
{
    v.visit(this);
}

/**
*/
bool ScatterPlotView::hasDataVarX ()
{
    if (settings_.data_var_x_dbo.empty() || settings_.data_var_x_name.empty())
        return false;

    if (settings_.data_var_x_dbo == META_OBJECT_NAME)
        return COMPASS::instance().dbContentManager().existsMetaVariable(settings_.data_var_x_name);
    else
        return COMPASS::instance().dbContentManager().dbContent(settings_.data_var_x_dbo).hasVariable(settings_.data_var_x_name);
}

/**
*/
bool ScatterPlotView::isDataVarXMeta ()
{
    return (settings_.data_var_x_dbo == META_OBJECT_NAME);
}

/**
*/
Variable& ScatterPlotView::dataVarX()
{
    assert (hasDataVarX());
    assert (!isDataVarXMeta());
    assert (COMPASS::instance().dbContentManager().dbContent(settings_.data_var_x_dbo).hasVariable(settings_.data_var_x_name));

    return COMPASS::instance().dbContentManager().dbContent(settings_.data_var_x_dbo).variable(settings_.data_var_x_name);
}

/**
*/
void ScatterPlotView::dataVarX (Variable& var, bool notify_changes)
{
    if (settings_.data_var_x_dbo == var.dbContentName() && 
        settings_.data_var_x_name == var.name())
        return;

    setParameter(settings_.data_var_x_dbo, var.dbContentName());
    setParameter(settings_.data_var_x_name, var.name());

    assert (hasDataVarX());
    assert (!isDataVarXMeta());

    if (notify_changes)
    {
        notifyRefreshNeeded();
    }
}

/**
*/
MetaVariable& ScatterPlotView::metaDataVarX()
{
    assert (hasDataVarX());
    assert (isDataVarXMeta());

    return COMPASS::instance().dbContentManager().metaVariable(settings_.data_var_x_name);
}

/**
*/
void ScatterPlotView::metaDataVarX (MetaVariable& var, bool notify_changes)
{
    if (settings_.data_var_x_dbo == META_OBJECT_NAME && 
        settings_.data_var_x_name == var.name())
        return;

    setParameter(settings_.data_var_x_dbo, META_OBJECT_NAME);
    setParameter(settings_.data_var_x_name, var.name());

    assert (hasDataVarX());
    assert (isDataVarXMeta());

    if (notify_changes)
    {
        notifyRefreshNeeded();
    }
}

/**
*/
std::string ScatterPlotView::dataVarXDBO() const
{
    return settings_.data_var_x_dbo;
}

/**
*/
std::string ScatterPlotView::dataVarXName() const
{
    return settings_.data_var_x_name;
}

/**
*/
bool ScatterPlotView::hasDataVarY ()
{
    if (settings_.data_var_y_dbo.empty() || settings_.data_var_y_name.empty())
        return false;

    if (settings_.data_var_y_dbo == META_OBJECT_NAME)
        return COMPASS::instance().dbContentManager().existsMetaVariable(settings_.data_var_y_name);
    else
        return COMPASS::instance().dbContentManager().dbContent(settings_.data_var_y_dbo).hasVariable(settings_.data_var_y_name);
}

/**
*/
bool ScatterPlotView::isDataVarYMeta ()
{
    return (settings_.data_var_y_dbo == META_OBJECT_NAME);
}

/**
*/
Variable& ScatterPlotView::dataVarY()
{
    assert (hasDataVarY());
    assert (!isDataVarYMeta());
    assert (COMPASS::instance().dbContentManager().dbContent(settings_.data_var_y_dbo).hasVariable(settings_.data_var_y_name));

    return COMPASS::instance().dbContentManager().dbContent(settings_.data_var_y_dbo).variable(settings_.data_var_y_name);
}

/**
*/
void ScatterPlotView::dataVarY (Variable& var, bool notify_changes)
{
    if (settings_.data_var_y_dbo == var.dbContentName() && 
        settings_.data_var_y_name == var.name())
        return;

    setParameter(settings_.data_var_y_dbo, var.dbContentName());
    setParameter(settings_.data_var_y_name, var.name());

    assert (hasDataVarY());
    assert (!isDataVarYMeta());

    if (notify_changes)
    {
        notifyRefreshNeeded();
    }
}

/**
*/
MetaVariable& ScatterPlotView::metaDataVarY()
{
    assert (hasDataVarY());
    assert (isDataVarYMeta());

    return COMPASS::instance().dbContentManager().metaVariable(settings_.data_var_y_name);
}

/**
*/
void ScatterPlotView::metaDataVarY (MetaVariable& var, bool notify_changes)
{
    if (settings_.data_var_y_dbo == META_OBJECT_NAME && 
        settings_.data_var_y_name == var.name())
        return;

    setParameter(settings_.data_var_y_dbo, META_OBJECT_NAME);
    setParameter(settings_.data_var_y_name, var.name());

    assert (hasDataVarY());
    assert (isDataVarYMeta());

    if (notify_changes)
    {
        notifyRefreshNeeded();
    }
}

/**
*/
std::string ScatterPlotView::dataVarYDBO() const
{
    return settings_.data_var_y_dbo;
}

/**
*/
std::string ScatterPlotView::dataVarYName() const
{
    return settings_.data_var_y_name;
}

/**
*/
bool ScatterPlotView::useConnectionLines()
{
    return settings_.use_connection_lines;
}

/**
*/
void ScatterPlotView::useConnectionLines(bool value)
{
    settings_.use_connection_lines = value;

    updateView(VU_Redraw);
}

/**
*/
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

/**
*/
void ScatterPlotView::unshowViewPointSlot (const ViewableDataConfig* vp)
{
    loginf << "ScatterPlotView: unshowViewPoint";

    assert (vp);
    assert (data_source_);
    data_source_->unshowViewPoint(vp);
}

/**
*/
void ScatterPlotView::showViewPointSlot (const ViewableDataConfig* vp)
{
    loginf << "ScatterPlotView: showViewPoint";

    assert (vp);
    assert (data_source_);
    data_source_->showViewPoint(vp);
    assert (widget_);
}

/**
 */
void ScatterPlotView::viewInfoJSON_impl(nlohmann::json& info) const
{
    info[ ParamDataVarXDBO  ] = settings_.data_var_x_dbo;
    info[ ParamDataVarXName ] = settings_.data_var_x_name;

    info[ ParamDataVarYDBO  ] = settings_.data_var_y_dbo;
    info[ ParamDataVarYName ] = settings_.data_var_y_name;

    info[ ParamUseConnectionLines ] = settings_.use_connection_lines;
}
