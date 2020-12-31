#ifndef SCATTERPLOTVIEWCHARTVIEW_H
#define SCATTERPLOTVIEWCHARTVIEW_H

#include <QChartView>
#include <QObject>
#include <QWidget>

#include <algorithm>

namespace QtCharts
{

class ScatterPlotViewChartView : public QChartView
{
    Q_OBJECT

signals:
    void rectangleSelectedSignal (QPointF p1, QPointF p2);

public:
    using QChartView::QChartView;

protected:
    bool show_rect_ {false};
    QPoint p1_;
    QPointF p1_data_;

    QPoint p2_;
    QPointF p2_data_;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

    void paintEvent (QPaintEvent *e) override;
};

}

#endif // SCATTERPLOTVIEWCHARTVIEW_H
