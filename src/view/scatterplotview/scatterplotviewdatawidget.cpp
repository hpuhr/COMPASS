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
#include "dbobjectmanager.h"
#include "dbobject.h"
#include "dbovariable.h"
#include "metadbovariable.h"
#include "scatterplotviewdatasource.h"
#include "logger.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QTabWidget>

#include <QtCharts/QChartView>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLegend>
#include <QtCharts/QValueAxis>
#include <QGraphicsLayout>

#include <algorithm>

QT_CHARTS_USE_NAMESPACE

using namespace std;

ScatterPlotViewDataWidget::ScatterPlotViewDataWidget(ScatterPlotView* view, ScatterPlotViewDataSource* data_source,
                                                     QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f), view_(view), data_source_(data_source)
{
    assert(data_source_);
    setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* layout = new QHBoxLayout();

//    chart_series_ = new QScatterSeries();
//    //chart_series_->setName("scatter1");
//    chart_series_->setMarkerShape(QScatterSeries::MarkerShapeCircle);
//    chart_series_->setMarkerSize(8.0);
//    chart_series_->setUseOpenGL(true);

    chart_ = new QChart();
    chart_->layout()->setContentsMargins(0, 0, 0, 0);
    chart_->setBackgroundRoundness(0);

    chart_->legend()->setVisible(true);
    chart_->legend()->setAlignment(Qt::AlignBottom);

    chart_view_ = new QChartView(chart_);
    chart_view_->setRenderHint(QPainter::Antialiasing);
    chart_view_->setRubberBand(QChartView::RectangleRubberBand);

    layout->addWidget(chart_view_);

    setLayout(layout);

//    if (identifier_value == "Radar")
//        color = QColor("#00FF00");
//    else if (identifier_value == "MLAT")
//        color = QColor("#FF0000");
//    else if (identifier_value == "ADSB")
//        color = QColor("#6666FF");
//    else if (identifier_value == "RefTraj")
//        color = QColor("#FFA500");
//    else if (identifier_value == "Tracker")
//        color = QColor("#DDDDDD");

    colors_["Radar"] = QColor("#00FF00");
    colors_["MLAT"] = QColor("#FF0000");
    colors_["ADSB"] = QColor("#6666FF");
    colors_["RefTraj"] = QColor("#FFA500");
    colors_["Tracker"] = QColor("#DDDDDD");
}

ScatterPlotViewDataWidget::~ScatterPlotViewDataWidget()
{
    loginf << "ScatterPlotViewDataWidget: dtor";

    delete chart_view_;
}

void ScatterPlotViewDataWidget::update()
{
    loginf << "ScatterPlotViewDataWidget: update";

    buffer_x_counts_.clear();
    buffer_y_counts_.clear();

    x_values_.clear();
    y_values_.clear();

    updateFromAllData();
    updateChart();
}

void ScatterPlotViewDataWidget::clear ()
{
    buffers_.clear();
    buffer_x_counts_.clear();
    buffer_y_counts_.clear();

    x_values_.clear();
    y_values_.clear();
}

void ScatterPlotViewDataWidget::loadingStartedSlot()
{
    clear();
    chart_->removeAllSeries();
}

void ScatterPlotViewDataWidget::updateDataSlot(DBObject& object, std::shared_ptr<Buffer> buffer)
{
    logdbg << "ScatterPlotViewDataWidget: updateDataSlot: start";

    if (!buffer->size())
        return;

    string dbo_name = object.name();
    buffers_[dbo_name] = buffer;

    unsigned int current_size = buffer->size();

    logdbg << "ScatterPlotViewDataWidget: updateDataSlot: before x " << x_values_[dbo_name].size()
           << " y " << y_values_[dbo_name].size();

    assert (x_values_[dbo_name].size() == y_values_[dbo_name].size());

    if (canUpdateFromDataX(dbo_name) && canUpdateFromDataY(dbo_name))
    {
        updateFromDataX(dbo_name, current_size);
        updateFromDataY(dbo_name, current_size);
    }
    else
        loginf << "ScatterPlotViewDataWidget: updateDataSlot: " << dbo_name
               << " update not possible";

    loginf << "ScatterPlotViewDataWidget: updateDataSlot: after x " << x_values_[dbo_name].size()
           << " y " << y_values_[dbo_name].size();

    assert (x_values_[dbo_name].size() == y_values_[dbo_name].size());

    updateChart();

    logdbg << "ScatterPlotViewDataWidget: updateDataSlot: end";
}

