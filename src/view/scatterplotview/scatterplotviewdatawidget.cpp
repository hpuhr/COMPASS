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
#include "scatterplotview.h"
#include "compass.h"
#include "buffer.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/metavariable.h"
#include "scatterplotviewdatasource.h"
#include "scatterplotviewchartview.h"
#include "logger.h"

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

ScatterPlotViewDataWidget::ScatterPlotViewDataWidget(ScatterPlotView* view, 
                                                     ScatterPlotViewDataSource* data_source,
                                                     QWidget* parent, 
                                                     Qt::WindowFlags f)
    : ViewDataWidget(parent, f), view_(view), data_source_(data_source)
{
    assert(data_source_);
    setContentsMargins(0, 0, 0, 0);

    main_layout_ = new QHBoxLayout();

    setLayout(main_layout_);

    colors_["CAT001"] = QColor("#00FF00");
    colors_["CAT010"] = QColor("#FFCC00");
    colors_["CAT020"] = QColor("#FF0000");
    colors_["CAT021"] = QColor("#6666FF");
    colors_["CAT048"] = QColor("#00FF00");
    colors_["RefTraj"] = QColor("#FFA500");
    colors_["CAT062"] = QColor("#CCCCCC");
}

ScatterPlotViewDataWidget::~ScatterPlotViewDataWidget()
{
    logdbg << "ScatterPlotViewDataWidget: dtor";
}

void ScatterPlotViewDataWidget::updatePlot()
{
    logdbg << "ScatterPlotViewDataWidget: updatePlot";

    buffer_x_counts_.clear();
    buffer_y_counts_.clear();

    x_values_.clear();
    y_values_.clear();
    has_x_min_max_ = false;
    has_y_min_max_ = false;

    selected_values_.clear();
    rec_num_values_.clear();

    chart_view_.reset(nullptr);

    shows_data_ = false;
    x_var_not_in_buffer_ = false;
    y_var_not_in_buffer_ = false;
    nan_value_cnt_ = 0;

    updateFromAllData();
    updateMinMax();
    updateChart();
}

void ScatterPlotViewDataWidget::clear ()
{
    buffers_.clear();
    buffer_x_counts_.clear();
    buffer_y_counts_.clear();

    x_values_.clear();
    y_values_.clear();
    has_x_min_max_ = false;
    has_y_min_max_ = false;

    selected_values_.clear();
    rec_num_values_.clear();

    chart_view_.reset(nullptr);

    shows_data_ = false;
    x_var_not_in_buffer_ = false;
    y_var_not_in_buffer_ = false;
    nan_value_cnt_ = 0;
}

ScatterPlotViewDataTool ScatterPlotViewDataWidget::selectedTool() const
{
    return selected_tool_;
}

bool ScatterPlotViewDataWidget::showsData() const
{
    return shows_data_;
}

bool ScatterPlotViewDataWidget::xVarNotInBuffer() const
{
    return x_var_not_in_buffer_;
}

bool ScatterPlotViewDataWidget::yVarNotInBuffer() const
{
    return y_var_not_in_buffer_;
}

QPixmap ScatterPlotViewDataWidget::renderPixmap()
{
    assert (chart_view_);
    return chart_view_->grab();
    //QPixmap p (chart_view_->size());
    //chart_view_->render(&p);
}

unsigned int ScatterPlotViewDataWidget::nullValueCnt() const
{
    return nan_value_cnt_;
}

QRectF ScatterPlotViewDataWidget::getDataBounds() const
{
    if (!has_x_min_max_ || !has_y_min_max_)
        return QRectF();

    return QRectF(x_min_, y_min_, x_max_ - x_min_, y_max_ - y_min_);
}

void ScatterPlotViewDataWidget::loadingStartedSlot()
{
    clear();
}

void ScatterPlotViewDataWidget::updateDataSlot(
        const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset)
{
    logdbg << "ScatterPlotViewDataWidget: updateDataSlot: start";

    buffers_ = data;
}

