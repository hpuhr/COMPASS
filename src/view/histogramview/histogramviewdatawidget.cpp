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
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/metavariable.h"
#include "histogramviewdatasource.h"
#include "histogramviewchartview.h"
#include "logger.h"
#include "evaluationmanager.h"
#include "histogramgenerator.h"

#include "eval/results/extra/datasingle.h"
#include "eval/results/extra/datajoined.h"
#include "eval/results/extra/tracksingle.h"
#include "eval/results/extra/trackjoined.h"

#include "eval/results/dubious/dubioustracksingle.h"
#include "eval/results/dubious/dubioustrackjoined.h"
#include "eval/results/dubious/dubioustargetsingle.h"
#include "eval/results/dubious/dubioustargetjoined.h"

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

#include "eval/results/speed/speedjoined.h"
#include "eval/results/speed/speedsingle.h"

#include "eval/results/identification/correctsingle.h"
#include "eval/results/identification/correctjoined.h"
#include "eval/results/identification/falsesingle.h"
#include "eval/results/identification/falsejoined.h"

#include "eval/results/mode_a/presentsingle.h"
#include "eval/results/mode_a/presentjoined.h"
#include "eval/results/mode_a/falsesingle.h"
#include "eval/results/mode_a/falsejoined.h"
#include "eval/results/mode_c/presentsingle.h"
#include "eval/results/mode_c/presentjoined.h"
#include "eval/results/mode_c/falsesingle.h"
#include "eval/results/mode_c/falsejoined.h"

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

#define USE_NEW_SHIT

using namespace EvaluationRequirementResult;

const QColor HistogramViewDataWidget::ColorSelected = Qt::yellow; // darker than yellow #808000
const QColor HistogramViewDataWidget::ColorCAT001   = QColor("#00FF00");
const QColor HistogramViewDataWidget::ColorCAT010   = QColor("#FFCC00");
const QColor HistogramViewDataWidget::ColorCAT020   = QColor("#FF0000");
const QColor HistogramViewDataWidget::ColorCAT021   = QColor("#6666FF");
const QColor HistogramViewDataWidget::ColorCAT048   = QColor("#00FF00");
const QColor HistogramViewDataWidget::ColorCAT062   = QColor("#CCCCCC");
const QColor HistogramViewDataWidget::ColorRefTraj  = QColor("#FFA500");

/**
 */
HistogramViewDataWidget::HistogramViewDataWidget(HistogramView* view, HistogramViewDataSource* data_source,
                                                 QWidget* parent, Qt::WindowFlags f)
:   ViewDataWidget(parent, f)
,   view_         (view)
,   data_source_  (data_source)
{
    assert(data_source_);

    setContentsMargins(0, 0, 0, 0);

    main_layout_ = new QHBoxLayout();

    setLayout(main_layout_);

    //@TODO: these strings should be defined globally
    colors_["CAT001" ] = ColorCAT001;
    colors_["CAT010" ] = ColorCAT010;
    colors_["CAT020" ] = ColorCAT020;
    colors_["CAT021" ] = ColorCAT021;
    colors_["CAT048" ] = ColorCAT048;
    colors_["CAT062" ] = ColorCAT062;
    colors_["RefTraj"] = ColorRefTraj;
}

/**
 */
HistogramViewDataWidget::~HistogramViewDataWidget() = default;

/**
 */
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
    data_not_in_buffer_ = false;

    histogram_generator_.reset();
}

/**
 */
void HistogramViewDataWidget::toolChanged_impl(int mode)
{
    selected_tool_ = (HistogramViewDataTool)mode;

    if (chart_view_)
        chart_view_->onToolChanged();
}

/**
 */
unsigned int HistogramViewDataWidget::numBins() const
{
    if (histogram_generator_)
        return histogram_generator_->currentBins();

    return 0;
}

/**
 */
HistogramViewDataTool HistogramViewDataWidget::selectedTool() const
{
    return selected_tool_;
}

/**
 */
QCursor HistogramViewDataWidget::currentCursor() const
{
    return current_cursor_;
}

/**
 */
bool HistogramViewDataWidget::showsData() const
{
    return shows_data_;
}

/**
 */
QPixmap HistogramViewDataWidget::renderPixmap()
{
    assert (chart_view_);
    return chart_view_->grab();
}

/**
 */
bool HistogramViewDataWidget::dataNotInBuffer() const
{
    return data_not_in_buffer_;
}

