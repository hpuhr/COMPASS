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
#include "histogramviewchartview.h"
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
#include <QShortcut>
#include <QApplication>

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

    main_layout_ = new QHBoxLayout();

    //    chart_series_ = new QBarSeries();

    //    chart_ = new QChart();
    //    chart_->addSeries(chart_series_);
    //    chart_->layout()->setContentsMargins(0, 0, 0, 0);
    //    chart_->setBackgroundRoundness(0);

    //    chart_->legend()->setVisible(true);
    //    chart_->legend()->setAlignment(Qt::AlignBottom);

    //    chart_view_ = new HistogramViewChartView(this, chart_);
    //    chart_view_->setRenderHint(QPainter::Antialiasing);
    //chart_view_->setRubberBand(QChartView::RectangleRubberBand);

    //    connect (chart_series_, &QBarSeries::clicked,
    //             chart_view_, &HistogramViewChartView::seriesPressedSlot);
    //    connect (chart_series_, &QBarSeries::released,
    //             chart_view_, &HistogramViewChartView::seriesReleasedSlot);

    //    connect (chart_view_, &HistogramViewChartView::rectangleSelectedSignal,
    //             this, &HistogramViewDataWidget::rectangleSelectedSlot, Qt::ConnectionType::QueuedConnection);

    //    layout->addWidget(chart_view_);

    setLayout(main_layout_);

    colors_["Radar"] = QColor("#00FF00");
    colors_["MLAT"] = QColor("#FF0000");
    colors_["ADSB"] = QColor("#6666FF");
    colors_["RefTraj"] = QColor("#FFA500");
    colors_["Tracker"] = QColor("#CCCCCC");

    // shortcuts
    {
        QShortcut* space_shortcut = new QShortcut(QKeySequence(Qt::Key_Space), this);
        connect (space_shortcut, &QShortcut::activated, this, &HistogramViewDataWidget::resetZoomSlot);
    }
}

HistogramViewDataWidget::~HistogramViewDataWidget()
{
}

void HistogramViewDataWidget::updateToData()
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
    selected_counts_.clear();
    data_null_cnt_.clear();
    data_null_selected_cnt_ = 0;
    labels_.clear();

    max_bin_cnt_ = 0;

    data_min_.clear();
    data_max_.clear();

    bin_size_valid_ = false;
    bin_size_ = 0;

    chart_view_.reset(nullptr);
    shows_data_ = false;
}

unsigned int HistogramViewDataWidget::numBins() const
{
    return num_bins_;
}

HistogramViewDataTool HistogramViewDataWidget::selectedTool() const
{
    return selected_tool_;
}

QCursor HistogramViewDataWidget::currentCursor() const
{
    return current_cursor_;
}

bool HistogramViewDataWidget::showsData() const
{
    return shows_data_;
}

QPixmap HistogramViewDataWidget::renderPixmap()
{
    assert (chart_view_);
    return chart_view_->grab();
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
    //updateFromData(object.name());

    logdbg << "HistogramViewDataWidget: updateDataSlot: end";
}

