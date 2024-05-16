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

#include "gridviewchart.h"
#include "gridviewdatawidget.h"

/**
*/
SimpleGridViewChart::SimpleGridViewChart(QWidget* parent)
:   QLabel(parent)
{
    setAlignment(Qt::AlignCenter);

    updateChart();
}

/**
*/
SimpleGridViewChart::~SimpleGridViewChart() = default;

/**
*/
void SimpleGridViewChart::setGrid(const QImage& grid_rendering)
{
    grid_rendering_ = QPixmap::fromImage(grid_rendering);

    updateChart();
}

/**
*/
void SimpleGridViewChart::resizeEvent(QResizeEvent* evt)
{
    updateChart();
}

/**
*/
void SimpleGridViewChart::updateChart()
{
    if (grid_rendering_.isNull())
    {
        setPixmap(QPixmap());
        setText("No Data");
        return;
    }

    setText("");
    setPixmap(grid_rendering_.scaled(size(), Qt::IgnoreAspectRatio, Qt::TransformationMode::FastTransformation));
}

/**************************************************************************************************
 * GridViewChart
 **************************************************************************************************/

namespace QtCharts
{

/**
 */
GridViewChart::GridViewChart(GridViewDataWidget* data_widget, 
                             QChart* chart, 
                             QWidget* parent)
:   ChartView   (chart, ChartView::SelectionStyle::RubberBand, parent)
,   data_widget_(data_widget)
{
    assert (data_widget_);
}

/**
 */
GridViewChart::~GridViewChart() = default;

/**
 */
void GridViewChart::onToolChanged()
{
    //disable drag
    drag_data_init_ = false;

    //don't forget to invoke base behaviour
    ChartView::onToolChanged();
}

/**
 */
bool GridViewChart::handleMousePress(Qt::MouseButtons buttons, const QPointF& widget_pos)
{
    if (buttons & Qt::LeftButton)
    {
        GridViewDataTool tool = data_widget_->selectedTool();

        if (tool == GV_NAVIGATE_TOOL)
        {
            logdbg << "GridViewChart: handleMousePress: NAVIGATE x " << widget_pos.x() << " y " << widget_pos.y();

            drag_data_      = widget_pos;
            drag_data_init_ = true;

            return true;
        }
        else if (tool == GV_ZOOM_RECT_TOOL || tool == GV_SELECT_TOOL)
        {
            loginf << "GridViewChart: handleMousePress: RECT x " << widget_pos.x() << " y " << widget_pos.y();

            // view widget coordinates to chart coordinates
            QPointF p = widgetToChart(widget_pos);

            loginf << "GridViewChart: handleMousePress: RECT xc " << p.x() << " yc " << p.y();

            p1_      = widget_pos.toPoint();
            p1_data_ = p;

            p2_      = widget_pos.toPoint();
            p2_data_ = p;

            beginSelection(p1_, p2_, p1_data_, p2_data_);

            return true;
        }
    }

    return false;
}

/**
 */
bool GridViewChart::handleMouseMove(Qt::MouseButtons buttons, const QPointF& widget_pos)
{
    if (buttons & Qt::LeftButton)
    {
        GridViewDataTool tool = data_widget_->selectedTool();

        if (tool == GV_NAVIGATE_TOOL)
        {
            logdbg << "GridViewChart: handleMouseMove: NAVIGATE x " << widget_pos.x() << " y " << widget_pos.y() << " drag_data_init " << drag_data_init_;

            if (drag_data_init_)
            {
                auto dPos = widget_pos - drag_data_;
                chart()->scroll(-dPos.x(), dPos.y());
            }

            drag_data_      = widget_pos;
            drag_data_init_ = true;

            return true;
        }
        else if ((tool == GV_ZOOM_RECT_TOOL || tool == GV_SELECT_TOOL) && isSelectionEnabled())
        {
            logdbg << "GridViewChart: handleMouseMove: RECT x " << widget_pos.x() << " y " << widget_pos.y();

            // view widget coordinates to chart coordinates
            QPointF p = widgetToChart(widget_pos);

            p2_      = widget_pos.toPoint();
            p2_data_ = p;

            updateSelection(p1_, p2_, p1_data_, p2_data_);

            return true;
        }
    }

    return false;
}

/**
 */
bool GridViewChart::handleMouseRelease(Qt::MouseButtons buttons, const QPointF& widget_pos, bool update_pos)
{
    if (buttons & Qt::LeftButton)
    {
        GridViewDataTool tool = data_widget_->selectedTool();

        if (tool == GV_NAVIGATE_TOOL)
        {
            logdbg << "GridViewChart: handleMouseRelease: NAVIGATE x " << widget_pos.x() << " y " << widget_pos.y();

            drag_data_init_ = false;

            return true;
        }
        else if ((tool == GV_ZOOM_RECT_TOOL || tool == GV_SELECT_TOOL) && isSelectionEnabled())
        {
            loginf << "GridViewChart: handleMouseRelease: RECT x " << widget_pos.x() << " y " << widget_pos.y();

            // view widget coordinates to chart coordinates
            QPointF p = widgetToChart(widget_pos);

            loginf << "GridViewChart: handleMouseRelease: RECT xc " << p.x() << " yc " << p.y();

            if (update_pos)
            {
                p2_      = widget_pos.toPoint();
                p2_data_ = p;
            }
            
            loginf << "GridViewChart: handleMouseRelease: REGION p1 " << p1_data_.x() << "," << p1_data_.y() << " p2 " << p2_data_.x() << "," << p2_data_.y();

            updateSelection(p1_, p2_, p1_data_, p2_data_);
            sendSelectedRegion();
            endSelection();

            return true;
        }
    }

    return false;
}

/**
 */
void GridViewChart::sendSelectedRegion()
{
    auto region = selectedChartRegion();

    emit rectangleSelectedSignal(region.topLeft(), region.bottomRight());
}

/**
 */
void GridViewChart::wheelEvent(QWheelEvent* event)
{
    const double ZoomFactorIn  = 1.1;
    const double ZoomFactorOut = 0.9;

    qreal factor = event->angleDelta().y() > 0 ? ZoomFactorIn : ZoomFactorOut;
    chart()->zoom(factor);

    event->accept();

    QChartView::wheelEvent(event);
}

/**
 */
void GridViewChart::paintCustomItems(QPaintEvent* event, QPainter& painter)
{
    const auto& grid_bounds    = data_widget_->gridBounds();
    const auto& grid_rendering = data_widget_->gridRendering();

    if (grid_bounds.isEmpty() || grid_rendering.isNull())
        return;

    auto tl_mapped = this->chart()->mapToPosition(QPointF(grid_bounds.topLeft()));
    auto br_mapped = this->chart()->mapToPosition(QPointF(grid_bounds.bottomRight()));

    double xmin = std::min(tl_mapped.x(), br_mapped.x());
    double xmax = std::max(tl_mapped.x(), br_mapped.x());
    double ymin = std::min(tl_mapped.y(), br_mapped.y());
    double ymax = std::max(tl_mapped.y(), br_mapped.y());

    QRectF r_paint(xmin, ymin, xmax - xmin, ymax - ymin);

    int w = r_paint.width();
    int h = r_paint.height();

    painter.setClipRect(chart()->plotArea());
    painter.drawImage(QPointF(xmin, ymin), grid_rendering.scaled(w, h, Qt::IgnoreAspectRatio, Qt::FastTransformation));
}

}