/**
 */
void HistogramViewDataWidget::loadingStartedSlot()
{
    clear();
    updateChart();
}

/**
 */
void HistogramViewDataWidget::updateDataSlot(const std::map<std::string, std::shared_ptr<Buffer>>& data, 
                                             bool requires_reset)
{
    logdbg << "HistogramViewDataWidget: updateDataSlot: start";

    buffers_ = data;

    histogram_generator_.reset(); //makes no sense any more

    logdbg << "HistogramViewDataWidget: updateDataSlot: end";
}

/**
 */
void HistogramViewDataWidget::loadingDoneSlot()
{
    updateToData();
}

/**
 */
void HistogramViewDataWidget::updateToData()
{
    loginf << "HistogramViewDataWidget: update";

    updateFromAllData();
    updateChart();
}

/**
 */
void HistogramViewDataWidget::updateFromAllData()
{
    loginf << "HistogramViewDataWidget: updateFromAllData";

    //get variable
    dbContent::Variable*     data_var = nullptr;
    dbContent::MetaVariable* meta_var = nullptr;

    if (!view_->hasDataVar())
    {
        logwrn << "HistogramViewDataWidget: updateFromAllData: no data var";
        return;
    }

    if (view_->isDataVarMeta())
        meta_var = &view_->metaDataVar();
    else
        data_var = &view_->dataVar();

    assert (meta_var || data_var);

    auto data_type = meta_var ? meta_var->dataType() : data_var->dataType();

    //create histogram generator of conrete data type matching the variable's
    histogram_generator_.reset();

    switch (data_type)
    {
        case PropertyDataType::BOOL:
        {
            histogram_generator_.reset(new HistogramGeneratorT<bool>);
            break;
        }
        case PropertyDataType::CHAR:
        {
            histogram_generator_.reset(new HistogramGeneratorT<char>);
            break;
        }
        case PropertyDataType::UCHAR:
        {
            histogram_generator_.reset(new HistogramGeneratorT<unsigned char>);
            break;
        }
        case PropertyDataType::INT:
        {
            histogram_generator_.reset(new HistogramGeneratorT<int>);
            break;
        }
        case PropertyDataType::UINT:
        {
            histogram_generator_.reset(new HistogramGeneratorT<unsigned int>);
            break;
        }
        case PropertyDataType::LONGINT:
        {
            histogram_generator_.reset(new HistogramGeneratorT<long int>);
            break;
        }
        case PropertyDataType::ULONGINT:
        {
            histogram_generator_.reset(new HistogramGeneratorT<unsigned long int>);
            break;
        }
        case PropertyDataType::FLOAT:
        {
            histogram_generator_.reset(new HistogramGeneratorT<float>);
            break;
        }
        case PropertyDataType::DOUBLE:
        {
            histogram_generator_.reset(new HistogramGeneratorT<double>);
            break;
        }
        case PropertyDataType::STRING:
        {
            histogram_generator_.reset(new HistogramGeneratorT<std::string>);
            break;
        }
        case PropertyDataType::JSON:
        {
            //@TODO
            break;
        }
        case PropertyDataType::TIMESTAMP:
        {
            histogram_generator_.reset(new HistogramGeneratorT<boost::posix_time::ptime>);
            break;
        }
        default:
        {
            const std::string msg = "HistogramViewDataWidget: updateFromAllData: impossible for property type " + Property::asString(data_type);

            logerr << msg;
            throw std::runtime_error(msg);
        }
    }

    if (!histogram_generator_)
        return;

    histogram_generator_->setBufferData(&buffers_);
    histogram_generator_->setVariable(data_var);
    histogram_generator_->setMetaVariable(meta_var);
    histogram_generator_->updateFromBufferData();
    //histogram_generator_->print();

    loginf << "HistogramViewDataWidget: updateFromAllData: done";
}

/**
 */
