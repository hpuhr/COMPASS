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
public:
    using QChartView::QChartView;

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

};

}

#endif // SCATTERPLOTVIEWCHARTVIEW_H
