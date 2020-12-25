#include "scatterplotviewchartview.h"
#include "logger.h"


namespace QtCharts
{

void ScatterPlotViewChartView::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        QPointF p = mapToScene(event->pos());

        loginf << "ScatterPlotViewChartView: mousePressEvent: x " << p.x() << " y " << p.y();
    }

    QChartView::mousePressEvent(event);
    //QGraphicsView::mousePressEvent(event);
}

void ScatterPlotViewChartView::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        QPointF p = mapToScene(event->pos());

        loginf << "ScatterPlotViewChartView: mouseMoveEvent: x " << p.x() << " y " << p.y();
    }

    QChartView::mouseMoveEvent(event);
    //QGraphicsView::mouseMoveEvent(event);
}


}
