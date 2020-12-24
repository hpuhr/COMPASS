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
#include "evaluationmanager.h"

#include "eval/results/extra/datasingle.h"
#include "eval/results/extra/datajoined.h"
#include "eval/results/extra/tracksingle.h"
#include "eval/results/extra/trackjoined.h"
#include "eval/results/detection/joined.h"
#include "eval/results/detection/single.h"
#include "eval/results/position/distancejoined.h"
#include "eval/results/position/distancesingle.h"
#include "eval/results/position/alongsingle.h"
#include "eval/results/position/alongjoined.h"
#include "eval/results/position/acrosssingle.h"
#include "eval/results/position/acrossjoined.h"
#include "eval/results/position/latencysingle.h"
#include "eval/results/position/latencyjoined.h"
#include "eval/results/identification/single.h"
#include "eval/results/identification/joined.h"
#include "eval/results/mode_a/single.h"
#include "eval/results/mode_a/joined.h"
#include "eval/results/mode_c/single.h"
#include "eval/results/mode_c/joined.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QTabWidget>

#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLogValueAxis>
#include <QGraphicsLayout>

#include <algorithm>

QT_CHARTS_USE_NAMESPACE

using namespace std;
using namespace EvaluationRequirementResult;

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

    layout->addWidget(chart_view_);

    setLayout(layout);

    colors_["Radar"] = QColor("#00FF00");
    colors_["MLAT"] = QColor("#FF0000");
    colors_["ADSB"] = QColor("#6666FF");
    colors_["RefTraj"] = QColor("#FFA500");
    colors_["Tracker"] = QColor("#CCCCCC");
}

HistogramViewDataWidget::~HistogramViewDataWidget()
{
    delete chart_view_;
}

void HistogramViewDataWidget::update()
{
    loginf << "HistogramViewDataWidget: update";

    updateFromAllData();
    updateChart();
}

void HistogramViewDataWidget::clear ()
{
    loginf << "HistogramViewDataWidget: clear";

    buffers_.clear();

    counts_.clear();
    data_null_cnt_.clear();
    labels_.clear();

    max_bin_cnt_ = 0;

    data_min_.clear();
    data_max_.clear();

    bin_size_valid_ = false;
    bin_size_ = 0;
}

void HistogramViewDataWidget::loadingStartedSlot()
{
    clear();
    updateChart();
}

void HistogramViewDataWidget::updateDataSlot(DBObject& object, std::shared_ptr<Buffer> buffer)
{
    logdbg << "HistogramViewDataWidget: updateDataSlot: start";

    if (!buffer->size())
        return;

    buffers_[object.name()] = buffer;
    updateFromData(object.name());

    updateChart();

    logdbg << "HistogramViewDataWidget: updateDataSlot: end";
}

