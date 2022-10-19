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

#ifndef HISTOGRAMVIEWDATAWIDGET_H_
#define HISTOGRAMVIEWDATAWIDGET_H_

#include "global.h"
#include "nullablevector.h"
#include "dbcontent/variable/variable.h"
#include "histogramviewdatatoolwidget.h"
#include "histogramviewchartview.h"
#include "viewdatawidget.h"
#include "histogram.h"

#include <QVariant>

#include <memory>

class HistogramView;
class HistogramViewDataSource;
class QTabWidget;
class QHBoxLayout;
class Buffer;
class DBContent;
class HistogramGenerator;

namespace EvaluationRequirementResult
{
    class Base;
    class SingleExtraData;
    class JoinedExtraData;

    class SingleExtraTrack;
    class JoinedExtraTrack;

    class SingleDubiousTrack;
    class JoinedDubiousTrack;

    class SingleDubiousTarget;
    class JoinedDubiousTarget;

    class SingleDetection;
    class JoinedDetection;

    class SinglePositionDistance;
    class JoinedPositionDistance;

    class SinglePositionAlong;
    class JoinedPositionAlong;

    class SinglePositionAcross;
    class JoinedPositionAcross;

    class SinglePositionLatency;
    class JoinedPositionLatency;

    class SingleSpeed;
    class JoinedSpeed;

    class SingleIdentificationCorrect;
    class JoinedIdentificationCorrect;
    class SingleIdentificationFalse;
    class JoinedIdentificationFalse;

    class SingleModeAPresent;
    class JoinedModeAPresent;
    class SingleModeAFalse;
    class JoinedModeAFalse;

    class SingleModeCPresent;
    class JoinedModeCPresent;
    class SingleModeCFalse;
    class JoinedModeCFalse;
}

//namespace QtCharts {
//    class HistogramViewChartView;
//}

/**
 * @brief Widget with tab containing BufferTableWidgets in HistogramView
 *
 */
class HistogramViewDataWidget : public ViewDataWidget
{
    Q_OBJECT

  signals:
    void exportDoneSignal(bool cancelled);
//    void showOnlySelectedSignal(bool value);
//    void usePresentationSignal(bool use_presentation);
//    void showAssociationsSignal(bool value);

  public slots:
    void loadingStartedSlot();
    /// @brief Called when new result Buffer was delivered
    void updateDataSlot(const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset);
    void loadingDoneSlot();

    void exportDataSlot(bool overwrite);
    void exportDoneSlot(bool cancelled);

    void resetZoomSlot();

    //void toolChangedSlot(ScatterPlotViewDataTool selected, QCursor cursor);
    void rectangleSelectedSlot (unsigned int index1, unsigned int index2);

    void invertSelectionSlot();
    void clearSelectionSlot();

//    void showOnlySelectedSlot(bool value);
//    void usePresentationSlot(bool use_presentation);
//    void showAssociationsSlot(bool value);

