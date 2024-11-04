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

#include "gridviewdatawidget.h"
#include "gridviewwidget.h"
#include "gridview.h"
#include "gridviewchart.h"
#include "grid2d.h"

#include "viewvariable.h"
#include "viewpointgenerator.h"
#include "colormapwidget.h"
#include "property_templates.h"

#include "buffer.h"

#include "dbcontent/dbcontent.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/metavariable.h"

#include "logger.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>

#include <QChart>
#include <QGraphicsLayout>

#include <QtCharts/QScatterSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QLegend>
#include <QtCharts/QValueAxis>

#include <algorithm>

/**
*/
GridViewDataWidget::GridViewDataWidget(GridViewWidget* view_widget,
                                       QWidget* parent,
                                       Qt::WindowFlags f)
:   VariableViewStashDataWidget(view_widget, view_widget->getView(), false, parent, f)
{
    view_ = view_widget->getView();
    assert(view_);

    main_layout_ = new QHBoxLayout();
    main_layout_->setMargin(0);

    setLayout(main_layout_);

    legend_ = new ColorLegendWidget(this);
    legend_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    legend_->showSelectionColor(true);
    legend_->showNullColor(false);
    legend_->setDescriptionMode(ColorMapDescriptionMode::Midpoints);

    main_layout_->addWidget(legend_);

    x_axis_name_ = view_->variable(0).description();
    y_axis_name_ = view_->variable(1).description();

    updateGridChart();
}

/**
*/
GridViewDataWidget::~GridViewDataWidget()
{
    logdbg << "GridViewDataWidget: dtor";
}

/**
*/
void GridViewDataWidget::resetGrid()
{
    grid_.reset();
}

/**
*/
void GridViewDataWidget::resetGridChart()
{
    grid_chart_.reset();

    grid_rendering_ = QImage();
    grid_roi_       = QRectF();
}

/**
*/
void GridViewDataWidget::resetGridLayers()
{
    grid_layers_.clear();

    grid_value_min_ = 0.0;
    grid_value_max_ = 1.0;

    x_axis_name_ = "";
    y_axis_name_ = "";
    title_       = "";
}

/**
*/
bool GridViewDataWidget::postLoadTrigger()
{
    //no redraw triggered
    return false;
}

/**
*/
void GridViewDataWidget::resetVariableDisplay() 
{
    resetGridChart();
}

/**
*/
void GridViewDataWidget::resetStashDependentData()
{
    resetGrid();
    resetGridLayers();
}

/**
*/
bool GridViewDataWidget::updateVariableDisplay() 
{
    return updateGridChart();
}

/**
*/
void GridViewDataWidget::processStash(const VariableViewStash<double>& stash)
{
    loginf << "GridViewDataWidget: processStash";

    const auto& data_ranges = getStash().dataRanges();

    x_axis_name_ = view_->variable(0).description();
    y_axis_name_ = view_->variable(1).description();
    title_       = "";

    bool has_data = (hasData() && 
                     getStash().hasData() && 
                     variablesOk() && 
                     data_ranges[ 0 ].has_value() && 
                     data_ranges[ 1 ].has_value() && 
                     data_ranges[ 2 ].has_value());
    if (!has_data)
        return;

    auto bounds = getXYBounds();
    if (bounds.isEmpty())
    {
        loginf << "GridViewDataWidget: processStash: bounds empty, skipping...";
        return;
    }
    assert(bounds.isValid());

    auto z_bounds = getZBounds();
    assert(z_bounds.has_value());

    const auto& settings = view_->settings();

    grid2d::GridResolution res = grid2d::GridResolution().setCellCount(settings.grid_resolution, settings.grid_resolution);

    grid_.reset(new Grid2D);
    bool ok = grid_->create(bounds, res);
    assert(ok);

    loginf << "GridViewDataWidget: renderGrid: Created grid of " << grid_->numCellsX() << "x" << grid_->numCellsY();

    for (const auto& dbc_values : getStash().groupedStashes())
    {
        const auto& x_values = dbc_values.second.variable_stashes[ 0 ].values;
        const auto& y_values = dbc_values.second.variable_stashes[ 1 ].values;
        const auto& z_values = dbc_values.second.variable_stashes[ 2 ].values;

        loginf << "GridViewDataWidget: renderGrid: dbcontent " << dbc_values.first
               << " #x " << x_values.size()
               << " #y " << y_values.size()
               << " #z " << z_values.size();

        for (size_t i = 0; i < z_values.size(); ++i)
            if (!dbc_values.second.nan_values[ i ])
                grid_->addValue(x_values[ i ], y_values[ i ], z_values[ i ]);
    }

    loginf << "GridViewDataWidget: renderGrid: getting layer";

    auto layer_name = grid2d::valueTypeToString((grid2d::ValueType)settings.value_type);

    grid_->addToLayers(grid_layers_, layer_name, (grid2d::ValueType)settings.value_type);

    auto range = grid_layers_.layer(0).range();

    grid_value_min_ = range.has_value() ? range->first  : 0.0;
    grid_value_max_ = range.has_value() ? range->second : 1.0;
    assert(grid_value_min_ <= grid_value_max_);

    loginf << "GridViewDataWidget: processStash: done, generated " << grid_layers_.numLayers() << " layers";
}