void HistogramViewDataWidget::loadingDoneSlot()
{
    //updateChart();
    updateToData();
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

    assert (buffer->has<bool>("selected"));
    NullableVector<bool>& selected_vec = buffer->get<bool>("selected");

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

        updateCounts<bool> (dbo_name, data, selected_vec, data_var);

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

        updateCounts<char> (dbo_name, data, selected_vec, data_var);

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

        updateCounts<unsigned char> (dbo_name, data, selected_vec, data_var);

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

        updateCounts<int> (dbo_name, data, selected_vec, data_var);

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

        updateCounts<unsigned int> (dbo_name, data, selected_vec, data_var);

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

        updateCounts<long int> (dbo_name, data, selected_vec, data_var);

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

        updateCounts<unsigned long> (dbo_name, data, selected_vec, data_var);

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


        updateCounts<float> (dbo_name, data, selected_vec, data_var);

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

        updateCounts<double> (dbo_name, data, selected_vec, data_var);

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
    selected_counts_.clear();
    data_null_cnt_.clear();
    data_null_selected_cnt_ = 0;
    labels_.clear();

    max_bin_cnt_ = 0;

    data_min_.clear();
    data_max_.clear();

    chart_view_.reset(nullptr);
    shows_data_ = false;

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
        updateCountResult(static_pointer_cast<SingleExtraData>(result));
    else if (result->type() == "JoinedExtraData")
        updateCountResult(static_pointer_cast<JoinedExtraData>(result));
    else if (result->type() == "SingleExtraTrack")
        updateCountResult(static_pointer_cast<SingleExtraTrack>(result));
    else if (result->type() == "JoinedExtraTrack")
        updateCountResult(static_pointer_cast<JoinedExtraTrack>(result));
    else if (result->type() == "SingleDetection")
        updateCountResult(static_pointer_cast<SingleDetection>(result));
    else if (result->type() == "JoinedDetection")
        updateCountResult(static_pointer_cast<JoinedDetection>(result));
    else if (result->type() == "SinglePositionDistance")
        updateCountResult(static_pointer_cast<SinglePositionDistance>(result));
    else if (result->type() == "JoinedPositionDistance")
        updateCountResult(static_pointer_cast<JoinedPositionDistance>(result));
    else if (result->type() == "SinglePositionAlong")
        updateCountResult(static_pointer_cast<SinglePositionAlong>(result));
    else if (result->type() == "JoinedPositionAlong")
        updateCountResult(static_pointer_cast<JoinedPositionAlong>(result));
    else if (result->type() == "SinglePositionAcross")
        updateCountResult(static_pointer_cast<SinglePositionAcross>(result));
    else if (result->type() == "JoinedPositionAcross")
        updateCountResult(static_pointer_cast<JoinedPositionAcross>(result));
    else if (result->type() == "SinglePositionLatency")
        updateCountResult(static_pointer_cast<SinglePositionLatency>(result));
    else if (result->type() == "JoinedPositionLatency")
        updateCountResult(static_pointer_cast<JoinedPositionLatency>(result));
    else if (result->type() == "SingleIdentification")
        updateCountResult(static_pointer_cast<SingleIdentification>(result));
    else if (result->type() == "JoinedIdentification")
        updateCountResult(static_pointer_cast<JoinedIdentification>(result));
    else if (result->type() == "SingleModeAPresent")
        updateCountResult(static_pointer_cast<SingleModeAPresent>(result));
    else if (result->type() == "JoinedModeAPresent")
        updateCountResult(static_pointer_cast<JoinedModeAPresent>(result));
    else if (result->type() == "SingleModeC")
        updateCountResult(static_pointer_cast<SingleModeC>(result));
    else if (result->type() == "JoinedModeC")
        updateCountResult(static_pointer_cast<JoinedModeC>(result));
    else
        throw runtime_error("HistogramViewDataWidget: updateFromResult: unknown result type");

    updateChart();
}

void HistogramViewDataWidget::updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleExtraData> result)
{
    logdbg << "HistogramViewDataWidget: showResult: single extra data";

    assert (result);

    string dbo_name = COMPASS::instance().evaluationManager().dboNameTst();

    if (!counts_.size()) // first
    {
        counts_[dbo_name].push_back(result->numOK()+result->numExtra());
        counts_[dbo_name].push_back(result->numOK());
        counts_[dbo_name].push_back(result->numExtra());

        labels_.push_back("#Check");
        labels_.push_back("#OK");
        labels_.push_back("#Extra");
    }
    else // add
    {
        counts_[dbo_name].at(0) += result->numOK()+result->numExtra();
        counts_[dbo_name].at(1) += result->numOK();
        counts_[dbo_name].at(2) += result->numExtra();
    }
}

void HistogramViewDataWidget::updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedExtraData> result)
{
    logdbg << "HistogramViewDataWidget: showResult: joined extra data";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleExtraData>(result_it));

        if (result_it->use())
            updateCountResult (static_pointer_cast<SingleExtraData>(result_it));
    }
}


void HistogramViewDataWidget::updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleExtraTrack> result)
{
    logdbg << "HistogramViewDataWidget: showResult: single extra track";

    assert (result);

    string dbo_name = COMPASS::instance().evaluationManager().dboNameTst();

    if (!counts_.size()) // first
    {
        counts_[dbo_name].push_back(result->numOK()+result->numExtra());
        counts_[dbo_name].push_back(result->numOK());
        counts_[dbo_name].push_back(result->numExtra());

        labels_.push_back("#Check");
        labels_.push_back("#OK");
        labels_.push_back("#Extra");
    }
    else // add
    {
        counts_[dbo_name].at(0) += result->numOK()+result->numExtra();
        counts_[dbo_name].at(1) += result->numOK();
        counts_[dbo_name].at(2) += result->numExtra();
    }
}