void HistogramViewDataWidget::updateChart()
{
    loginf << "HistogramViewDataWidget: updateChart";

    chart_view_.reset(nullptr);

    //create chart
    QChart* chart = new QChart();
    chart->setBackgroundRoundness(0);
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    //create bar series
    QBarSeries* chart_series = new QBarSeries();
    chart->addSeries(chart_series);

    //create chart view
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

    if (!histogram_generator_)
        return;

    const auto& results = histogram_generator_->getResults();

    bool show_results  = view_->showResults();
    bool use_log_scale = view_->useLogScale();
    bool add_null      = results.hasNullValues();
    bool has_selected  = results.hasSelectedValues();

    unsigned int max_count = 0;

    auto addCount = [ & ] (QBarSet* set, unsigned int count) 
    {
        if (count > max_count)
            max_count = count;

        if (use_log_scale && count == 0)
            *set << 10e-3; // Logarithms of zero and negative values are undefined.
        else
            *set << count;
    };

    //generate a bar set for each DBContent
    for (const auto& elem : results.db_content_results)
    {
        const auto& r = elem.second;

        const QString bar_legend_name = QString::fromStdString(elem.first) + " (" + QString::number(r.valid_count) + ")";

        QBarSet* set = new QBarSet(bar_legend_name);

        for (const auto& bin : r.bins)
            addCount(set, bin.count);
        
        if (add_null)
            addCount(set, r.null_count);
        
        set->setColor(colors_[elem.first]);
        chart_series->append(set);
    }

    //generate selected bar set
    if (has_selected)
    {
        const QString bar_legend_name = "Selected (" + QString::number(results.selected_count + results.null_selected_count) + ")";

        QBarSet* set = new QBarSet(bar_legend_name);

        for (auto bin : results.selected_counts)
            addCount(set, bin);

        if (add_null)
            addCount(set, results.null_selected_count);

        set->setColor(ColorSelected); 
        chart_series->append(set);
    }

    //create categories
    QStringList categories;
    if (!results.db_content_results.empty())
    {
        const auto& r = results.db_content_results.begin()->second;
        for (const auto& b : r.bins)
            categories << QString::fromStdString(b.label);
    }

    if (add_null)
        categories << "NULL";

    //create x axis
    QBarCategoryAxis* chart_x_axis = new QBarCategoryAxis;
    chart_x_axis->setLabelsAngle(LabelAngleX);
    chart_x_axis->append(categories);

    if (show_results)
        chart_x_axis->setTitleText((view_->evalResultGrpReq() + ":" + view_->evalResultsID()).c_str());
    else
        chart_x_axis->setTitleText((view_->dataVarDBO() + ": " + view_->dataVarName()).c_str());

    chart->addAxis(chart_x_axis, Qt::AlignBottom);
    chart_series->attachAxis(chart_x_axis);

    //create y axis
    QAbstractAxis* chart_y_axis = nullptr;

    if (use_log_scale)
    {
        QLogValueAxis* tmp_chart_y_axis = new QLogValueAxis();
        tmp_chart_y_axis->setLabelFormat("%g");
        tmp_chart_y_axis->setBase(10.0);
        //tmp_chart_y_axis->setMinorTickCount(10);
        //tmp_chart_y_axis->setMinorTickCount(-1);
        tmp_chart_y_axis->setRange(10e-2, std::pow(10.0, 1 + std::ceil(std::log10(max_count))));

        chart_y_axis = tmp_chart_y_axis;
    }
    else
    {
        chart_y_axis = new QValueAxis();
        chart_y_axis->setRange(0, max_count);
    }
    assert (chart_y_axis);

    chart_y_axis->setTitleText("Count");

    chart->addAxis(chart_y_axis, Qt::AlignLeft);
    chart_series->attachAxis(chart_y_axis);

    //add outliers to legend
    if (results.hasOutOfRangeValues())
    {
        auto outlier_count = results.not_inserted_count;

        const QString name = "Outliers: " + QString::number(outlier_count);

        chart_view_->addLegendOnlyItem(name, QColor(255, 255, 0));
    }

    //update chart
    chart->update();

    shows_data_ = true;

    loginf << "HistogramViewDataWidget: updateChart: done";
}

/**
 */
void HistogramViewDataWidget::exportDataSlot(bool overwrite)
{
    logdbg << "HistogramViewDataWidget: exportDataSlot";

}

/**
 */
void HistogramViewDataWidget::exportDoneSlot(bool cancelled)
{
    emit exportDoneSignal(cancelled);
}

/**
 */
void HistogramViewDataWidget::selectData(unsigned int index1, unsigned int index2)
{
    loginf << "HistogramViewDataWidget: rectangleSelectedSlot: index1 " << index1 << " index2 " << index2;

    if (histogram_generator_)
        histogram_generator_->select(index1, index2);

    //note: triggers a view update, otherwise a manual update would be needed
    emit view_->selectionChangedSignal();
}

/**
 */