void HistogramViewDataWidget::updateFromData(std::string dbo_name)
{
    loginf << "HistogramViewDataWidget: updateFromData: start dbo " << dbo_name;

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

    switch (data_type)
    {
    case PropertyDataType::BOOL:
    {
        if (!buffer->has<bool>(current_var_name))
        {
            loginf << "HistogramViewDataWidget: updateFromData: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<bool>(current_var_name));
        NullableVector<bool>& data = buffer->get<bool>(current_var_name);

        bool data_min {true};
        bool data_max {false};

        if (!bin_size_valid_ ||
                (data_min_.isValid() && data_max_.isValid() && (data_min < data_min_.toDouble()
                                                                || data_max > data_max_.toDouble())))
        {
            updateFromAllData(); // clear, recalc min/max, update
            return;
        }

        updateCounts<bool> (dbo_name, data, data_var);

        break;
    }
    case PropertyDataType::CHAR:
    {
        if (!buffer->has<char>(current_var_name))
        {
            loginf << "HistogramViewDataWidget: updateFromData: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<char>(current_var_name));
        NullableVector<char>& data = buffer->get<char>(current_var_name);

        bool min_max_set {true};
        char data_min, data_max;

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
            updateFromAllData(); // clear, recalc min/max, update
            return;
        }

        updateCounts<char> (dbo_name, data, data_var);

        break;
    }
    case PropertyDataType::UCHAR:
    {
        if (!buffer->has<unsigned char>(current_var_name))
        {
            loginf << "HistogramViewDataWidget: updateFromData: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<unsigned char>(current_var_name));
        NullableVector<unsigned char>& data = buffer->get<unsigned char>(current_var_name);

        bool min_max_set {true};
        unsigned char data_min, data_max;

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
            updateFromAllData(); // clear, recalc min/max, update
            return;
        }

        updateCounts<unsigned char> (dbo_name, data, data_var);

        break;
    }
    case PropertyDataType::INT:
    {
        if (!buffer->has<int>(current_var_name))
        {
            loginf << "HistogramViewDataWidget: updateFromData: buffer does not contain " << current_var_name;
            return;
        }

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
            updateFromAllData(); // clear, recalc min/max, update
            return;
        }

        updateCounts<int> (dbo_name, data, data_var);

        break;
    }
    case PropertyDataType::UINT:
    {
        if (!buffer->has<unsigned int>(current_var_name))
        {
            loginf << "HistogramViewDataWidget: updateFromData: buffer does not contain " << current_var_name;
            return;
        }

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
            updateFromAllData(); // clear, recalc min/max, update
            return;
        }

        updateCounts<unsigned int> (dbo_name, data, data_var);

        break;
    }
    case PropertyDataType::LONGINT:
    {
        if (!buffer->has<long int>(current_var_name))
        {
            loginf << "HistogramViewDataWidget: updateFromData: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<long int>(current_var_name));
        NullableVector<long int>& data = buffer->get<long int>(current_var_name);

        bool min_max_set {true};
        long int data_min, data_max;

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
            updateFromAllData(); // clear, recalc min/max, update
            return;
        }

        updateCounts<long int> (dbo_name, data, data_var);

        break;
    }
    case PropertyDataType::ULONGINT:
    {
        if (!buffer->has<unsigned long>(current_var_name))
        {
            loginf << "HistogramViewDataWidget: updateFromData: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<unsigned long>(current_var_name));
        NullableVector<unsigned long>& data = buffer->get<unsigned long>(current_var_name);

        bool min_max_set {true};
        unsigned long data_min, data_max;

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
            updateFromAllData(); // clear, recalc min/max, update
            return;
        }

        updateCounts<unsigned long> (dbo_name, data, data_var);

        break;
    }
    case PropertyDataType::FLOAT:
    {
        if (!buffer->has<float>(current_var_name))
        {
            loginf << "HistogramViewDataWidget: updateFromData: buffer does not contain " << current_var_name;
            return;
        }

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
            updateFromAllData(); // clear, recalc min/max, update
            return;
        }


        updateCounts<float> (dbo_name, data, data_var);

        break;
    }
    case PropertyDataType::DOUBLE:
    {
        if (!buffer->has<double>(current_var_name))
        {
            loginf << "HistogramViewDataWidget: updateFromData: buffer does not contain " << current_var_name;
            return;
        }

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
//            loginf << "UGA up all bin_size_valid " << bin_size_valid_;

//            loginf << " data_min_.isValid() " << data_min_.isValid()
//                   << " data_min < data_min_ " << (data_min < data_min_.toDouble())
//                   << " data_min " << data_min << " data_min_ " << data_min_.toString().toStdString();

//            loginf << " data_max_.isValid() " << data_max_.isValid()
//                   << " data_max > data_max_ " << (data_max > data_max_.toDouble())
//                   << " data_max " << data_max << " data_max_ " << data_max_.toString().toStdString();

            updateFromAllData(); // clear, recalc min/max, update
            return;
        }

//        loginf << "UGA data_min " << data_min << " data_max " << data_max << " num_bins " << num_bins_
//               << " bin_size " << bin_size_;

        updateCounts<double> (dbo_name, data, data_var);

        break;
    }
    case PropertyDataType::STRING:
    {
        if (!buffer->has<string>(current_var_name))
        {
            loginf << "HistogramViewDataWidget: updateFromData: buffer does not contain " << current_var_name;
            return;
        }

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

    loginf << "HistogramViewDataWidget: updateFromData: done dbo " << dbo_name;
}