void HistogramViewDataWidget::updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedExtraTrack> result)
{
    logdbg << "HistogramViewDataWidget: showResult: joined track";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleExtraTrack>(result_it));

        if (result_it->use())
            updateCountResult (static_pointer_cast<SingleExtraTrack>(result_it));
    }
}


void HistogramViewDataWidget::updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleDetection> result)
{
    logdbg << "HistogramViewDataWidget: showResult: single detection";

    assert (result);

    string dbo_name = COMPASS::instance().evaluationManager().dboNameTst();

    if (!counts_.size()) // first
    {
        counts_[dbo_name].push_back(result->sumUIs());
        counts_[dbo_name].push_back(result->missedUIs());

        labels_.push_back("#EUIs");
        labels_.push_back("#MUIs");
    }
    else // add
    {
        counts_[dbo_name].at(0) += result->sumUIs();
        counts_[dbo_name].at(1) += result->missedUIs();
    }
}


void HistogramViewDataWidget::updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedDetection> result)
{
    logdbg << "HistogramViewDataWidget: showResult: joined detection";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleDetection>(result_it));

        if (result_it->use())
            updateCountResult (static_pointer_cast<SingleDetection>(result_it));
    }
}


void HistogramViewDataWidget::updateCountResult (
        std::shared_ptr<EvaluationRequirementResult::SinglePositionDistance> result)
{
    assert (result);

    const vector<double>& values = result->values(); // distance values

    updateMinMax (values);
    updateCounts(values);
}


void HistogramViewDataWidget::updateCountResult (
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
}


void HistogramViewDataWidget::updateCountResult (
        std::shared_ptr<EvaluationRequirementResult::SinglePositionAlong> result)
{
    assert (result);

    const vector<double>& values = result->values(); // along values

    updateMinMax (values);
    updateCounts(values);
}


void HistogramViewDataWidget::updateCountResult (
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
}


void HistogramViewDataWidget::updateCountResult (
        std::shared_ptr<EvaluationRequirementResult::SinglePositionAcross> result)
{
    assert (result);

    const vector<double>& values = result->values(); // across values

    updateMinMax (values);
    updateCounts(values);
}


void HistogramViewDataWidget::updateCountResult (
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
}


void HistogramViewDataWidget::updateCountResult (
        std::shared_ptr<EvaluationRequirementResult::SinglePositionLatency> result)
{
    assert (result);

    const vector<double>& values = result->values(); // latency values

    updateMinMax (values);
    updateCounts(values);
    updateChart();
}


void HistogramViewDataWidget::updateCountResult (
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
}


void HistogramViewDataWidget::updateCountResult (
        std::shared_ptr<EvaluationRequirementResult::SingleIdentification> result)
{
    logdbg << "HistogramViewDataWidget: showResult: single identification";

    assert (result);

    string dbo_name = COMPASS::instance().evaluationManager().dboNameTst();

    if (!counts_.size()) // first
    {
        counts_[dbo_name].push_back(result->numNoRefId());
        counts_[dbo_name].push_back(result->numUnknownId());
        counts_[dbo_name].push_back(result->numCorrectId());
        counts_[dbo_name].push_back(result->numFalseId());

        labels_.push_back("#NoRef");
        labels_.push_back("#UID");
        labels_.push_back("#CID");
        labels_.push_back("#FID");
    }
    else // add
    {
        counts_[dbo_name].at(0) += result->numNoRefId();
        counts_[dbo_name].at(1) += result->numUnknownId();
        counts_[dbo_name].at(2) += result->numCorrectId();
        counts_[dbo_name].at(3) += result->numFalseId();
    }
}


void HistogramViewDataWidget::updateCountResult (
        std::shared_ptr<EvaluationRequirementResult::JoinedIdentification> result)
{
    logdbg << "HistogramViewDataWidget: updateFromResult: joined identification";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleIdentification>(result_it));
        if (result_it->use())
            updateCountResult (static_pointer_cast<SingleIdentification>(result_it));
    }
}