/**
*/
void GridViewDataWidget::updateFromAnnotations()
{
    loginf << "GridViewDataWidget: updateFromAnnotations";

    if (!view_->hasCurrentAnnotation())
        return;

    const auto& anno = view_->currentAnnotation();

    title_       = anno.metadata.title_;
    x_axis_name_ = anno.metadata.xAxisLabel();
    y_axis_name_ = anno.metadata.yAxisLabel();

    const auto& feature = anno.feature_json;

    if (!feature.is_object() || !feature.contains(ViewPointGenFeatureGrid::FeatureGridFieldNameGrid))
        return;

    std::unique_ptr<Grid2DLayer> layer(new Grid2DLayer);
    if (!layer->fromJSON(feature[ ViewPointGenFeatureGrid::FeatureGridFieldNameGrid ]))
    {
        logerr << "GridViewDataWidget: updateFromAnnotations: could not read grid layer";
        return;
    }

    if (layer->data.cols() < 1 || 
        layer->data.rows() < 1)
    {
        logerr << "GridViewDataWidget: updateFromAnnotations: grid layer empty";
        return;
    }
    
    grid_layers_.addLayer(std::move(layer));

    auto range = grid_layers_.layer(0).range();

    grid_value_min_ = range.has_value() ? range->first  : 0.0;
    grid_value_max_ = range.has_value() ? range->second : 1.0;
    assert(grid_value_min_ <= grid_value_max_);

    loginf << "GridViewDataWidget: updateFromAnnotations: done, generated " << grid_layers_.numLayers() << " layers";
}

/**
*/
bool GridViewDataWidget::hasValidGrid() const
{
    return grid_layers_.numLayers() > 0;
}

/**
*/
void GridViewDataWidget::mouseMoveEvent(QMouseEvent* event)
{
    //setCursor(current_cursor_);
    //osgEarth::QtGui::ViewerWidget::mouseMoveEvent(event);

    QWidget::mouseMoveEvent(event);
}

/**
*/
void GridViewDataWidget::toolChanged_impl(int mode)
{
    selected_tool_ = (GridViewDataTool)mode;

    if (grid_chart_)
        grid_chart_->onToolChanged();
}

/**
*/
GridViewDataTool GridViewDataWidget::selectedTool() const
{
    return selected_tool_;
}

/**
*/
QPixmap GridViewDataWidget::renderPixmap()
{
    assert (grid_chart_);
    return grid_chart_->grab();
}

/**
*/
QRectF GridViewDataWidget::getXYBounds() const
{
    return getPlanarBounds(0, 1);
}

/**
*/
boost::optional<std::pair<double, double>> GridViewDataWidget::getZBounds() const
{
    return getBounds(2);
}

/**
*/
void GridViewDataWidget::rectangleSelectedSlot (QPointF p1, QPointF p2)
{
    loginf << "GridViewDataWidget: rectangleSelectedSlot";

    if (grid_chart_ && grid_chart_->chart())
    {
        if (selected_tool_ == GV_ZOOM_RECT_TOOL)
        {
            loginf << "GridViewDataWidget: rectangleSelectedSlot: zoom";

            //TODO: prevent from going nuts when zero rect is passed!
            grid_chart_->zoom(p1, p2);
        }
        else if (selected_tool_ == GV_SELECT_TOOL)
        {
            loginf << "GridViewDataWidget: rectangleSelectedSlot: select";

            selectData(std::min(p1.x(), p2.x()), std::max(p1.x(), p2.x()), std::min(p1.y(), p2.y()), std::max(p1.y(), p2.y()), 0, 1);
        }
        else
        {
            throw std::runtime_error("GridViewDataWidget: rectangleSelectedSlot: unknown tool " + std::to_string((unsigned int)selected_tool_));
        }
    }

    endTool();
}