void ScatterPlotViewDataWidget::loadingDoneSlot()
{
    for (auto& buf_it : buffers_)
    {
        string dbcontent_name = buf_it.first;
        std::shared_ptr<Buffer> buffer = buf_it.second;

        unsigned int current_size = buffer->size();

        logdbg << "ScatterPlotViewDataWidget: loadingDoneSlot: before x " << x_values_[dbcontent_name].size()
               << " y " << y_values_[dbcontent_name].size();

        assert (x_values_[dbcontent_name].size() == y_values_[dbcontent_name].size());
        assert (x_values_[dbcontent_name].size() == selected_values_[dbcontent_name].size());
        assert (x_values_[dbcontent_name].size() == rec_num_values_[dbcontent_name].size());

        loginf << "ScatterPlotViewDataWidget: loadingDoneSlot: dbo " << dbcontent_name
               << " canUpdateFromDataX " << canUpdateFromDataX(dbcontent_name)
               << " canUpdateFromDataY " << canUpdateFromDataY(dbcontent_name);

        if (canUpdateFromDataX(dbcontent_name) && canUpdateFromDataY(dbcontent_name))
        {
            loginf << "ScatterPlotViewDataWidget: loadingDoneSlot: updating data";

            // add selected flags & rec_nums
            assert (buffer->has<bool>(DBContent::selected_var.name()));
            assert (buffer->has<unsigned int>(DBContent::meta_var_rec_num_.name()));

            NullableVector<bool>& selected_vec = buffer->get<bool>(DBContent::selected_var.name());
            NullableVector<unsigned int>& rec_num_vec = buffer->get<unsigned int>(DBContent::meta_var_rec_num_.name());

            std::vector<bool>& selected_data = selected_values_[dbcontent_name];
            std::vector<unsigned int>& rec_num_data = rec_num_values_[dbcontent_name];

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

            updateFromDataX(dbcontent_name, current_size);
            updateFromDataY(dbcontent_name, current_size);
        }
        else
            logdbg << "ScatterPlotViewDataWidget: loadingDoneSlot: " << dbcontent_name
                   << " update not possible";


        logdbg << "ScatterPlotViewDataWidget: loadingDoneSlot: after x " << x_values_[dbcontent_name].size()
               << " y " << y_values_[dbcontent_name].size();

        assert (x_values_[dbcontent_name].size() == y_values_[dbcontent_name].size());
        assert (x_values_[dbcontent_name].size() == selected_values_[dbcontent_name].size());
        assert (x_values_[dbcontent_name].size() == rec_num_values_[dbcontent_name].size());
    }

    updateMinMax();
    updateChart();
}

/**
 */
void ScatterPlotViewDataWidget::toolChanged_impl(int mode)
{
    selected_tool_ = (ScatterPlotViewDataTool)mode;

    if (chart_view_)
        chart_view_->onToolChanged();
}

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

