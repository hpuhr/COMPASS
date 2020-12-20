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

#include "histogramviewdatawidget.h"
#include "histogramview.h"
#include "compass.h"
#include "buffer.h"
#include "dbobjectmanager.h"
#include "dbobject.h"
#include "dbovariable.h"
#include "metadbovariable.h"
#include "histogramviewdatasource.h"
#include "logger.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QTabWidget>

#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QGraphicsLayout>

#include <algorithm>

QT_CHARTS_USE_NAMESPACE

using namespace std;

HistogramViewDataWidget::HistogramViewDataWidget(HistogramView* view, HistogramViewDataSource* data_source,
                                                 QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f), view_(view), data_source_(data_source)
{
    assert(data_source_);
    setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* layout = new QHBoxLayout();

    chart_series_ = new QBarSeries();

    chart_ = new QChart();
    chart_->addSeries(chart_series_);
    chart_->layout()->setContentsMargins(0, 0, 0, 0);
    chart_->setBackgroundRoundness(0);

    chart_->legend()->setVisible(true);
    chart_->legend()->setAlignment(Qt::AlignBottom);

    chart_view_ = new QChartView(chart_);
    chart_view_->setRenderHint(QPainter::Antialiasing);
    chart_view_->setRubberBand(QChartView::RectangleRubberBand);

//    QBarSet *set0 = new QBarSet("Jane");
//    QBarSet *set1 = new QBarSet("John");
//    QBarSet *set2 = new QBarSet("Axel");
//    QBarSet *set3 = new QBarSet("Mary");
//    QBarSet *set4 = new QBarSet("Samantha");

//    *set0 << 1 << 2 << 3 << 4 << 5 << 6;
//    *set1 << 5 << 0 << 0 << 4 << 0 << 7;
//    *set2 << 3 << 5 << 8 << 13 << 8 << 5;
//    *set3 << 5 << 6 << 7 << 3 << 4 << 5;
//    *set4 << 9 << 7 << 5 << 3 << 1 << 2;

//    QBarSeries *series = new QBarSeries();
//    series->setUseOpenGL(true); // not used in this chart
//    series->append(set0);
//    series->append(set1);
//    series->append(set2);
//    series->append(set3);
//    series->append(set4);

//    QChart *chart = new QChart();
//    chart->addSeries(series);
//    chart->setTitle("Simple barchart example");
//    chart->layout()->setContentsMargins(0, 0, 0, 0);
//    chart->setBackgroundRoundness(0);
//    //chart->setAnimationOptions(QChart::SeriesAnimations);

//    QStringList categories;
//    categories << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun";

//    QBarCategoryAxis *axisX = new QBarCategoryAxis();
//    axisX->append(categories);
//    chart->addAxis(axisX, Qt::AlignBottom);
//    series->attachAxis(axisX);

//    //QLogValueAxis *axisY = new QLogValueAxis();
////    axisY->setBase(8.0);
////    axisY->setMinorTickCount(-1);
//    QValueAxis *axisY = new QValueAxis();
//    axisY->setRange(0,15);
//    chart->addAxis(axisY, Qt::AlignLeft);
//    series->attachAxis(axisY);

//    chart->legend()->setVisible(true);
//    chart->legend()->setAlignment(Qt::AlignBottom);

//    QChartView *chartView = new QChartView(chart);
//    chartView->setRenderHint(QPainter::Antialiasing);
//    chartView->setRubberBand(QChartView::RectangleRubberBand);

    layout->addWidget(chart_view_);

    setLayout(layout);
}

HistogramViewDataWidget::~HistogramViewDataWidget()
{
    // TODO
    // buffer_tables_.clear();
}

void HistogramViewDataWidget::clear ()
{
    buffers_.clear();

    counts_.clear();
    data_null_cnt_ = 0;
    labels_.clear();

    max_bin_cnt_ = 0;

    data_min_.clear();
    data_max_.clear();

    bin_size_valid_ = false;
    bin_size_ = 0;
}

//void HistogramViewDataWidget::clearTables()
//{
//    logdbg << "HistogramViewDataWidget: updateTables: start";
//    // TODO
//    //  std::map <DB_OBJECT_TYPE, BufferTableWidget*>::iterator it;

//    //  for (it = buffer_tables_.begin(); it != buffer_tables_.end(); it++)
//    //  {
//    //    it->second->show (0, 0, false);
//    //  }

//    logdbg << "HistogramViewDataWidget: updateTables: end";
//}

void HistogramViewDataWidget::loadingStartedSlot()
{
    clear();
}