void HistogramViewDataWidget::updateFromAllData()
{
    loginf << "HistogramViewDataWidget: updateFromAllData";

    counts_.clear();
    data_null_cnt_.clear();
    labels_.clear();

    max_bin_cnt_ = 0;

    data_min_.clear();
    data_max_.clear();

    loginf << "HistogramViewDataWidget: updateFromAllData: num buffers " << buffers_.size();

    calculateGlobalMinMax();

    for (auto& buf_it : buffers_)
        updateFromData(buf_it.first);

    loginf << "HistogramViewDataWidget: updateFromAllData: done";
}

void HistogramViewDataWidget::updateFromResult(std::shared_ptr<EvaluationRequirementResult::Base> result)
{
    loginf << "HistogramViewDataWidget: updateFromResult";

    if (result->type() == "SingleExtraData")
        showResult(static_pointer_cast<SingleExtraData>(result));
    else if (result->type() == "JoinedExtraData")
        showResult(static_pointer_cast<JoinedExtraData>(result));
    else if (result->type() == "SingleExtraTrack")
        showResult(static_pointer_cast<SingleExtraTrack>(result));
    else if (result->type() == "JoinedExtraTrack")
        showResult(static_pointer_cast<JoinedExtraTrack>(result));
    else if (result->type() == "SingleDetection")
        showResult(static_pointer_cast<SingleDetection>(result));
    else if (result->type() == "JoinedDetection")
        showResult(static_pointer_cast<JoinedDetection>(result));
    else if (result->type() == "SinglePositionDistance")
        showResult(static_pointer_cast<SinglePositionDistance>(result));
    else if (result->type() == "JoinedPositionDistance")
        showResult(static_pointer_cast<JoinedPositionDistance>(result));
    else if (result->type() == "SinglePositionAlong")
        showResult(static_pointer_cast<SinglePositionAlong>(result));
    else if (result->type() == "JoinedPositionAlong")
        showResult(static_pointer_cast<JoinedPositionAlong>(result));
    else if (result->type() == "SinglePositionAcross")
        showResult(static_pointer_cast<SinglePositionAcross>(result));
    else if (result->type() == "JoinedPositionAcross")
        showResult(static_pointer_cast<JoinedPositionAcross>(result));
    else if (result->type() == "SinglePositionLatency")
        showResult(static_pointer_cast<SinglePositionLatency>(result));
    else if (result->type() == "JoinedPositionLatency")
        showResult(static_pointer_cast<JoinedPositionLatency>(result));
    else if (result->type() == "SingleIdentification")
        showResult(static_pointer_cast<SingleIdentification>(result));
    else if (result->type() == "JoinedIdentification")
        showResult(static_pointer_cast<JoinedIdentification>(result));
    else if (result->type() == "SingleModeA")
        showResult(static_pointer_cast<SingleModeA>(result));
    else if (result->type() == "JoinedModeA")
        showResult(static_pointer_cast<JoinedModeA>(result));
    else if (result->type() == "SingleModeC")
        showResult(static_pointer_cast<SingleModeC>(result));
    else if (result->type() == "JoinedModeC")
        showResult(static_pointer_cast<JoinedModeC>(result));
    else
        throw runtime_error("HistogramViewDataWidget: updateFromResult: unknown result type");
}

void HistogramViewDataWidget::showResult (std::shared_ptr<EvaluationRequirementResult::SingleExtraData> result)
{

}

void HistogramViewDataWidget::showResult (std::shared_ptr<EvaluationRequirementResult::JoinedExtraData> result)
{
    logdbg << "HistogramViewDataWidget: showResult: joined data";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleExtraData>(result_it));

        if (result_it->use())
            showResult (static_pointer_cast<SingleExtraData>(result_it));
    }
}


void HistogramViewDataWidget::showResult (std::shared_ptr<EvaluationRequirementResult::SingleExtraTrack> result)
{

}