  public:
    /// @brief Constructor
    HistogramViewDataWidget(HistogramView* view, HistogramViewDataSource* data_source,
                          QWidget* parent = nullptr, Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~HistogramViewDataWidget();

    void updateToData();
    void updateChart();

    void updateResults();

    void clear();

    unsigned int numBins() const;

    HistogramViewDataTool selectedTool() const;
    QCursor currentCursor() const;

    bool showsData() const;
    bool dataNotInBuffer() const;

    QPixmap renderPixmap();

protected:
    static const unsigned int NumBins     = 20;
    static const int          LabelAngleX = 85;

    static const QColor ColorSelected;
    static const QColor ColorCAT001;
    static const QColor ColorCAT010;
    static const QColor ColorCAT020;
    static const QColor ColorCAT021;
    static const QColor ColorCAT048;
    static const QColor ColorCAT062;
    static const QColor ColorRefTraj;

    struct HistogramData
    {
        std::vector<unsigned int> counts;
        std::vector<std::string>  labels;
        unsigned int              null_values     = 0;
        unsigned int              selected_values = 0;
    };

    HistogramView* view_{nullptr};
    /// Data source
    HistogramViewDataSource* data_source_{nullptr};

    std::map<std::string, std::shared_ptr<Buffer>> buffers_;

    unsigned int num_bins_{NumBins};
    std::map<std::string, std::vector<unsigned int>> counts_;
    std::vector<unsigned int> selected_counts_;

    std::map<std::string, HistogramData> histogram_data_;

    std::map<std::string, unsigned int> data_null_cnt_;
    unsigned int data_null_selected_cnt_{0};

    std::vector<std::string> labels_;
    unsigned int max_bin_cnt_ {0};
    QVariant data_min_;
    QVariant data_max_;

    bool bin_size_valid_ {false};
    double bin_size_;

    std::map<std::string, QColor> colors_;

    QCursor current_cursor_{Qt::CrossCursor};
    HistogramViewDataTool selected_tool_{HG_DEFAULT_TOOL};

    QHBoxLayout* main_layout_ {nullptr};

    std::unique_ptr<QtCharts::HistogramViewChartView> chart_view_;

    std::unique_ptr<HistogramGenerator> histogram_generator_;

    bool shows_data_ {false};
    bool data_not_in_buffer_ {false};

    virtual void toolChanged_impl(int mode) override;

    void selectData(unsigned int index1, unsigned int index2);
    void zoomToSubrange(unsigned int index1, unsigned int index2);

    void updateFromData(std::string dbcontent_name);
    void updateFromAllData();
    void updateFromResult(std::shared_ptr<EvaluationRequirementResult::Base> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleExtraData> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedExtraData> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleExtraTrack> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedExtraTrack> result);

    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleDetection> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedDetection> result);

    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SinglePositionDistance> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedPositionDistance> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SinglePositionAlong> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedPositionAlong> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SinglePositionAcross> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedPositionAcross> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SinglePositionLatency> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedPositionLatency> result);

    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleSpeed> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedSpeed> result);

    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleIdentificationCorrect> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedIdentificationCorrect> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleIdentificationFalse> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedIdentificationFalse> result);

    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleModeAPresent> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedModeAPresent> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleModeAFalse> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedModeAFalse> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleModeCPresent> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedModeCPresent> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleModeCFalse> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedModeCFalse> result);

    void calculateGlobalMinMax();

    virtual void mouseMoveEvent(QMouseEvent* event) override;

    template<typename T>
    void generateHistogram(const std::string& dbcontent_name, 
                           NullableVector<T>& data, 
                           NullableVector<bool>& selected_vec,
                           dbContent::Variable* data_var)
    {
        loginf << "HistogramViewDataWidget: generateHistogram: start dbcontent " << dbcontent_name;



        



        if (!labels_.size()) // set labels
        {
            for (unsigned int bin_cnt = 0; bin_cnt < num_bins_; ++bin_cnt)
            {
                const std::string s = stringRepresentation<T>(data_var, data_min_, bin_cnt, bin_size_);

                std::cout << "LABEL " << s << std::endl;

                labels_.push_back(s);
            }
        }

        std::vector<unsigned int>& counts = counts_[dbcontent_name];

        if (!counts.size()) // set 0 bins
        {
            for (unsigned int bin_cnt = 0; bin_cnt < num_bins_; ++bin_cnt)
                counts.push_back(0);
        }

        unsigned int bin_number;
        unsigned int data_size = data.size();
        bool selected;

        for (unsigned int cnt=0; cnt < data_size; ++cnt)
        {
            selected = !selected_vec.isNull(cnt) && selected_vec.get(cnt);

            if (data.isNull(cnt))
            {
                if (selected)
                    ++data_null_selected_cnt_;
                else
                    ++data_null_cnt_[dbcontent_name];
                continue;
            }

            //map to bin
            bin_number = binFromValue(data.get(cnt), data_min_, bin_size_);

            if (bin_number >= num_bins_)
                logerr << "HistogramViewDataWidget: updateFromData: bin_size " << bin_size_
                       << " bin number " << bin_number << " data " << data.get(cnt);

            assert (bin_number < num_bins_);

            if (selected) // is selected
            {
                if (!selected_counts_.size()) // set 0 bins
                {
                    for (unsigned int bin_cnt = 0; bin_cnt < num_bins_; ++bin_cnt)
                        selected_counts_.push_back(0);
                }

                selected_counts_.at(bin_number) += 1;
            }
            else
            {
                counts.at(bin_number) += 1;
            }
        }

        loginf << "HistogramViewDataWidget: updateCounts: end dbo " << dbcontent_name;
    }




    template<typename T>
    void updateMinMax(NullableVector<T>& data)
    {
        bool min_max_set {true};
        T data_min, data_max;

        std::tie(min_max_set, data_min, data_max) = data.minMaxValues();

        if (!min_max_set)
            return;

        QVariant min_var {data_min};
        QVariant max_var {data_max};

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

    void updateMinMax(NullableVector<std::string>& data);
    void updateMinMax(NullableVector<long int>& data);
    void updateMinMax(NullableVector<unsigned long int>& data);
    void updateMinMax(NullableVector<boost::posix_time::ptime>& data);
    void updateMinMax(const std::vector<double>& data);

    template<typename T>
    std::string stringRepresentation(dbContent::Variable* data_var, const QVariant& data_min, unsigned int bin, double bin_size)
    {
        std::string s;

        //per default just convert to string
        //TODO: solve the representation problem for all data types
        const double v = data_min.toDouble() + bin * bin_size + bin_size / 2.0f;
        s = std::to_string(v);

        return s;
    }

    template<typename T>
    unsigned int binFromValue(const T& value, const QVariant& data_min, double bin_size)
    {
        //per default compute double based
        return std::min(num_bins_ - 1, (unsigned int) ((value - data_min.toDouble())/bin_size_));
    }

    double binSize(const QVariant& data_min, const QVariant& data_max, unsigned int num_bins)
    {
        if (data_min.type() == QVariant::Type::LongLong)
        {
            return (data_max.toLongLong()-data_min.toLongLong())/((double)num_bins);
        }
        return (data_max.toDouble()-data_min.toDouble())/((double)num_bins);
    }

    template<typename T>
    void updateCounts(const std::string& dbcontent_name, 
                      NullableVector<T>& data, 
                      NullableVector<bool>& selected_vec,
                      dbContent::Variable* data_var)
    {
        loginf << "HistogramViewDataWidget: updateCounts: start dbcontent " << dbcontent_name;

        if (!bin_size_valid_)
        {
            logerr << "HistogramViewDataWidget: updateCounts: no bin size set";
            return;
        }

        if (!labels_.size()) // set labels
        {
            for (unsigned int bin_cnt = 0; bin_cnt < num_bins_; ++bin_cnt)
            {
                const std::string s = stringRepresentation<T>(data_var, data_min_, bin_cnt, bin_size_);

                logdbg << "HistogramViewDataWidget: updateCounts: LABEL " << s;

                labels_.push_back(s);
            }
        }

        std::vector<unsigned int>& counts = counts_[dbcontent_name];

        if (!counts.size()) // set 0 bins
        {
            for (unsigned int bin_cnt = 0; bin_cnt < num_bins_; ++bin_cnt)
                counts.push_back(0);
        }

        unsigned int bin_number;
        unsigned int data_size = data.size();
        bool selected;

        for (unsigned int cnt=0; cnt < data_size; ++cnt)
        {
            selected = !selected_vec.isNull(cnt) && selected_vec.get(cnt);

            if (data.isNull(cnt))
            {
                if (selected)
                    ++data_null_selected_cnt_;
                else
                    ++data_null_cnt_[dbcontent_name];
                continue;
            }

            //map to bin
            bin_number = binFromValue(data.get(cnt), data_min_, bin_size_);

//            if (bin_number >= num_bins_)
//                logerr << "HistogramViewDataWidget: updateFromData: bin_size " << bin_size_
//                       << " bin number " << bin_number << " data " << data.get(cnt);

            assert (bin_number < num_bins_);

            if (selected) // is selected
            {
                if (!selected_counts_.size()) // set 0 bins
                {
                    for (unsigned int bin_cnt = 0; bin_cnt < num_bins_; ++bin_cnt)
                        selected_counts_.push_back(0);
                }

                selected_counts_.at(bin_number) += 1;
            }
            else
            {
                counts.at(bin_number) += 1;
            }
        }

        loginf << "HistogramViewDataWidget: updateCounts: end dbo " << dbcontent_name;
    }

    void updateCounts(const std::vector<double>& data);

    /**
     * Select data based on numerical comparison with a double interval.
     * For non numerical comparisons specialize template.
     */
    template<typename T>
    void selectData(NullableVector<T>& data, 
                    NullableVector<bool>& selected_vec,
                    bool select_min_max, 
                    unsigned int bin_min, 
                    unsigned int bin_max, 
                    bool select_null, 
                    bool add_to_selection,
                    QVariant* global_minimum = nullptr)
    {
        loginf << "HistogramViewDataWidget: selectData: bin_min " << bin_min << " bin_max " << bin_max
               << " select_null " << select_null << " add_to_selection " << add_to_selection << " select_min_max " << select_min_max;

        unsigned int data_size = data.size();
        bool select;
        unsigned int select_cnt = 0;

        for (unsigned int cnt=0; cnt < data_size; ++cnt)
        {
            if (data.isNull(cnt))
            {
                if (select_null)
                {
                    selected_vec.set(cnt, true);
                    ++select_cnt;
                }
                else if (!add_to_selection)
                {
                    selected_vec.set(cnt, false);
                }
                continue;
            }

            if (!select_min_max)
            {
                //leave value "as is"
                if (!add_to_selection || selected_vec.isNull(cnt))
                    selected_vec.set(cnt, false);

                continue;
            }
                
            //mapping to the bin and checking on min max bin index prevents rounding errors 
            //(or at least shows the same rounding errors as when sorting into the bins)
            auto bin = binFromValue(data.get(cnt), data_min_, bin_size_);
            select = (bin >= bin_min && bin <= bin_max);

            if (!select && add_to_selection && !selected_vec.isNull(cnt))
                select = selected_vec.get(cnt);

            selected_vec.set(cnt, select);

            if (select)
                ++select_cnt;
        }

        loginf << "HistogramViewDataWidget: selectData: select_cnt: " << select_cnt;
    }
};