void HistogramViewDataWidget::updateDataSlot(DBObject& object, std::shared_ptr<Buffer> buffer)
{
    logdbg << "HistogramViewDataWidget: updateDataSlot: start";

    if (!buffer->size())
        return;

    buffers_[object.name()] = buffer;
    updateFromData(object.name());

    chart_series_->clear();

    QBarSet *set0 = new QBarSet((view_->dataVarDBO()+": "+view_->dataVarName()).c_str());
    //*set0 << 1 << 2 << 3 << 4 << 5 << 6;
    for (auto bin : counts_)
    {
        if (bin > max_bin_cnt_)
            max_bin_cnt_ = bin;

        *set0 << bin;
    }

    chart_series_->append(set0);

    if (chart_x_axis_)
        delete chart_x_axis_;

    QStringList categories;

    for (auto lbl : labels_)
        categories << lbl.c_str();
    //categories << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun";

    chart_x_axis_ = new QBarCategoryAxis();
    //chart_x_axis_->setTickCount(5);
    chart_x_axis_->append(categories);
    chart_->addAxis(chart_x_axis_, Qt::AlignBottom);
    chart_series_->attachAxis(chart_x_axis_);

    if (chart_y_axis_)
        delete chart_y_axis_;

    chart_y_axis_ = new QValueAxis();
    chart_y_axis_->setRange(0, max_bin_cnt_);
    chart_->addAxis(chart_y_axis_, Qt::AlignLeft);
    chart_series_->attachAxis(chart_y_axis_);

    chart_->zoomReset();
    //chart_->setTitle("Simple barchart example");

    logdbg << "HistogramViewDataWidget: updateTables: end";
}