void HistogramViewDataWidget::zoomToSubrange(unsigned int index1, unsigned int index2)
{
    if (histogram_generator_)
    {
        //zoom to bin range and refill with data
        histogram_generator_->zoom(index1, index2);

        updateChart();
    } 
}

/**
 */
void HistogramViewDataWidget::rectangleSelectedSlot(unsigned int index1, unsigned int index2)
{
    if (selected_tool_ == HG_SELECT_TOOL)
    {
        selectData(index1, index2);
    }
    else if (selected_tool_ == HG_ZOOM_TOOL)
    {
        zoomToSubrange(index1, index2);
    }
    endTool();
}

/**
 */
void HistogramViewDataWidget::invertSelectionSlot()
{
    loginf << "HistogramViewDataWidget: invertSelectionSlot";

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

/**
 */
void HistogramViewDataWidget::clearSelectionSlot()
{
    loginf << "HistogramViewDataWidget: clearSelectionSlot";

    for (auto& buf_it : buffers_)
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
void HistogramViewDataWidget::resetZoomSlot()
{
    loginf << "HistogramViewDataWidget: resetZoomSlot";

    if (histogram_generator_ && histogram_generator_->subRangeActive())
    {
        //@TODO: we could implement this in the histogram generator instead of a complete view update, 
        //but it would need a data refill anyway...
        updateToData();
    }
    else if (chart_view_ && chart_view_->chart())
    {
        //no bin zoom active, just reset the chart view
        //@TODO: actually not needed any more
        chart_view_->chart()->zoomReset();
    }
}

/**
 */
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

    else if (result->type() == "SingleDubiousTrack")
        ; //updateCountResult(static_pointer_cast<SingleExtraTrack>(result)); TODO
    else if (result->type() == "JoinedDubiousTrack")
        ; //updateCountResult(static_pointer_cast<JoinedExtraTrack>(result));
    else if (result->type() == "SingleDubiousTarget")
        ; //updateCountResult(static_pointer_cast<SingleExtraTrack>(result)); TODO
    else if (result->type() == "JoinedDubiousTarget")
        ; //updateCountResult(static_pointer_cast<JoinedExtraTrack>(result));

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

    else if (result->type() == "SingleSpeed")
        updateCountResult(static_pointer_cast<SingleSpeed>(result));
    else if (result->type() == "JoinedSpeed")
        updateCountResult(static_pointer_cast<JoinedSpeed>(result));

    else if (result->type() == "SingleIdentificationCorrect")
        updateCountResult(static_pointer_cast<SingleIdentificationCorrect>(result));
    else if (result->type() == "JoinedIdentificationCorrect")
        updateCountResult(static_pointer_cast<JoinedIdentificationCorrect>(result));
    else if (result->type() == "SingleIdentificationFalse")
        updateCountResult(static_pointer_cast<SingleIdentificationFalse>(result));
    else if (result->type() == "JoinedIdentificationFalse")
        updateCountResult(static_pointer_cast<JoinedIdentificationFalse>(result));

    else if (result->type() == "SingleModeAPresent")
        updateCountResult(static_pointer_cast<SingleModeAPresent>(result));
    else if (result->type() == "JoinedModeAPresent")
        updateCountResult(static_pointer_cast<JoinedModeAPresent>(result));
    else if (result->type() == "SingleModeAFalse")
        updateCountResult(static_pointer_cast<SingleModeAFalse>(result));
    else if (result->type() == "JoinedModeAFalse")
        updateCountResult(static_pointer_cast<JoinedModeAFalse>(result));

    else if (result->type() == "SingleModeCPresent")
        updateCountResult(static_pointer_cast<SingleModeCPresent>(result));
    else if (result->type() == "JoinedModeCPresent")
        updateCountResult(static_pointer_cast<JoinedModeCPresent>(result));
    else if (result->type() == "SingleModeCFalse")
        updateCountResult(static_pointer_cast<SingleModeCFalse>(result));
    else if (result->type() == "JoinedModeCFalse")
        updateCountResult(static_pointer_cast<JoinedModeCFalse>(result));
    else
        throw runtime_error("HistogramViewDataWidget: updateFromResult: unknown result type '"+result->type()+"'");

    updateChart();
}

/**
 */
void HistogramViewDataWidget::updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleExtraData> result)
{
    logdbg << "HistogramViewDataWidget: showResult: single extra data";

    assert (result);

    string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();

    if (!counts_.size()) // first
    {
        counts_[dbcontent_name].push_back(result->numOK()+result->numExtra());
        counts_[dbcontent_name].push_back(result->numOK());
        counts_[dbcontent_name].push_back(result->numExtra());

        labels_.push_back("#Check");
        labels_.push_back("#OK");
        labels_.push_back("#Extra");
    }
    else // add
    {
        counts_[dbcontent_name].at(0) += result->numOK()+result->numExtra();
        counts_[dbcontent_name].at(1) += result->numOK();
        counts_[dbcontent_name].at(2) += result->numExtra();
    }
}

