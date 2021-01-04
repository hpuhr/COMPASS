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
#include "dbovariable.h"
#include "histogramviewdatatoolwidget.h"
#include "histogramviewchartview.h"

#include <QWidget>
#include <QVariant>

#include <memory>

class HistogramView;
class HistogramViewDataSource;
class QTabWidget;
class QHBoxLayout;
class Buffer;
class DBObject;

namespace EvaluationRequirementResult
{
    class Base;
    class SingleExtraData;
    class JoinedExtraData;

    class SingleExtraTrack;
    class JoinedExtraTrack;

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

    class SingleIdentification;
    class JoinedIdentification;

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
class HistogramViewDataWidget : public QWidget
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
    void updateDataSlot(DBObject& object, std::shared_ptr<Buffer> buffer);
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

    QPixmap renderPixmap();

protected:
    HistogramView* view_{nullptr};
    /// Data source
    HistogramViewDataSource* data_source_{nullptr};

    std::map<std::string, std::shared_ptr<Buffer>> buffers_;

    unsigned int num_bins_{20};
    std::map<std::string, std::vector<unsigned int>> counts_;
    std::vector<unsigned int> selected_counts_;

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
    HistogramViewDataTool selected_tool_{HG_SELECT_TOOL};

    QHBoxLayout* main_layout_ {nullptr};

    std::unique_ptr<QtCharts::HistogramViewChartView> chart_view_;

    bool shows_data_ {false};

    void updateFromData(std::string dbo_name);
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
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleIdentification> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedIdentification> result);
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
    void updateMinMax(NullableVector<T>& data)
    {
        bool min_max_set {true};
        T data_min, data_max;

        std::tie(min_max_set, data_min, data_max) = data.minMaxValues();

//        loginf << "UGA min_max_set " << min_max_set << " data_min " << data_min << " data_max " << data_max;

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
//        loginf << "UGA2 data_min_ " << data_min_.toString().toStdString()
//               << " data_max_ " << data_max_.toString().toStdString();
    }

    void updateMinMax(NullableVector<std::string>& data);
    void updateMinMax(NullableVector<long int>& data);
    void updateMinMax(NullableVector<unsigned long int>& data);
    void updateMinMax(const std::vector<double>& data);

    template<typename T>
    void updateCounts(const std::string& dbo_name, NullableVector<T>& data, NullableVector<bool>& selected_vec,
                      DBOVariable* data_var)
    {
        loginf << "HistogramViewDataWidget: updateCounts: start dbo " << dbo_name;

        if (!bin_size_valid_)
        {
            logerr << "HistogramViewDataWidget: updateCounts: no bin size set";
            return;
        }

        if (!labels_.size()) // set labels
        {
            for (unsigned int bin_cnt = 0; bin_cnt < num_bins_; ++bin_cnt)
            {
                if (data_var->representation() != DBOVariable::Representation::STANDARD)
                    labels_.push_back(data_var->getAsSpecialRepresentationString(
                                          data_min_.toDouble()+bin_cnt*bin_size_+bin_size_/2.0f));
                else
                    labels_.push_back(std::to_string(data_min_.toDouble()+bin_cnt*bin_size_+bin_size_/2.0f));

            }
        }

        std::vector<unsigned int>& counts = counts_[dbo_name];

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
                    ++data_null_cnt_[dbo_name];
                continue;
            }

            bin_number = (unsigned int) ((data.get(cnt)-data_min_.toDouble())/bin_size_);

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

        loginf << "HistogramViewDataWidget: updateCounts: end dbo " << dbo_name;
    }

    void updateCounts(const std::vector<double>& data);

    template<typename T>
    void selectData(NullableVector<T>& data, NullableVector<bool>& selected_vec,
                      bool select_min_max, T data_min, T data_max, bool select_null, bool add_to_selection)
    {
        loginf << "HistogramViewDataWidget: selectData: data_min " << data_min << " data_max " << data_max
               << " select_null " << select_null << " add_to_selection " << add_to_selection;

        unsigned int data_size = data.size();
        T value;
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
                    selected_vec.set(cnt, false);

                continue;
            }

            if (!select_min_max)
                continue;

            value = data.get(cnt);

            select = value >= data_min && value < data_max;

            if (!select && add_to_selection && !selected_vec.isNull(cnt))
                select = selected_vec.get(cnt);

            selected_vec.set(cnt, select);

            if (select)
                ++select_cnt;
        }

        loginf << "HistogramViewDataWidget: selectData: select_cnt: " << select_cnt;
    }
};

#endif /* HISTOGRAMVIEWDATAWIDGET_H_ */
