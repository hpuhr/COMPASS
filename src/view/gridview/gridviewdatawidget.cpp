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
#include "grid2dlayer.h"

#include "viewvariable.h"
#include "buffer.h"

#include "dbcontent/dbcontent.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/metavariable.h"

#include "logger.h"
#include "property_templates.h"

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
:   VariableViewDataWidget(view_widget, view_widget->getView(), parent, f)
{
    view_ = view_widget->getView();
    assert(view_);

    main_layout_ = new QHBoxLayout();
    main_layout_->setMargin(0);

    //grid_chart_ = new SimpleGridViewChart;
    //grid_chart_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    //main_layout_->addWidget(grid_chart_);

    setLayout(main_layout_);

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
bool GridViewDataWidget::postLoadTrigger()
{
    //no redraw triggered
    return false;
}

/**
*/
void GridViewDataWidget::resetVariableData()
{
    resetCounts();
}

/**
*/
void GridViewDataWidget::resetVariableDisplay() 
{
    resetGridChart();
}

/**
*/
void GridViewDataWidget::preUpdateVariableDataEvent() 
{
    resetCounts();
}

/**
*/
void GridViewDataWidget::postUpdateVariableDataEvent() 
{
    updateMinMax();
}

/**
*/
bool GridViewDataWidget::updateVariableDisplay() 
{
    return updateGridChart();
}

/**
*/
void GridViewDataWidget::updateVariableData(const std::string& dbcontent_name,
                                            Buffer& buffer)
{
    loginf << "GridViewDataWidget: updateVariableData: updating data for dbcontent " << dbcontent_name;

    logdbg << "GridViewDataWidget: updateVariableData: before"
           << " x " << x_values_[dbcontent_name].size()
           << " y " << y_values_[dbcontent_name].size()
           << " z " << z_values_[dbcontent_name].size();

    unsigned int current_size = buffer.size();

    // add selected flags & rec_nums
    assert (buffer.has<bool>(DBContent::selected_var.name()));
    assert (buffer.has<unsigned long>(DBContent::meta_var_rec_num_.name()));

    const NullableVector<bool>&          selected_vec = buffer.get<bool>(DBContent::selected_var.name());
    const NullableVector<unsigned long>& rec_num_vec  = buffer.get<unsigned long>(DBContent::meta_var_rec_num_.name());

    std::vector<bool>&          selected_data = selected_values_[dbcontent_name];
    std::vector<unsigned long>& rec_num_data  = rec_num_values_ [dbcontent_name];

    unsigned int last_size = 0;

    if (buffer_x_counts_.count(dbcontent_name))
        last_size = buffer_x_counts_.at(dbcontent_name);

    for (unsigned int cnt=last_size; cnt < current_size; ++cnt)
    {
        if (selected_vec.isNull(cnt))
            selected_data.push_back(false);
        else
            selected_data.push_back(selected_vec.get(cnt));

        assert (!rec_num_vec.isNull(cnt));
        rec_num_data.push_back(rec_num_vec.get(cnt));
    }

    //collect data for both variables
    updateVariableData(0, dbcontent_name, current_size);
    updateVariableData(1, dbcontent_name, current_size);
    updateVariableData(2, dbcontent_name, current_size);

    //check counts
    assert (x_values_[dbcontent_name].size() == y_values_[dbcontent_name].size());
    assert (y_values_[dbcontent_name].size() == z_values_[dbcontent_name].size());
    assert (x_values_[dbcontent_name].size() == selected_values_[dbcontent_name].size());
    assert (x_values_[dbcontent_name].size() == rec_num_values_[dbcontent_name].size());

    logdbg << "GridViewDataWidget: updateVariableData: after"
           << " x " << x_values_[dbcontent_name].size()
           << " y " << y_values_[dbcontent_name].size()
           << " z " << z_values_[dbcontent_name].size();
}

/**
*/
void GridViewDataWidget::updateVariableData(int var_idx, std::string dbcontent_name, unsigned int current_size)
{
    logdbg << "GridViewDataWidget: updateVariableData: dbcontent_name " << dbcontent_name << " current_size " << current_size;
    
    assert(var_idx == 0 || var_idx == 1 || var_idx == 2);
    assert (viewData().count(dbcontent_name));
    Buffer* buffer = viewData().at(dbcontent_name).get();

    auto& buffer_counts = var_idx == 0 ? buffer_x_counts_ : (var_idx == 1 ? buffer_y_counts_ : buffer_z_counts_);
    auto& values        = var_idx == 0 ? x_values_        : (var_idx == 1 ? y_values_        : z_values_       );

    unsigned int last_size = 0;

    if (buffer_counts.count(dbcontent_name))
        last_size = buffer_counts.at(dbcontent_name);

    ViewVariable& view_var = view_->variable(var_idx);
    dbContent::Variable* data_var = view_var.getFor(dbcontent_name);
    if (!data_var)
    {
        logwrn << "GridViewDataWidget: updateVariableData: could not retrieve data var";
        return;
    }

    PropertyDataType data_type = data_var->dataType();
    std::string current_var_name = data_var->name();

    logdbg << "GridViewDataWidget: updateVariableData: updating, last size " << last_size;

    #define UpdateFunc(PDType, DType)                                       \
        assert(view_var.settings().valid_data_types.count(PDType) != 0);    \
        assert(buffer->has<DType>(current_var_name));                       \
                                                                            \
        NullableVector<DType>& data = buffer->get<DType>(current_var_name); \
                                                                            \
        appendData(data, values[dbcontent_name], last_size, current_size);  \
        buffer_counts[dbcontent_name] = current_size;

    #define NotFoundFunc                                                                                                           \
        logerr << "GridViewDataWidget: updateVariableData: impossible for property type " << Property::asString(data_type); \
        throw std::runtime_error("GridViewDataWidget: updateVariableData: impossible property type " + Property::asString(data_type));

    SwitchPropertyDataType(data_type, UpdateFunc, NotFoundFunc)

    logdbg << "GridViewDataWidget: updateVariableData: updated size " << buffer_counts.at(dbcontent_name);
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
void GridViewDataWidget::resetCounts()
{
    buffer_x_counts_.clear();
    buffer_y_counts_.clear();
    buffer_z_counts_.clear();

    x_values_.clear();
    y_values_.clear();
    z_values_.clear();

    has_x_min_max_ = false;
    has_y_min_max_ = false;
    has_z_min_max_ = false;

    selected_values_.clear();
    rec_num_values_.clear();

    nan_value_cnt_ = 0;
    valid_cnt_     = 0;
    selected_cnt_  = 0;

    dbo_valid_counts_.clear();
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
unsigned int GridViewDataWidget::nullValueCount() const
{
    return nan_value_cnt_;
}

/**
*/
QRectF GridViewDataWidget::getXYBounds() const
{
    if (!has_x_min_max_ || !has_y_min_max_)
        return QRectF();

    return QRectF(x_min_, y_min_, x_max_ - x_min_, y_max_ - y_min_);
}

/**
*/
boost::optional<std::pair<double, double>> GridViewDataWidget::getZBounds() const
{
    if (!has_z_min_max_)
        return {};

    return std::make_pair(z_min_, z_max_);
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

            selectData(std::min(p1.x(), p2.x()), std::max(p1.x(), p2.x()), std::min(p1.y(), p2.y()), std::max(p1.y(), p2.y()));
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
void GridViewDataWidget::selectData (double x_min, double x_max, double y_min, double y_max)
{
    bool ctrl_pressed = QApplication::keyboardModifiers() & Qt::ControlModifier;

    loginf << "GridViewDataWidget: selectData: x_min " << x_min << " x_max " << x_max
           << " y_min " << y_min << " y_max " << y_max << " ctrl pressed " << ctrl_pressed;

    unsigned int sel_cnt = 0;
    for (auto& buf_it : viewData())
    {
        assert (buf_it.second->has<bool>(DBContent::selected_var.name()));
        NullableVector<bool>& selected_vec = buf_it.second->get<bool>(DBContent::selected_var.name());

        assert (buf_it.second->has<unsigned long>(DBContent::meta_var_rec_num_.name()));
        NullableVector<unsigned long>& rec_num_vec = buf_it.second->get<unsigned long>(
                    DBContent::meta_var_rec_num_.name());

        std::map<unsigned long, std::vector<unsigned int>> rec_num_indexes =
                rec_num_vec.distinctValuesWithIndexes(0, rec_num_vec.size());
        // rec_num -> index

        std::vector<double>&        x_values       = x_values_.at(buf_it.first);
        std::vector<double>&        y_values       = y_values_.at(buf_it.first);
        std::vector<unsigned long>& rec_num_values = rec_num_values_.at(buf_it.first);

        assert (x_values.size() == y_values.size());
        assert (x_values.size() == rec_num_values.size());

        double x, y;
        bool in_range;
        unsigned long rec_num, index;

        for (unsigned int cnt=0; cnt < x_values.size(); ++cnt)
        {
            x = x_values.at(cnt);
            y = y_values.at(cnt);
            rec_num = rec_num_values.at(cnt);

            in_range = false;

            if (!std::isnan(x) && !std::isnan(y))
                in_range =  x >= x_min && x <= x_max && y >= y_min && y <= y_max;

            assert (rec_num_indexes.count(rec_num));
            std::vector<unsigned int>& indexes = rec_num_indexes.at(rec_num);
            assert (indexes.size() == 1);

            index = indexes.at(0);

            if (ctrl_pressed && !selected_vec.isNull(index) && selected_vec.get(index))
                in_range = true; // add selection to existing

            selected_vec.set(index, in_range);

            if (in_range)
                ++sel_cnt;
        }
    }

    loginf << "GridViewDataWidget: selectData: sel_cnt " << sel_cnt;

    emit view_->selectionChangedSignal();
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
void GridViewDataWidget::updateMinMax()
{
    auto calcMinMax = [ & ] (bool& has_min_max, 
                             double& vmin,
                             double& vmax,
                             const std::map<std::string, std::vector<double>>& values)
    {
        has_min_max = false;

        for (const auto& dbc_values_it : values)
        {
            for (const auto v_it : dbc_values_it.second)
            {
                if (has_min_max)
                {
                    vmin = std::min(vmin, v_it);
                    vmax = std::max(vmax, v_it);
                }
                else
                {
                    vmin        = v_it;
                    vmax        = v_it;
                    has_min_max = true;
                }
            }
        }
    };

    calcMinMax(has_x_min_max_, x_min_, x_max_, x_values_);
    calcMinMax(has_y_min_max_, y_min_, y_max_, y_values_);
    calcMinMax(has_z_min_max_, z_min_, z_max_, z_values_);

    logdbg << "GridViewDataWidget: updateMinMax: "
           << "has_x_min_max " << has_x_min_max_ << " x_min " << x_min_ << " x_max " << x_max_ << " "
           << "has_y_min_max " << has_y_min_max_ << " y_min " << y_min_ << " y_max " << y_max_ << " "
           << "has_z_min_max " << has_z_min_max_ << " z_min " << z_min_ << " z_max " << z_max_;
}

/**
*/
void GridViewDataWidget::resetGridChart()
{
    grid_chart_.reset();
    grid_.reset();
    grid_rendering_ = QImage();
    grid_roi_ = QRectF();
}

/**
*/
bool GridViewDataWidget::updateGridChart()
{
    bool has_data = (hasData() && 
                     !x_values_.empty() && 
                     variablesOk() && 
                     has_x_min_max_ && 
                     has_y_min_max_ && 
                     has_z_min_max_);

    resetGridChart();

    updateGrid();

    QtCharts::QChart* chart = new QtCharts::QChart();
    updateChart(chart, has_data);
    
    grid_chart_.reset(new QtCharts::GridViewChart(this, chart));
    grid_chart_->setObjectName("chart_view");

    connect (grid_chart_.get(), &QtCharts::GridViewChart::rectangleSelectedSignal,
             this, &GridViewDataWidget::rectangleSelectedSlot, Qt::ConnectionType::QueuedConnection);

    main_layout_->addWidget(grid_chart_.get());

    return has_data;
}

/**
*/
void GridViewDataWidget::updateGrid()
{
    loginf << "GridViewDataWidget: updateGrid";

    bool has_data = (hasData() && 
                     !x_values_.empty() && 
                     variablesOk() && 
                     has_x_min_max_ && 
                     has_y_min_max_ && 
                     has_z_min_max_);
    if (!has_data)
        return;

    auto bounds = getXYBounds();
    assert(bounds.isValid());

    auto z_bounds = getZBounds();
    assert(z_bounds.has_value());

    const auto& settings = view_->settings();

    Grid2DLayers layers;

    grid2d::GridResolution res = grid2d::GridResolution().setCellCount(settings.grid_resolution, settings.grid_resolution).setBorder(0.01);

    grid_.reset(new Grid2D);
    bool ok = grid_->create(bounds, res);
    assert(ok);

    loginf << "GridViewDataWidget: renderGrid: Created grid of " << grid_->numCellsX() << "x" << grid_->numCellsY();

    for (const auto& dbc_values : z_values_)
    {
        const auto& x_values = x_values_.at(dbc_values.first);
        const auto& y_values = y_values_.at(dbc_values.first);
        const auto& z_values = dbc_values.second;

        loginf << "GridViewDataWidget: renderGrid: dbcontent " << dbc_values.first
               << " #x " << x_values.size()
               << " #y " << y_values.size()
               << " #z " << z_values.size();

        for (size_t i = 0; i < z_values.size(); ++i)
            if (std::isfinite(z_values[ i ]))
                grid_->addValue(x_values[ i ], y_values[ i ], z_values[ i ]);
    }

    loginf << "GridViewDataWidget: renderGrid: getting layer";

    auto V = grid_->getValues((grid2d::ValueType)settings.value_type);
    auto layer_name = grid2d::valueTypeToString((grid2d::ValueType)settings.value_type);

    loginf << "GridViewDataWidget: renderGrid: layer = "<< V.cols() << "x" << V.rows();

    layers.addLayer(layer_name, grid_->getReference(), V);

    loginf << "GridViewDataWidget: renderGrid: rendering";

    Grid2DRenderSettings render_settings;
    render_settings.pixels_per_cell = settings.render_pixels_per_cell;
    render_settings.color_map.set(QColor(settings.render_color_min.c_str()), 
                                  QColor(settings.render_color_max.c_str()), 
                                  settings.render_color_num_steps);

    auto rendering = Grid2DLayerRenderer::render(layers.layers().at(layer_name), render_settings);

    loginf << "GridViewDataWidget: renderGrid: finished";

    grid_rendering_ = rendering.first;
    grid_roi_       = grid_->gridBounds();
}

/**
*/
void GridViewDataWidget::updateChart(QtCharts::QChart* chart, bool has_data)
{
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    chart->setBackgroundRoundness(0);
    chart->setDropShadowEnabled(false);
    chart->legend()->setAlignment(Qt::AlignBottom);

    auto createAxes = [ & ] ()
    {
        chart->createDefaultAxes();

        //config x axis
        loginf << "GridViewDataWidget: updateDataSeries: title x ' "
               << view_->variable(0).description() << "'";
        assert (chart->axes(Qt::Horizontal).size() == 1);
        chart->axes(Qt::Horizontal).at(0)->setTitleText(view_->variable(0).description().c_str());

        //config y axis
        loginf << "GridViewDataWidget: updateDataSeries: title y ' "
               << view_->variable(1).description() << "'";
        assert (chart->axes(Qt::Vertical).size() == 1);
        chart->axes(Qt::Vertical).at(0)->setTitleText(view_->variable(1).description().c_str());
    };

    if (has_data)
    {
        chart->legend()->setVisible(false);

        QtCharts::QScatterSeries* series = new QtCharts::QScatterSeries;

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
    VariableViewDataWidget::viewInfoJSON_impl(info);

    info[ "num_valid"   ] = valid_cnt_;
    info[ "num_selected"] = selected_cnt_;
    info[ "num_nan"     ] = nan_value_cnt_;

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
    
    nlohmann::json input_value_info = nlohmann::json::array();

    for (const auto& it : x_values_)
    {
        nlohmann::json dbo_info;

        const auto& y_values    = y_values_.at(it.first);
        const auto& z_values    = y_values_.at(it.first);
        const auto& valid_count = dbo_valid_counts_.at(it.first);

        dbo_info[ "dbo_type"       ] = it.first;
        dbo_info[ "num_input_x"    ] = it.second.size();
        dbo_info[ "num_input_y"    ] = y_values.size();
        dbo_info[ "num_input_z"    ] = z_values.size();
        dbo_info[ "num_input_valid"] = valid_count;

        input_value_info.push_back(dbo_info);
    }

    info[ "input_values" ] = input_value_info;

    auto zoomActive = [ & ] (const QRectF& bounds_data, const QRectF& bounds_axis)
    {
        if (!bounds_data.isValid() || !bounds_axis.isValid())
            return false;

        return (bounds_axis.left()   > bounds_data.left()  ||
                bounds_axis.right()  < bounds_data.right() ||
                bounds_axis.top()    > bounds_data.top()   ||
                bounds_axis.bottom() < bounds_data.bottom());
    };

    //@TODO
}
