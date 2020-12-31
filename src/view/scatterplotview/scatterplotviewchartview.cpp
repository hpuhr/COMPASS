#include "scatterplotviewchartview.h"
#include "logger.h"


namespace QtCharts
{

void ScatterPlotViewChartView::mousePressEvent(QMouseEvent* event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        loginf << "ScatterPlotViewChartView: mousePressEvent: ev x " << event->pos().x() << " y " << event->pos().y();

        // Start with the view widget coordinates
        QPointF widget_pos = event->localPos();
        // view->mapToScene: widget (view) coordinates -> scene coordinates
        QPointF scene_pos = mapToScene(QPoint(static_cast<int>(widget_pos.x()), static_cast<int>(widget_pos.y())));
        // chart->mapFromScene: scene coordinates -> chart item coordinates
        QPointF chart_item_pos = chart()->mapFromScene(scene_pos);
        // chart->mapToValue: chart item coordinates -> value in a given series.
        QPointF p = chart()->mapToValue(chart_item_pos);

        loginf << "ScatterPlotViewChartView: mousePressEvent: x " << p.x() << " y " << p.y();

        show_rect_ = true;
        p1_ = widget_pos.toPoint();
        p1_data_ = p;

        p2_ = widget_pos.toPoint();
        p2_data_ = p;
    }

    //QChartView::mousePressEvent(event);
    //QGraphicsView::mousePressEvent(event);
}

void ScatterPlotViewChartView::mouseMoveEvent(QMouseEvent* event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        loginf << "ScatterPlotViewChartView: mouseMoveEvent: ev x " << event->pos().x() << " y " << event->pos().y();

        // Start with the view widget coordinates
        QPointF widget_pos = event->localPos();
        // view->mapToScene: widget (view) coordinates -> scene coordinates
        QPointF scene_pos = mapToScene(QPoint(static_cast<int>(widget_pos.x()), static_cast<int>(widget_pos.y())));
        // chart->mapFromScene: scene coordinates -> chart item coordinates
        QPointF chart_item_pos = chart()->mapFromScene(scene_pos);
        // chart->mapToValue: chart item coordinates -> value in a given series.
        QPointF p = chart()->mapToValue(chart_item_pos);

        loginf << "ScatterPlotViewChartView: mouseMoveEvent: x " << p.x() << " y " << p.y();

        p2_ = widget_pos.toPoint();
        p2_data_ = p;

        update();
    }

    //QChartView::mouseMoveEvent(event);
    //QGraphicsView::mouseMoveEvent(event);
}

void ScatterPlotViewChartView::mouseReleaseEvent(QMouseEvent* event)
{
    loginf << "ScatterPlotViewChartView: mouseReleaseEvent: ev x " << event->pos().x() << " y " << event->pos().y();

    // Start with the view widget coordinates
    QPointF widget_pos = event->localPos();
    // view->mapToScene: widget (view) coordinates -> scene coordinates
    QPointF scene_pos = mapToScene(QPoint(static_cast<int>(widget_pos.x()), static_cast<int>(widget_pos.y())));
    // chart->mapFromScene: scene coordinates -> chart item coordinates
    QPointF chart_item_pos = chart()->mapFromScene(scene_pos);
    // chart->mapToValue: chart item coordinates -> value in a given series.
    QPointF p = chart()->mapToValue(chart_item_pos);

    loginf << "ScatterPlotViewChartView: mouseReleaseEvent: x " << p.x() << " y " << p.y();

    p2_ = widget_pos.toPoint();
    p2_data_ = p;

    // do stuff
    emit rectangleSelectedSignal(p1_data_, p2_data_);

    show_rect_ = false;
    update();

    //QChartView::mouseReleaseEvent(event);
}

void ScatterPlotViewChartView::paintEvent (QPaintEvent *e)
{
    loginf << "ScatterPlotViewChartView: paintEvent";

    QChartView::paintEvent(e);

    if (show_rect_)
    {
        loginf << "ScatterPlotViewChartView: paintEvent: rect";

        QPainter *painter = new QPainter (viewport());
        painter->begin (this);
        painter->setPen(Qt::red);
        painter->drawRect(QRect(p1_, p2_));
        painter->end ();
    }
}



}
