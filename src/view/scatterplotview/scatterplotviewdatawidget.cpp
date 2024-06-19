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

#include "scatterplotviewdatawidget.h"
#include "scatterplotviewwidget.h"
#include "scatterplotview.h"
#include "viewvariable.h"
//#include "compass.h"
#include "buffer.h"
//#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/metavariable.h"
#include "scatterplotviewdatasource.h"
#include "scatterplotviewchartview.h"
#include "logger.h"
#include "property_templates.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QTabWidget>
#include <QStackedLayout>

//#include <QtCharts/QChartView>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QLegend>
#include <QtCharts/QValueAxis>
#include <QGraphicsLayout>
#include <QShortcut>
#include <QApplication>

#include <algorithm>

QT_CHARTS_USE_NAMESPACE

using namespace std;
using namespace dbContent;

/**
*/
ScatterPlotViewDataWidget::ScatterPlotViewDataWidget(ScatterPlotViewWidget* view_widget,
                                                     QWidget* parent,
                                                     Qt::WindowFlags f)
:   VariableViewDataWidget(view_widget, view_widget->getView(), parent, f)
{
    view_ = view_widget->getView();
    assert(view_);

    data_source_ = view_->getDataSource();
    assert(data_source_);

    main_layout_ = new QHBoxLayout();
    main_layout_->setMargin(0);

    setLayout(main_layout_);

    updateChart();
}

/**
*/
ScatterPlotViewDataWidget::~ScatterPlotViewDataWidget()
{
    logdbg << "ScatterPlotViewDataWidget: dtor";
}

/**
*/
bool ScatterPlotViewDataWidget::postLoadTrigger()
{
    // disable connection lines? (triggers redraw)
    if (view_->useConnectionLines() && loadedDataCount() > ConnectLinesDataCountMax) 
    {
        loginf << "ScatterPlotViewDataWidget: loadingDone_impl: loaded data items >" << ConnectLinesDataCountMax << ", disabling connection lines";

        view_->useConnectionLines(false);
        emit displayChanged();

        return true;
    }

    //no redraw triggered
    return false;
}

/**
*/
void ScatterPlotViewDataWidget::resetVariableData()
{
    resetCounts();
}

/**
*/
void ScatterPlotViewDataWidget::resetVariableDisplay() 
{
    chart_view_.reset(nullptr);
}

/**
*/
void ScatterPlotViewDataWidget::preUpdateVariableDataEvent() 
{
    resetCounts();
}

/**
*/
void ScatterPlotViewDataWidget::postUpdateVariableDataEvent() 
{
    updateMinMax();
}

/**
*/
bool ScatterPlotViewDataWidget::updateVariableDisplay() 
{
    return updateChart();
}

/**
*/
void ScatterPlotViewDataWidget::updateVariableData(const std::string& dbcontent_name,
                                                   Buffer& buffer)
{
    loginf << "ScatterPlotViewDataWidget: updateVariableData: updating data";

    logdbg << "ScatterPlotViewDataWidget: updateVariableData: before"
           << " x " << x_values_[dbcontent_name].size()
           << " y " << y_values_[dbcontent_name].size();

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

    //check counts
    assert (x_values_[dbcontent_name].size() == y_values_[dbcontent_name].size());
    assert (x_values_[dbcontent_name].size() == selected_values_[dbcontent_name].size());
    assert (x_values_[dbcontent_name].size() == rec_num_values_[dbcontent_name].size());

    logdbg << "ScatterPlotViewDataWidget: updateVariableData: after"
           << " x " << x_values_[dbcontent_name].size()
           << " y " << y_values_[dbcontent_name].size();
}

