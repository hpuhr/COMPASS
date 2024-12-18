#include "scatterplotviewchartview.h"
#include "scatterplotviewdatawidget.h"
#include "logger.h"

#include <QApplication>
#include <QRubberBand>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QLineSeries>

//#define USE_CHART_SERIES_BASED_SELECTION_BOX

namespace QtCharts
{

/**
 */
ScatterPlotViewChartView::ScatterPlotViewChartView (ScatterPlotViewDataWidget* data_widget, 
                                                    QChart* chart, 
                                                    QWidget* parent)
:   ChartView   (chart, ChartView::SelectionStyle::RubberBand, parent)
,   data_widget_(data_widget)
{
    assert (data_widget_);
}

/**
 */
ScatterPlotViewChartView::~ScatterPlotViewChartView() = default;

/**
 */
void ScatterPlotViewChartView::onToolChanged()
{
    //disable drag
    drag_data_init_ = false;

    //don't forget to invoke base behaviour
    ChartView::onToolChanged();
}

/**
 */
bool ScatterPlotViewChartView::handleMousePress(Qt::MouseButtons buttons, const QPointF& widget_pos)
{
    if (buttons & Qt::LeftButton)
    {
        ScatterPlotViewDataTool tool = data_widget_->selectedTool();

        if (tool == SP_NAVIGATE_TOOL)
        {
            logdbg << "ScatterPlotViewChartView: handleMousePress: NAVIGATE x " << widget_pos.x() << " y " << widget_pos.y();

            drag_data_      = widget_pos;
            drag_data_init_ = true;

            return true;
        }
        else if (tool == SP_ZOOM_RECT_TOOL || tool == SP_SELECT_TOOL)
        {
            loginf << "ScatterPlotViewChartView: handleMousePress: RECT x " << widget_pos.x() << " y " << widget_pos.y();

            // view widget coordinates to chart coordinates
            QPointF p = widgetToChart(widget_pos);

            loginf << "ScatterPlotViewChartView: handleMousePress: RECT xc " << p.x() << " yc " << p.y();

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
bool ScatterPlotViewChartView::handleMouseMove(Qt::MouseButtons buttons, const QPointF& widget_pos)
{
    if (buttons & Qt::LeftButton)
    {
        ScatterPlotViewDataTool tool = data_widget_->selectedTool();

        if (tool == SP_NAVIGATE_TOOL)
        {
            logdbg << "ScatterPlotViewChartView: handleMouseMove: NAVIGATE x " << widget_pos.x() << " y " << widget_pos.y() << " drag_data_init " << drag_data_init_;

            if (drag_data_init_)
            {
                auto dPos = widget_pos - drag_data_;
                chart()->scroll(-dPos.x(), dPos.y());
            }

            drag_data_      = widget_pos;
            drag_data_init_ = true;

            return true;
        }
        else if ((tool == SP_ZOOM_RECT_TOOL || tool == SP_SELECT_TOOL) && isSelectionEnabled())
        {
            logdbg << "ScatterPlotViewChartView: handleMouseMove: RECT x " << widget_pos.x() << " y " << widget_pos.y();

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
bool ScatterPlotViewChartView::handleMouseRelease(Qt::MouseButtons buttons, const QPointF& widget_pos, bool update_pos)
{
    if (buttons & Qt::LeftButton)
    {
        ScatterPlotViewDataTool tool = data_widget_->selectedTool();

        if (tool == SP_NAVIGATE_TOOL)
        {
            logdbg << "ScatterPlotViewChartView: handleMouseRelease: NAVIGATE x " << widget_pos.x() << " y " << widget_pos.y();

            drag_data_init_ = false;

            return true;
        }
        else if ((tool == SP_ZOOM_RECT_TOOL || tool == SP_SELECT_TOOL) && isSelectionEnabled())
        {
            loginf << "ScatterPlotViewChartView: handleMouseRelease: RECT x " << widget_pos.x() << " y " << widget_pos.y();

            // view widget coordinates to chart coordinates
            QPointF p = widgetToChart(widget_pos);

            loginf << "ScatterPlotViewChartView: handleMouseRelease: RECT xc " << p.x() << " yc " << p.y();

            if (update_pos)
            {
                p2_      = widget_pos.toPoint();
                p2_data_ = p;
            }
            
            loginf << "ScatterPlotViewChartView: handleMouseRelease: REGION p1 " << p1_data_.x() << "," << p1_data_.y() << " p2 " << p2_data_.x() << "," << p2_data_.y();

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
void ScatterPlotViewChartView::sendSelectedRegion()
{
    auto region = selectedChartRegion();

    emit rectangleSelectedSignal(region.topLeft(), region.bottomRight());
}

/**
 */
void ScatterPlotViewChartView::wheelEvent(QWheelEvent* event)
{
    const double ZoomFactorIn  = 1.1;
    const double ZoomFactorOut = 0.9;

    qreal factor = event->angleDelta().y() > 0 ? ZoomFactorIn : ZoomFactorOut;
    chart()->zoom(factor);

    event->accept();

    QChartView::wheelEvent(event);
}

}