void HistogramViewDataWidget::updateFromData(std::string dbo_name)
{
    assert (buffers_.count(dbo_name));
    Buffer* buffer = buffers_.at(dbo_name).get();

    DBOVariable* data_var {nullptr};

    if (!view_->hasDataVar())
    {
        logwrn << "HistogramViewDataWidget: updateFromData: no data var";
        return;
    }

    if (view_->isDataVarMeta())
    {
        MetaDBOVariable& meta_var = view_->metaDataVar();
        if (!meta_var.existsIn(dbo_name))
        {
            logwrn << "HistogramViewDataWidget: updateFromData: meta var does not exist in dbo";
            return;
        }

        data_var = &meta_var.getFor(dbo_name);
    }
    else
    {
        data_var = &view_->dataVar();

        if (data_var->dboName() != dbo_name)
            return;
    }
    assert (data_var);

    PropertyDataType data_type = data_var->dataType();
    string current_var_name = data_var->name();

    //unsigned num_bins = 20;

    switch (data_type)
    {
        case PropertyDataType::BOOL:
        {
            assert(buffer->has<bool>(current_var_name));
            NullableVector<bool>& data = buffer->get<bool>(current_var_name);

            break;
        }
        case PropertyDataType::CHAR:
        {
            assert(buffer->has<char>(current_var_name));
            NullableVector<char>& data = buffer->get<char>(current_var_name);

            break;
        }
        case PropertyDataType::UCHAR:
        {
            assert(buffer->has<unsigned char>(current_var_name));
            NullableVector<unsigned char>& data = buffer->get<unsigned char>(current_var_name);

            break;
        }
        case PropertyDataType::INT:
        {
            assert(buffer->has<int>(current_var_name));
            NullableVector<int>& data = buffer->get<int>(current_var_name);

            bool min_max_set {true};
            int data_min, data_max;

            tie(min_max_set, data_min, data_max) = data.minMaxValues();

            if (!min_max_set)
            {
                logwrn << "HistogramViewDataWidget: updateFromData: no data set in buffer";
                return;
            }

            if (!bin_size_valid_ ||
                    (data_min_.isValid() && data_max_.isValid() && (data_min < data_min_.toDouble()
                                                                    || data_max > data_max_.toDouble())))
            {
                loginf << "UGA up all bin_size_valid " << bin_size_valid_;

                loginf << " data_min_.isValid() " << data_min_.isValid()
                       << " data_min < data_min_ " << (data_min < data_min_.toDouble())
                          << " data_min " << data_min << " data_min_ " << data_min_.toString().toStdString();

                loginf << " data_max_.isValid() " << data_max_.isValid()
                       << " data_max > data_max_ " << (data_max > data_max_.toDouble())
                       << " data_max " << data_max << " data_max_ " << data_max_.toString().toStdString();

                updateFromAllData(); // clear, recalc min/max, update
                return;
            }

            loginf << "UGA data_min " << data_min << " data_max " << data_max << " num_bins " << num_bins_
                   << " bin_size " << bin_size_;

            updateCounts<int> (data, data_var);

            break;
        }
        case PropertyDataType::UINT:
        {
            assert(buffer->has<unsigned int>(current_var_name));
            NullableVector<unsigned int>& data = buffer->get<unsigned int>(current_var_name);

            bool min_max_set {true};
            unsigned int data_min, data_max;

            tie(min_max_set, data_min, data_max) = data.minMaxValues();

            if (!min_max_set)
            {
                logwrn << "HistogramViewDataWidget: updateFromData: no data set in buffer";
                return;
            }

            if (!bin_size_valid_ ||
                    (data_min_.isValid() && data_max_.isValid() && (data_min < data_min_.toDouble()
                                                                    || data_max > data_max_.toDouble())))
            {
                loginf << "UGA up all bin_size_valid " << bin_size_valid_;

                loginf << " data_min_.isValid() " << data_min_.isValid()
                       << " data_min < data_min_ " << (data_min < data_min_.toDouble())
                          << " data_min " << data_min << " data_min_ " << data_min_.toString().toStdString();

                loginf << " data_max_.isValid() " << data_max_.isValid()
                       << " data_max > data_max_ " << (data_max > data_max_.toDouble())
                       << " data_max " << data_max << " data_max_ " << data_max_.toString().toStdString();

                updateFromAllData(); // clear, recalc min/max, update
                return;
            }

            loginf << "UGA data_min " << data_min << " data_max " << data_max << " num_bins " << num_bins_
                   << " bin_size " << bin_size_;

            updateCounts<unsigned int> (data, data_var);

            break;
        }
        case PropertyDataType::LONGINT:
        {
            assert(buffer->has<long int>(current_var_name));
            NullableVector<long int>& data = buffer->get<long int>(current_var_name);


            break;
        }
        case PropertyDataType::ULONGINT:
        {
            assert(buffer->has<unsigned long>(current_var_name));
            NullableVector<unsigned long>& data = buffer->get<unsigned long>(current_var_name);


            break;
        }
        case PropertyDataType::FLOAT:
        {
            assert(buffer->has<float>(current_var_name));
            NullableVector<float>& data = buffer->get<float>(current_var_name);

            bool min_max_set {true};
            float data_min, data_max;

            tie(min_max_set, data_min, data_max) = data.minMaxValues();

            if (!min_max_set)
            {
                logwrn << "HistogramViewDataWidget: updateFromData: no data set in buffer";
                return;
            }

            if (!bin_size_valid_ ||
                    (data_min_.isValid() && data_max_.isValid() && (data_min < data_min_.toDouble()
                                                                    || data_max > data_max_.toDouble())))
            {
                loginf << "UGA up all bin_size_valid " << bin_size_valid_;

                loginf << " data_min_.isValid() " << data_min_.isValid()
                       << " data_min < data_min_ " << (data_min < data_min_.toDouble())
                          << " data_min " << data_min << " data_min_ " << data_min_.toString().toStdString();

                loginf << " data_max_.isValid() " << data_max_.isValid()
                       << " data_max > data_max_ " << (data_max > data_max_.toDouble())
                       << " data_max " << data_max << " data_max_ " << data_max_.toString().toStdString();

                updateFromAllData(); // clear, recalc min/max, update
                return;
            }

            loginf << "UGA data_min " << data_min << " data_max " << data_max << " num_bins " << num_bins_
                   << " bin_size " << bin_size_;

            updateCounts<float> (data, data_var);

            break;
        }
        case PropertyDataType::DOUBLE:
        {
            assert(buffer->has<double>(current_var_name));
            NullableVector<double>& data = buffer->get<double>(current_var_name);

            bool min_max_set {true};
            double data_min, data_max;

            tie(min_max_set, data_min, data_max) = data.minMaxValues();

            if (!min_max_set)
            {
                logwrn << "HistogramViewDataWidget: updateFromData: no data set in buffer";
                return;
            }

            if (!bin_size_valid_ ||
                    (data_min_.isValid() && data_max_.isValid() && (data_min < data_min_.toDouble()
                                                                    || data_max > data_max_.toDouble())))
            {
                loginf << "UGA up all bin_size_valid " << bin_size_valid_;

                loginf << " data_min_.isValid() " << data_min_.isValid()
                       << " data_min < data_min_ " << (data_min < data_min_.toDouble())
                          << " data_min " << data_min << " data_min_ " << data_min_.toString().toStdString();

                loginf << " data_max_.isValid() " << data_max_.isValid()
                       << " data_max > data_max_ " << (data_max > data_max_.toDouble())
                       << " data_max " << data_max << " data_max_ " << data_max_.toString().toStdString();

                updateFromAllData(); // clear, recalc min/max, update
                return;
            }

            loginf << "UGA data_min " << data_min << " data_max " << data_max << " num_bins " << num_bins_
                   << " bin_size " << bin_size_;

            updateCounts<double> (data, data_var);

            break;
        }
        case PropertyDataType::STRING:
        {
            assert(buffer->has<string>(current_var_name));
            //NullableVector<string>& data = buffer->get<string>(current_var_name);

            break;
        }
        default:
            logerr << "HistogramViewDataWidget: updateFromData: impossible for property type "
                   << Property::asString(data_type);
            throw std::runtime_error(
                "HistogramViewDataWidget: updateFromData: impossible property type " +
                Property::asString(data_type));
    }

}