void HistogramViewDataWidget::updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleModeAPresent> result)
{
    logdbg << "HistogramViewDataWidget: showResult: single mode a";

    assert (result);

    string dbo_name = COMPASS::instance().evaluationManager().dboNameTst();

    if (!counts_.size()) // first
    {
        counts_[dbo_name].push_back(result->numNoRefValue());
        counts_[dbo_name].push_back(result->numUnknown());
        counts_[dbo_name].push_back(result->numCorrect());
        counts_[dbo_name].push_back(result->numFalse());

        labels_.push_back("#NoRef");
        labels_.push_back("#Unknown");
        labels_.push_back("#Correct");
        labels_.push_back("#False");
    }
    else // add
    {
        counts_[dbo_name].at(0) += result->numNoRefValue();
        counts_[dbo_name].at(1) += result->numUnknown();
        counts_[dbo_name].at(2) += result->numCorrect();
        counts_[dbo_name].at(3) += result->numFalse();
    }
}


void HistogramViewDataWidget::updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedModeAPresent> result)
{
    logdbg << "HistogramViewDataWidget: showResult: joined mode 3/a";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleModeAPresent>(result_it));
        if (result_it->use())
            updateCountResult (static_pointer_cast<SingleModeAPresent>(result_it));
    }
}


void HistogramViewDataWidget::updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleModeC> result)
{
    logdbg << "HistogramViewDataWidget: showResult: single mode c";

    assert (result);

    string dbo_name = COMPASS::instance().evaluationManager().dboNameTst();

    if (!counts_.size()) // first
    {
        counts_[dbo_name].push_back(result->numNoRefValue());
        counts_[dbo_name].push_back(result->numUnknown());
        counts_[dbo_name].push_back(result->numCorrect());
        counts_[dbo_name].push_back(result->numFalse());

        labels_.push_back("#NoRef");
        labels_.push_back("#Unknown");
        labels_.push_back("#Correct");
        labels_.push_back("#False");
    }
    else // add
    {
        counts_[dbo_name].at(0) += result->numNoRefValue();
        counts_[dbo_name].at(1) += result->numUnknown();
        counts_[dbo_name].at(2) += result->numCorrect();
        counts_[dbo_name].at(3) += result->numFalse();
    }
}


void HistogramViewDataWidget::updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedModeC> result)
{
    logdbg << "HistogramViewDataWidget: showResult: joined mode c";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleModeC>(result_it));
        if (result_it->use())
            updateCountResult (static_pointer_cast<SingleModeC>(result_it));
    }
}