/**
 */
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

/**
 */
void HistogramViewDataWidget::updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleExtraTrack> result)
{
    logdbg << "HistogramViewDataWidget: showResult: single extra track";

    assert (result);

    string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();

    if (!counts_.size()) // first
    {
        counts_[dbcontent_name].push_back(result->numOK()+result->numExtra());
        counts_[dbcontent_name].push_back(result->numOK());
        counts_[dbcontent_name].push_back(result->numExtra());

        labels_.push_back("#Check");
        labels_.push_back("#OK");
        labels_.push_back("#Extra");
    }
    else // add
    {
        counts_[dbcontent_name].at(0) += result->numOK()+result->numExtra();
        counts_[dbcontent_name].at(1) += result->numOK();
        counts_[dbcontent_name].at(2) += result->numExtra();
    }
}

/**
 */
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

/**
 */
void HistogramViewDataWidget::updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleDetection> result)
{
    logdbg << "HistogramViewDataWidget: showResult: single detection";

    assert (result);

    string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();

    if (!counts_.size()) // first
    {
        counts_[dbcontent_name].push_back(result->sumUIs());
        counts_[dbcontent_name].push_back(result->missedUIs());

        labels_.push_back("#EUIs");
        labels_.push_back("#MUIs");
    }
    else // add
    {
        counts_[dbcontent_name].at(0) += result->sumUIs();
        counts_[dbcontent_name].at(1) += result->missedUIs();
    }
}

/**
 */
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

/**
 */
void HistogramViewDataWidget::updateCountResult (
        std::shared_ptr<EvaluationRequirementResult::SinglePositionDistance> result)
{
    assert (result);

    const vector<double>& values = result->values(); // distance values

    updateMinMax (values);
    updateCounts(values);
}

/**
 */
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

/**
 */
void HistogramViewDataWidget::updateCountResult (
        std::shared_ptr<EvaluationRequirementResult::SinglePositionAlong> result)
{
    assert (result);

    const vector<double>& values = result->values(); // along values

    updateMinMax (values);
    updateCounts(values);
}

/**
 */
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

/**
 */
void HistogramViewDataWidget::updateCountResult (
        std::shared_ptr<EvaluationRequirementResult::SinglePositionAcross> result)
{
    assert (result);

    const vector<double>& values = result->values(); // across values

    updateMinMax (values);
    updateCounts(values);
}

/**
 */
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

/**
 */
void HistogramViewDataWidget::updateCountResult (
        std::shared_ptr<EvaluationRequirementResult::SinglePositionLatency> result)
{
    assert (result);

    const vector<double>& values = result->values(); // latency values

    updateMinMax (values);
    updateCounts(values);
    updateChart();
}

/**
 */
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

/**
 */
void HistogramViewDataWidget::updateCountResult (
        std::shared_ptr<EvaluationRequirementResult::SingleSpeed> result)
{
    assert (result);

    const vector<double>& values = result->values(); // distance values

    updateMinMax (values);
    updateCounts(values);
}

/**
 */
void HistogramViewDataWidget::updateCountResult (
        std::shared_ptr<EvaluationRequirementResult::JoinedSpeed> result)
{
    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleSpeed>(result_it));
        if (result_it->use())
            updateCountResult (static_pointer_cast<SingleSpeed>(result_it));
    }
}

/**
 */
void HistogramViewDataWidget::updateCountResult (
        std::shared_ptr<EvaluationRequirementResult::SingleIdentificationCorrect> result)
{
    logdbg << "HistogramViewDataWidget: showResult: single identification correct";

    assert (result);

    string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();

    if (!counts_.size()) // first
    {
        counts_[dbcontent_name].push_back(result->numNoRefId());
        counts_[dbcontent_name].push_back(result->numCorrect());
        counts_[dbcontent_name].push_back(result->numNotCorrect());

        labels_.push_back("#NoRef");
        labels_.push_back("#CID");
        labels_.push_back("#NCID");
    }
    else // add
    {
        counts_[dbcontent_name].at(0) += result->numNoRefId();
        counts_[dbcontent_name].at(1) += result->numCorrect();
        counts_[dbcontent_name].at(2) += result->numNotCorrect();
    }
}