void HistogramViewDataWidget::updateFromAllData()
{
    loginf << "HistogramViewDataWidget: updateFromAllData";

    counts_.clear();
    labels_.clear();

    max_bin_cnt_ = 0;

    data_min_.clear();
    data_max_.clear();
    calculateGlobalMinMax();

    for (auto& buf_it : buffers_)
        updateFromData(buf_it.first);

    loginf << "HistogramViewDataWidget: updateFromAllData: done";
}

void HistogramViewDataWidget::calculateGlobalMinMax()
{
    for (auto& buf_it : buffers_)
    {
        Buffer* buffer = buf_it.second.get();
        string dbo_name = buf_it.first;

        DBOVariable* data_var {nullptr};

        if (!view_->hasDataVar())
            continue;

        if (view_->isDataVarMeta())
        {
            MetaDBOVariable& meta_var = view_->metaDataVar();
            if (!meta_var.existsIn(dbo_name))
                continue;

            data_var = &meta_var.getFor(dbo_name);
        }
        else
        {
            data_var = &view_->dataVar();

            if (data_var->dboName() != dbo_name)
                continue;
        }
        assert (data_var);

        PropertyDataType data_type = data_var->dataType();
        string current_var_name = data_var->name();

        switch (data_type)
        {
            case PropertyDataType::BOOL:
            {
                assert(buffer->has<bool>(current_var_name));
                //NullableVector<bool>& data = buffer->get<bool>(current_var_name);
                data_min_ = false;
                data_max_ = true;

                break;
            }
            case PropertyDataType::CHAR:
            {
                assert(buffer->has<char>(current_var_name));
                NullableVector<char>& data = buffer->get<char>(current_var_name);
                updateMinMax (data);

                break;
            }
            case PropertyDataType::UCHAR:
            {
                assert(buffer->has<unsigned char>(current_var_name));
                NullableVector<unsigned char>& data = buffer->get<unsigned char>(current_var_name);
                updateMinMax (data);

                break;
            }
            case PropertyDataType::INT:
            {
                assert(buffer->has<int>(current_var_name));
                NullableVector<int>& data = buffer->get<int>(current_var_name);
                updateMinMax (data);

                break;
            }
            case PropertyDataType::UINT:
            {
                assert(buffer->has<unsigned int>(current_var_name));
                NullableVector<unsigned int>& data = buffer->get<unsigned int>(current_var_name);
                updateMinMax (data);

                break;
            }
            case PropertyDataType::LONGINT:
            {
                assert(buffer->has<long int>(current_var_name));
                NullableVector<long int>& data = buffer->get<long int>(current_var_name);
                updateMinMax (data);

                break;
            }
            case PropertyDataType::ULONGINT:
            {
                assert(buffer->has<unsigned long>(current_var_name));
                NullableVector<unsigned long>& data = buffer->get<unsigned long>(current_var_name);
                updateMinMax (data);

                break;
            }
            case PropertyDataType::FLOAT:
            {
                assert(buffer->has<float>(current_var_name));
                NullableVector<float>& data = buffer->get<float>(current_var_name);
                updateMinMax (data);

                break;
            }
            case PropertyDataType::DOUBLE:
            {
                assert(buffer->has<double>(current_var_name));
                NullableVector<double>& data = buffer->get<double>(current_var_name);
                updateMinMax (data);

                break;
            }
            case PropertyDataType::STRING:
            {
                assert(buffer->has<string>(current_var_name));
                NullableVector<string>& data = buffer->get<string>(current_var_name);
                updateMinMax (data);

                break;
            }
            default:
                logerr << "HistogramViewDataWidget: updateFromData: impossible for property type "
                       << Property::asString(data_type);
                throw std::runtime_error(
                    "HistogramViewDataWidget: updateFromData: impossible property type " +
                    Property::asString(data_type));
        }
    }

    if (data_max_.isValid() && data_min_.isValid())
    {
        bin_size_ = (data_max_.toDouble()-data_min_.toDouble())/((double)num_bins_-1);
        bin_size_valid_ = true;
    }
    else
    {
        bin_size_ = 0;
        bin_size_valid_ = false;
    }

    loginf << "HistogramViewDataWidget: updateFromAllData: done: data_min_ " << data_min_.toString().toStdString()
           << " data_max_ " << data_max_.toString().toStdString();
}

