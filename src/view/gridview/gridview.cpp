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

#include "gridview.h"
#include "gridviewdatawidget.h"
#include "gridviewwidget.h"

#include "dbcontent.h"

#include "compass.h"
#include "logger.h"
#include "latexvisitor.h"

#include <QApplication>

GridView::Settings::Settings() 
:   value_type            (grid2d::ValueType::ValueTypeCount)
,   grid_resolution       (50       )
,   render_pixels_per_cell(10       )
,   render_color_min      ("#00FF00")
,   render_color_max      ("#FF0000")
,   render_color_num_steps(10       )
{
}

/**
*/
GridView::GridView(const std::string& class_id, 
                   const std::string& instance_id,
                   ViewContainer* w, 
                   ViewManager& view_manager)
:   VariableView(class_id, instance_id, w, view_manager)
{
    const std::vector<PropertyDataType> valid_types_xy = { PropertyDataType::BOOL,
                                                           PropertyDataType::CHAR,
                                                           PropertyDataType::UCHAR,
                                                           PropertyDataType::INT,
                                                           PropertyDataType::UINT,
                                                           PropertyDataType::LONGINT,
                                                           PropertyDataType::ULONGINT,
                                                           PropertyDataType::FLOAT,
                                                           PropertyDataType::DOUBLE,
                                                           PropertyDataType::TIMESTAMP };
    const std::vector<PropertyDataType> valid_types_z =  { PropertyDataType::BOOL,
                                                           PropertyDataType::CHAR,
                                                           PropertyDataType::UCHAR,
                                                           PropertyDataType::INT,
                                                           PropertyDataType::UINT,
                                                           PropertyDataType::LONGINT,
                                                           PropertyDataType::ULONGINT,
                                                           PropertyDataType::FLOAT,
                                                           PropertyDataType::DOUBLE };

    addVariable("data_var_x", "X"          , "x", META_OBJECT_NAME, DBContent::meta_var_longitude_.name(), true, false, valid_types_xy);
    addVariable("data_var_y", "Y"          , "y", META_OBJECT_NAME, DBContent::meta_var_latitude_.name() , true, false, valid_types_xy);
    addVariable("data_var_z", "Distributed", "z", META_OBJECT_NAME, DBContent::meta_var_mc_.name()       , true, false, valid_types_z);

    // create sub done in init
}

/**
*/
GridView::~GridView()
{
    loginf << "GridView: dtor";

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

    loginf << "GridView: dtor: done";
}

/**
*/
bool GridView::init_impl()
{
    createSubConfigurables();

    return true;
}

/**
*/
void GridView::generateSubConfigurable(const std::string& class_id,
                                       const std::string& instance_id)
{
    logdbg << "GridView: generateSubConfigurable: class_id " << class_id << " instance_id "
           << instance_id;
    
    if (class_id == "GridViewWidget")
    {
        widget_ = new GridViewWidget(class_id, instance_id, this, this, central_widget_);
        setWidget(widget_);
    }
    else
    {
        throw std::runtime_error("GridView: generateSubConfigurable: unknown class_id " + class_id);
    }
}

/**
*/
void GridView::checkSubConfigurables()
{
    if (!widget_)
        generateSubConfigurable("GridViewWidget", "GridViewWidget0");
}

/**
*/
GridViewDataWidget* GridView::getDataWidget()
{
    assert (widget_);
    return widget_->getViewDataWidget();
}

/**
*/
dbContent::VariableSet GridView::getBaseSet(const std::string& dbcontent_name)
{
    return dbContent::VariableSet();
}

/**
*/
void GridView::accept(LatexVisitor& v)
{
    v.visit(this);
}

/**
*/
void GridView::updateSelection()
{
    loginf << "GridView: updateSelection";
    assert(widget_);

    widget_->getViewDataWidget()->redrawData(true);
}

/**
 */
void GridView::viewInfoJSON_impl(nlohmann::json& info) const
{
    //!call base!
    VariableView::viewInfoJSON_impl(info);

    //@TODO?
}

/**
 */
void GridView::setValueType(grid2d::ValueType type, bool notify_changes)
{
    if (settings_.value_type == type)
        return;

    setParameter(settings_.value_type, (int)type);

    if (notify_changes)
        notifyRefreshNeeded();
}

/**
*/
void GridView::setGridResolution(unsigned int n, bool notify_changes)
{
    if (settings_.grid_resolution == n)
        return;

    setParameter(settings_.grid_resolution, n);

    if (notify_changes)
        notifyRefreshNeeded();
}

/**
*/
void GridView::setPixelsPerCell(unsigned int n, bool notify_changes)
{
    if (settings_.render_pixels_per_cell == n)
        return;

    setParameter(settings_.render_pixels_per_cell, n);

    if (notify_changes)
        notifyRefreshNeeded();
}

/**
*/
void GridView::setColorSteps(unsigned int n, bool notify_changes)
{
    if (settings_.render_color_num_steps == n)
        return;

    setParameter(settings_.render_color_num_steps, n);

    if (notify_changes)
        notifyRefreshNeeded();
}

/**
*/
void GridView::setColorMin(const QColor& color, bool notify_changes)
{
    if (settings_.render_color_min == color.name().toStdString())
        return;

    setParameter(settings_.render_color_min, color.name().toStdString());

    if (notify_changes)
        notifyRefreshNeeded();
}

/**
*/
void GridView::setColorMax(const QColor& color, bool notify_changes)
{
    if (settings_.render_color_max == color.name().toStdString())
        return;

    setParameter(settings_.render_color_max, color.name().toStdString());

    if (notify_changes)
        notifyRefreshNeeded();
}
