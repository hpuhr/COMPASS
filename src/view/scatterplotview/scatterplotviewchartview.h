
#ifndef SCATTERPLOTVIEWCHARTVIEW_H
#define SCATTERPLOTVIEWCHARTVIEW_H

#include <chartview.h>

#include <QObject>
#include <QWidget>

#include <algorithm>

class ScatterPlotViewDataWidget;

namespace QtCharts
{

/**
 */
class ScatterPlotViewChartView : public ChartView
{
    Q_OBJECT
public:
    ScatterPlotViewChartView (ScatterPlotViewDataWidget* data_widget, QChart* chart, QWidget* parent = nullptr);
    virtual ~ScatterPlotViewChartView();

    virtual void onToolChanged() override;

signals:
    void rectangleSelectedSignal (QPointF p1, QPointF p2);

protected:
    void sendSelectedRegion();

    virtual void wheelEvent(QWheelEvent* event) override;

    virtual bool handleMousePress(Qt::MouseButtons buttons, const QPointF& widget_pos) override;
    virtual bool handleMouseRelease(Qt::MouseButtons buttons, const QPointF& widget_pos, bool update_pos) override;
    virtual bool handleMouseMove(Qt::MouseButtons buttons, const QPointF& widget_pos) override;

    ScatterPlotViewDataWidget* data_widget_ {nullptr};

    QPoint  p1_;
    QPointF p1_data_;

    QPoint  p2_;
    QPointF p2_data_;

    bool    drag_data_init_ {false};
    QPointF drag_data_;
};

}

#endif // SCATTERPLOTVIEWCHARTVIEW_H
