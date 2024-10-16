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
//#include "compass.h"
#include "buffer.h"
//#include "dbcontent/dbcontentmanager.h"
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

const double ScatterPlotViewDataWidget::MarkerSize         = 8.0;
const double ScatterPlotViewDataWidget::MarkerSizeSelected = 6.0;

const std::string ScatterPlotViewDataWidget::Color_CAT001  = "#00FF00";
const std::string ScatterPlotViewDataWidget::Color_CAT010  = "#FFCC00";
const std::string ScatterPlotViewDataWidget::Color_CAT020  = "#FF0000";
const std::string ScatterPlotViewDataWidget::Color_CAT021  = "#6666FF";
const std::string ScatterPlotViewDataWidget::Color_CAT048  = "#00FF00";
const std::string ScatterPlotViewDataWidget::Color_RefTraj = "#FFA500";
const std::string ScatterPlotViewDataWidget::Color_CAT062  = "#CCCCCC";

ScatterPlotViewDataWidget::ScatterPlotViewDataWidget(ScatterPlotViewWidget* view_widget,
                                                     QWidget* parent,
                                                     Qt::WindowFlags f)
    :   ViewDataWidget(view_widget, parent, f)
{
    view_ = view_widget->getView();
    assert(view_);

    data_source_ = view_->getDataSource();
    assert(data_source_);

    main_layout_ = new QHBoxLayout();
    main_layout_->setMargin(0);

    setLayout(main_layout_);

    colors_["CAT001" ] = QColor(QString::fromStdString(Color_CAT001 ));
    colors_["CAT010" ] = QColor(QString::fromStdString(Color_CAT010 ));
    colors_["CAT020" ] = QColor(QString::fromStdString(Color_CAT020 ));
    colors_["CAT021" ] = QColor(QString::fromStdString(Color_CAT021 ));
    colors_["CAT048" ] = QColor(QString::fromStdString(Color_CAT048 ));
    colors_["RefTraj"] = QColor(QString::fromStdString(Color_RefTraj));
    colors_["CAT062" ] = QColor(QString::fromStdString(Color_CAT062 ));

    updateChart();
}

ScatterPlotViewDataWidget::~ScatterPlotViewDataWidget()
{
    logdbg << "ScatterPlotViewDataWidget: dtor";
}

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

    x_var_not_in_buffer_ = false;
    y_var_not_in_buffer_ = false;

    nan_value_cnt_ = 0;
    valid_cnt_     = 0;
    selected_cnt_  = 0;

    dbo_valid_counts_.clear();
}

void ScatterPlotViewDataWidget::clearData_impl()
{
    logdbg << "ScatterPlotViewDataWidget: clearData_impl: start";

    chart_view_.reset(nullptr);

    resetCounts();

    logdbg << "ScatterPlotViewDataWidget: clearData_impl: end";
}

void ScatterPlotViewDataWidget::loadingStarted_impl()
{
    logdbg << "ScatterPlotViewDataWidget: loadingStarted_impl: start";

    //nothing to do yet

    logdbg << "ScatterPlotViewDataWidget: loadingStarted_impl: end";
}

void ScatterPlotViewDataWidget::updateData_impl(bool requires_reset)
{
    logdbg << "ScatterPlotViewDataWidget: updateData_impl: start";

    //nothing to do yet

    logdbg << "ScatterPlotViewDataWidget: updateData_impl: end";
}

void ScatterPlotViewDataWidget::loadingDone_impl()
{
    logdbg << "ScatterPlotViewDataWidget: loadingDone_impl: start";

    if (view_->useConnectionLines() && loadedDataCount() > 100000) // disable connection lines
    {
        loginf << "ScatterPlotViewDataWidget: loadingDone_impl: loaded data >100k, disabling connection lines";

        view_->useConnectionLines(false);
        emit displayChanged();
    }

    //default behavior
    ViewDataWidget::loadingDone_impl();

    logdbg << "ScatterPlotViewDataWidget: loadingDone_impl: end";
}

void ScatterPlotViewDataWidget::liveReload_impl()
{
    //implement live reload behavior here
}

bool ScatterPlotViewDataWidget::redrawData_impl(bool recompute)
{
    logdbg << "ScatterPlotViewDataWidget: redrawData_impl: start - recompute = " << recompute;

    if (recompute)
    {
        resetCounts();
        updateFromAllData();
        updateMinMax();
    }
    bool drawn = updateChart();

    logdbg << "ScatterPlotViewDataWidget: redrawData_impl: end";

    return drawn;
}