void HistogramViewDataWidget::showResult (std::shared_ptr<EvaluationRequirementResult::JoinedExtraTrack> result)
{
    logdbg << "HistogramViewDataWidget: showResult: joined track";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleExtraTrack>(result_it));

        if (result_it->use())
            showResult (static_pointer_cast<SingleExtraTrack>(result_it));
    }
}


void HistogramViewDataWidget::showResult (std::shared_ptr<EvaluationRequirementResult::SingleDetection> result)
{

}


void HistogramViewDataWidget::showResult (std::shared_ptr<EvaluationRequirementResult::JoinedDetection> result)
{
    logdbg << "HistogramViewDataWidget: showResult: joined detection";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleDetection>(result_it));

        if (result_it->use())
            showResult (static_pointer_cast<SingleDetection>(result_it));
    }
}


void HistogramViewDataWidget::showResult (
        std::shared_ptr<EvaluationRequirementResult::SinglePositionDistance> result)
{
    assert (result);

    const vector<double>& values = result->values(); // distance values

    updateMinMax (values);
    updateCounts(values);
    updateChart();
}


void HistogramViewDataWidget::showResult (
        std::shared_ptr<EvaluationRequirementResult::JoinedPositionDistance> result)
{
    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results) // calculate global min max
    {
        shared_ptr<EvaluationRequirementResult::SinglePositionDistance> single_result =
                static_pointer_cast<SinglePositionDistance>(result_it);
        assert (single_result);

        if (single_result->use())
            updateMinMax (single_result->values());
    }

    for (auto& result_it : results) // update counts
    {
        shared_ptr<EvaluationRequirementResult::SinglePositionDistance> single_result =
                static_pointer_cast<SinglePositionDistance>(result_it);
        assert (single_result);

        if (single_result->use())
            updateCounts (single_result->values());
    }

    updateChart();
}


void HistogramViewDataWidget::showResult (
        std::shared_ptr<EvaluationRequirementResult::SinglePositionAlong> result)
{
    assert (result);

    const vector<double>& values = result->values(); // along values

    updateMinMax (values);
    updateCounts(values);
    updateChart();
}


void HistogramViewDataWidget::showResult (
        std::shared_ptr<EvaluationRequirementResult::JoinedPositionAlong> result)
{
    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results) // calculate global min max
    {
        shared_ptr<EvaluationRequirementResult::SinglePositionAlong> single_result =
                static_pointer_cast<SinglePositionAlong>(result_it);
        assert (single_result);

        if (single_result->use())
            updateMinMax (single_result->values());
    }

    for (auto& result_it : results) // update counts
    {
        shared_ptr<EvaluationRequirementResult::SinglePositionAlong> single_result =
                static_pointer_cast<SinglePositionAlong>(result_it);
        assert (single_result);

        if (single_result->use())
            updateCounts (single_result->values());
    }

    updateChart();
}


void HistogramViewDataWidget::showResult (
        std::shared_ptr<EvaluationRequirementResult::SinglePositionAcross> result)
{
    assert (result);

    const vector<double>& values = result->values(); // across values

    updateMinMax (values);
    updateCounts(values);
    updateChart();
}


void HistogramViewDataWidget::showResult (
        std::shared_ptr<EvaluationRequirementResult::JoinedPositionAcross> result)
{
    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results) // calculate global min max
    {
        shared_ptr<EvaluationRequirementResult::SinglePositionAcross> single_result =
                static_pointer_cast<SinglePositionAcross>(result_it);
        assert (single_result);

        if (single_result->use())
            updateMinMax (single_result->values());
    }

    for (auto& result_it : results) // update counts
    {
        shared_ptr<EvaluationRequirementResult::SinglePositionAcross> single_result =
                static_pointer_cast<SinglePositionAcross>(result_it);
        assert (single_result);

        if (single_result->use())
            updateCounts (single_result->values());
    }

    updateChart();
}


void HistogramViewDataWidget::showResult (
        std::shared_ptr<EvaluationRequirementResult::SinglePositionLatency> result)
{
    assert (result);

    const vector<double>& values = result->values(); // latency values

    updateMinMax (values);
    updateCounts(values);
    updateChart();
}