/**
*/
void ScatterPlotViewDataWidget::updateVariableData(int var_idx, std::string dbcontent_name, unsigned int current_size)
{
    logdbg << "ScatterPlotViewDataWidget: updateVariableData: dbcontent_name " << dbcontent_name << " current_size " << current_size;
    
    assert(var_idx == 0 || var_idx == 1);
    assert (viewData().count(dbcontent_name));
    Buffer* buffer = viewData().at(dbcontent_name).get();

    auto& buffer_counts = var_idx == 0 ? buffer_x_counts_ : buffer_y_counts_;
    auto& values        = var_idx == 0 ? x_values_ : y_values_;

    unsigned int last_size = 0;

    if (buffer_counts.count(dbcontent_name))
        last_size = buffer_counts.at(dbcontent_name);

    ViewVariable& view_var = view_->variable(var_idx);
    Variable* data_var = view_var.getFor(dbcontent_name);
    if (!data_var)
    {
        logwrn << "ScatterPlotViewDataWidget: updateVariableData: could not retrieve data var";
        return;
    }

    PropertyDataType data_type = data_var->dataType();
    string current_var_name = data_var->name();

    logdbg << "ScatterPlotViewDataWidget: updateVariableData: updating, last size " << last_size;

    #define UpdateFunc(PDType, DType)                                       \
        assert(view_var.settings().valid_data_types.count(PDType) != 0);    \
        assert(buffer->has<DType>(current_var_name));                       \
                                                                            \
        NullableVector<DType>& data = buffer->get<DType>(current_var_name); \
                                                                            \
        appendData(data, values[dbcontent_name], last_size, current_size);  \
        buffer_counts[dbcontent_name] = current_size;

    #define NotFoundFunc                                                                                                           \
        logerr << "ScatterPlotViewDataWidget: updateVariableData: impossible for property type " << Property::asString(data_type); \
        throw std::runtime_error("ScatterPlotViewDataWidget: updateVariableData: impossible property type " + Property::asString(data_type));

    SwitchPropertyDataType(data_type, UpdateFunc, NotFoundFunc)

    logdbg << "ScatterPlotViewDataWidget: updateVariableData: updated size " << buffer_counts.at(dbcontent_name);
}

/**
*/
void ScatterPlotViewDataWidget::resetCounts()
{
    buffer_x_counts_.clear();
    buffer_y_counts_.clear();

    x_values_.clear();
    y_values_.clear();

    has_x_min_max_ = false;
    has_y_min_max_ = false;

    selected_values_.clear();
    rec_num_values_.clear();

    nan_value_cnt_ = 0;
    valid_cnt_     = 0;
    selected_cnt_  = 0;

    dbcont_valid_counts_.clear();
}

/**
*/
void ScatterPlotViewDataWidget::toolChanged_impl(int mode)
{
    selected_tool_ = (ScatterPlotViewDataTool)mode;

    if (chart_view_)
        chart_view_->onToolChanged();
}

/**
*/
ScatterPlotViewDataTool ScatterPlotViewDataWidget::selectedTool() const
{
    return selected_tool_;
}

/**
*/
QPixmap ScatterPlotViewDataWidget::renderPixmap()
{
    assert (chart_view_);
    return chart_view_->grab();
    //QPixmap p (chart_view_->size());
    //chart_view_->render(&p);
}

/**
*/
unsigned int ScatterPlotViewDataWidget::nullValueCount() const
{
    return nan_value_cnt_;
}

/**
*/
QRectF ScatterPlotViewDataWidget::getDataBounds() const
{
    if (!has_x_min_max_ || !has_y_min_max_)
        return QRectF();

    return QRectF(x_min_, y_min_, x_max_ - x_min_, y_max_ - y_min_);
}