void ScatterPlotViewDataWidget::toolChanged_impl(int mode)
{
    selected_tool_ = (ScatterPlotViewDataTool)mode;

    if (chart_view_)
        chart_view_->onToolChanged();
}

void ScatterPlotViewDataWidget::updateFromAllData()
{
    logdbg << "ScatterPlotViewDataWidget: updateFromAllData";
    logdbg << "ScatterPlotViewDataWidget: updateFromAllData: before x " << x_values_.size()
           << " y " << y_values_.size();

    for (const auto& buf_it : viewData())
    {
        string dbcontent_name = buf_it.first;
        std::shared_ptr<Buffer> buffer = buf_it.second;

        unsigned int current_size = buffer->size();

        logdbg << "ScatterPlotViewDataWidget: updateFromAllData: before x " << x_values_[dbcontent_name].size()
               << " y " << y_values_[dbcontent_name].size();

        assert (x_values_[dbcontent_name].size() == y_values_[dbcontent_name].size());
        assert (x_values_[dbcontent_name].size() == selected_values_[dbcontent_name].size());
        assert (x_values_[dbcontent_name].size() == rec_num_values_[dbcontent_name].size());

        loginf << "ScatterPlotViewDataWidget: updateFromAllData: dbo " << dbcontent_name
               << " canUpdateFromDataX " << canUpdateFromDataX(dbcontent_name)
               << " canUpdateFromDataY " << canUpdateFromDataY(dbcontent_name);

        if (canUpdateFromDataX(dbcontent_name) && canUpdateFromDataY(dbcontent_name))
        {
            loginf << "ScatterPlotViewDataWidget: updateFromAllData: updating data";

            // add selected flags & rec_nums
            assert (buffer->has<bool>(DBContent::selected_var.name()));
            assert (buffer->has<unsigned long>(DBContent::meta_var_rec_num_.name()));

            NullableVector<bool>& selected_vec = buffer->get<bool>(DBContent::selected_var.name());
            NullableVector<unsigned long>& rec_num_vec = buffer->get<unsigned long>(DBContent::meta_var_rec_num_.name());

            std::vector<bool>& selected_data = selected_values_[dbcontent_name];
            std::vector<unsigned long>& rec_num_data = rec_num_values_[dbcontent_name];

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
            logdbg << "ScatterPlotViewDataWidget: updateFromAllData: " << dbcontent_name
                   << " update not possible";


        logdbg << "ScatterPlotViewDataWidget: updateFromAllData: after x " << x_values_[dbcontent_name].size()
               << " y " << y_values_[dbcontent_name].size();

        assert (x_values_[dbcontent_name].size() == y_values_[dbcontent_name].size());
        assert (x_values_[dbcontent_name].size() == selected_values_[dbcontent_name].size());
        assert (x_values_[dbcontent_name].size() == rec_num_values_[dbcontent_name].size());
    }

    logdbg << "ScatterPlotViewDataWidget: updateFromAllData: after x " << x_values_.size()
           << " y " << y_values_.size();

    assert (x_values_.size() == y_values_.size());

    logdbg << "ScatterPlotViewDataWidget: updateFromAllData: done";
}

ScatterPlotViewDataTool ScatterPlotViewDataWidget::selectedTool() const
{
    return selected_tool_;
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

unsigned int ScatterPlotViewDataWidget::nullValueCount() const
{
    return nan_value_cnt_;
}

QRectF ScatterPlotViewDataWidget::getDataBounds() const
{
    if (!has_x_min_max_ || !has_y_min_max_)
        return QRectF();

    return QRectF(x_min_, y_min_, x_max_ - x_min_, y_max_ - y_min_);
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
    if (!viewData().count(dbcontent_name))
        return false;

    Buffer* buffer = viewData().at(dbcontent_name).get();

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

    assert (viewData().count(dbcontent_name));
    Buffer* buffer = viewData().at(dbcontent_name).get();

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
    if (!viewData().count(dbcontent_name))
        return false;

    Buffer* buffer = viewData().at(dbcontent_name).get();

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

    assert (viewData().count(dbcontent_name));
    Buffer* buffer = viewData().at(dbcontent_name).get();

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

    bool has_data = (x_values_.size() && y_values_.size() && !xVarNotInBuffer() && !yVarNotInBuffer());

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

void ScatterPlotViewDataWidget::updateDataSeries(QtCharts::QChart* chart)
{
    //we obtain valid data if a data range is available and if the variables are available in the buffer data
    bool has_data = (x_values_.size() && y_values_.size() && !xVarNotInBuffer() && !yVarNotInBuffer());

    auto createAxes = [ & ] ()
    {
        chart->createDefaultAxes();

        //config x axis
        loginf << "ScatterPlotViewDataWidget: updateDataSeries: title x ' "
               << view_->dataVarXDBO()+": "+view_->dataVarXName() << "'";
        assert (chart->axes(Qt::Horizontal).size() == 1);
        chart->axes(Qt::Horizontal).at(0)->setTitleText((view_->dataVarXDBO()+": "+view_->dataVarXName()).c_str());

        //config y axis
        loginf << "ScatterPlotViewDataWidget: updateDataSeries: title y ' "
               << view_->dataVarYDBO()+": "+view_->dataVarYName() << "'";
        assert (chart->axes(Qt::Vertical).size() == 1);
        chart->axes(Qt::Vertical).at(0)->setTitleText((view_->dataVarYDBO()+": "+view_->dataVarYName()).c_str());
    };

    if (has_data)
    {
        //data available

        chart->legend()->setVisible(true);

        nan_value_cnt_ = 0;
        valid_cnt_     = 0;
        selected_cnt_  = 0;

        dbo_valid_counts_.clear();

        bool use_connection_lines = view_->useConnectionLines();

        struct SymbolLineSeries { QScatterSeries* scatter_series;
                                  QLineSeries* line_series;};

        //generate needed series and sort pointers
        //qtcharts when rendering opengl will sort the series into a map using the pointer of the series as a key
        //we exploit this behavior to obtain the correct render order for our data

        //generate pointers
        std::vector<SymbolLineSeries> series(x_values_.size() + 1);
        for (size_t i = 0; i < series.size(); ++i)
        {
            series[ i ].scatter_series = new QScatterSeries;
            series[ i ].scatter_series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
            series[ i ].scatter_series->setMarkerSize(MarkerSize);
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

        //the biggest pointer is thus the one for the selection, so the selection will be rendered on top
        QScatterSeries* selected_symbol_series = series.back().scatter_series;
        selected_symbol_series->setMarkerSize(MarkerSizeSelected);
        selected_symbol_series->setColor(Qt::yellow);

        QLineSeries* selected_line_series = nullptr;

        if (use_connection_lines)
        {
            selected_line_series = series.back().line_series;
            selected_line_series->setColor(Qt::yellow);
        }

        //distribute sorted pointers to dbcontent types
        std::map<std::string, SymbolLineSeries> series_map;
        {
            int cnt = 0;
            for (auto it = x_values_.begin(); it != x_values_.end(); ++it)
                series_map[ it->first ] = series[ cnt++ ];
        }

        //now add data to series
        for (auto& data : x_values_)
        {
            auto& dbo_value_cnt = dbo_valid_counts_[ data.first ];
            dbo_value_cnt = 0;

            vector<double>& x_values        = data.second;
            vector<double>& y_values        = y_values_[data.first];
            vector<bool>&   selected_values = selected_values_[data.first];

            QScatterSeries* chart_symbol_series = series_map[ data.first ].scatter_series;
            chart_symbol_series->setColor(colors_[data.first]);

            QLineSeries* chart_line_series = nullptr;

            if (use_connection_lines)
            {
                chart_line_series = series_map[ data.first ].line_series;
                chart_line_series->setColor(colors_[data.first]);
            }

            assert (x_values.size() == y_values.size());
            assert (x_values.size() == selected_values.size());

            unsigned int sum_cnt {0};

            for (unsigned int cnt=0; cnt < x_values.size(); ++cnt)
            {
                if (!std::isnan(x_values.at(cnt)) && !std::isnan(y_values.at(cnt)))
                {
                    ++valid_cnt_;
                    ++dbo_value_cnt;

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

            if (!dbo_value_cnt)
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
                }
            }
            else
            {
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
            loginf << "ScatterPlotViewDataWidget: updateDataSeries: no valid data";
        }
    }
    else
    {
        //bad data range or vars not in buffer
        loginf << "ScatterPlotViewDataWidget: updateDataSeries: no data, size x "
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
        const auto& valid_count = dbo_valid_counts_.at(it.first);

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
