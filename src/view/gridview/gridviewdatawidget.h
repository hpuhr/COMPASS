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

#include "variableviewdatawidget.h"

#include "nullablevector.h"
#include "timeconv.h"

#include <memory>
#include <limits>

#include <boost/optional.hpp>

#include <QImage>
#include <QRectF>

class GridView;
class GridViewWidget;
class Grid2D;

namespace QtCharts
{
    class GridViewChart;
    class QChart;
}

class Buffer;

class QHBoxLayout;
class QPixmap;

enum GridViewDataTool
{
    GV_NAVIGATE_TOOL = 0,
    GV_ZOOM_RECT_TOOL,
    GV_SELECT_TOOL
};

/**
 * @brief Widget with tab containing BufferTableWidgets in ScatterPlotView
 *
 */
class GridViewDataWidget : public VariableViewDataWidget
{
    Q_OBJECT
public:
    /// @brief Constructor
    GridViewDataWidget(GridViewWidget* view_widget,
                       QWidget* parent = nullptr, 
                       Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~GridViewDataWidget();

    GridViewDataTool selectedTool() const;

    QRectF getXYBounds() const;
    boost::optional<std::pair<double, double>> getZBounds() const;

    QPixmap renderPixmap();
    unsigned int nullValueCount() const;

    const Grid2D* grid() const { return grid_.get(); }
    const QImage& gridRendering() const { return grid_rendering_; }
    const QRectF& gridBounds() const { return grid_roi_; }

    const GridView* getView() const { return view_; }

public slots:
    void rectangleSelectedSlot(QPointF p1, QPointF p2);

    void invertSelectionSlot();
    void clearSelectionSlot();

    void resetZoomSlot();

protected:
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
    void resetGridChart();

    void updateMinMax();
    bool updateGridChart();
    void updateGrid();
    void updateChart(QtCharts::QChart* chart, bool has_data);

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

    GridView* view_ = nullptr;

    std::map<std::string, unsigned int>               buffer_x_counts_;
    std::map<std::string, unsigned int>               buffer_y_counts_;
    std::map<std::string, unsigned int>               buffer_z_counts_;

    std::map<std::string, std::vector<double>>        x_values_;
    std::map<std::string, std::vector<double>>        y_values_;
    std::map<std::string, std::vector<double>>        z_values_;

    std::map<std::string, std::vector<bool>>          selected_values_;
    std::map<std::string, std::vector<unsigned long>> rec_num_values_;
    std::map<std::string, unsigned int>               dbo_valid_counts_;

    unsigned int nan_value_cnt_ {0};
    unsigned int valid_cnt_     {0};
    unsigned int selected_cnt_  {0};

    bool has_x_min_max_ {false};
    double x_min_ {0}, x_max_ {0};

    bool has_y_min_max_ {false};
    double y_min_ {0}, y_max_ {0};

    bool has_z_min_max_ {false};
    double z_min_ {0}, z_max_ {0};

    GridViewDataTool selected_tool_ = GV_NAVIGATE_TOOL;

    QHBoxLayout*         main_layout_ = nullptr;
    //SimpleGridViewChart* grid_chart_  = nullptr;

    std::unique_ptr<QtCharts::GridViewChart> grid_chart_;

    std::unique_ptr<Grid2D> grid_;
    QImage                  grid_rendering_;
    QRectF                  grid_roi_;
};

/**
*/
template<>
inline void GridViewDataWidget::appendData<boost::posix_time::ptime>(NullableVector<boost::posix_time::ptime>& data, 
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
inline void GridViewDataWidget::appendData<std::string>(NullableVector<std::string>& data, 
                                                        std::vector<double>& target, 
                                                        unsigned int last_size,
                                                        unsigned int current_size)
{
    throw std::runtime_error("GridViewDataWidget: appendData: string not supported");
}