/**
*/
void ScatterPlotViewDataWidget::rectangleSelectedSlot (QPointF p1, QPointF p2)
{
    loginf << "ScatterPlotViewDataWidget: rectangleSelectedSlot";

    if (chart_view_ && chart_view_->chart())
    {
        if (selected_tool_ == SP_ZOOM_RECT_TOOL)
        {
            loginf << "ScatterPlotViewDataWidget: rectangleSelectedSlot: zoom";

            //TODO: prevent from going nuts when zero rect is passed!

            if (chart_view_->chart()->axisX() && chart_view_->chart()->axisY())
            {
                chart_view_->chart()->axisX()->setRange(min(p1.x(), p2.x()), max(p1.x(), p2.x()));
                chart_view_->chart()->axisY()->setRange(min(p1.y(), p2.y()), max(p1.y(), p2.y()));
            }
        }
        else if (selected_tool_ == SP_SELECT_TOOL)
        {
            loginf << "ScatterPlotViewDataWidget: rectangleSelectedSlot: select";

            selectData(min(p1.x(), p2.x()), max(p1.x(), p2.x()), min(p1.y(), p2.y()), max(p1.y(), p2.y()));
        }
        else
            throw std::runtime_error("ScatterPlotViewDataWidget: rectangleSelectedSlot: unknown tool "
                                     +to_string((unsigned int)selected_tool_));
    }

    endTool();
}

