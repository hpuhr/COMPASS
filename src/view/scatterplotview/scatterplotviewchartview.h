#ifndef SCATTERPLOTVIEWCHARTVIEW_H
#define SCATTERPLOTVIEWCHARTVIEW_H

#include <QChartView>
#include <QObject>
#include <QWidget>

#include <algorithm>

class ScatterPlotViewDataWidget;

class QRubberBand;

namespace QtCharts
{

class ScatterPlotViewChartView : public QChartView
{
    Q_OBJECT

signals:
    void rectangleSelectedSignal (QPointF p1, QPointF p2);

public slots:
    void seriesPressedSlot(const QPointF& point);
    void seriesReleasedSlot(const QPointF& point);


public:
    ScatterPlotViewChartView (ScatterPlotViewDataWidget* data_widget, QChart* chart, QWidget* parent = nullptr);
    virtual ~ScatterPlotViewChartView();

protected:
    ScatterPlotViewDataWidget* data_widget_ {nullptr};

    bool show_rect_ {false};
    QRubberBand* rectangle_ {nullptr};

    bool has_p1_ {false};
    QPoint p1_;
    QPointF p1_data_;

    QPoint p2_;
    QPointF p2_data_;

    bool drag_data_init_ {false};
    QPointF drag_data_;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

    void paintEvent (QPaintEvent *e) override;
};

}

#endif // SCATTERPLOTVIEWCHARTVIEW_H