/**
 */
void HistogramViewDataWidget::updateCountResult (
        std::shared_ptr<EvaluationRequirementResult::JoinedIdentificationCorrect> result)
{
    logdbg << "HistogramViewDataWidget: updateFromResult: joined identification correct";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleIdentificationCorrect>(result_it));
        if (result_it->use())
            updateCountResult (static_pointer_cast<SingleIdentificationCorrect>(result_it));
    }
}

/**
 */
void HistogramViewDataWidget::updateCountResult (
        std::shared_ptr<EvaluationRequirementResult::SingleIdentificationFalse> result)
{
    logdbg << "HistogramViewDataWidget: showResult: single identification false";

    assert (result);

    string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();

    if (!counts_.size()) // first
    {
        counts_[dbcontent_name].push_back(result->numNoRefValue());
        counts_[dbcontent_name].push_back(result->numUnknown());
        counts_[dbcontent_name].push_back(result->numCorrect());
        counts_[dbcontent_name].push_back(result->numFalse());

        labels_.push_back("#NoRef");
        labels_.push_back("#Unknown");
        labels_.push_back("#Correct");
        labels_.push_back("#False");
    }
    else // add
    {
        counts_[dbcontent_name].at(0) += result->numNoRefValue();
        counts_[dbcontent_name].at(1) += result->numUnknown();
        counts_[dbcontent_name].at(2) += result->numCorrect();
        counts_[dbcontent_name].at(3) += result->numFalse();
    }
}

/**
 */
void HistogramViewDataWidget::updateCountResult (
        std::shared_ptr<EvaluationRequirementResult::JoinedIdentificationFalse> result)
{
    logdbg << "HistogramViewDataWidget: updateFromResult: joined identification false";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleIdentificationFalse>(result_it));
        if (result_it->use())
            updateCountResult (static_pointer_cast<SingleIdentificationFalse>(result_it));
    }
}

/**
 */
void HistogramViewDataWidget::updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleModeAPresent> result)
{
    logdbg << "HistogramViewDataWidget: showResult: single mode a present";

    assert (result);

    string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();

    if (!counts_.size()) // first
    {
        counts_[dbcontent_name].push_back(result->numNoRefId());
        counts_[dbcontent_name].push_back(result->numPresent());
        counts_[dbcontent_name].push_back(result->numMissing());

        labels_.push_back("#NoRefId");
        labels_.push_back("#Present");
        labels_.push_back("#Missing");
    }
    else // add
    {
        counts_[dbcontent_name].at(0) += result->numNoRefId();
        counts_[dbcontent_name].at(1) += result->numPresent();
        counts_[dbcontent_name].at(2) += result->numMissing();
    }
}

/**
 */
void HistogramViewDataWidget::updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedModeAPresent> result)
{
    logdbg << "HistogramViewDataWidget: showResult: joined mode 3/a present";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleModeAPresent>(result_it));
        if (result_it->use())
            updateCountResult (static_pointer_cast<SingleModeAPresent>(result_it));
    }
}

/**
 */
void HistogramViewDataWidget::updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleModeAFalse> result)
{
    logdbg << "HistogramViewDataWidget: showResult: single mode a false";

    assert (result);

    string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();

    if (!counts_.size()) // first
    {
        counts_[dbcontent_name].push_back(result->numNoRefValue());
        counts_[dbcontent_name].push_back(result->numUnknown());
        counts_[dbcontent_name].push_back(result->numCorrect());
        counts_[dbcontent_name].push_back(result->numFalse());

        labels_.push_back("#NoRef");
        labels_.push_back("#Unknown");
        labels_.push_back("#Correct");
        labels_.push_back("#False");
    }
    else // add
    {
        counts_[dbcontent_name].at(0) += result->numNoRefValue();
        counts_[dbcontent_name].at(1) += result->numUnknown();
        counts_[dbcontent_name].at(2) += result->numCorrect();
        counts_[dbcontent_name].at(3) += result->numFalse();
    }
}

/**
 */