void HistogramViewDataWidget::showResult (
        std::shared_ptr<EvaluationRequirementResult::JoinedPositionLatency> result)
{
    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results) // calculate global min max
    {
        shared_ptr<EvaluationRequirementResult::SinglePositionLatency> single_result =
                static_pointer_cast<SinglePositionLatency>(result_it);
        assert (single_result);

        if (single_result->use())
            updateMinMax (single_result->values());
    }

    for (auto& result_it : results) // update counts
    {
        shared_ptr<EvaluationRequirementResult::SinglePositionLatency> single_result =
                static_pointer_cast<SinglePositionLatency>(result_it);
        assert (single_result);

        if (single_result->use())
            updateCounts (single_result->values());
    }

    updateChart();
}


void HistogramViewDataWidget::showResult (
        std::shared_ptr<EvaluationRequirementResult::SingleIdentification> result)
{

}


void HistogramViewDataWidget::showResult (
        std::shared_ptr<EvaluationRequirementResult::JoinedIdentification> result)
{
    logdbg << "HistogramViewDataWidget: updateFromResult: joined identification";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleIdentification>(result_it));
        if (result_it->use())
            showResult (static_pointer_cast<SingleIdentification>(result_it));
    }
}


void HistogramViewDataWidget::showResult (std::shared_ptr<EvaluationRequirementResult::SingleModeA> result)
{

}


void HistogramViewDataWidget::showResult (std::shared_ptr<EvaluationRequirementResult::JoinedModeA> result)
{
    logdbg << "HistogramViewDataWidget: showResult: joined mode 3/a";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleModeA>(result_it));
        if (result_it->use())
            showResult (static_pointer_cast<SingleModeA>(result_it));
    }
}


void HistogramViewDataWidget::showResult (std::shared_ptr<EvaluationRequirementResult::SingleModeC> result)
{

}


void HistogramViewDataWidget::showResult (std::shared_ptr<EvaluationRequirementResult::JoinedModeC> result)
{
    logdbg << "HistogramViewDataWidget: showResult: joined mode c";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleModeC>(result_it));
        if (result_it->use())
            showResult (static_pointer_cast<SingleModeC>(result_it));
    }
}

void HistogramViewDataWidget::updateChart()
{
    loginf << "HistogramViewDataWidget: updateChart";

    chart_series_->clear();

    bool use_log_scale = view_->useLogScale();

    for (auto& cnt_it : counts_)
    {
        unsigned int sum_cnt {0};
        for (auto bin : cnt_it.second)
            sum_cnt += bin;

        QBarSet *set = new QBarSet((cnt_it.first+" ("+to_string(sum_cnt)+")").c_str());
        //*set0 << 1 << 2 << 3 << 4 << 5 << 6;
        for (auto bin : cnt_it.second)
        {
            if (bin > max_bin_cnt_)
                max_bin_cnt_ = bin;

            if (use_log_scale && bin == 0)
                *set << 10e-6; // Logarithms of zero and negative values are undefined.
            else
                *set << bin;

        }

        if (data_null_cnt_.size())
        {
            *set << data_null_cnt_[cnt_it.first];
        }

        set->setColor(colors_[cnt_it.first]);
        chart_series_->append(set);
    }

    if (chart_x_axis_)
        delete chart_x_axis_;

    QStringList categories;

    for (auto lbl : labels_)
        categories << lbl.c_str();

    if (data_null_cnt_.size())
        categories << "NULL";
    //categories << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun";

    chart_x_axis_ = new QBarCategoryAxis();

    if (view_->showResults())
        chart_x_axis_->setTitleText(( view_->evalResultGrpReq()+":"+view_->evalResultsID()).c_str());
    else
        chart_x_axis_->setTitleText((view_->dataVarDBO()+": "+view_->dataVarName()).c_str());

    chart_x_axis_->setLabelsAngle(85);
    chart_x_axis_->append(categories);
    chart_->addAxis(chart_x_axis_, Qt::AlignBottom);
    chart_series_->attachAxis(chart_x_axis_);

    if (chart_y_axis_)
    {
        chart_series_->detachAxis(chart_y_axis_);
        delete chart_y_axis_;
    }

    if (view_->useLogScale())
    {
        QLogValueAxis* chart_y_axis = new QLogValueAxis();

        chart_y_axis->setLabelFormat("%g");
        chart_y_axis->setBase(10.0);
        chart_y_axis->setMinorTickCount(-1);

        chart_y_axis_ = chart_y_axis;
        chart_y_axis_->setRange(10e-2, max_bin_cnt_);
    }
    else
    {
        chart_y_axis_ = new QValueAxis();
        chart_y_axis_->setRange(0, max_bin_cnt_);
    }

    chart_y_axis_->setTitleText("Count");

    chart_->addAxis(chart_y_axis_, Qt::AlignLeft);
    chart_series_->attachAxis(chart_y_axis_);

    chart_->zoomReset();
    //chart_->setTitle("Simple barchart example");

    loginf << "HistogramViewDataWidget: updateChart: done";
}