void ScatterPlotViewDataWidget::invertSelectionSlot()
{
    loginf << "ScatterPlotViewDataWidget: invertSelectionSlot";

    for (auto& buf_it : buffers_)
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

void ScatterPlotViewDataWidget::clearSelectionSlot()
{
    loginf << "ScatterPlotViewDataWidget: clearSelectionSlot";

    for (auto& buf_it : buffers_)
    {
        assert (buf_it.second->has<bool>(DBContent::selected_var.name()));
        NullableVector<bool>& selected_vec = buf_it.second->get<bool>(DBContent::selected_var.name());

        for (unsigned int cnt=0; cnt < buf_it.second->size(); ++cnt)
            selected_vec.set(cnt, false);
    }

    emit view_->selectionChangedSignal();
}

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

bool ScatterPlotViewDataWidget::canUpdateFromDataX(std::string dbcontent_name)
{
    if (!buffers_.count(dbcontent_name))
        return false;

    Buffer* buffer = buffers_.at(dbcontent_name).get();

    Variable* data_var {nullptr};

    if (!view_->hasDataVarX())
        return false;

    if (view_->isDataVarXMeta())
    {
        MetaVariable& meta_var = view_->metaDataVarX();
        if (!meta_var.existsIn(dbcontent_name))
            return false;

        data_var = &meta_var.getFor(dbcontent_name);
    }
    else
    {
        data_var = &view_->dataVarX();

        if (data_var->dbContentName() != dbcontent_name)
            return false;
    }
    assert (data_var);

    PropertyDataType data_type = data_var->dataType();
    string current_var_name = data_var->name();

    logdbg << "ScatterPlotViewDataWidget: canUpdateFromDataX: dbo " << dbcontent_name << " var "  << current_var_name;

    switch (data_type)
    {
    case PropertyDataType::BOOL:
    {
        if (!buffer->has<bool>(current_var_name))
        {
            x_var_not_in_buffer_ = true;
            return false;
        }
        else
            return true;

        break;
    }
    case PropertyDataType::CHAR:
    {
        if (!buffer->has<char>(current_var_name))
        {
            x_var_not_in_buffer_ = true;
            return false;
        }
        else
            return true;

        break;
    }
    case PropertyDataType::UCHAR:
    {
        if (!buffer->has<unsigned char>(current_var_name))
        {
            x_var_not_in_buffer_ = true;
            return false;
        }
        else
            return true;

        break;
    }
    case PropertyDataType::INT:
    {
        if (!buffer->has<int>(current_var_name))
        {
            x_var_not_in_buffer_ = true;
            return false;
        }
        else
            return true;

        break;
    }
    case PropertyDataType::UINT:
    {
        if (!buffer->has<unsigned int>(current_var_name))
        {
            x_var_not_in_buffer_ = true;
            return false;
        }
        else
            return true;

        break;
    }
    case PropertyDataType::LONGINT:
    {
        if (!buffer->has<long int>(current_var_name))
        {
            x_var_not_in_buffer_ = true;
            return false;
        }
        else
            return true;

        break;
    }
    case PropertyDataType::ULONGINT:
    {
        if (!buffer->has<unsigned long>(current_var_name))
        {
            x_var_not_in_buffer_ = true;
            return false;
        }
        else
            return true;

        break;
    }
    case PropertyDataType::FLOAT:
    {
        if (!buffer->has<float>(current_var_name))
        {
            x_var_not_in_buffer_ = true;
            return false;
        }
        else
            return true;

        break;
    }
    case PropertyDataType::DOUBLE:
    {
        if (!buffer->has<double>(current_var_name))
        {
            x_var_not_in_buffer_ = true;
            return false;
        }
        else
            return true;

        break;
    }
    case PropertyDataType::TIMESTAMP:
    {
        if (!buffer->has<boost::posix_time::ptime>(current_var_name))
        {
            x_var_not_in_buffer_ = true;
            return false;
        }
        else
            return true;

        break;
    }
    case PropertyDataType::STRING:
    case PropertyDataType::JSON:
    {
        return false;

        break;
    }
    default:
        logerr << "ScatterPlotViewDataWidget: canUpdateFromDataX: impossible for property type "
               << Property::asString(data_type);
        throw std::runtime_error(
                    "ScatterPlotViewDataWidget: canUpdateFromDataX: impossible property type " +
                    Property::asString(data_type));
    }
}

void ScatterPlotViewDataWidget::updateFromDataX(std::string dbcontent_name, unsigned int current_size)
{
    logdbg << "ScatterPlotViewDataWidget: updateFromDataX: dbcontent_name " << dbcontent_name << " current_size " << current_size;

    assert (buffers_.count(dbcontent_name));
    Buffer* buffer = buffers_.at(dbcontent_name).get();


    unsigned int last_size = 0;

    if (buffer_x_counts_.count(dbcontent_name))
        last_size = buffer_x_counts_.at(dbcontent_name);
    Variable* data_var {nullptr};

    if (!view_->hasDataVarX())
    {
        logwrn << "ScatterPlotViewDataWidget: updateFromDataX: no data var";
        return;
    }

    if (view_->isDataVarXMeta())
    {
        MetaVariable& meta_var = view_->metaDataVarX();
        if (!meta_var.existsIn(dbcontent_name))
        {
            logwrn << "ScatterPlotViewDataWidget: updateFromDataX: meta var does not exist in dbo";
            return;
        }

        data_var = &meta_var.getFor(dbcontent_name);
    }
    else
    {
        data_var = &view_->dataVarX();

        if (data_var->dbContentName() != dbcontent_name)
            return;
    }
    assert (data_var);

    PropertyDataType data_type = data_var->dataType();
    string current_var_name = data_var->name();

    logdbg << "ScatterPlotViewDataWidget: updateFromDataX: updating, last size " << last_size;

    switch (data_type)
    {
    case PropertyDataType::BOOL:
    {
        if (!buffer->has<bool>(current_var_name))
        {
            logdbg << "ScatterPlotViewDataWidget: updateFromDataX: buffer does not contain " << current_var_name;
            x_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<bool>(current_var_name));
        NullableVector<bool>& data = buffer->get<bool>(current_var_name);

        appendData(data, x_values_[dbcontent_name], last_size, current_size);
        buffer_x_counts_[dbcontent_name] = current_size;

        break;
    }
    case PropertyDataType::CHAR:
    {
        if (!buffer->has<char>(current_var_name))
        {
            logdbg << "ScatterPlotViewDataWidget: updateFromDataX: buffer does not contain " << current_var_name;
            x_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<char>(current_var_name));
        NullableVector<char>& data = buffer->get<char>(current_var_name);

        appendData(data, x_values_[dbcontent_name], last_size, current_size);
        buffer_x_counts_[dbcontent_name] = current_size;

        break;
    }
    case PropertyDataType::UCHAR:
    {
        if (!buffer->has<unsigned char>(current_var_name))
        {
            logdbg << "ScatterPlotViewDataWidget: updateFromDataX: buffer does not contain " << current_var_name;
            x_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<unsigned char>(current_var_name));
        NullableVector<unsigned char>& data = buffer->get<unsigned char>(current_var_name);

        appendData(data, x_values_[dbcontent_name], last_size, current_size);
        buffer_x_counts_[dbcontent_name] = current_size;

        break;
    }
    case PropertyDataType::INT:
    {
        if (!buffer->has<int>(current_var_name))
        {
            logdbg << "ScatterPlotViewDataWidget: updateFromDataX: buffer does not contain " << current_var_name;
            x_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<int>(current_var_name));
        NullableVector<int>& data = buffer->get<int>(current_var_name);

        appendData(data, x_values_[dbcontent_name], last_size, current_size);
        buffer_x_counts_[dbcontent_name] = current_size;

        break;
    }
    case PropertyDataType::UINT:
    {
        if (!buffer->has<unsigned int>(current_var_name))
        {
            logdbg << "ScatterPlotViewDataWidget: updateFromDataX: buffer does not contain " << current_var_name;
            x_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<unsigned int>(current_var_name));
        NullableVector<unsigned int>& data = buffer->get<unsigned int>(current_var_name);

        appendData(data, x_values_[dbcontent_name], last_size, current_size);
        buffer_x_counts_[dbcontent_name] = current_size;

        break;
    }
    case PropertyDataType::LONGINT:
    {
        if (!buffer->has<long int>(current_var_name))
        {
            logdbg << "ScatterPlotViewDataWidget: updateFromDataX: buffer does not contain " << current_var_name;
            x_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<long int>(current_var_name));
        NullableVector<long int>& data = buffer->get<long int>(current_var_name);

        appendData(data, x_values_[dbcontent_name], last_size, current_size);
        buffer_x_counts_[dbcontent_name] = current_size;

        break;
    }
    case PropertyDataType::ULONGINT:
    {
        if (!buffer->has<unsigned long>(current_var_name))
        {
            logdbg << "ScatterPlotViewDataWidget: updateFromDataX: buffer does not contain " << current_var_name;
            x_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<unsigned long>(current_var_name));
        NullableVector<unsigned long>& data = buffer->get<unsigned long>(current_var_name);

        appendData(data, x_values_[dbcontent_name], last_size, current_size);
        buffer_x_counts_[dbcontent_name] = current_size;

        break;
    }
    case PropertyDataType::FLOAT:
    {
        if (!buffer->has<float>(current_var_name))
        {
            logdbg << "ScatterPlotViewDataWidget: updateFromDataX: buffer does not contain " << current_var_name;
            x_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<float>(current_var_name));
        NullableVector<float>& data = buffer->get<float>(current_var_name);

        appendData(data, x_values_[dbcontent_name], last_size, current_size);
        buffer_x_counts_[dbcontent_name] = current_size;

        break;
    }
    case PropertyDataType::DOUBLE:
    {
        if (!buffer->has<double>(current_var_name))
        {
            logdbg << "ScatterPlotViewDataWidget: updateFromDataX: buffer does not contain " << current_var_name;
            x_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<double>(current_var_name));
        NullableVector<double>& data = buffer->get<double>(current_var_name);

        appendData(data, x_values_[dbcontent_name], last_size, current_size);
        buffer_x_counts_[dbcontent_name] = current_size;

        break;
    }
    case PropertyDataType::STRING:
    {
        if (!buffer->has<string>(current_var_name))
        {
            logdbg << "ScatterPlotViewDataWidget: updateFromDataX: buffer does not contain " << current_var_name;
            x_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<string>(current_var_name));
        //NullableVector<string>& data = buffer->get<string>(current_var_name);

        break;
    }
    case PropertyDataType::JSON:
    {
        if (!buffer->has<nlohmann::json>(current_var_name))
        {
            logdbg << "ScatterPlotViewDataWidget: updateFromDataX: buffer does not contain " << current_var_name;
            x_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<nlohmann::json>(current_var_name));
        //NullableVector<string>& data = buffer->get<string>(current_var_name);

        break;
    }
    case PropertyDataType::TIMESTAMP:
    {
        if (!buffer->has<boost::posix_time::ptime>(current_var_name))
        {
            logdbg << "ScatterPlotViewDataWidget: updateFromDataX: buffer does not contain " << current_var_name;
            x_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<boost::posix_time::ptime>(current_var_name));
        NullableVector<boost::posix_time::ptime>& data = buffer->get<boost::posix_time::ptime>(current_var_name);

        appendData(data, x_values_[dbcontent_name], last_size, current_size);
        buffer_x_counts_[dbcontent_name] = current_size;

        break;
    }
    default:
        logerr << "ScatterPlotViewDataWidget: updateFromDataX: impossible for property type "
               << Property::asString(data_type);
        throw std::runtime_error(
                    "ScatterPlotViewDataWidget: updateFromDataX: impossible property type " +
                    Property::asString(data_type));
    }

    logdbg << "ScatterPlotViewDataWidget: updateFromDataX: updated size " << buffer_x_counts_.at(dbcontent_name);
}

bool ScatterPlotViewDataWidget::canUpdateFromDataY(std::string dbcontent_name)
{
    if (!buffers_.count(dbcontent_name))
        return false;

    Buffer* buffer = buffers_.at(dbcontent_name).get();

    Variable* data_var {nullptr};

    if (!view_->hasDataVarY())
        return false;

    if (view_->isDataVarYMeta())
    {
        MetaVariable& meta_var = view_->metaDataVarY();
        if (!meta_var.existsIn(dbcontent_name))
            return false;

        data_var = &meta_var.getFor(dbcontent_name);
    }
    else
    {
        data_var = &view_->dataVarY();

        if (data_var->dbContentName() != dbcontent_name)
            return false;
    }
    assert (data_var);
    PropertyDataType data_type = data_var->dataType();
    string current_var_name = data_var->name();

    logdbg << "ScatterPlotViewDataWidget: canUpdateFromDataY: dbo " << dbcontent_name << " var "  << current_var_name;

    switch (data_type)
    {
    case PropertyDataType::BOOL:
    {
        if (!buffer->has<bool>(current_var_name))
        {
            y_var_not_in_buffer_ = true;
            return false;
        }
        else
            return true;

        break;
    }
    case PropertyDataType::CHAR:
    {
        if (!buffer->has<char>(current_var_name))
        {
            y_var_not_in_buffer_ = true;
            return false;
        }
        else
            return true;

        break;
    }
    case PropertyDataType::UCHAR:
    {
        if (!buffer->has<unsigned char>(current_var_name))
        {
            y_var_not_in_buffer_ = true;
            return false;
        }
        else
            return true;

        break;
    }
    case PropertyDataType::INT:
    {
        if (!buffer->has<int>(current_var_name))
        {
            y_var_not_in_buffer_ = true;
            return false;
        }
        else
            return true;

        break;
    }
    case PropertyDataType::UINT:
    {
        if (!buffer->has<unsigned int>(current_var_name))
        {
            y_var_not_in_buffer_ = true;
            return false;
        }
        else
            return true;

        break;
    }
    case PropertyDataType::LONGINT:
    {
        if (!buffer->has<long int>(current_var_name))
        {
            y_var_not_in_buffer_ = true;
            return false;
        }
        else
            return true;

        break;
    }
    case PropertyDataType::ULONGINT:
    {
        if (!buffer->has<unsigned long>(current_var_name))
        {
            y_var_not_in_buffer_ = true;
            return false;
        }
        else
            return true;

        break;
    }
    case PropertyDataType::FLOAT:
    {
        if (!buffer->has<float>(current_var_name))
        {
            y_var_not_in_buffer_ = true;
            return false;
        }
        else
            return true;

        break;
    }
    case PropertyDataType::DOUBLE:
    {
        if (!buffer->has<double>(current_var_name))
        {
            y_var_not_in_buffer_ = true;
            return false;
        }
        else
            return true;

        break;
    }
    case PropertyDataType::TIMESTAMP:
    {
        if (!buffer->has<boost::posix_time::ptime>(current_var_name))
        {
            y_var_not_in_buffer_ = true;
            return false;
        }
        else
            return true;

        break;
    }
    case PropertyDataType::STRING:
    case PropertyDataType::JSON:
    {
        return false;

        break;
    }
    default:
        logerr << "ScatterPlotViewDataWidget: canUpdateFromDataY: impossible for property type "
               << Property::asString(data_type);
        throw std::runtime_error(
                    "ScatterPlotViewDataWidget: canUpdateFromDataY: impossible property type " +
                    Property::asString(data_type));
    }
}

void ScatterPlotViewDataWidget::updateFromDataY(std::string dbcontent_name, unsigned int current_size)
{
    logdbg << "ScatterPlotViewDataWidget: updateFromDataY: dbcontent_name " << dbcontent_name << " current_size " << current_size;

    assert (buffers_.count(dbcontent_name));
    Buffer* buffer = buffers_.at(dbcontent_name).get();

    unsigned int last_size = 0;

    if (buffer_y_counts_.count(dbcontent_name))
        last_size = buffer_y_counts_.at(dbcontent_name);

    Variable* data_var {nullptr};

    if (!view_->hasDataVarY())
    {
        logwrn << "ScatterPlotViewDataWidget: updateFromDataY: no data var";
        return;
    }

    if (view_->isDataVarYMeta())
    {
        MetaVariable& meta_var = view_->metaDataVarY();
        if (!meta_var.existsIn(dbcontent_name))
        {
            logwrn << "ScatterPlotViewDataWidget: updateFromDataY: meta var does not eyist in dbo";
            return;
        }

        data_var = &meta_var.getFor(dbcontent_name);
    }
    else
    {
        data_var = &view_->dataVarY();

        if (data_var->dbContentName() != dbcontent_name)
            return;
    }
    assert (data_var);

    PropertyDataType data_type = data_var->dataType();
    string current_var_name = data_var->name();

    logdbg << "ScatterPlotViewDataWidget: updateFromDataY: updating, last size " << last_size;

    switch (data_type)
    {
    case PropertyDataType::BOOL:
    {
        if (!buffer->has<bool>(current_var_name))
        {
            logdbg << "ScatterPlotViewDataWidget: updateFromDataY: buffer does not contain " << current_var_name;
            y_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<bool>(current_var_name));
        NullableVector<bool>& data = buffer->get<bool>(current_var_name);

        appendData(data, y_values_[dbcontent_name], last_size, current_size);
        buffer_y_counts_[dbcontent_name] = current_size;

        break;
    }
    case PropertyDataType::CHAR:
    {
        if (!buffer->has<char>(current_var_name))
        {
            logdbg << "ScatterPlotViewDataWidget: updateFromDataY: buffer does not contain " << current_var_name;
            y_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<char>(current_var_name));
        NullableVector<char>& data = buffer->get<char>(current_var_name);

        appendData(data, y_values_[dbcontent_name], last_size, current_size);
        buffer_y_counts_[dbcontent_name] = current_size;

        break;
    }
    case PropertyDataType::UCHAR:
    {
        if (!buffer->has<unsigned char>(current_var_name))
        {
            logdbg << "ScatterPlotViewDataWidget: updateFromDataY: buffer does not contain " << current_var_name;
            y_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<unsigned char>(current_var_name));
        NullableVector<unsigned char>& data = buffer->get<unsigned char>(current_var_name);

        appendData(data, y_values_[dbcontent_name], last_size, current_size);
        buffer_y_counts_[dbcontent_name] = current_size;

        break;
    }
    case PropertyDataType::INT:
    {
        if (!buffer->has<int>(current_var_name))
        {
            logdbg << "ScatterPlotViewDataWidget: updateFromDataY: buffer does not contain " << current_var_name;
            y_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<int>(current_var_name));
        NullableVector<int>& data = buffer->get<int>(current_var_name);

        appendData(data, y_values_[dbcontent_name], last_size, current_size);
        buffer_y_counts_[dbcontent_name] = current_size;

        break;
    }
    case PropertyDataType::UINT:
    {
        if (!buffer->has<unsigned int>(current_var_name))
        {
            logdbg << "ScatterPlotViewDataWidget: updateFromDataY: buffer does not contain " << current_var_name;
            y_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<unsigned int>(current_var_name));
        NullableVector<unsigned int>& data = buffer->get<unsigned int>(current_var_name);

        appendData(data, y_values_[dbcontent_name], last_size, current_size);
        buffer_y_counts_[dbcontent_name] = current_size;

        break;
    }
    case PropertyDataType::LONGINT:
    {
        if (!buffer->has<long int>(current_var_name))
        {
            logdbg << "ScatterPlotViewDataWidget: updateFromDataY: buffer does not contain " << current_var_name;
            y_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<long int>(current_var_name));
        NullableVector<long int>& data = buffer->get<long int>(current_var_name);

        appendData(data, y_values_[dbcontent_name], last_size, current_size);
        buffer_y_counts_[dbcontent_name] = current_size;

        break;
    }
    case PropertyDataType::ULONGINT:
    {
        if (!buffer->has<unsigned long>(current_var_name))
        {
            loginf << "ScatterPlotViewDataWidget: updateFromDataY: buffer does not contain " << current_var_name;
            y_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<unsigned long>(current_var_name));
        NullableVector<unsigned long>& data = buffer->get<unsigned long>(current_var_name);

        appendData(data, y_values_[dbcontent_name], last_size, current_size);
        buffer_y_counts_[dbcontent_name] = current_size;

        break;
    }
    case PropertyDataType::FLOAT:
    {
        if (!buffer->has<float>(current_var_name))
        {
            logdbg << "ScatterPlotViewDataWidget: updateFromDataY: buffer does not contain " << current_var_name;
            y_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<float>(current_var_name));
        NullableVector<float>& data = buffer->get<float>(current_var_name);

        appendData(data, y_values_[dbcontent_name], last_size, current_size);
        buffer_y_counts_[dbcontent_name] = current_size;

        break;
    }
    case PropertyDataType::DOUBLE:
    {
        if (!buffer->has<double>(current_var_name))
        {
            logdbg << "ScatterPlotViewDataWidget: updateFromDataY: buffer does not contain " << current_var_name;
            y_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<double>(current_var_name));
        NullableVector<double>& data = buffer->get<double>(current_var_name);

        appendData(data, y_values_[dbcontent_name], last_size, current_size);
        buffer_y_counts_[dbcontent_name] = current_size;

        break;
    }
    case PropertyDataType::STRING:
    {
        if (!buffer->has<string>(current_var_name))
        {
            logdbg << "ScatterPlotViewDataWidget: updateFromDataY: buffer does not contain " << current_var_name;
            y_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<string>(current_var_name));
        //NullableVector<string>& data = buffer->get<string>(current_var_name);

        break;
    }
    case PropertyDataType::JSON:
    {
        if (!buffer->has<nlohmann::json>(current_var_name))
        {
            logdbg << "ScatterPlotViewDataWidget: updateFromDataY: buffer does not contain " << current_var_name;
            y_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<nlohmann::json>(current_var_name));
        //NullableVector<string>& data = buffer->get<string>(current_var_name);

        break;
    }
    case PropertyDataType::TIMESTAMP:
    {
        if (!buffer->has<boost::posix_time::ptime>(current_var_name))
        {
            logdbg << "ScatterPlotViewDataWidget: updateFromDataY: buffer does not contain " << current_var_name;
            y_var_not_in_buffer_ = true;
            return;
        }

        assert(buffer->has<boost::posix_time::ptime>(current_var_name));

        NullableVector<boost::posix_time::ptime>& data = buffer->get<boost::posix_time::ptime>(current_var_name);

        appendData(data, y_values_[dbcontent_name], last_size, current_size);
        buffer_y_counts_[dbcontent_name] = current_size;

        break;
    }
    default:
        logerr << "ScatterPlotViewDataWidget: updateFromDataY: impossible for property type "
               << Property::asString(data_type);
        throw std::runtime_error(
                    "ScatterPlotViewDataWidget: updateFromDataY: impossible property type " +
                    Property::asString(data_type));
    }

    logdbg << "ScatterPlotViewDataWidget: updateFromDataY: updated size " << buffer_y_counts_.at(dbcontent_name);

}

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

    logdbg << "ScatterPlotViewDataWidget: loadingDoneSlot: has_x_min_max " << has_x_min_max_
           << " x_min " << x_min_ << " x_max " << x_max_
           << " has_y_min_max " << has_y_min_max_ << " y_min " << y_min_ << " y_max " << y_max_;
}