void HistogramViewDataWidget::updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedModeAFalse> result)
{
    logdbg << "HistogramViewDataWidget: showResult: joined mode 3/a false";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleModeAFalse>(result_it));
        if (result_it->use())
            updateCountResult (static_pointer_cast<SingleModeAFalse>(result_it));
    }
}

/**
 */
void HistogramViewDataWidget::updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleModeCPresent> result)
{
    logdbg << "HistogramViewDataWidget: showResult: single mode a present";

    assert (result);

    string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();

    if (!counts_.size()) // first
    {
        counts_[dbcontent_name].push_back(result->numNoRefC());
        counts_[dbcontent_name].push_back(result->numPresent());
        counts_[dbcontent_name].push_back(result->numMissing());

        labels_.push_back("#NoRefC");
        labels_.push_back("#Present");
        labels_.push_back("#Missing");
    }
    else // add
    {
        counts_[dbcontent_name].at(0) += result->numNoRefC();
        counts_[dbcontent_name].at(1) += result->numPresent();
        counts_[dbcontent_name].at(2) += result->numMissing();
    }
}

/**
 */
void HistogramViewDataWidget::updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedModeCPresent> result)
{
    logdbg << "HistogramViewDataWidget: showResult: joined mode 3/a present";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleModeCPresent>(result_it));
        if (result_it->use())
            updateCountResult (static_pointer_cast<SingleModeCPresent>(result_it));
    }
}

/**
 */
void HistogramViewDataWidget::updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleModeCFalse> result)
{
    logdbg << "HistogramViewDataWidget: showResult: single mode c";

    assert (result);

    string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();

    if (!counts_.size()) // first
    {
        counts_[dbcontent_name].push_back(result->numNoRefValue());
        counts_[dbcontent_name].push_back(result->numUnknown());
        counts_[dbcontent_name].push_back(result->numCorrect());
        counts_[dbcontent_name].push_back(result->numFalse());

        labels_.push_back("#NoRef");
        labels_.push_back("#Unknown");
        labels_.push_back("#Correct");
        labels_.push_back("#False");
    }
    else // add
    {
        counts_[dbcontent_name].at(0) += result->numNoRefValue();
        counts_[dbcontent_name].at(1) += result->numUnknown();
        counts_[dbcontent_name].at(2) += result->numCorrect();
        counts_[dbcontent_name].at(3) += result->numFalse();
    }
}

/**
 */
void HistogramViewDataWidget::updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedModeCFalse> result)
{
    logdbg << "HistogramViewDataWidget: showResult: joined mode c";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleModeCFalse>(result_it));
        if (result_it->use())
            updateCountResult (static_pointer_cast<SingleModeCFalse>(result_it));
    }
}


/**
 */
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
        data_not_in_buffer_ = false;

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

/**
 */
void HistogramViewDataWidget::mouseMoveEvent(QMouseEvent* event)
// for some reason not called, added to HistogramViewChartView::mouseMoveEvent
{
    // loginf << "HistogramViewDataWidget: mouseMoveEvent";

    // setCursor(current_cursor_);
    // //osgEarth::QtGui::ViewerWidget::mouseMoveEvent(event);

    // QWidget::mouseMoveEvent(event);
}

/**
 */
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
        bin_size_ = binSize(data_min_, data_max, num_bins_);
        bin_size_valid_ = true;
    }
    else
    {
        bin_size_ = 0;
        bin_size_valid_ = false;
    }
}

/**
 */
void HistogramViewDataWidget::updateCounts(const std::vector<double>& data)
{
    loginf << "HistogramViewDataWidget: updateCounts: from result, label size " << labels_.size();

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

    string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();
    std::vector<unsigned int>& counts = counts_[dbcontent_name];

    if (!counts.size()) // set 0 bins
    {
        for (unsigned int bin_cnt = 0; bin_cnt < num_bins_; ++bin_cnt)
            counts.push_back(0);
    }

    unsigned int bin_number;
    unsigned int data_size = data.size();

    assert (num_bins_);

    for (unsigned int cnt=0; cnt < data_size; ++cnt)
    {
        bin_number = (unsigned int) ((data.at(cnt)-data_min_.toDouble())/bin_size_);

        assert (bin_number <= num_bins_);

        if (bin_number == num_bins_) // check for 1 max value which is num_bins_
            bin_number = num_bins_ - 1;

        counts.at(bin_number) += 1;
    }

    loginf << "HistogramViewDataWidget: updateCounts: end dbo " << dbcontent_name;
}