template<>
inline unsigned int HistogramViewDataWidget::binFromValue<boost::posix_time::ptime>(const boost::posix_time::ptime& value, 
                                                                                    const QVariant& data_min, 
                                                                                    double bin_size)
{
    qlonglong v = (qlonglong)Utils::Time::toLong(value);

    //prevent precision errors by only computing the offset to the minimum value as double
    return std::min(num_bins_ - 1, (unsigned int) ((double)(v - data_min.toLongLong())/bin_size_));
}

template<>
inline std::string HistogramViewDataWidget::stringRepresentation<boost::posix_time::ptime>(dbContent::Variable* data_var, 
                                                                                           const QVariant& data_min, 
                                                                                           unsigned int bin, 
                                                                                           double bin_size)
{
    //prevent precision errors by only computing the offset to the minimum value as double
    const qlonglong v = data_min.toLongLong() + (qlonglong)(bin * bin_size + bin_size / 2.0f);

    unsigned long v_long = static_cast<unsigned long>(v);

    return Utils::Time::toString(Utils::Time::fromLong(v_long));
}

template<>
inline std::string HistogramViewDataWidget::stringRepresentation<float>(dbContent::Variable* data_var, 
                                                                        const QVariant& data_min, 
                                                                        unsigned int bin, 
                                                                        double bin_size)
{
    const double v = data_min.toDouble() + bin * bin_size + bin_size / 2.0f;

    std::string s;

    //float values may have a special representation
    //TODO: solve the representation problem for all data types
    if (data_var->representation() != dbContent::Variable::Representation::STANDARD)
        s = data_var->getAsSpecialRepresentationString(v);
    else
        s = std::to_string(v);

    return s;
}

template<>
inline std::string HistogramViewDataWidget::stringRepresentation<double>(dbContent::Variable* data_var, 
                                                                         const QVariant& data_min, 
                                                                         unsigned int bin, 
                                                                         double bin_size)
{
    const double v = data_min.toDouble() + bin * bin_size + bin_size / 2.0f;

    std::string s;

    //double values may have a special representation
    //TODO: solve the representation problem for all data types
    if (data_var->representation() != dbContent::Variable::Representation::STANDARD)
        s = data_var->getAsSpecialRepresentationString(v);
    else
        s = std::to_string(v);

    return s;
}



#endif /* HISTOGRAMVIEWDATAWIDGET_H_ */