/**
*/
void GridViewDataWidget::invertSelectionSlot()
{
    loginf << "GridViewDataWidget: invertSelectionSlot";

    for (auto& buf_it : viewData())
    {
        assert (buf_it.second->has<bool>(DBContent::selected_var.name()));
        NullableVector<bool>& selected_vec = buf_it.second->get<bool>(DBContent::selected_var.name());

        for (unsigned int cnt=0; cnt < buf_it.second->size(); ++cnt)
        {
            if (selected_vec.isNull(cnt))
                selected_vec.set(cnt, true);
            else
                selected_vec.set(cnt, !selected_vec.get(cnt));
        }
    }

    emit view_->selectionChangedSignal();
}

/**
*/
void GridViewDataWidget::clearSelectionSlot()
{
    loginf << "GridViewDataWidget: clearSelectionSlot";

    for (auto& buf_it : viewData())
    {
        assert (buf_it.second->has<bool>(DBContent::selected_var.name()));
        NullableVector<bool>& selected_vec = buf_it.second->get<bool>(DBContent::selected_var.name());

        for (unsigned int cnt=0; cnt < buf_it.second->size(); ++cnt)
            selected_vec.set(cnt, false);
    }

    emit view_->selectionChangedSignal();
}

/**
*/
void GridViewDataWidget::resetZoomSlot()
{
    loginf << "GridViewDataWidget: resetZoomSlot";

    if (grid_chart_)
        grid_chart_->resetZoom();
}


/**
*/
bool GridViewDataWidget::updateGridChart()
{
    bool has_data = (grid_layers_.numLayers() > 0 && variablesOk());

    resetGridChart();
    updateRendering();

    if (grid_rendering_.isNull() || grid_roi_.isEmpty())
        has_data = false;

    QtCharts::QChart* chart = new QtCharts::QChart();
    updateChart(chart, has_data);
    
    grid_chart_.reset(new QtCharts::GridViewChart(this, chart));
    grid_chart_->setObjectName("chart_view");
    grid_chart_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect (grid_chart_.get(), &QtCharts::GridViewChart::rectangleSelectedSignal,
             this, &GridViewDataWidget::rectangleSelectedSlot, Qt::ConnectionType::QueuedConnection);

    main_layout_->insertWidget(0, grid_chart_.get());

    return has_data;
}

/**
*/
void GridViewDataWidget::updateRendering()
{
    loginf << "GridViewDataWidget: updateRendering: rendering";

    if (!hasValidGrid())
        return;

    const auto& layer    = grid_layers_.layer(0);
    const auto& settings = view_->settings();

    loginf << "GridViewDataWidget: updateRendering: min = " << grid_value_min_ << " max = " << grid_value_max_;

    std::pair<double, double> range(grid_value_min_, grid_value_max_);

    auto vmin = view_->getMinValue();
    auto vmax = view_->getMaxValue();

    if (vmin.has_value() && vmax.has_value() && vmin.value() <= vmax.value())
        range = std::pair<double, double>(vmin.value(), vmax.value());
    else if (vmin.has_value() && !vmax.has_value() && vmin.value() <= grid_value_max_)
        range.first = vmin.value();
    else if (vmax.has_value() && !vmin.has_value() && vmax.value() >= grid_value_min_)
        range.second = vmax.value();

    auto dtype = view_->currentDataType();

    auto decoratorFunc = [ = ] (double v)
    {
        return property_templates::double2String(dtype, v, GridView::DecimalsDefault);
    };

    bool is_bool = (dtype == PropertyDataType::BOOL);

    Grid2DRenderSettings render_settings;
    render_settings.color_map.create((colorscale::ColorScale)settings.render_color_scale,
                                      settings.render_color_num_steps,
                                      is_bool ? ColorMap::Type::Binary : ColorMap::Type::Linear,
                                      range);

    legend_->setColorMap(render_settings.color_map);
    legend_->setDecorator(decoratorFunc);    

    auto rendering = Grid2DLayerRenderer::render(layer, render_settings);

    grid_rendering_ = rendering.first;
    grid_roi_       = rendering.second.getROI(grid_rendering_.width(), grid_rendering_.height());
    grid_north_up_  = rendering.second.is_north_up;
    ref_            = rendering.second;
}

