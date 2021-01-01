#include "scatterplotviewchartview.h"
#include "scatterplotviewdatawidget.h"
#include "logger.h"

#include <QApplication>

namespace QtCharts
{

ScatterPlotViewChartView::ScatterPlotViewChartView (
        ScatterPlotViewDataWidget* data_widget, QChart* chart, QWidget* parent)
    : QChartView(chart, parent), data_widget_(data_widget)
{
    assert (data_widget_);
}

void ScatterPlotViewChartView::seriesPressedSlot(const QPointF& point)
{
    logdbg << "ScatterPlotViewChartView: seriesPressedSlot: x " << point.x() << " y " << point.y();
}

void ScatterPlotViewChartView::seriesReleasedSlot(const QPointF& point)
{
    logdbg << "ScatterPlotViewChartView: seriesReleasedSlot";
    drag_data_init_ = false;
}

void ScatterPlotViewChartView::mousePressEvent(QMouseEvent* event)
{
    ScatterPlotViewDataTool tool = data_widget_->selectedTool();

    if(event->buttons() & Qt::LeftButton)
    {
        if (tool == SP_NAVIGATE_TOOL)
        {
            logdbg << "ScatterPlotViewChartView: mousePressEvent: nav ev x " << event->pos().x()
                   << " y " << event->pos().y();

            //QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));
            drag_data_ = event->pos();
            drag_data_init_ = true;
        }
        else if (tool == SP_ZOOM_RECT_TOOL || tool == SP_SELECT_TOOL)
        {

            logdbg << "ScatterPlotViewChartView: mousePressEvent: rect ev x " << event->pos().x()
                   << " y " << event->pos().y();

            // Start with the view widget coordinates
            QPointF widget_pos = event->localPos();
            // view->mapToScene: widget (view) coordinates -> scene coordinates
            QPointF scene_pos = mapToScene(QPoint(static_cast<int>(widget_pos.x()), static_cast<int>(widget_pos.y())));
            // chart->mapFromScene: scene coordinates -> chart item coordinates
            QPointF chart_item_pos = chart()->mapFromScene(scene_pos);
            // chart->mapToValue: chart item coordinates -> value in a given series.
            QPointF p = chart()->mapToValue(chart_item_pos);

            logdbg << "ScatterPlotViewChartView: mousePressEvent: x " << p.x() << " y " << p.y();

            show_rect_ = true;
            p1_ = widget_pos.toPoint();
            p1_data_ = p;

            p2_ = widget_pos.toPoint();
            p2_data_ = p;
        }

        event->accept();
        return;
    }

    QChartView::mousePressEvent(event);
    //QGraphicsView::mousePressEvent(event);
}



void ScatterPlotViewChartView::mouseMoveEvent(QMouseEvent* event)
{
    setCursor(data_widget_->currentCursor());

    ScatterPlotViewDataTool tool = data_widget_->selectedTool();

    if (event->buttons() & Qt::LeftButton)
    {
        if (tool == SP_NAVIGATE_TOOL)
        {
            logdbg << "ScatterPlotViewChartView: mouseMoveEvent: nav ev x " << event->pos().x()
                   << " y " << event->pos().y() << " drag_data_init " << drag_data_init_;

            if (drag_data_init_)
            {
                auto dPos = event->pos() - drag_data_;
                chart()->scroll(-dPos.x(), dPos.y());
            }

            drag_data_ = event->pos();
            drag_data_init_ = true;
            event->accept();

            //QApplication::restoreOverrideCursor();
            return;
        }
        else if (tool == SP_ZOOM_RECT_TOOL || tool == SP_SELECT_TOOL)
        {

            logdbg << "ScatterPlotViewChartView: mouseMoveEvent: rect ev x " << event->pos().x()
                   << " y " << event->pos().y();

            // Start with the view widget coordinates
            QPointF widget_pos = event->localPos();
            // view->mapToScene: widget (view) coordinates -> scene coordinates
            QPointF scene_pos = mapToScene(QPoint(static_cast<int>(widget_pos.x()), static_cast<int>(widget_pos.y())));
            // chart->mapFromScene: scene coordinates -> chart item coordinates
            QPointF chart_item_pos = chart()->mapFromScene(scene_pos);
            // chart->mapToValue: chart item coordinates -> value in a given series.
            QPointF p = chart()->mapToValue(chart_item_pos);

            logdbg << "ScatterPlotViewChartView: mouseMoveEvent: x " << p.x() << " y " << p.y();

            p2_ = widget_pos.toPoint();
            p2_data_ = p;

            update();
        }

        event->accept();
        return;
    }

    QChartView::mouseMoveEvent(event);
    //QGraphicsView::mouseMoveEvent(event);
}

void ScatterPlotViewChartView::mouseReleaseEvent(QMouseEvent* event)
{
    //ScatterPlotViewDataTool tool = data_widget_->selectedTool();

    logdbg << "ScatterPlotViewChartView: mouseReleaseEvent: ev x " << event->pos().x() << " y " << event->pos().y();

    if (show_rect_) // only if rect was shown
    {
        // Start with the view widget coordinates
        QPointF widget_pos = event->localPos();
        // view->mapToScene: widget (view) coordinates -> scene coordinates
        QPointF scene_pos = mapToScene(QPoint(static_cast<int>(widget_pos.x()), static_cast<int>(widget_pos.y())));
        // chart->mapFromScene: scene coordinates -> chart item coordinates
        QPointF chart_item_pos = chart()->mapFromScene(scene_pos);
        // chart->mapToValue: chart item coordinates -> value in a given series.
        QPointF p = chart()->mapToValue(chart_item_pos);

        logdbg << "ScatterPlotViewChartView: mouseReleaseEvent: x " << p.x() << " y " << p.y();

        p2_ = widget_pos.toPoint();
        p2_data_ = p;

        // do stuff
        emit rectangleSelectedSignal(p1_data_, p2_data_);

        show_rect_ = false;
        update();

        event->accept();
        return;
    }

    drag_data_init_ = false;

    QChartView::mouseReleaseEvent(event);
}

void ScatterPlotViewChartView::wheelEvent(QWheelEvent* event)
{
    qreal factor = event->angleDelta().y() > 0 ? 1.1 : 0.9;
    chart()->zoom(factor);
    event->accept();
    QChartView::wheelEvent(event);
}

void ScatterPlotViewChartView::paintEvent (QPaintEvent *e)
{
    logdbg << "ScatterPlotViewChartView: paintEvent";

    QChartView::paintEvent(e);

    if (show_rect_)
    {
        logdbg << "ScatterPlotViewChartView: paintEvent: rect";

        QPainter* painter = new QPainter (viewport());
        //painter->begin (this);
        painter->setPen(Qt::red);
        painter->drawRect(QRect(p1_, p2_));
        painter->end ();
    }
}



}
