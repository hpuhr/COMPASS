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
#include "histogramviewwidget.h"
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
#include "histogramgeneratorbuffer.h"
#include "histogramgeneratorresults.h"

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
HistogramViewDataWidget::HistogramViewDataWidget(HistogramViewWidget* view_widget, 
                                                 QWidget* parent, 
                                                 Qt::WindowFlags f)
:   ViewDataWidget(view_widget, parent, f)
{
    view_ = view_widget->getView();
    assert(view_);

    data_source_ = view_->getDataSource();
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

    updateChart();
}

/**
 */
HistogramViewDataWidget::~HistogramViewDataWidget() = default;

/**
 */
void HistogramViewDataWidget::clearData_impl()
{
    logdbg << "HistogramViewDataWidget: clearData_impl: start";

    chart_view_.reset();
    histogram_generator_.reset();

    data_not_in_buffer_ = false;

    logdbg << "HistogramViewDataWidget: clearData_impl: end";
}

/**
 */
void HistogramViewDataWidget::loadingStarted_impl()
{
    logdbg << "HistogramViewDataWidget: loadingStarted_impl: start";

    //nothing to do yet

    logdbg << "HistogramViewDataWidget: loadingStarted_impl: end";
}

/**
 */
void HistogramViewDataWidget::updateData_impl(bool requires_reset)
{
    logdbg << "HistogramViewDataWidget: updateData_impl: start";

    histogram_generator_.reset(); //current generator makes no sense any more

    logdbg << "HistogramViewDataWidget: updateData_impl: end";
}

/**
 */
void HistogramViewDataWidget::loadingDone_impl()
{
    logdbg << "HistogramViewDataWidget: loadingDone_impl: start";

    //default behavior
    ViewDataWidget::loadingDone_impl();

    logdbg << "HistogramViewDataWidget: loadingDone_impl: end";
}

/**
 */
void HistogramViewDataWidget::liveReload_impl()
{
    //implement live reload behavior here
}

/**
 */