void HistogramViewDataWidget::updateResults()
{
    loginf << "HistogramViewDataWidget: updateResults";

    EvaluationManager& eval_man = COMPASS::instance().evaluationManager();

    if (eval_man.hasResults() && view_->showResults())
    {
        counts_.clear();
        data_null_cnt_.clear();
        labels_.clear();

        max_bin_cnt_ = 0;

        data_min_.clear();
        data_max_.clear();

        bin_size_valid_ = false;
        bin_size_ = 0;

        string eval_grpreq = view_->evalResultGrpReq();
        string eval_id = view_->evalResultsID();

        // check if ids are set
        if (!eval_grpreq.size() || !eval_id.size())
            return;

        const std::map<std::string, std::map<std::string, std::shared_ptr<EvaluationRequirementResult::Base>>>& results =
                eval_man.results();

        // check if ids are in result
        if (!results.count(eval_grpreq) || !results.at(eval_grpreq).count(eval_id))
        {
            logwrn << "HistogramViewDataWidget: updateResults: ids set but not in results";
            return;
        }

        std::shared_ptr<EvaluationRequirementResult::Base> result = results.at(eval_grpreq).at(eval_id);
        assert (result);
        updateFromResult(result);
    }

//    osg_layer_model_->beginResetModel();
//    osg_layer_model_->resultsItem().update();
//    osg_layer_model_->endResetModel();

//    drawSlot();
}

