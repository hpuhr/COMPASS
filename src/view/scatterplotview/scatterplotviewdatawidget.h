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

#ifndef SCATTERPLOTVIEWDATAWIDGET_H_
#define SCATTERPLOTVIEWDATAWIDGET_H_

#include "global.h"
#include "nullablevector.h"
#include "dbcontent/variable/variable.h"
#include "scatterplotviewchartview.h"
#include "viewdatawidget.h"

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
class ScatterPlotViewDataWidget : public ViewDataWidget
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

    bool xVarNotInBuffer() const;
    bool yVarNotInBuffer() const;

    QRectF getDataBounds() const;
    QPixmap renderPixmap();
    unsigned int nullValueCount() const;

signals:
//    void showOnlySelectedSignal(bool value);
//    void usePresentationSignal(bool use_presentation);
//    void showAssociationsSignal(bool value);

public slots:
    void rectangleSelectedSlot (QPointF p1, QPointF p2);

    void invertSelectionSlot();
    void clearSelectionSlot();

//    void showOnlySelectedSlot(bool value);
//    void usePresentationSlot(bool use_presentation);
//    void showAssociationsSlot(bool value);

    void resetZoomSlot();

protected:
    virtual void mouseMoveEvent(QMouseEvent* event) override;

    virtual void toolChanged_impl(int mode) override;

    virtual void loadingStarted_impl() override;
    virtual void loadingDone_impl() override;
    virtual void updateData_impl(bool requires_reset) override;
    virtual void clearData_impl() override;
    virtual bool redrawData_impl(bool recompute) override;
    virtual void liveReload_impl() override;

    void resetCounts();

    bool canUpdateFromDataX(std::string dbcontent_name);
    void updateFromDataX(std::string dbcontent_name, unsigned int current_size);
    bool canUpdateFromDataY(std::string dbcontent_name);
    void updateFromDataY(std::string dbcontent_name, unsigned int current_size);
    void updateMinMax();
    void updateFromAllData();
    bool updateChart();

    void selectData (double x_min, double x_max, double y_min, double y_max);

    template<typename T>
    void appendData(NullableVector<T>& data, std::vector<double>& target, unsigned int last_size,
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

    std::map<std::string, unsigned int>            buffer_x_counts_;
    std::map<std::string, unsigned int>            buffer_y_counts_;

    std::map<std::string, std::vector<double>> x_values_;
    std::map<std::string, std::vector<double>> y_values_;

    bool has_x_min_max_ {false};
    double x_min_ {0}, x_max_ {0};

    bool has_y_min_max_ {false};
    double y_min_ {0}, y_max_ {0};

    std::map<std::string, std::vector<bool>>         selected_values_;
    std::map<std::string, std::vector<unsigned int>> rec_num_values_;

    std::map<std::string, QColor> colors_;

    ScatterPlotViewDataTool selected_tool_{SP_NAVIGATE_TOOL};

    QHBoxLayout*                                        main_layout_ {nullptr};
    //QtCharts::QChart*                                 chart_       {nullptr};
    //QtCharts::QChartView*                             chart_view_  {nullptr};
    std::unique_ptr<QtCharts::ScatterPlotViewChartView> chart_view_  {nullptr};

    bool x_var_not_in_buffer_ {false};
    bool y_var_not_in_buffer_ {false};

    unsigned int nan_value_cnt_ {0};
};

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

#endif /* SCATTERPLOTVIEWDATAWIDGET_H_ */