void HistogramViewDataWidget::updateMinMax(NullableVector<std::string>& data)
{
    bool min_max_set {true};
    std::string data_min, data_max;

    std::tie(min_max_set, data_min, data_max) = data.minMaxValues();

    if (!min_max_set)
        return;

    QVariant min_var {data_min.c_str()};
    QVariant max_var {data_max.c_str()};

    if (data_min_.isValid() && data_max_.isValid())
    {
        if (min_var < data_min_)
            data_min_ = min_var;

        if (max_var > data_max_)
            data_max_ = max_var;
    }
    else
    {
        data_min_ = min_var;
        data_max_ = max_var;
    }
}

void HistogramViewDataWidget::updateMinMax(NullableVector<long int>& data)
{
    bool min_max_set {true};
    long int data_min, data_max;

    std::tie(min_max_set, data_min, data_max) = data.minMaxValues();

    if (!min_max_set)
        return;

    QVariant min_var = QVariant::fromValue(data_min);
    QVariant max_var = QVariant::fromValue(data_max);

    if (data_min_.isValid() && data_max_.isValid())
    {
        if (min_var < data_min_)
            data_min_ = min_var;

        if (max_var > data_max_)
            data_max_ = max_var;
    }
    else
    {
        data_min_ = min_var;
        data_max_ = max_var;
    }
}

void HistogramViewDataWidget::updateMinMax(NullableVector<unsigned long int>& data)
{
    bool min_max_set {true};
    long int data_min, data_max;

    std::tie(min_max_set, data_min, data_max) = data.minMaxValues();

    if (!min_max_set)
        return;

    QVariant min_var = QVariant::fromValue(data_min);
    QVariant max_var = QVariant::fromValue(data_max);

    if (data_min_.isValid() && data_max_.isValid())
    {
        if (min_var < data_min_)
            data_min_ = min_var;

        if (max_var > data_max_)
            data_max_ = max_var;
    }
    else
    {
        data_min_ = min_var;
        data_max_ = max_var;
    }
}

void HistogramViewDataWidget::exportDataSlot(bool overwrite)
{
    logdbg << "HistogramViewDataWidget: exportDataSlot";

}

void HistogramViewDataWidget::exportDoneSlot(bool cancelled) { emit exportDoneSignal(cancelled); }

//void HistogramViewDataWidget::showOnlySelectedSlot(bool value)
//{
//    loginf << "HistogramViewDataWidget: showOnlySelectedSlot: " << value;
//    emit showOnlySelectedSignal(value);
//}

//void HistogramViewDataWidget::usePresentationSlot(bool use_presentation)
//{
//    loginf << "HistogramViewDataWidget: usePresentationSlot";

//    emit usePresentationSignal(use_presentation);
//}

//void HistogramViewDataWidget::showAssociationsSlot(bool value)
//{
//    loginf << "HistogramViewDataWidget: showAssociationsSlot: " << value;
//    emit showAssociationsSignal(value);
//}

//void HistogramViewDataWidget::resetModels()
//{
//    if (all_buffer_table_widget_)
//        all_buffer_table_widget_->resetModel();

//    for (auto& table_widget_it : buffer_tables_)
//        table_widget_it.second->resetModel();
//}

//void HistogramViewDataWidget::updateToSelection()
//{
//    if (all_buffer_table_widget_)
//        all_buffer_table_widget_->updateToSelection();

//    for (auto& table_widget_it : buffer_tables_)
//        table_widget_it.second->updateToSelection();
//}

//void HistogramViewDataWidget::selectFirstSelectedRow()
//{
//    if (all_buffer_table_widget_)
//        all_buffer_table_widget_->selectSelectedRows();
//}

//AllBufferTableWidget* HistogramViewDataWidget::getAllBufferTableWidget ()
//{
//    assert (all_buffer_table_widget_);
//    return all_buffer_table_widget_;
//}
