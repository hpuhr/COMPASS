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

#pragma once

//#include "global.h"
#include "nullablevector.h"
//#include "dbcontent/variable/variable.h"
#include "scatterplotviewchartview.h"
#include "variableviewdatawidget.h"
#include "util/timeconv.h"

#include <QWidget>
#include <QVariant>

#include <memory>
#include <limits>

class ScatterPlotView;
class ScatterPlotViewWidget;
class ScatterPlotViewDataSource;

//class QTabWidget;
class QHBoxLayout;
class Buffer;
class DBContent;

namespace QtCharts {
    class QChart;
    class QScatterSeries;
    //class ScatterPlotViewChartView;
    class QChartView;
    class QBarCategoryAxis;
    class QValueAxis;
}

enum ScatterPlotViewDataTool
{
    SP_NAVIGATE_TOOL = 0,
    SP_ZOOM_RECT_TOOL,
    SP_SELECT_TOOL
};

/**
 * @brief Widget with tab containing BufferTableWidgets in ScatterPlotView
 *
 */
class ScatterPlotViewDataWidget : public VariableViewDataWidget
{
    Q_OBJECT
public:
    /// @brief Constructor
    ScatterPlotViewDataWidget(ScatterPlotViewWidget* view_widget,
                              QWidget* parent = nullptr, 
                              Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~ScatterPlotViewDataWidget();

    ScatterPlotViewDataTool selectedTool() const;

    QRectF getDataBounds() const;
    QPixmap renderPixmap();
    unsigned int nullValueCount() const;

    static const int ConnectLinesDataCountMax = 100000;

signals:
//    void showOnlySelectedSignal(bool value);
//    void usePresentationSignal(bool use_presentation);
//    void showAssociationsSignal(bool value);

public slots:
    void rectangleSelectedSlot(QPointF p1, QPointF p2);

    void invertSelectionSlot();
    void clearSelectionSlot();

//    void showOnlySelectedSlot(bool value);
//    void usePresentationSlot(bool use_presentation);
//    void showAssociationsSlot(bool value);

    void resetZoomSlot();

protected:
    typedef std::unique_ptr<QtCharts::ScatterPlotViewChartView> ChartViewPtr;

    virtual void mouseMoveEvent(QMouseEvent* event) override;

    virtual void toolChanged_impl(int mode) override;

    virtual bool postLoadTrigger() override final;
    virtual void resetVariableData() override final;
    virtual void resetVariableDisplay() override final;
    virtual void preUpdateVariableDataEvent() override final;
    virtual void postUpdateVariableDataEvent() override final;
    virtual bool updateVariableDisplay() override final;
    virtual void updateVariableData(const std::string& dbcontent_name,
                                    Buffer& buffer) override final;

    void viewInfoJSON_impl(nlohmann::json& info) const override;

private:
    void resetCounts();

    void updateMinMax();
    bool updateChart();
    void updateDataSeries(QtCharts::QChart* chart);

    void selectData (double x_min, double x_max, double y_min, double y_max);

    void updateVariableData(int var_idx, 
                            std::string dbcontent_name, 
                            unsigned int current_size);

    template<typename T>
    void appendData(NullableVector<T>& data, 
                    std::vector<double>& target, 
                    unsigned int last_size,
                    unsigned int current_size)
    {
        for (unsigned int cnt=last_size; cnt < current_size; ++cnt)
        {
            if (data.isNull(cnt))
                target.push_back(std::numeric_limits<double>::signaling_NaN());
            else
                target.push_back(data.get(cnt));
        }
    }

    ScatterPlotView*           view_       {nullptr};
    ScatterPlotViewDataSource* data_source_{nullptr};

    std::map<std::string, unsigned int>               buffer_x_counts_;
    std::map<std::string, unsigned int>               buffer_y_counts_;
    std::map<std::string, std::vector<double>>        x_values_;
    std::map<std::string, std::vector<double>>        y_values_;
    std::map<std::string, std::vector<bool>>          selected_values_;
    std::map<std::string, std::vector<unsigned long>> rec_num_values_;
    std::map<std::string, unsigned int>               dbcont_valid_counts_;

    unsigned int nan_value_cnt_ {0};
    unsigned int valid_cnt_     {0};
    unsigned int selected_cnt_  {0};

    bool has_x_min_max_ {false};
    double x_min_ {0}, x_max_ {0};

    bool has_y_min_max_ {false};
    double y_min_ {0}, y_max_ {0};

    ScatterPlotViewDataTool selected_tool_{SP_NAVIGATE_TOOL};

    QHBoxLayout* main_layout_ {nullptr};
    ChartViewPtr chart_view_  {nullptr};
};

/**
*/
template<>
inline void ScatterPlotViewDataWidget::appendData<boost::posix_time::ptime>(NullableVector<boost::posix_time::ptime>& data, 
                                                                            std::vector<double>& target, 
                                                                            unsigned int last_size,
                                                                            unsigned int current_size)
{
    for (unsigned int cnt=last_size; cnt < current_size; ++cnt)
    {
        if (data.isNull(cnt))
        {
            target.push_back(std::numeric_limits<double>::signaling_NaN());
            continue;
        }

        long t = Utils::Time::toLong(data.get(cnt));
            
        target.push_back(t);
    }
}

/**
*/
template<>
inline void ScatterPlotViewDataWidget::appendData<std::string>(NullableVector<std::string>& data, 
                                                               std::vector<double>& target, 
                                                               unsigned int last_size,
                                                               unsigned int current_size)
{
    throw std::runtime_error("ScatterPlotViewDataWidget: appendData: string not supported");
}