bool HistogramViewDataWidget::redrawData_impl(bool recompute)
{
    logdbg << "HistogramViewDataWidget: redrawData_impl: start - recompute = " << recompute;

    if (recompute)
        updateGenerator();
    
    bool drawn = updateChart();

    logdbg << "HistogramViewDataWidget: redrawData_impl: end";

    return drawn;
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
void HistogramViewDataWidget::updateGenerator()
{
    if (view_->showResults())
        updateGeneratorFromResults();
    else
        updateGeneratorFromData();
}

/**
 */
void HistogramViewDataWidget::updateGeneratorFromData()
{
    loginf << "HistogramViewDataWidget: updateGeneratorFromData";

    histogram_generator_.reset();

    if (viewData().empty())
        return;

    data_not_in_buffer_ = false;

    //get variable
    dbContent::Variable*     data_var = nullptr;
    dbContent::MetaVariable* meta_var = nullptr;

    if (!view_->hasDataVar())
    {
        logwrn << "HistogramViewDataWidget: updateGeneratorFromData: no data var";
        return;
    }

    if (view_->isDataVarMeta())
        meta_var = &view_->metaDataVar();
    else
        data_var = &view_->dataVar();

    assert (meta_var || data_var);

    auto data_type = meta_var ? meta_var->dataType() : data_var->dataType();

    //create histogram generator of conrete data type matching the variable's
    switch (data_type)
    {
        case PropertyDataType::BOOL:
        {
            histogram_generator_.reset(new HistogramGeneratorBufferT<bool>(&viewData(), data_var, meta_var));
            break;
        }
        case PropertyDataType::CHAR:
        {
            histogram_generator_.reset(new HistogramGeneratorBufferT<char>(&viewData(), data_var, meta_var));
            break;
        }
        case PropertyDataType::UCHAR:
        {
            histogram_generator_.reset(new HistogramGeneratorBufferT<unsigned char>(&viewData(), data_var, meta_var));
            break;
        }
        case PropertyDataType::INT:
        {
            histogram_generator_.reset(new HistogramGeneratorBufferT<int>(&viewData(), data_var, meta_var));
            break;
        }
        case PropertyDataType::UINT:
        {
            histogram_generator_.reset(new HistogramGeneratorBufferT<unsigned int>(&viewData(), data_var, meta_var));
            break;
        }
        case PropertyDataType::LONGINT:
        {
            histogram_generator_.reset(new HistogramGeneratorBufferT<long int>(&viewData(), data_var, meta_var));
            break;
        }
        case PropertyDataType::ULONGINT:
        {
            histogram_generator_.reset(new HistogramGeneratorBufferT<unsigned long int>(&viewData(), data_var, meta_var));
            break;
        }
        case PropertyDataType::FLOAT:
        {
            histogram_generator_.reset(new HistogramGeneratorBufferT<float>(&viewData(), data_var, meta_var));
            break;
        }
        case PropertyDataType::DOUBLE:
        {
            histogram_generator_.reset(new HistogramGeneratorBufferT<double>(&viewData(), data_var, meta_var));
            break;
        }
        case PropertyDataType::STRING:
        {
            histogram_generator_.reset(new HistogramGeneratorBufferT<std::string>(&viewData(), data_var, meta_var));
            break;
        }
        case PropertyDataType::JSON:
        {
            //@TODO
            break;
        }
        case PropertyDataType::TIMESTAMP:
        {
            histogram_generator_.reset(new HistogramGeneratorBufferT<boost::posix_time::ptime>(&viewData(), data_var, meta_var));
            break;
        }
        default:
        {
            const std::string msg = "HistogramViewDataWidget: updateGeneratorFromData: impossible for property type " + Property::asString(data_type);

            logerr << msg;
            throw std::runtime_error(msg);
        }
    }

    if (histogram_generator_)
    {
        histogram_generator_->update();
        //histogram_generator_->print();

        HistogramGeneratorBuffer* generator = dynamic_cast<HistogramGeneratorBuffer*>(histogram_generator_.get());
        assert(generator);

        data_not_in_buffer_ = generator->dataNotInBuffer();
    }

    loginf << "HistogramViewDataWidget: updateFromAllData: done";
}

/**
 */
void HistogramViewDataWidget::updateGeneratorFromResults()
{
    loginf << "HistogramViewDataWidget: updateGeneratorFromResults";

    histogram_generator_.reset();

    EvaluationManager& eval_man = COMPASS::instance().evaluationManager();

    if (eval_man.hasResults() && view_->showResults())
    {
        data_not_in_buffer_ = false;

        string eval_grpreq = view_->evalResultGrpReq();
        string eval_id     = view_->evalResultsID();

        histogram_generator_.reset(new HistogramGeneratorResults(eval_grpreq, eval_id));
    }

    if (histogram_generator_)
        histogram_generator_->update();

    loginf << "HistogramViewDataWidget: updateGeneratorFromResults: done";
}

/**
 */
bool HistogramViewDataWidget::updateChart()
{
    loginf << "HistogramViewDataWidget: updateChart";

    chart_view_.reset(nullptr);

    //create chart
    QChart* chart = new QChart();
    chart->setBackgroundRoundness(0);
    chart->layout()->setContentsMargins(0, 0, 0, 0);

    bool show_results  = view_->showResults();
    bool use_log_scale = view_->useLogScale();

    QString x_axis_name;
    if (show_results)
        x_axis_name = QString((view_->evalResultGrpReq() + ":" + view_->evalResultsID()).c_str());
    else
        x_axis_name = QString((view_->dataVarDBO() + ": " + view_->dataVarName()).c_str());

    QString y_axis_name = "Count";

    chart->legend()->setAlignment(Qt::AlignBottom);

    //create bar series
    QBarSeries* chart_series = new QBarSeries();
    chart->addSeries(chart_series);

    //create x axis
    QBarCategoryAxis* chart_x_axis = new QBarCategoryAxis;
    chart_x_axis->setLabelsAngle(LabelAngleX);
    chart_x_axis->setTitleText(x_axis_name);

    chart->addAxis(chart_x_axis, Qt::AlignBottom);
    chart_series->attachAxis(chart_x_axis);

    //create y axis
    QAbstractAxis* chart_y_axis = nullptr;

    auto generateYAxis = [ & ] (bool log_scale, double max_count)
    {
        if (log_scale)
        {
            QLogValueAxis* tmp_chart_y_axis = new QLogValueAxis;
            tmp_chart_y_axis->setLabelFormat("%g");
            tmp_chart_y_axis->setBase(10.0);
            //tmp_chart_y_axis->setMinorTickCount(10);
            //tmp_chart_y_axis->setMinorTickCount(-1);
            tmp_chart_y_axis-> setRange(10e-2, std::pow(10.0, 1 + std::ceil(std::log10(max_count))));

            chart_y_axis = tmp_chart_y_axis;
        }
        else
        {
            chart_y_axis = new QValueAxis;
            chart_y_axis->setRange(0, (int)max_count);
        }
        assert (chart_y_axis);

        chart_y_axis->setTitleText(y_axis_name);

        chart->addAxis(chart_y_axis, Qt::AlignLeft);
        chart_series->attachAxis(chart_y_axis);
    };

    //we obtain valid data if a generator has been created and if the needed data is in the buffer
    bool has_data = (histogram_generator_ != nullptr && !dataNotInBuffer());

    if (has_data)
    {
        //data available
        
        chart->legend()->setVisible(true);
        
        const auto& results = histogram_generator_->getResults();

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
        for (const auto& elem : results.content_results)
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
        if (!results.content_results.empty())
        {
            const auto& r = results.content_results.begin()->second;
            for (const auto& b : r.bins)
                categories << QString::fromStdString(b.labels.label);
        }

        if (add_null)
            categories << "NULL";

        chart_x_axis->append(categories);

        //to generate a safe range we set max count to 1
        max_count = std::max(max_count, (unsigned)1);

        generateYAxis(use_log_scale, max_count);

        #if 0
        //add outliers to legend
        if (results.hasOutOfRangeValues())
        {
            auto outlier_count = results.not_inserted_count;

            const QString name = "Out of range: " + QString::number(outlier_count);

            chart_view_->addLegendOnlyItem(name, QColor(255, 255, 0));
        }
        #endif
    }
    else 
    {
        //no data, generate empty display

        chart->legend()->setVisible(false);

        //we need some bogus category in order to make the bar plot work
        chart_x_axis->append("Category"); 

        chart_x_axis->setLabelsVisible(false);
        chart_x_axis->setGridLineVisible(false);
        chart_x_axis->setMinorGridLineVisible(false);

        //just generate linear axis
        generateYAxis(false, 1);
        
        chart_y_axis->setLabelsVisible(false);
        chart_y_axis->setGridLineVisible(false);
        chart_y_axis->setMinorGridLineVisible(false);
    }

    //update chart
    chart->update();

    //create new chart view
    chart_view_.reset(new HistogramViewChartView(this, chart));

    //    connect (chart_series_, &QBarSeries::clicked,
    //             chart_view_, &HistogramViewChartView::seriesPressedSlot);
    //    connect (chart_series_, &QBarSeries::released,
    //             chart_view_, &HistogramViewChartView::seriesReleasedSlot);

    connect (chart_view_.get(), &HistogramViewChartView::rectangleSelectedSignal,
            this, &HistogramViewDataWidget::rectangleSelectedSlot, Qt::ConnectionType::QueuedConnection);

    main_layout_->addWidget(chart_view_.get());

    loginf << "HistogramViewDataWidget: updateChart: done";

    return has_data;
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
void HistogramViewDataWidget::clearSelectionSlot()
{
    loginf << "HistogramViewDataWidget: clearSelectionSlot";

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
void HistogramViewDataWidget::resetZoomSlot()
{
    loginf << "HistogramViewDataWidget: resetZoomSlot";

    if (histogram_generator_ && histogram_generator_->subRangeActive())
    {
        redrawData(true); //@TODO: maybe redrawData(false) may suffice?
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
HistogramViewDataWidget::ViewInfo HistogramViewDataWidget::getViewInfo() const
{
    if (!histogram_generator_ || !histogram_generator_->hasValidResult())
        return {};

    auto range       = histogram_generator_->currentRangeAsLabels();
    auto zoom_active = histogram_generator_->subRangeActive();

    const auto& results = histogram_generator_->getResults();
    
    ViewInfo vi;
    vi.min          = QString::fromStdString(range.first);
    vi.max          = QString::fromStdString(range.second);
    vi.out_of_range = results.not_inserted_count;
    vi.has_result   = true;
    vi.zoom_active  = zoom_active;

    return vi;
}