void HistogramViewDataWidget::updateChart()
{
    loginf << "HistogramViewDataWidget: updateChart";

    if (chart_view_)
        chart_view_.reset(nullptr);

    //    if (chart_x_axis_)
    //    {
    //        chart_series_->detachAxis(chart_x_axis_);
    //        delete chart_x_axis_;
    //        chart_x_axis_ = nullptr;
    //    }

    //    if (chart_y_axis_)
    //    {
    //        chart_series_->detachAxis(chart_y_axis_);
    //        delete chart_y_axis_;
    //        chart_y_axis_ = nullptr;
    //    }

    //chart_series_->clear();

    QBarSeries* chart_series = new QBarSeries();

    QChart* chart = new QChart();
    chart->addSeries(chart_series);
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    chart->setBackgroundRoundness(0);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    chart_view_.reset(new HistogramViewChartView(this, chart));
    chart_view_->setRenderHint(QPainter::Antialiasing);
    //chart_view_->setRubberBand(QChartView::RectangleRubberBand);

    //    connect (chart_series_, &QBarSeries::clicked,
    //             chart_view_, &HistogramViewChartView::seriesPressedSlot);
    //    connect (chart_series_, &QBarSeries::released,
    //             chart_view_, &HistogramViewChartView::seriesReleasedSlot);

    connect (chart_view_.get(), &HistogramViewChartView::rectangleSelectedSignal,
             this, &HistogramViewDataWidget::rectangleSelectedSlot, Qt::ConnectionType::QueuedConnection);

    main_layout_->addWidget(chart_view_.get());

    bool use_log_scale = view_->useLogScale();

    bool add_null = data_null_cnt_.size() || data_null_selected_cnt_;

    if (add_null && !data_null_cnt_.size()) // null to be shown, but no dbo counts, only selected
    {
        for (auto& cnt_it : counts_) // add zero counts to dbo
            data_null_cnt_[cnt_it.first] = 0;
    }

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
                *set << 10e-3; // Logarithms of zero and negative values are undefined.
            else
                *set << bin;

        }

        if (add_null)
        {
            if (use_log_scale && data_null_cnt_[cnt_it.first] == 0)
                *set << 10e-3;
            else
                *set << data_null_cnt_[cnt_it.first];
        }

        set->setColor(colors_[cnt_it.first]);
        chart_series->append(set);
    }

    if (selected_counts_.size() || data_null_selected_cnt_)
    {
        if (!selected_counts_.size()) // none selected
        {
            for (unsigned int cnt=0; cnt < num_bins_; ++cnt) // add zero counts to selected
                selected_counts_.push_back(0);
        }

        unsigned int sum_cnt {0};
        for (auto bin : selected_counts_)
            sum_cnt += bin;

        QBarSet *set = new QBarSet(("Selected ("+to_string(sum_cnt+data_null_selected_cnt_)+")").c_str());

        for (auto bin : selected_counts_)
            if (use_log_scale && bin == 0)
                *set << 10e-3; // Logarithms of zero and negative values are undefined.
            else
                *set << bin;

        if (add_null)
        {
            if (data_null_selected_cnt_ == 0)
                *set << 10e-3;
            else
                *set << data_null_selected_cnt_;
        }

        set->setColor(Qt::yellow); // darker than yellow #808000
        chart_series->append(set);
    }

    QStringList categories;

    for (auto lbl : labels_)
        categories << lbl.c_str();

    if (data_null_cnt_.size())
        categories << "NULL";
    //categories << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun";

    QBarCategoryAxis* chart_x_axis = new QBarCategoryAxis();

    if (view_->showResults())
        chart_x_axis->setTitleText((view_->evalResultGrpReq()+":"+view_->evalResultsID()).c_str());
    else
        chart_x_axis->setTitleText((view_->dataVarDBO()+": "+view_->dataVarName()).c_str());

    chart_x_axis->setLabelsAngle(85);
    chart_x_axis->append(categories);
    chart->addAxis(chart_x_axis, Qt::AlignBottom);
    chart_series->attachAxis(chart_x_axis);

    QAbstractAxis* chart_y_axis {nullptr};

    if (view_->useLogScale())
    {
        QLogValueAxis* tmp_chart_y_axis = new QLogValueAxis();

        tmp_chart_y_axis->setLabelFormat("%g");
        tmp_chart_y_axis->setBase(10.0);
        tmp_chart_y_axis->setMinorTickCount(-1);

        chart_y_axis = tmp_chart_y_axis;
        chart_y_axis->setRange(10e-2, max_bin_cnt_);
    }
    else
    {
        chart_y_axis = new QValueAxis();
        chart_y_axis->setRange(0, max_bin_cnt_);
    }
    assert (chart_y_axis);

    chart_y_axis->setTitleText("Count");

    chart->addAxis(chart_y_axis, Qt::AlignLeft);
    chart_series->attachAxis(chart_y_axis);

    chart->update();
    //chart_->setTitle("Simple barchart example");

    shows_data_ = true;

    loginf << "HistogramViewDataWidget: updateChart: done";
}