/**
*/
void ScatterPlotViewDataWidget::invertSelectionSlot()
{
    loginf << "ScatterPlotViewDataWidget: invertSelectionSlot";

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
void ScatterPlotViewDataWidget::clearSelectionSlot()
{
    loginf << "ScatterPlotViewDataWidget: clearSelectionSlot";

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
void ScatterPlotViewDataWidget::resetZoomSlot()
{
    loginf << "ScatterPlotViewDataWidget: resetZoomSlot";

    if (chart_view_ && chart_view_->chart())
    {
        //chart_view_->chart()->createDefaultAxes();

        if (chart_view_->chart()->axisX() && chart_view_->chart()->axisY()
                && has_x_min_max_ && has_y_min_max_)
        {
            chart_view_->chart()->axisX()->setRange(x_min_, x_max_);
            chart_view_->chart()->axisY()->setRange(y_min_, y_max_);
        }

        //chart_->zoomReset();
    }
}

/**
*/
void ScatterPlotViewDataWidget::updateMinMax()
{
    has_x_min_max_ = false;

    for (auto& x_values_it : x_values_)
    {
        for (auto x_it : x_values_it.second)
        {
            if (has_x_min_max_)
            {
                x_min_ = std::min(x_min_, x_it);
                x_max_ = std::max(x_max_, x_it);
            }
            else
            {
                x_min_ = x_it;
                x_max_ = x_it;

                has_x_min_max_ = true;
            }
        }
    }

    has_y_min_max_ = false;

    for (auto& y_values_it : y_values_)
    {
        for (auto y_it : y_values_it.second)
        {
            if (has_y_min_max_)
            {
                y_min_ = std::min(y_min_, y_it);
                y_max_ = std::max(y_max_, y_it);
            }
            else
            {
                y_min_ = y_it;
                y_max_ = y_it;

                has_y_min_max_ = true;
            }
        }
    }

    logdbg << "ScatterPlotViewDataWidget: updateMinMax: "
           << "has_x_min_max " << has_x_min_max_ << " "
           << "x_min " << x_min_ << " "
           << "x_max " << x_max_ << " "
           << "has_y_min_max " << has_y_min_max_ << " "
           << "y_min " << y_min_ << " "
           << "y_max " << y_max_;
}

/**
*/
bool ScatterPlotViewDataWidget::updateChart()
{
    logdbg << "ScatterPlotViewDataWidget: updateChart";

    assert (main_layout_);

    chart_view_.reset(nullptr);

    QChart* chart = new QChart();
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    chart->setBackgroundRoundness(0);
    chart->setDropShadowEnabled(false);

    chart->legend()->setAlignment(Qt::AlignBottom);

    bool has_data = (x_values_.size() && y_values_.size() && variablesOk());

    updateDataSeries(chart);

    chart->update();

    chart_view_.reset(new ScatterPlotViewChartView(this, chart));
    chart_view_->setObjectName("chart_view");

    connect (chart_view_.get(), &ScatterPlotViewChartView::rectangleSelectedSignal,
             this, &ScatterPlotViewDataWidget::rectangleSelectedSlot, Qt::ConnectionType::QueuedConnection);

    // queued needed, otherwise crash when signal is emitted in ScatterPlotViewChartView::seriesReleasedSlot

    for (auto series_it : chart->series())
    {
        QScatterSeries* scat_series = dynamic_cast<QScatterSeries*>(series_it);
        if (!scat_series)
            continue;

        assert (scat_series);

        connect (scat_series, &QScatterSeries::pressed,
                 chart_view_.get(), &ScatterPlotViewChartView::seriesPressedSlot);
        connect (scat_series, &QScatterSeries::released,
                 chart_view_.get(), &ScatterPlotViewChartView::seriesReleasedSlot);
    }

    main_layout_->addWidget(chart_view_.get());

    return has_data;
}

/**
*/
void ScatterPlotViewDataWidget::updateDataSeries(QtCharts::QChart* chart)
{
    //we obtain valid data if a data range is available and if the variables are available in the buffer data
    bool has_data = (x_values_.size() && y_values_.size() && variablesOk());

    auto createAxes = [ & ] ()
    {
        chart->createDefaultAxes();

        //config x axis
        loginf << "ScatterPlotViewDataWidget: updateDataSeries: title x '"
               << view_->variable(0).description() << "'";
        assert (chart->axes(Qt::Horizontal).size() == 1);
        chart->axes(Qt::Horizontal).at(0)->setTitleText(view_->variable(0).description().c_str());

        //config y axis
        loginf << "ScatterPlotViewDataWidget: updateDataSeries: title y '"
               << view_->variable(1).description() << "'";
        assert (chart->axes(Qt::Vertical).size() == 1);
        chart->axes(Qt::Vertical).at(0)->setTitleText(view_->variable(1).description().c_str());
    };

    if (has_data)
    {
        //data available

        chart->legend()->setVisible(true);

        nan_value_cnt_ = 0;
        valid_cnt_     = 0;
        selected_cnt_  = 0;

        dbcont_valid_counts_.clear();

        bool use_connection_lines = view_->useConnectionLines();

        struct SymbolLineSeries { QScatterSeries* scatter_series;
                                  QLineSeries* line_series;};

        //generate needed series and sort pointers
        //qtcharts when rendering opengl will sort the series into a map using the pointer of the series as a key
        //we exploit this behavior to obtain the correct render order for our data

        //generate pointers
        std::vector<SymbolLineSeries> series (x_values_.size() + 1); // last one selected
        for (size_t i = 0; i < series.size(); ++i)
        {
            series[ i ].scatter_series = new QScatterSeries;
            series[ i ].scatter_series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
            series[ i ].scatter_series->setMarkerSize(MarkerSizePx);
            series[ i ].scatter_series->setUseOpenGL(true);

            if (use_connection_lines)
            {
                series[ i ].line_series = new QLineSeries;
                QPen pen = series[ i ].line_series->pen();
                pen.setWidth(2);
                //pen.setBrush(QBrush("red")); // or just pen.setColor("red");
                series[ i ].line_series->setPen(pen);

                series[ i ].line_series->setUseOpenGL(true);
            }
            else
                series[ i ].line_series = nullptr;
        }

        //sort pointers to obtain correct render order (as it happens inside qtcharts)
        sort(series.begin(), series.end(),
             [&](const SymbolLineSeries& a, const SymbolLineSeries& b) {return a.scatter_series > b.scatter_series; });

        //the smallest pointer is the one for the selection, so the selection will be rendered on top
        QScatterSeries* selected_symbol_series = series.front().scatter_series;
        selected_symbol_series->setMarkerSize(MarkerSizeSelectedPx);
        selected_symbol_series->setColor(Qt::yellow);

        QLineSeries* selected_line_series = nullptr;

        if (use_connection_lines)
        {
            selected_line_series = series.front().line_series;
            selected_line_series->setColor(Qt::yellow);
        }

        //distribute sorted pointers to dbcontent types
        std::map<std::string, SymbolLineSeries> series_map;
        {
            int cnt = 1;
            for (auto it = x_values_.begin(); it != x_values_.end(); ++it)
                series_map[ it->first ] = series[ cnt++ ];
        }

        //now add data to series
        for (auto& data : x_values_)
        {
            auto& dbcont_value_cnt = dbcont_valid_counts_[ data.first ];
            dbcont_value_cnt = 0;

            vector<double>& x_values        = data.second;
            vector<double>& y_values        = y_values_[data.first];
            vector<bool>&   selected_values = selected_values_[data.first];

            QScatterSeries* chart_symbol_series = series_map[ data.first ].scatter_series;
            chart_symbol_series->setColor(colorForDBContent(data.first));

            QLineSeries* chart_line_series = nullptr;

            if (use_connection_lines)
            {
                chart_line_series = series_map[ data.first ].line_series;
                chart_line_series->setColor(colorForDBContent(data.first));
            }

            assert (x_values.size() == y_values.size());
            assert (x_values.size() == selected_values.size());

            unsigned int sum_cnt {0};

            for (unsigned int cnt=0; cnt < x_values.size(); ++cnt)
            {
                if (!std::isnan(x_values.at(cnt)) && !std::isnan(y_values.at(cnt)))
                {
                    ++valid_cnt_;
                    ++dbcont_value_cnt;

                    if (selected_values.at(cnt))
                    {
                        selected_symbol_series->append(x_values.at(cnt), y_values.at(cnt));

                        if (use_connection_lines)
                        {
                            assert (selected_line_series);
                            selected_line_series->append(x_values.at(cnt), y_values.at(cnt));
                        }

                        ++selected_cnt_;
                    }
                    else
                    {
                        chart_symbol_series->append(x_values.at(cnt), y_values.at(cnt));

                        if (use_connection_lines)
                        {
                            assert (chart_line_series);
                            chart_line_series->append(x_values.at(cnt), y_values.at(cnt));
                        }

                        ++sum_cnt;
                    }
                }
                else
                {
                    ++nan_value_cnt_;
                }
            }

            if (!dbcont_value_cnt)
                continue;

            if (sum_cnt)
            {
                logdbg << "ScatterPlotViewDataWidget: updateDataSeries: adding " << data.first << " (" << sum_cnt << ")";

                chart_symbol_series->setName((data.first+" ("+to_string(sum_cnt)+")").c_str());
                chart->addSeries(chart_symbol_series);

                if (use_connection_lines)
                {
                    assert (chart_line_series);
                    chart->addSeries(chart_line_series);

                    logdbg << "ScatterPlotViewDataWidget: updateDataSeries: connection lines "
                           << chart_line_series->count();
                }
            }
            else
            {
                logdbg << "ScatterPlotViewDataWidget: updateDataSeries: removing empty " << data.first;

                delete chart_symbol_series;

                if (use_connection_lines)
                {
                    assert (chart_line_series);
                    delete chart_line_series;
                }
            }
        }

        if (valid_cnt_)
        {
            //valid values found

            if (selected_cnt_ > 0)
            {
                logdbg << "ScatterPlotViewDataWidget: updateDataSeries: adding " << " Selected (" << selected_cnt_ << ")";

                //add series for selected values
                selected_symbol_series->setName(("Selected ("+to_string(selected_cnt_)+")").c_str());
                chart->addSeries(selected_symbol_series);

                if (use_connection_lines)
                {
                    assert (selected_line_series);
                    chart->addSeries(selected_line_series);
                }
            }
            else
            {
                logdbg << "ScatterPlotViewDataWidget: updateDataSeries: deleting not needed selected";

                //series for selection not needed
                delete selected_symbol_series;
                selected_symbol_series = nullptr;

                if (use_connection_lines)
                {
                    assert (selected_line_series);
                    delete selected_line_series;
                    selected_line_series = nullptr;
                }
            }

            createAxes();
        }
        else
        {
            //no valid values at all
            has_data = false;
            logdbg << "ScatterPlotViewDataWidget: updateDataSeries: no valid data";
        }
    }
    else
    {
        //bad data range or vars not in buffer
        logdbg << "ScatterPlotViewDataWidget: updateDataSeries: no data, size x "
               << x_values_.size() << " y " << y_values_.size();
    }

    if (!has_data)
    {
        //no data -> generate default empty layout
        chart->legend()->setVisible(false);

        QScatterSeries* series = new QScatterSeries;
        chart->addSeries(series);

        createAxes();

        chart->axes(Qt::Horizontal).at(0)->setLabelsVisible(false);
        chart->axes(Qt::Horizontal).at(0)->setGridLineVisible(false);
        chart->axes(Qt::Horizontal).at(0)->setMinorGridLineVisible(false);

        chart->axes(Qt::Vertical).at(0)->setLabelsVisible(false);
        chart->axes(Qt::Vertical).at(0)->setGridLineVisible(false);
        chart->axes(Qt::Vertical).at(0)->setMinorGridLineVisible(false);
    }
}

/**
*/
void ScatterPlotViewDataWidget::mouseMoveEvent(QMouseEvent* event)
{
    //setCursor(current_cursor_);
    //osgEarth::QtGui::ViewerWidget::mouseMoveEvent(event);

    QWidget::mouseMoveEvent(event);
}

/**
*/
void ScatterPlotViewDataWidget::selectData (double x_min, double x_max, double y_min, double y_max)
{
    bool ctrl_pressed = QApplication::keyboardModifiers() & Qt::ControlModifier;

    loginf << "ScatterPlotViewDataWidget: selectData: x_min " << x_min << " x_max " << x_max
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

    loginf << "ScatterPlotViewDataWidget: selectData: sel_cnt " << sel_cnt;

    emit view_->selectionChangedSignal();
}

/**
 */
void ScatterPlotViewDataWidget::viewInfoJSON_impl(nlohmann::json& info) const
{
    //!call base!
    VariableViewDataWidget::viewInfoJSON_impl(info);

    info[ "num_valid"   ] = valid_cnt_;
    info[ "num_selected"] = selected_cnt_;
    info[ "num_nan"     ] = nan_value_cnt_;

    auto bounds       = getDataBounds();
    bool bounds_valid = bounds.isValid();

    info[ "data_bounds_valid" ] = bounds_valid;
    info[ "data_bounds_xmin"  ] = bounds_valid ? bounds.left()   : 0.0;
    info[ "data_bounds_ymin"  ] = bounds_valid ? bounds.top()    : 0.0;
    info[ "data_bounds_xmax"  ] = bounds_valid ? bounds.right()  : 0.0;
    info[ "data_bounds_ymax"  ] = bounds_valid ? bounds.bottom() : 0.0;
    
    nlohmann::json input_value_info = nlohmann::json::array();

    for (const auto& it : x_values_)
    {
        nlohmann::json dbo_info;

        const auto& y_values    = y_values_.at(it.first);
        const auto& valid_count = dbcont_valid_counts_.at(it.first);

        dbo_info[ "dbo_type"       ] = it.first;
        dbo_info[ "num_input_x"    ] = it.second.size();
        dbo_info[ "num_input_y"    ] = y_values.size();
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

    if (chart_view_)
    {
        nlohmann::json chart_info;

        auto series = chart_view_->chart()->series();

        chart_info[ "x_axis_label" ] = chart_view_->chart()->axisX()->titleText().toStdString();
        chart_info[ "y_axis_label" ] = chart_view_->chart()->axisY()->titleText().toStdString();

        auto axis_x = dynamic_cast<QValueAxis*>(chart_view_->chart()->axisX());
        auto axis_y = dynamic_cast<QValueAxis*>(chart_view_->chart()->axisY());

        bool   has_axis_bounds = (axis_x != nullptr && axis_y != nullptr);
        QRectF axis_bounds     = has_axis_bounds ? QRectF(axis_x->min(), 
                                                          axis_y->min(),
                                                          axis_x->max() - axis_x->min(),
                                                          axis_y->max() - axis_y->min()) : QRectF();

        info[ "axis_bounds_valid" ] = has_axis_bounds;
        info[ "axis_bounds_xmin"  ] = has_axis_bounds ? axis_bounds.left()   : 0.0;
        info[ "axis_bounds_xmax"  ] = has_axis_bounds ? axis_bounds.right()  : 0.0;
        info[ "axis_bounds_ymin"  ] = has_axis_bounds ? axis_bounds.top()    : 0.0;
        info[ "axis_bounds_ymax"  ] = has_axis_bounds ? axis_bounds.bottom() : 0.0;

        info[ "axis_zoom_active"  ] = zoomActive(bounds, axis_bounds);

        chart_info[ "num_series"] = series.count();

        nlohmann::json series_infos = nlohmann::json::array();

        bool has_connection_lines = false;

        for (auto s : series)
        {
            auto xy_series = dynamic_cast<QXYSeries*>(s);
            if (!xy_series)
                continue;

            bool line_type = xy_series->type() == QAbstractSeries::SeriesType::SeriesTypeLine;

            nlohmann::json series_info;
            series_info[ "type"  ] = line_type ? "lines" : "points";
            series_info[ "count" ] = xy_series->count();
            series_info[ "name"  ] = xy_series->name().toStdString();
            series_info[ "color" ] = xy_series->color().name().toStdString();

            series_infos.push_back(series_info);

            if (line_type)
                has_connection_lines = true;
        }

        chart_info[ "series"               ] = series_infos;
        chart_info[ "has_connection_lines" ] = has_connection_lines;

        info[ "chart" ] = chart_info;
    }
}

//void ScatterPlotViewDataWidget::showOnlySelectedSlot(bool value)
//{
//    loginf << "ScatterPlotViewDataWidget: showOnlySelectedSlot: " << value;
//    emit showOnlySelectedSignal(value);
//}

//void ScatterPlotViewDataWidget::usePresentationSlot(bool use_presentation)
//{
//    loginf << "ScatterPlotViewDataWidget: usePresentationSlot";

//    emit usePresentationSignal(use_presentation);
//}

//void ScatterPlotViewDataWidget::showAssociationsSlot(bool value)
//{
//    loginf << "ScatterPlotViewDataWidget: showAssociationsSlot: " << value;
//    emit showAssociationsSignal(value);
//}

//void ScatterPlotViewDataWidget::resetModels()
//{
//    if (all_buffer_table_widget_)
//        all_buffer_table_widget_->resetModel();

//    for (auto& table_widget_it : buffer_tables_)
//        table_widget_it.second->resetModel();
//}

//void ScatterPlotViewDataWidget::updateToSelection()
//{
//    if (all_buffer_table_widget_)
//        all_buffer_table_widget_->updateToSelection();

//    for (auto& table_widget_it : buffer_tables_)
//        table_widget_it.second->updateToSelection();
//}

//void ScatterPlotViewDataWidget::selectFirstSelectedRow()
//{
//    if (all_buffer_table_widget_)
//        all_buffer_table_widget_->selectSelectedRows();
//}

//AllBufferTableWidget* ScatterPlotViewDataWidget::getAllBufferTableWidget ()
//{
//    assert (all_buffer_table_widget_);
//    return all_buffer_table_widget_;
//}
