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

using namespace std;
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
    : ViewDataWidget(parent, f), view_(view), data_source_(data_source)
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

    // shortcuts
    {
        QShortcut* space_shortcut = new QShortcut(QKeySequence(Qt::Key_Space), this);
        connect (space_shortcut, &QShortcut::activated, this, &HistogramViewDataWidget::resetZoomSlot);
    }
}

/**
 */
HistogramViewDataWidget::~HistogramViewDataWidget() = default;

/**
 */
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
    data_not_in_buffer_ = false;

    histogram_generator_.reset();
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

bool HistogramViewDataWidget::dataNotInBuffer() const
{
    return data_not_in_buffer_;
}

void HistogramViewDataWidget::loadingStartedSlot()
{
    clear();
    updateChart();
}

void HistogramViewDataWidget::updateDataSlot(const std::map<std::string, std::shared_ptr<Buffer>>& data, 
                                             bool requires_reset)
{
    logdbg << "HistogramViewDataWidget: updateDataSlot: start";

    buffers_ = data;

    histogram_generator_.reset(); //makes no sense any more

    logdbg << "HistogramViewDataWidget: updateDataSlot: end";
}

void HistogramViewDataWidget::loadingDoneSlot()
{
    updateToData();
}

void HistogramViewDataWidget::updateFromData(std::string dbcontent_name)
{
    loginf << "HistogramViewDataWidget: updateFromData: start dbo " << dbcontent_name;

    assert (buffers_.count(dbcontent_name));
    Buffer* buffer = buffers_.at(dbcontent_name).get();

    dbContent::Variable* data_var {nullptr};

    if (!view_->hasDataVar())
    {
        logwrn << "HistogramViewDataWidget: updateFromData: no data var";
        return;
    }

    if (view_->isDataVarMeta())
    {
        dbContent::MetaVariable& meta_var = view_->metaDataVar();
        if (!meta_var.existsIn(dbcontent_name))
        {
            logwrn << "HistogramViewDataWidget: updateFromData: meta var does not exist in dbo";
            return;
        }

        data_var = &meta_var.getFor(dbcontent_name);
    }
    else
    {
        data_var = &view_->dataVar();

        if (data_var->dbContentName() != dbcontent_name)
            return;
    }
    assert (data_var);

    PropertyDataType data_type = data_var->dataType();
    string current_var_name = data_var->name();

    assert (buffer->has<bool>(DBContent::selected_var.name()));
    NullableVector<bool>& selected_vec = buffer->get<bool>(DBContent::selected_var.name());

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

        updateCounts<bool> (dbcontent_name, data, selected_vec, data_var);

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

        updateCounts<char> (dbcontent_name, data, selected_vec, data_var);

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

        updateCounts<unsigned char> (dbcontent_name, data, selected_vec, data_var);

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

        updateCounts<int> (dbcontent_name, data, selected_vec, data_var);

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

        updateCounts<unsigned int> (dbcontent_name, data, selected_vec, data_var);

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

        updateCounts<long int> (dbcontent_name, data, selected_vec, data_var);

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

        updateCounts<unsigned long> (dbcontent_name, data, selected_vec, data_var);

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

        updateCounts<float> (dbcontent_name, data, selected_vec, data_var);

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

        updateCounts<double> (dbcontent_name, data, selected_vec, data_var);

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
    case PropertyDataType::JSON:
    {
        if (!buffer->has<nlohmann::json>(current_var_name))
        {
            loginf << "HistogramViewDataWidget: updateFromData: buffer does not contain " << current_var_name;
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
            loginf << "HistogramViewDataWidget: updateFromData: buffer does not contain " << current_var_name;
            return;
        }

        assert(buffer->has<boost::posix_time::ptime>(current_var_name));
        NullableVector<boost::posix_time::ptime>& data = buffer->get<boost::posix_time::ptime>(current_var_name);

        bool min_max_set {true};
        boost::posix_time::ptime data_min, data_max;

        tie(min_max_set, data_min, data_max) = data.minMaxValues();

        qlonglong data_min_q = static_cast<qlonglong>(Utils::Time::toLong(data_min));
        qlonglong data_max_q = static_cast<qlonglong>(Utils::Time::toLong(data_max));

        if (!min_max_set)
        {
            logwrn << "HistogramViewDataWidget: updateFromData: no data set in buffer";
            return;
        }

        if (!bin_size_valid_ ||
                (data_min_.isValid() && data_max_.isValid() && (data_min_q < data_min_.toLongLong()
                                                             || data_max_q > data_max_.toLongLong())))
        {
            updateFromAllData(); // clear, recalc min/max, update
            return;
        }

        updateCounts<boost::posix_time::ptime> (dbcontent_name, data, selected_vec, data_var);

        break;
    }
    default:
        logerr << "HistogramViewDataWidget: updateFromData: impossible for property type "
               << Property::asString(data_type);
        throw std::runtime_error(
                    "HistogramViewDataWidget: updateFromData: impossible property type " +
                    Property::asString(data_type));
    }

    loginf << "HistogramViewDataWidget: updateFromData: done dbo " << dbcontent_name;
}