void HistogramViewDataWidget::calculateGlobalMinMax()
{
    loginf << "HistogramViewDataWidget: calculateGlobalMinMax";

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
            if (!buffer->has<bool>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: calculateGlobalMinMax: buffer does not contain " << current_var_name;
                return;
            }

            assert(buffer->has<bool>(current_var_name));
            //NullableVector<bool>& data = buffer->get<bool>(current_var_name);
            data_min_ = false;
            data_max_ = true;

            break;
        }
        case PropertyDataType::CHAR:
        {
            if (!buffer->has<char>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: calculateGlobalMinMax: buffer does not contain " << current_var_name;
                return;
            }

            assert(buffer->has<char>(current_var_name));
            NullableVector<char>& data = buffer->get<char>(current_var_name);
            updateMinMax (data);

            break;
        }
        case PropertyDataType::UCHAR:
        {
            if (!buffer->has<unsigned char>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: calculateGlobalMinMax: buffer does not contain " << current_var_name;
                return;
            }

            assert(buffer->has<unsigned char>(current_var_name));
            NullableVector<unsigned char>& data = buffer->get<unsigned char>(current_var_name);
            updateMinMax (data);

            break;
        }
        case PropertyDataType::INT:
        {
            if (!buffer->has<int>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: calculateGlobalMinMax: buffer does not contain " << current_var_name;
                return;
            }

            assert(buffer->has<int>(current_var_name));
            NullableVector<int>& data = buffer->get<int>(current_var_name);
            updateMinMax (data);

            break;
        }
        case PropertyDataType::UINT:
        {
            if (!buffer->has<unsigned int>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: calculateGlobalMinMax: buffer does not contain " << current_var_name;
                return;
            }

            assert(buffer->has<unsigned int>(current_var_name));
            NullableVector<unsigned int>& data = buffer->get<unsigned int>(current_var_name);
            updateMinMax (data);

            break;
        }
        case PropertyDataType::LONGINT:
        {
            if (!buffer->has<long int>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: calculateGlobalMinMax: buffer does not contain " << current_var_name;
                return;
            }

            assert(buffer->has<long int>(current_var_name));
            NullableVector<long int>& data = buffer->get<long int>(current_var_name);
            updateMinMax (data);

            break;
        }
        case PropertyDataType::ULONGINT:
        {
            if (!buffer->has<unsigned long>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: calculateGlobalMinMax: buffer does not contain " << current_var_name;
                return;
            }

            assert(buffer->has<unsigned long>(current_var_name));
            NullableVector<unsigned long>& data = buffer->get<unsigned long>(current_var_name);
            updateMinMax (data);

            break;
        }
        case PropertyDataType::FLOAT:
        {
            if (!buffer->has<float>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: calculateGlobalMinMax: buffer does not contain " << current_var_name;
                return;
            }

            assert(buffer->has<float>(current_var_name));
            NullableVector<float>& data = buffer->get<float>(current_var_name);
            updateMinMax (data);

            break;
        }
        case PropertyDataType::DOUBLE:
        {
            if (!buffer->has<double>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: calculateGlobalMinMax: buffer does not contain " << current_var_name;
                return;
            }

            assert(buffer->has<double>(current_var_name));
            NullableVector<double>& data = buffer->get<double>(current_var_name);
            updateMinMax (data);

            break;
        }
        case PropertyDataType::STRING:
        {
            if (!buffer->has<string>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: calculateGlobalMinMax: buffer does not contain " << current_var_name;
                return;
            }

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

void HistogramViewDataWidget::updateMinMax(const std::vector<double>& data)
{
    bool min_max_set {false};
    double data_min, data_max;

    if (data_min_.isValid() && data_max_.isValid())
    {
        data_min = data_min_.toDouble();
        data_max = data_max_.toDouble();
        min_max_set = true;
    }

    for (auto val_it : data)
    {
        if (!min_max_set)
        {
            data_min = val_it;
            data_max = val_it;
            min_max_set = true;
        }
        else
        {
            data_min = std::min(data_min, val_it);
            data_max = std::max(data_max, val_it);
        }
    }

    if (min_max_set) // min max must be set
    {
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
}

void HistogramViewDataWidget::updateCounts(const std::vector<double>& data)
{
    loginf << "HistogramViewDataWidget: updateCounts: from result";

    if (!bin_size_valid_)
    {
        logerr << "HistogramViewDataWidget: updateCounts: no bin size set";
        return;
    }

    if (!labels_.size()) // set labels
    {
        for (unsigned int bin_cnt = 0; bin_cnt < num_bins_; ++bin_cnt)
        {
//            if (data_var->representation() != DBOVariable::Representation::STANDARD)
//                labels_.push_back(data_var->getAsSpecialRepresentationString(
//                                      data_min_.toDouble()+bin_cnt*bin_size_+bin_size_/2.0f));
//            else
                labels_.push_back(std::to_string(data_min_.toDouble()+bin_cnt*bin_size_+bin_size_/2.0f));

        }
    }

    string dbo_name = COMPASS::instance().evaluationManager().dboNameTst();
    std::vector<unsigned int>& counts = counts_[dbo_name];

    if (!counts.size()) // set 0 bins
    {
        for (unsigned int bin_cnt = 0; bin_cnt < num_bins_; ++bin_cnt)
            counts.push_back(0);
    }

    unsigned int bin_number;
    unsigned int data_size = data.size();

    for (unsigned int cnt=0; cnt < data_size; ++cnt)
    {
        bin_number = (unsigned int) ((data.at(cnt)-data_min_.toDouble())/bin_size_);

        if (bin_number >= num_bins_)
            logerr << "HistogramViewDataWidget: updateFromData: bin_size " << bin_size_
                   << " bin number " << bin_number << " data " << data.at(cnt);

        assert (bin_number < num_bins_);
        counts.at(bin_number) += 1;
    }

    loginf << "HistogramViewDataWidget: updateCounts: end dbo " << dbo_name;
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