void HistogramViewDataWidget::updateResults()
{
    loginf << "HistogramViewDataWidget: updateResults";

    EvaluationManager& eval_man = COMPASS::instance().evaluationManager();

    if (eval_man.hasResults() && view_->showResults())
    {
        counts_.clear();
        selected_counts_.clear();
        data_null_selected_cnt_ = 0;
        data_null_cnt_.clear();
        labels_.clear();

        max_bin_cnt_ = 0;

        data_min_.clear();
        data_max_.clear();

        bin_size_valid_ = false;
        bin_size_ = 0;

        shows_data_ = false;

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

void HistogramViewDataWidget::mouseMoveEvent(QMouseEvent* event)
// for some reason not called, added to HistogramViewChartView::mouseMoveEvent
{
    loginf << "HistogramViewDataWidget: mouseMoveEvent";

    setCursor(current_cursor_);
    //osgEarth::QtGui::ViewerWidget::mouseMoveEvent(event);

    QWidget::mouseMoveEvent(event);
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

void HistogramViewDataWidget::exportDoneSlot(bool cancelled)
{
    emit exportDoneSignal(cancelled);
}

void HistogramViewDataWidget::resetZoomSlot()
{
    loginf << "HistogramViewDataWidget: resetZoomSlot";

    if (chart_view_ && chart_view_->chart())
        chart_view_->chart()->zoomReset();
}

void HistogramViewDataWidget::rectangleSelectedSlot (unsigned int index1, unsigned int index2)
{
    loginf << "HistogramViewDataWidget: rectangleSelectedSlot: index1 " << index1 << " index2 " << index2;

    unsigned int min_index = min(index1, index2);
    unsigned int max_index = max(index1, index2);

    bool select_null = max_index == num_bins_;

    bool select_min_max = !(min_index == max_index && max_index == num_bins_); // not b´oth num bins

    if (select_null)
        max_index -= 1;

    bool add_to_selection = QApplication::keyboardModifiers() & Qt::ControlModifier;

    loginf << "HistogramViewDataWidget: rectangleSelectedSlot: select_min_max " << select_min_max
           << " min_index " << min_index << " max_index " << max_index << " select_null " << select_null;

    if (!bin_size_valid_ )
    {
        loginf << "HistogramViewDataWidget: rectangleSelectedSlot: bin size not valid";
        return;
    }

    if (view_->showResults())
    {
        loginf << "HistogramViewDataWidget: rectangleSelectedSlot: selection of evalution results not available";
        return;
    }

    double val_min = min_index * bin_size_ + data_min_.toDouble();
    double val_max = (max_index+1) * bin_size_ + data_min_.toDouble();

    //bin_number = (unsigned int) ((data.get(cnt)-data_min_.toDouble())/bin_size_);

    loginf << "HistogramViewDataWidget: rectangleSelectedSlot: val_min " << val_min << " val_max " << val_max;

    DBOVariable* data_var {nullptr};

    if (!view_->hasDataVar())
    {
        logwrn << "HistogramViewDataWidget: rectangleSelectedSlot: no data var";
        return;
    }

    for (auto& buf_it : buffers_)
    {
        string dbo_name = buf_it.first;

        if (view_->isDataVarMeta())
        {
            MetaDBOVariable& meta_var = view_->metaDataVar();
            if (!meta_var.existsIn(dbo_name))
            {
                logwrn << "HistogramViewDataWidget: rectangleSelectedSlot: meta var does not exist in dbo";
                continue;
            }

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

        assert (buf_it.second->has<bool>("selected"));
        NullableVector<bool>& selected_vec = buf_it.second->get<bool>("selected");

        shared_ptr<Buffer> buffer = buf_it.second;

        switch (data_type)
        {
        case PropertyDataType::BOOL:
        {
            if (!buffer->has<bool>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: rectangleSelectedSlot: buffer does not contain "
                       << current_var_name;
                return;
            }

            assert(buffer->has<bool>(current_var_name));
            NullableVector<bool>& data = buffer->get<bool>(current_var_name);

            selectData<bool> (data, selected_vec, select_min_max, (bool) val_min, (bool) val_max,
                              select_null, add_to_selection);

            break;
        }
        case PropertyDataType::CHAR:
        {
            if (!buffer->has<char>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: rectangleSelectedSlot: buffer does not contain "
                       << current_var_name;
                return;
            }

            assert(buffer->has<char>(current_var_name));
            NullableVector<char>& data = buffer->get<char>(current_var_name);

            selectData<char> (data, selected_vec, select_min_max, (char) val_min, (char) val_max,
                              select_null, add_to_selection);

            break;
        }
        case PropertyDataType::UCHAR:
        {
            if (!buffer->has<unsigned char>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: rectangleSelectedSlot: buffer does not contain "
                       << current_var_name;
                return;
            }

            assert(buffer->has<unsigned char>(current_var_name));
            NullableVector<unsigned char>& data = buffer->get<unsigned char>(current_var_name);

            selectData<unsigned char> (data, selected_vec, select_min_max,
                                       (unsigned char) val_min, (unsigned char) val_max,
                                       select_null, add_to_selection);

            break;
        }
        case PropertyDataType::INT:
        {
            if (!buffer->has<int>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: rectangleSelectedSlot: buffer does not contain "
                       << current_var_name;
                return;
            }

            assert(buffer->has<int>(current_var_name));
            NullableVector<int>& data = buffer->get<int>(current_var_name);

            selectData<int> (data, selected_vec, select_min_max, (int) val_min, (int) val_max,
                             select_null, add_to_selection);

            break;
        }
        case PropertyDataType::UINT:
        {
            if (!buffer->has<unsigned int>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: rectangleSelectedSlot: buffer does not contain "
                       << current_var_name;
                return;
            }

            assert(buffer->has<unsigned int>(current_var_name));
            NullableVector<unsigned int>& data = buffer->get<unsigned int>(current_var_name);

            selectData<unsigned int> (data, selected_vec, select_min_max,
                                      (unsigned int) val_min, (unsigned int) val_max,
                                      select_null, add_to_selection);

            break;
        }
        case PropertyDataType::LONGINT:
        {
            if (!buffer->has<long int>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: rectangleSelectedSlot: buffer does not contain "
                       << current_var_name;
                return;
            }

            assert(buffer->has<long int>(current_var_name));
            NullableVector<long int>& data = buffer->get<long int>(current_var_name);

            selectData<long int> (data, selected_vec, select_min_max, (long int) val_min, (long int) val_max,
                                  select_null, add_to_selection);

            break;
        }
        case PropertyDataType::ULONGINT:
        {
            if (!buffer->has<unsigned long>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: rectangleSelectedSlot: buffer does not contain "
                       << current_var_name;
                return;
            }

            assert(buffer->has<unsigned long>(current_var_name));
            NullableVector<unsigned long>& data = buffer->get<unsigned long>(current_var_name);

            selectData<unsigned long> (data, selected_vec, select_min_max,
                                       (unsigned long) val_min, (unsigned long) val_max,
                                       select_null, add_to_selection);

            break;
        }
        case PropertyDataType::FLOAT:
        {
            if (!buffer->has<float>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: rectangleSelectedSlot: buffer does not contain "
                       << current_var_name;
                return;
            }

            assert(buffer->has<float>(current_var_name));
            NullableVector<float>& data = buffer->get<float>(current_var_name);

            selectData<float> (data, selected_vec, select_min_max, val_min, val_max, select_null, add_to_selection);

            break;
        }
        case PropertyDataType::DOUBLE:
        {
            if (!buffer->has<double>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: rectangleSelectedSlot: buffer does not contain "
                       << current_var_name;
                return;
            }

            assert(buffer->has<double>(current_var_name));
            NullableVector<double>& data = buffer->get<double>(current_var_name);

            selectData<double> (data, selected_vec, select_min_max, val_min, val_max, select_null, add_to_selection);

            break;
        }
        case PropertyDataType::STRING:
        {
            if (!buffer->has<string>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: rectangleSelectedSlot: buffer does not contain "
                       << current_var_name;
                return;
            }

            assert(buffer->has<string>(current_var_name));
            //NullableVector<string>& data = buffer->get<string>(current_var_name);

            break;
        }
        default:
            logerr << "HistogramViewDataWidget: rectangleSelectedSlot: impossible for property type "
                   << Property::asString(data_type);
            throw std::runtime_error(
                        "HistogramViewDataWidget: rectangleSelectedSlot: impossible property type " +
                        Property::asString(data_type));
        }
    }

    emit view_->selectionChangedSignal();
}

void HistogramViewDataWidget::invertSelectionSlot()
{
    loginf << "HistogramViewDataWidget: invertSelectionSlot";

    for (auto& buf_it : buffers_)
    {
        assert (buf_it.second->has<bool>("selected"));
        NullableVector<bool>& selected_vec = buf_it.second->get<bool>("selected");

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

void HistogramViewDataWidget::clearSelectionSlot()
{
    loginf << "HistogramViewDataWidget: clearSelectionSlot";

    for (auto& buf_it : buffers_)
    {
        assert (buf_it.second->has<bool>("selected"));
        NullableVector<bool>& selected_vec = buf_it.second->get<bool>("selected");

        for (unsigned int cnt=0; cnt < buf_it.second->size(); ++cnt)
            selected_vec.set(cnt, false);
    }

    emit view_->selectionChangedSignal();
}

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