void HistogramViewDataWidget::updateFromAllData()
{
    loginf << "HistogramViewDataWidget: updateFromAllData";

#ifdef USE_NEW_SHIT

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

    histogram_generator_->setVariable(data_var);
    histogram_generator_->setMetaVariable(meta_var);
    histogram_generator_->updateFromBuffers(buffers_);
    //histogram_generator_->print();
    
#else

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
    data_not_in_buffer_ = false;

    loginf << "HistogramViewDataWidget: updateFromAllData: num buffers " << buffers_.size();

    calculateGlobalMinMax();

    for (auto& buf_it : buffers_)
        updateFromData(buf_it.first);

#endif

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
        std::shared_ptr<EvaluationRequirementResult::SingleSpeed> result)
{
    assert (result);

    const vector<double>& values = result->values(); // distance values

    updateMinMax (values);
    updateCounts(values);
}

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

void HistogramViewDataWidget::updateChart()
{
    loginf << "HistogramViewDataWidget: updateChart";

#ifdef USE_NEW_SHIT

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
            addCount(set, bin.isSelected() ? 0 : bin.count);
        
        if (add_null)
            addCount(set, r.nullSelected() ? 0 : r.null_count);
        
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

    //update chart
    chart->update();

    shows_data_ = true;

#else

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
    bool add_null      = data_null_cnt_.size() || data_null_selected_cnt_;

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
        //tmp_chart_y_axis->setMinorTickCount(10);
        //tmp_chart_y_axis->setMinorTickCount(-1);
        tmp_chart_y_axis->setRange(10e-2, pow(10.0, 1+ceil(log10(max_bin_cnt_))));

        chart_y_axis = tmp_chart_y_axis;
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

#endif

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

void HistogramViewDataWidget::calculateGlobalMinMax()
{
    loginf << "HistogramViewDataWidget: calculateGlobalMinMax";

    for (auto& buf_it : buffers_)
    {
        Buffer* buffer = buf_it.second.get();
        string dbcontent_name = buf_it.first;

        dbContent::Variable* data_var {nullptr};

        if (!view_->hasDataVar())
            continue;

        if (view_->isDataVarMeta())
        {
            dbContent::MetaVariable& meta_var = view_->metaDataVar();
            if (!meta_var.existsIn(dbcontent_name))
                continue;

            data_var = &meta_var.getFor(dbcontent_name);
        }
        else
        {
            data_var = &view_->dataVar();

            if (data_var->dbContentName() != dbcontent_name)
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
                loginf << "HistogramViewDataWidget: calculateGlobalMinMax: buffer does not contain "
                       << current_var_name;
                data_not_in_buffer_ = true;
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
                loginf << "HistogramViewDataWidget: calculateGlobalMinMax: buffer does not contain "
                       << current_var_name;
                data_not_in_buffer_ = true;
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
                loginf << "HistogramViewDataWidget: calculateGlobalMinMax: buffer does not contain "
                       << current_var_name;
                data_not_in_buffer_ = true;
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
                loginf << "HistogramViewDataWidget: calculateGlobalMinMax: buffer does not contain "
                       << current_var_name;
                data_not_in_buffer_ = true;
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
                loginf << "HistogramViewDataWidget: calculateGlobalMinMax: buffer does not contain "
                       << current_var_name;
                data_not_in_buffer_ = true;
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
                loginf << "HistogramViewDataWidget: calculateGlobalMinMax: buffer does not contain "
                       << current_var_name;
                data_not_in_buffer_ = true;
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
                loginf << "HistogramViewDataWidget: calculateGlobalMinMax: buffer does not contain "
                       << current_var_name;
                data_not_in_buffer_ = true;
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
                loginf << "HistogramViewDataWidget: calculateGlobalMinMax: buffer does not contain "
                       << current_var_name;
                data_not_in_buffer_ = true;
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
                loginf << "HistogramViewDataWidget: calculateGlobalMinMax: buffer does not contain "
                       << current_var_name;
                data_not_in_buffer_ = true;
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
                loginf << "HistogramViewDataWidget: calculateGlobalMinMax: buffer does not contain "
                       << current_var_name;
                data_not_in_buffer_ = true;
                return;
            }

            assert(buffer->has<string>(current_var_name));
            NullableVector<string>& data = buffer->get<string>(current_var_name);
            updateMinMax (data);

            break;
        }
        case PropertyDataType::JSON:
        {
            if (!buffer->has<nlohmann::json>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: calculateGlobalMinMax: buffer does not contain "
                       << current_var_name;
                data_not_in_buffer_ = true;
                return;
            }

            assert(buffer->has<nlohmann::json>(current_var_name));
//            NullableVector<nlohmann::json>& data = buffer->get<nlohmann::json>(current_var_name);
//            updateMinMax (data);

            break;
        }
        case PropertyDataType::TIMESTAMP:
        {
            if (!buffer->has<boost::posix_time::ptime>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: calculateGlobalMinMax: buffer does not contain "
                       << current_var_name;
                data_not_in_buffer_ = true;
                return;
            }

            assert(buffer->has<boost::posix_time::ptime>(current_var_name));
            NullableVector<boost::posix_time::ptime>& data = buffer->get<boost::posix_time::ptime>(current_var_name);
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
        bin_size_ = binSize(data_min_, data_max_, num_bins_);
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

void HistogramViewDataWidget::updateMinMax(NullableVector<boost::posix_time::ptime>& data)
{
    bool min_max_set {true};
    boost::posix_time::ptime data_min, data_max;

    std::tie(min_max_set, data_min, data_max) = data.minMaxValues();

    auto data_min_num = (qlonglong)Utils::Time::toLong(data_min);
    auto data_max_num = (qlonglong)Utils::Time::toLong(data_max);

    QVariant min_var = QVariant::fromValue(data_min_num);
    QVariant max_var = QVariant::fromValue(data_max_num);

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
        bin_size_ = binSize(data_min_, data_max, num_bins_);
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

    string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();
    std::vector<unsigned int>& counts = counts_[dbcontent_name];

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

    loginf << "HistogramViewDataWidget: updateCounts: end dbo " << dbcontent_name;
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

void HistogramViewDataWidget::selectData(unsigned int index1, unsigned int index2)
{
    loginf << "HistogramViewDataWidget: rectangleSelectedSlot: index1 " << index1 << " index2 " << index2;

#ifdef USE_NEW_SHIT

    if (histogram_generator_)
        histogram_generator_->select(buffers_, index1, index2);

#else

    unsigned int min_index = min(index1, index2);
    unsigned int max_index = max(index1, index2);

    bool select_null    = max_index == num_bins_;
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

        QMessageBox m_warning(QMessageBox::Warning, "Selection Not Possible",
                              "Selection of evaluation results is not yet implemented.",
                              QMessageBox::Ok);
        m_warning.exec();

        return;
    }

    double val_min_zerobased =  min_index    * bin_size_;
    double val_max_zerobased = (max_index+1) * bin_size_;

    double val_min = val_min_zerobased + data_min_.toDouble();
    double val_max = val_max_zerobased + data_min_.toDouble();

    //bin_number = (unsigned int) ((data.get(cnt)-data_min_.toDouble())/bin_size_);

    loginf << "HistogramViewDataWidget: rectangleSelectedSlot: val_min " << val_min << " val_max " << val_max;

    dbContent::Variable* data_var {nullptr};

    if (!view_->hasDataVar())
    {
        logwrn << "HistogramViewDataWidget: rectangleSelectedSlot: no data var";
        return;
    }

    for (auto& buf_it : buffers_)
    {
        string dbcontent_name = buf_it.first;

        if (view_->isDataVarMeta())
        {
            dbContent::MetaVariable& meta_var = view_->metaDataVar();
            if (!meta_var.existsIn(dbcontent_name))
            {
                logwrn << "HistogramViewDataWidget: rectangleSelectedSlot: meta var does not exist in dbo";
                continue;
            }

            data_var = &meta_var.getFor(dbcontent_name);
        }
        else
        {
            data_var = &view_->dataVar();

            if (data_var->dbContentName() != dbcontent_name)
                continue;
        }
        assert (data_var);

        PropertyDataType data_type = data_var->dataType();
        string current_var_name = data_var->name();

        assert (buf_it.second->has<bool>(DBContent::selected_var.name()));
        NullableVector<bool>& selected_vec = buf_it.second->get<bool>(DBContent::selected_var.name());

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

            selectData<bool> (data, selected_vec, select_min_max, min_index, max_index, select_null, add_to_selection);

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

            selectData<char> (data, selected_vec, select_min_max, min_index, max_index, select_null, add_to_selection);

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

            selectData<unsigned char> (data, selected_vec, select_min_max, min_index, max_index, select_null, add_to_selection);

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

            selectData<int> (data, selected_vec, select_min_max, min_index, max_index, select_null, add_to_selection);

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

            selectData<unsigned int> (data, selected_vec, select_min_max, min_index, max_index, select_null, add_to_selection);

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

            selectData<long int> (data, selected_vec, select_min_max, min_index, max_index, select_null, add_to_selection);

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

            selectData<unsigned long> (data, selected_vec, select_min_max, min_index, max_index, select_null, add_to_selection);

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

            selectData<float> (data, selected_vec, select_min_max, min_index, max_index, select_null, add_to_selection);

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

            selectData<double> (data, selected_vec, select_min_max, min_index, max_index, select_null, add_to_selection);

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

            //TODO: write type-specific template specialization for selectData

            break;
        }
        case PropertyDataType::JSON:
        {
            if (!buffer->has<nlohmann::json>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: rectangleSelectedSlot: buffer does not contain "
                       << current_var_name;
                return;
            }

            assert(buffer->has<nlohmann::json>(current_var_name));
            //NullableVector<string>& data = buffer->get<string>(current_var_name);

            //TODO: write type-specific template specialization for selectData

            break;
        }
        case PropertyDataType::TIMESTAMP:
        {
            if (!buffer->has<boost::posix_time::ptime>(current_var_name))
            {
                loginf << "HistogramViewDataWidget: rectangleSelectedSlot: buffer does not contain "
                       << current_var_name;
                return;
            }

            assert(buffer->has<boost::posix_time::ptime>(current_var_name));
            NullableVector<boost::posix_time::ptime>& data = buffer->get<boost::posix_time::ptime>(current_var_name);

            selectData<boost::posix_time::ptime> (data, selected_vec, select_min_max, min_index, max_index, select_null, add_to_selection, &data_min_);

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

#endif

    emit view_->selectionChangedSignal();
}

void HistogramViewDataWidget::zoomToSubrange(unsigned int index1, unsigned int index2)
{
    if (histogram_generator_)
    {
        histogram_generator_->zoom(buffers_, index1, index2);

        updateChart();
    } 
}

void HistogramViewDataWidget::rectangleSelectedSlot (unsigned int index1, unsigned int index2)
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
void HistogramViewDataWidget::toolChanged_impl(int mode)
{
    selected_tool_ = (HistogramViewDataTool)mode;

    if (chart_view_)
        chart_view_->onToolChanged();
}