bool ScatterPlotViewDataWidget::canUpdateFromDataX(std::string dbo_name)
{
    if (!buffers_.count(dbo_name))
        return false;

    Buffer* buffer = buffers_.at(dbo_name).get();

    DBOVariable* data_var {nullptr};

    if (!view_->hasDataVarX())
        return false;

    if (view_->isDataVarXMeta())
    {
        MetaDBOVariable& meta_var = view_->metaDataVarX();
        if (!meta_var.existsIn(dbo_name))
            return false;

        data_var = &meta_var.getFor(dbo_name);
    }
    else
    {
        data_var = &view_->dataVarX();

        if (data_var->dboName() != dbo_name)
            return false;
    }
    assert (data_var);

    PropertyDataType data_type = data_var->dataType();
    string current_var_name = data_var->name();

    switch (data_type)
    {
    case PropertyDataType::BOOL:
    {
        return buffer->has<bool>(current_var_name);

        break;
    }
    case PropertyDataType::CHAR:
    {
        return buffer->has<char>(current_var_name);

        break;
    }
    case PropertyDataType::UCHAR:
    {
        return buffer->has<unsigned char>(current_var_name);

        break;
    }
    case PropertyDataType::INT:
    {
        return buffer->has<int>(current_var_name);

        break;
    }
    case PropertyDataType::UINT:
    {
        return buffer->has<unsigned int>(current_var_name);

        break;
    }
    case PropertyDataType::LONGINT:
    {
        return buffer->has<long int>(current_var_name);

        break;
    }
    case PropertyDataType::ULONGINT:
    {
        return buffer->has<unsigned long>(current_var_name);

        break;
    }
    case PropertyDataType::FLOAT:
    {
        return buffer->has<float>(current_var_name);

        break;
    }
    case PropertyDataType::DOUBLE:
    {
        return buffer->has<double>(current_var_name);

        break;
    }
    case PropertyDataType::STRING:
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

void ScatterPlotViewDataWidget::updateFromDataX(std::string dbo_name, unsigned int current_size)
{
    loginf << "ScatterPlotViewDataWidget: updateFromDataX: dbo_name " << dbo_name << " current_size " << current_size;

    assert (buffers_.count(dbo_name));
    Buffer* buffer = buffers_.at(dbo_name).get();

    unsigned int last_size = 0;

    if (buffer_x_counts_.count(dbo_name))
        last_size = buffer_x_counts_.at(dbo_name);

    DBOVariable* data_var {nullptr};

    if (!view_->hasDataVarX())
    {
        logwrn << "ScatterPlotViewDataWidget: updateFromDataX: no data var";
        return;
    }

    if (view_->isDataVarXMeta())
    {
        MetaDBOVariable& meta_var = view_->metaDataVarX();
        if (!meta_var.existsIn(dbo_name))
        {
            logwrn << "ScatterPlotViewDataWidget: updateFromDataX: meta var does not exist in dbo";
            return;
        }

        data_var = &meta_var.getFor(dbo_name);
    }
    else
    {
        data_var = &view_->dataVarX();

        if (data_var->dboName() != dbo_name)
            return;
    }
    assert (data_var);

    PropertyDataType data_type = data_var->dataType();
    string current_var_name = data_var->name();

    switch (data_type)
    {
    case PropertyDataType::BOOL:
    {
        if (!buffer->has<bool>(current_var_name))
        {
            loginf << "ScatterPlotViewDataWidget: updateFromDataX: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<bool>(current_var_name));
        NullableVector<bool>& data = buffer->get<bool>(current_var_name);

        appendData(data, x_values_[dbo_name], last_size, current_size);
        buffer_x_counts_[dbo_name] = current_size;

        break;
    }
    case PropertyDataType::CHAR:
    {
        if (!buffer->has<char>(current_var_name))
        {
            loginf << "ScatterPlotViewDataWidget: updateFromDataX: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<char>(current_var_name));
        NullableVector<char>& data = buffer->get<char>(current_var_name);

        appendData(data, x_values_[dbo_name], last_size, current_size);
        buffer_x_counts_[dbo_name] = current_size;

        break;
    }
    case PropertyDataType::UCHAR:
    {
        if (!buffer->has<unsigned char>(current_var_name))
        {
            loginf << "ScatterPlotViewDataWidget: updateFromDataX: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<unsigned char>(current_var_name));
        NullableVector<unsigned char>& data = buffer->get<unsigned char>(current_var_name);

        appendData(data, x_values_[dbo_name], last_size, current_size);
        buffer_x_counts_[dbo_name] = current_size;

        break;
    }
    case PropertyDataType::INT:
    {
        if (!buffer->has<int>(current_var_name))
        {
            loginf << "ScatterPlotViewDataWidget: updateFromDataX: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<int>(current_var_name));
        NullableVector<int>& data = buffer->get<int>(current_var_name);

        appendData(data, x_values_[dbo_name], last_size, current_size);
        buffer_x_counts_[dbo_name] = current_size;

        break;
    }
    case PropertyDataType::UINT:
    {
        if (!buffer->has<unsigned int>(current_var_name))
        {
            loginf << "ScatterPlotViewDataWidget: updateFromDataX: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<unsigned int>(current_var_name));
        NullableVector<unsigned int>& data = buffer->get<unsigned int>(current_var_name);

        appendData(data, x_values_[dbo_name], last_size, current_size);
        buffer_x_counts_[dbo_name] = current_size;

        break;
    }
    case PropertyDataType::LONGINT:
    {
        if (!buffer->has<long int>(current_var_name))
        {
            loginf << "ScatterPlotViewDataWidget: updateFromDataX: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<long int>(current_var_name));
        NullableVector<long int>& data = buffer->get<long int>(current_var_name);

        appendData(data, x_values_[dbo_name], last_size, current_size);
        buffer_x_counts_[dbo_name] = current_size;

        break;
    }
    case PropertyDataType::ULONGINT:
    {
        if (!buffer->has<unsigned long>(current_var_name))
        {
            loginf << "ScatterPlotViewDataWidget: updateFromDataX: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<unsigned long>(current_var_name));
        NullableVector<unsigned long>& data = buffer->get<unsigned long>(current_var_name);

        appendData(data, x_values_[dbo_name], last_size, current_size);
        buffer_x_counts_[dbo_name] = current_size;

        break;
    }
    case PropertyDataType::FLOAT:
    {
        if (!buffer->has<float>(current_var_name))
        {
            loginf << "ScatterPlotViewDataWidget: updateFromDataX: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<float>(current_var_name));
        NullableVector<float>& data = buffer->get<float>(current_var_name);

        appendData(data, x_values_[dbo_name], last_size, current_size);
        buffer_x_counts_[dbo_name] = current_size;

        break;
    }
    case PropertyDataType::DOUBLE:
    {
        if (!buffer->has<double>(current_var_name))
        {
            loginf << "ScatterPlotViewDataWidget: updateFromDataX: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<double>(current_var_name));
        NullableVector<double>& data = buffer->get<double>(current_var_name);

        appendData(data, x_values_[dbo_name], last_size, current_size);
        buffer_x_counts_[dbo_name] = current_size;

        break;
    }
    case PropertyDataType::STRING:
    {
        if (!buffer->has<string>(current_var_name))
        {
            loginf << "ScatterPlotViewDataWidget: updateFromDataX: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<string>(current_var_name));
        //NullableVector<string>& data = buffer->get<string>(current_var_name);

        break;
    }
    default:
        logerr << "ScatterPlotViewDataWidget: updateFromDataX: impossible for property type "
               << Property::asString(data_type);
        throw std::runtime_error(
                    "ScatterPlotViewDataWidget: updateFromDataX: impossible property type " +
                    Property::asString(data_type));
    }

}

bool ScatterPlotViewDataWidget::canUpdateFromDataY(std::string dbo_name)
{
    if (!buffers_.count(dbo_name))
        return false;

    Buffer* buffer = buffers_.at(dbo_name).get();

    DBOVariable* data_var {nullptr};

    if (!view_->hasDataVarY())
        return false;

    if (view_->isDataVarYMeta())
    {
        MetaDBOVariable& meta_var = view_->metaDataVarY();
        if (!meta_var.existsIn(dbo_name))
            return false;

        data_var = &meta_var.getFor(dbo_name);
    }
    else
    {
        data_var = &view_->dataVarY();

        if (data_var->dboName() != dbo_name)
            return false;
    }
    assert (data_var);
    PropertyDataType data_type = data_var->dataType();
    string current_var_name = data_var->name();

    switch (data_type)
    {
    case PropertyDataType::BOOL:
    {
        return buffer->has<bool>(current_var_name);

        break;
    }
    case PropertyDataType::CHAR:
    {
        return buffer->has<char>(current_var_name);

        break;
    }
    case PropertyDataType::UCHAR:
    {
        return buffer->has<unsigned char>(current_var_name);

        break;
    }
    case PropertyDataType::INT:
    {
        return buffer->has<int>(current_var_name);

        break;
    }
    case PropertyDataType::UINT:
    {
        return buffer->has<unsigned int>(current_var_name);

        break;
    }
    case PropertyDataType::LONGINT:
    {
        return buffer->has<long int>(current_var_name);

        break;
    }
    case PropertyDataType::ULONGINT:
    {
        return buffer->has<unsigned long>(current_var_name);

        break;
    }
    case PropertyDataType::FLOAT:
    {
        return buffer->has<float>(current_var_name);

        break;
    }
    case PropertyDataType::DOUBLE:
    {
        return buffer->has<double>(current_var_name);

        break;
    }
    case PropertyDataType::STRING:
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

void ScatterPlotViewDataWidget::updateFromDataY(std::string dbo_name, unsigned int current_size)
{
    loginf << "ScatterPlotViewDataWidget: updateFromDataY: dbo_name " << dbo_name << " current_size " << current_size;

    assert (buffers_.count(dbo_name));
    Buffer* buffer = buffers_.at(dbo_name).get();

    unsigned int last_size = 0;

    if (buffer_y_counts_.count(dbo_name))
        last_size = buffer_y_counts_.at(dbo_name);

    DBOVariable* data_var {nullptr};

    if (!view_->hasDataVarY())
    {
        logwrn << "ScatterPlotViewDataWidget: updateFromDataY: no data var";
        return;
    }

    if (view_->isDataVarYMeta())
    {
        MetaDBOVariable& meta_var = view_->metaDataVarY();
        if (!meta_var.existsIn(dbo_name))
        {
            logwrn << "ScatterPlotViewDataWidget: updateFromDataY: meta var does not eyist in dbo";
            return;
        }

        data_var = &meta_var.getFor(dbo_name);
    }
    else
    {
        data_var = &view_->dataVarY();

        if (data_var->dboName() != dbo_name)
            return;
    }
    assert (data_var);

    PropertyDataType data_type = data_var->dataType();
    string current_var_name = data_var->name();

    switch (data_type)
    {
    case PropertyDataType::BOOL:
    {
        if (!buffer->has<bool>(current_var_name))
        {
            loginf << "ScatterPlotViewDataWidget: updateFromDataY: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<bool>(current_var_name));
        NullableVector<bool>& data = buffer->get<bool>(current_var_name);

        appendData(data, y_values_[dbo_name], last_size, current_size);
        buffer_y_counts_[dbo_name] = current_size;

        break;
    }
    case PropertyDataType::CHAR:
    {
        if (!buffer->has<char>(current_var_name))
        {
            loginf << "ScatterPlotViewDataWidget: updateFromDataY: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<char>(current_var_name));
        NullableVector<char>& data = buffer->get<char>(current_var_name);

        appendData(data, y_values_[dbo_name], last_size, current_size);
        buffer_y_counts_[dbo_name] = current_size;

        break;
    }
    case PropertyDataType::UCHAR:
    {
        if (!buffer->has<unsigned char>(current_var_name))
        {
            loginf << "ScatterPlotViewDataWidget: updateFromDataY: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<unsigned char>(current_var_name));
        NullableVector<unsigned char>& data = buffer->get<unsigned char>(current_var_name);

        appendData(data, y_values_[dbo_name], last_size, current_size);
        buffer_y_counts_[dbo_name] = current_size;

        break;
    }
    case PropertyDataType::INT:
    {
        if (!buffer->has<int>(current_var_name))
        {
            loginf << "ScatterPlotViewDataWidget: updateFromDataY: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<int>(current_var_name));
        NullableVector<int>& data = buffer->get<int>(current_var_name);

        appendData(data, y_values_[dbo_name], last_size, current_size);
        buffer_y_counts_[dbo_name] = current_size;

        break;
    }
    case PropertyDataType::UINT:
    {
        if (!buffer->has<unsigned int>(current_var_name))
        {
            loginf << "ScatterPlotViewDataWidget: updateFromDataY: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<unsigned int>(current_var_name));
        NullableVector<unsigned int>& data = buffer->get<unsigned int>(current_var_name);

        appendData(data, y_values_[dbo_name], last_size, current_size);
        buffer_y_counts_[dbo_name] = current_size;

        break;
    }
    case PropertyDataType::LONGINT:
    {
        if (!buffer->has<long int>(current_var_name))
        {
            loginf << "ScatterPlotViewDataWidget: updateFromDataY: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<long int>(current_var_name));
        NullableVector<long int>& data = buffer->get<long int>(current_var_name);

        appendData(data, y_values_[dbo_name], last_size, current_size);
        buffer_y_counts_[dbo_name] = current_size;

        break;
    }
    case PropertyDataType::ULONGINT:
    {
        if (!buffer->has<unsigned long>(current_var_name))
        {
            loginf << "ScatterPlotViewDataWidget: updateFromDataY: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<unsigned long>(current_var_name));
        NullableVector<unsigned long>& data = buffer->get<unsigned long>(current_var_name);

        appendData(data, y_values_[dbo_name], last_size, current_size);
        buffer_y_counts_[dbo_name] = current_size;

        break;
    }
    case PropertyDataType::FLOAT:
    {
        if (!buffer->has<float>(current_var_name))
        {
            loginf << "ScatterPlotViewDataWidget: updateFromDataY: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<float>(current_var_name));
        NullableVector<float>& data = buffer->get<float>(current_var_name);

        appendData(data, y_values_[dbo_name], last_size, current_size);
        buffer_y_counts_[dbo_name] = current_size;

        break;
    }
    case PropertyDataType::DOUBLE:
    {
        if (!buffer->has<double>(current_var_name))
        {
            loginf << "ScatterPlotViewDataWidget: updateFromDataY: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<double>(current_var_name));
        NullableVector<double>& data = buffer->get<double>(current_var_name);

        appendData(data, y_values_[dbo_name], last_size, current_size);
        buffer_y_counts_[dbo_name] = current_size;

        break;
    }
    case PropertyDataType::STRING:
    {
        if (!buffer->has<string>(current_var_name))
        {
            loginf << "ScatterPlotViewDataWidget: updateFromDataY: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<string>(current_var_name));
        //NullableVector<string>& data = buffer->get<string>(current_var_name);

        break;
    }
    default:
        logerr << "ScatterPlotViewDataWidget: updateFromDataY: impossible for property type "
               << Property::asString(data_type);
        throw std::runtime_error(
                    "ScatterPlotViewDataWidget: updateFromDataY: impossible property type " +
                    Property::asString(data_type));
    }

}

void ScatterPlotViewDataWidget::updateFromAllData()
{
    loginf << "ScatterPlotViewDataWidget: updateFromAllData";

    loginf << "ScatterPlotViewDataWidget: updateFromAllData: before x " << x_values_.size()
           << " y " << y_values_.size();

    assert (x_values_.size() == y_values_.size());

    for (auto& buf_it : buffers_)
    {
        unsigned int current_size = buf_it.second->size();

        if (canUpdateFromDataX(buf_it.first) && canUpdateFromDataY(buf_it.first))
        {
            updateFromDataX(buf_it.first, current_size);
            updateFromDataY(buf_it.first, current_size);
        }
        else
            loginf << "ScatterPlotViewDataWidget: updateFromAllData: " << buf_it.first
                   << " update not possible";
    }

    loginf << "ScatterPlotViewDataWidget: updateFromAllData: after x " << x_values_.size()
           << " y " << y_values_.size();


    assert (x_values_.size() == y_values_.size());

    loginf << "ScatterPlotViewDataWidget: updateFromAllData: done";
}

void ScatterPlotViewDataWidget::updateChart()
{
    chart_->removeAllSeries();

    for (auto& data : x_values_)
    {
        vector<double>& x_values = data.second;
        vector<double>& y_values = y_values_[data.first];

        QScatterSeries* chart_series = new QScatterSeries();
        chart_series->setName(data.first.c_str());
        chart_series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
        chart_series->setMarkerSize(8.0);
        chart_series->setUseOpenGL(true);
        chart_series->setColor(colors_[data.first]);

        assert (x_values.size() == y_values.size());

        for (unsigned int cnt=0; cnt < x_values.size(); ++cnt)
        {
            if (!std::isnan(x_values.at(cnt)) && !std::isnan(y_values.at(cnt)))
                chart_series->append(x_values.at(cnt), y_values.at(cnt));
        }

        chart_->addSeries(chart_series);
    }

    chart_->createDefaultAxes();
    chart_->setDropShadowEnabled(false);
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