void ScatterPlotViewDataWidget::updateFromAllData()
{
    logdbg << "ScatterPlotViewDataWidget: updateFromAllData";

    logdbg << "ScatterPlotViewDataWidget: updateFromAllData: before x " << x_values_.size()
           << " y " << y_values_.size();

    for (auto& buf_it : buffers_)
    {
        assert (x_values_[buf_it.first].size() == y_values_[buf_it.first].size());
        assert (x_values_[buf_it.first].size() == selected_values_[buf_it.first].size());
        assert (x_values_[buf_it.first].size() == rec_num_values_[buf_it.first].size());

        unsigned int current_size = buf_it.second->size();

        if (canUpdateFromDataX(buf_it.first) && canUpdateFromDataY(buf_it.first))
        {
            assert (buf_it.second->has<bool>(DBContent::selected_var.name()));
            assert (buf_it.second->has<unsigned int>(DBContent::meta_var_rec_num_.name()));

            NullableVector<bool>& selected_vec = buf_it.second->get<bool>(DBContent::selected_var.name());
            NullableVector<unsigned int>& rec_num_vec = buf_it.second->get<unsigned int>(
                        DBContent::meta_var_rec_num_.name());

            std::vector<bool>& selected_data = selected_values_[buf_it.first];
            std::vector<unsigned int>& rec_num_data = rec_num_values_[buf_it.first];

            unsigned int last_size = 0;

            if (buffer_x_counts_.count(buf_it.first))
                last_size = buffer_x_counts_.at(buf_it.first);

            for (unsigned int cnt=last_size; cnt < current_size; ++cnt)
            {
                if (selected_vec.isNull(cnt))
                    selected_data.push_back(false);
                else
                    selected_data.push_back(selected_vec.get(cnt));

                assert (!rec_num_vec.isNull(cnt));
                rec_num_data.push_back(rec_num_vec.get(cnt));
            }

            updateFromDataX(buf_it.first, current_size);
            updateFromDataY(buf_it.first, current_size);
        }
        else
            logdbg << "ScatterPlotViewDataWidget: updateFromAllData: " << buf_it.first
                   << " update not possible";

        assert (x_values_[buf_it.first].size() == y_values_[buf_it.first].size());
        assert (x_values_[buf_it.first].size() == selected_values_[buf_it.first].size());
        assert (x_values_[buf_it.first].size() == rec_num_values_[buf_it.first].size());
    }

    logdbg << "ScatterPlotViewDataWidget: updateFromAllData: after x " << x_values_.size()
           << " y " << y_values_.size();


    assert (x_values_.size() == y_values_.size());

    logdbg << "ScatterPlotViewDataWidget: updateFromAllData: done";
}