/**
*/
void GridViewDataWidget::updateChart(QtCharts::QChart* chart, bool has_data)
{
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    chart->setBackgroundRoundness(0);
    chart->setDropShadowEnabled(false);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setTitle(QString::fromStdString(title_));

    auto createAxes = [ & ] ()
    {
        chart->createDefaultAxes();

        //config x axis
        loginf << "GridViewDataWidget: updateDataSeries: title x ' "
               << view_->variable(0).description() << "'";
        assert (chart->axes(Qt::Horizontal).size() == 1);
        chart->axes(Qt::Horizontal).at(0)->setTitleText(x_axis_name_.c_str());

        //config y axis
        loginf << "GridViewDataWidget: updateDataSeries: title y ' "
               << view_->variable(1).description() << "'";
        assert (chart->axes(Qt::Vertical).size() == 1);
        chart->axes(Qt::Vertical).at(0)->setTitleText(y_axis_name_.c_str());
    };

    if (has_data)
    {
        chart->legend()->setVisible(false);

        QtCharts::QScatterSeries* series = new QtCharts::QScatterSeries;
        series->setMarkerSize(0.1);
        series->setColor(Qt::white);

        series->append(grid_roi_.topLeft());
        series->append(grid_roi_.topRight());
        series->append(grid_roi_.bottomRight());
        series->append(grid_roi_.bottomLeft());

        chart->addSeries(series);

        createAxes();
    }
    else
    {
        //no data -> generate default empty layout
        chart->legend()->setVisible(false);

        QtCharts::QScatterSeries* series = new QtCharts::QScatterSeries;
        chart->addSeries(series);

        createAxes();

        chart->axes(Qt::Horizontal).at(0)->setLabelsVisible(false);
        chart->axes(Qt::Horizontal).at(0)->setGridLineVisible(false);
        chart->axes(Qt::Horizontal).at(0)->setMinorGridLineVisible(false);

        chart->axes(Qt::Vertical).at(0)->setLabelsVisible(false);
        chart->axes(Qt::Vertical).at(0)->setGridLineVisible(false);
        chart->axes(Qt::Vertical).at(0)->setMinorGridLineVisible(false);
    }

    chart->update();
}

/**
 */
void GridViewDataWidget::viewInfoJSON_impl(nlohmann::json& info) const
{
    //!call base!
    VariableViewStashDataWidget::viewInfoJSON_impl(info);

    info[ "num_valid"   ] = getStash().valid_count_;
    info[ "num_selected"] = getStash().selected_count_;
    info[ "num_nan"     ] = getStash().nan_value_count_;

    auto xy_bounds     = getXYBounds();
    auto z_bounds      = getZBounds();
    bool bounds_valid  = xy_bounds.isValid() && z_bounds.has_value();

    info[ "data_bounds_valid" ] = bounds_valid;
    info[ "data_bounds_xmin"  ] = bounds_valid ? xy_bounds.left()         : 0.0;
    info[ "data_bounds_ymin"  ] = bounds_valid ? xy_bounds.top()          : 0.0;
    info[ "data_bounds_zmin"  ] = bounds_valid ?  z_bounds.value().first  : 0.0;
    info[ "data_bounds_xmax"  ] = bounds_valid ? xy_bounds.right()        : 0.0;
    info[ "data_bounds_ymax"  ] = bounds_valid ? xy_bounds.bottom()       : 0.0;
    info[ "data_bounds_zmax"  ] = bounds_valid ?  z_bounds.value().second : 0.0;

    // auto zoomActive = [ & ] (const QRectF& bounds_data, const QRectF& bounds_axis)
    // {
    //     if (!bounds_data.isValid() || !bounds_axis.isValid())
    //         return false;

    //     return (bounds_axis.left()   > bounds_data.left()  ||
    //             bounds_axis.right()  < bounds_data.right() ||
    //             bounds_axis.top()    > bounds_data.top()   ||
    //             bounds_axis.bottom() < bounds_data.bottom());
    // };

    //@TODO
}

/**
*/
boost::optional<std::pair<QImage, RasterReference>> GridViewDataWidget::currentGeoImage() const
{
    return std::make_pair(grid_rendering_, ref_);
}