void ScatterPlotViewDataWidget::updateChart()
{
    logdbg << "ScatterPlotViewDataWidget: updateChart";

    assert (main_layout_);

    chart_view_.reset(nullptr);

    QChart* chart = new QChart();
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    chart->setBackgroundRoundness(0);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    if (!x_values_.size() || !y_values_.size())
    {
        loginf << "ScatterPlotViewDataWidget: updateChart: no data, size x "
               << x_values_.size() << " y " << y_values_.size();
        return;
    }

    unsigned int value_cnt {0};
    unsigned int dbo_value_cnt {0};
    nan_value_cnt_ = 0;
    unsigned int selected_cnt {0};

    const double MarkerSize = 8.0;

    //generate needed series and sort pointers
    //qtcharts when rendering opengl will sort the series into a map using the pointer of the series as a key
    std::vector<QScatterSeries*> series(x_values_.size() + 1);
    for (size_t i = 0; i < series.size(); ++i)
    {
        series[ i ] = new QScatterSeries;
        series[ i ]->setMarkerShape(QScatterSeries::MarkerShapeCircle);
        series[ i ]->setMarkerSize(MarkerSize);
        series[ i ]->setUseOpenGL(true);
    }
    std::sort(series.begin(), series.end()); //sort pointers to obtain correct render order

    //the biggest pointer is thus the one for the selection, so the selection will be rendered on top
    QScatterSeries* selected_series = series.back();
    selected_series->setColor(Qt::yellow);

    //distribute sorted pointers to dbcontent types
    std::map<std::string, QScatterSeries*> series_map;
    {
        int cnt = 0;
        for (auto it = x_values_.begin(); it != x_values_.end(); ++it)
            series_map[ it->first ] = series[ cnt++ ];
    }
    
    for (auto& data : x_values_)
    {
        dbo_value_cnt = 0;

        vector<double>& x_values = data.second;
        vector<double>& y_values = y_values_[data.first];
        vector<bool>& selected_values = selected_values_[data.first];
        //vector<unsigned int>& rec_num_values = rec_num_values_[data.first];

        QScatterSeries* chart_series = series_map[ data.first ];
        chart_series->setColor(colors_[data.first]);

        assert (x_values.size() == y_values.size());
        assert (x_values.size() == selected_values.size());

        unsigned int sum_cnt {0};

        for (unsigned int cnt=0; cnt < x_values.size(); ++cnt)
        {
            if (!std::isnan(x_values.at(cnt)) && !std::isnan(y_values.at(cnt)))
            {
                ++value_cnt;
                ++dbo_value_cnt;

                if (selected_values.at(cnt))
                {
                    selected_series->append(x_values.at(cnt), y_values.at(cnt));
                    ++selected_cnt;
                }
                else
                {
                    chart_series->append(x_values.at(cnt), y_values.at(cnt));
                    ++sum_cnt;
                }
            }
            else
                ++nan_value_cnt_;
        }

        if (!dbo_value_cnt)
            continue;

        if (sum_cnt)
        {
            logdbg << "ScatterPlotViewDataWidget: updateChart: adding " << data.first << " (" << sum_cnt << ")";

            chart_series->setName((data.first+" ("+to_string(sum_cnt)+")").c_str());
            chart->addSeries(chart_series);
        }
        else
        {
            delete chart_series;
        }
    }

    if (!value_cnt)
    {
        loginf << "ScatterPlotViewDataWidget: updateChart: no valid data";
        return;
    }

    if (selected_cnt > 0)
    {
        logdbg << "ScatterPlotViewDataWidget: updateChart: adding " << " Selected (" << selected_cnt << ")";

        //add series for selected values
        selected_series->setName(("Selected ("+to_string(selected_cnt)+")").c_str());
        chart->addSeries(selected_series);
    }
    else
    {
        //series for selection not needed
        delete selected_series;
        selected_series = nullptr;
    }

    chart->createDefaultAxes();
    loginf << "ScatterPlotViewDataWidget: updateChart: title x ' "
           << view_->dataVarXDBO()+": "+view_->dataVarXName() << "'";
    assert (chart->axes(Qt::Horizontal).size() == 1);
    chart->axes(Qt::Horizontal).at(0)->setTitleText((view_->dataVarXDBO()+": "+view_->dataVarXName()).c_str());
    loginf << "ScatterPlotViewDataWidget: updateChart: title y ' "
           << view_->dataVarYDBO()+": "+view_->dataVarYName() << "'";
    assert (chart->axes(Qt::Vertical).size() == 1);
    chart->axes(Qt::Vertical).at(0)->setTitleText((view_->dataVarYDBO()+": "+view_->dataVarYName()).c_str());
    chart->setDropShadowEnabled(false);

    //chart_view_ = new QChartView(chart_);
    chart_view_.reset(new ScatterPlotViewChartView(this, chart));
    chart_view_->setRenderHint(QPainter::Antialiasing);
    //chart_view_->setRubberBand(QChartView::RectangleRubberBand);
    //chart_view_->setDragMode(QGraphicsView::ScrollHandDrag);

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

    shows_data_ = true;
}

void ScatterPlotViewDataWidget::mouseMoveEvent(QMouseEvent* event)
{
    //setCursor(current_cursor_);
    //osgEarth::QtGui::ViewerWidget::mouseMoveEvent(event);

    QWidget::mouseMoveEvent(event);
}

void ScatterPlotViewDataWidget::selectData (double x_min, double x_max, double y_min, double y_max)
{
    bool ctrl_pressed = QApplication::keyboardModifiers() & Qt::ControlModifier;

    loginf << "ScatterPlotViewDataWidget: selectData: x_min " << x_min << " x_max " << x_max
           << " y_min " << y_min << " y_max " << y_max << " ctrl pressed " << ctrl_pressed;

    unsigned int sel_cnt = 0;
    for (auto& buf_it : buffers_)
    {
        assert (buf_it.second->has<bool>(DBContent::selected_var.name()));
        NullableVector<bool>& selected_vec = buf_it.second->get<bool>(DBContent::selected_var.name());

        assert (buf_it.second->has<unsigned int>(DBContent::meta_var_rec_num_.name()));
        NullableVector<unsigned int>& rec_num_vec = buf_it.second->get<unsigned int>(
                    DBContent::meta_var_rec_num_.name());

        std::map<unsigned int, std::vector<unsigned int>> rec_num_indexes =
                rec_num_vec.distinctValuesWithIndexes(0, rec_num_vec.size());
        // rec_num -> index

        std::vector<double>& x_values = x_values_.at(buf_it.first);
        std::vector<double>& y_values = y_values_.at(buf_it.first);
        std::vector<unsigned int>& rec_num_values = rec_num_values_.at(buf_it.first);

        assert (x_values.size() == y_values.size());
        assert (x_values.size() == rec_num_values.size());

        double x, y;
        bool in_range;
        unsigned int rec_num, index;

        for (unsigned int cnt=0; cnt < x_values.size(); ++cnt)
        {
            x = x_values.at(cnt);
            y = y_values.at(cnt);
            rec_num = rec_num_values.at(cnt);

            in_range = false;

            if (!std::isnan(x) && !std::isnan(y))
                in_range =  x >= x_min && x <= x_max && y >= y_min && y <= y_max;

            assert (rec_num_indexes.count(rec_num));
            std::vector<unsigned int>& indexes = rec_num_indexes.at((int)rec_num);
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
