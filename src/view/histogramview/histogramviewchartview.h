#ifndef HISTOGRAMVIEWCHARTVIEW_H
#define HISTOGRAMVIEWCHARTVIEW_H

#include <chartview.h>

#include <QObject>
#include <QWidget>

class HistogramViewDataWidget;

namespace QtCharts
{
class QBarSet;

/**
 */
class HistogramViewChartView : public ChartView
{
    Q_OBJECT
public:
    HistogramViewChartView(HistogramViewDataWidget* data_widget, 
                           QChart* chart, 
                           QWidget* parent = nullptr);
    virtual ~HistogramViewChartView();

signals:
    void rectangleSelectedSignal (unsigned int index1, unsigned int index2);

protected:
    void sendSelectedBins();

    void wheelEvent(QWheelEvent* event) override;

    virtual bool handleMousePress(Qt::MouseButtons buttons, const QPointF& widget_pos) override;
    virtual bool handleMouseRelease(Qt::MouseButtons buttons, const QPointF& widget_pos, bool update_pos) override;
    virtual bool handleMouseMove(Qt::MouseButtons buttons, const QPointF& widget_pos) override;

    HistogramViewDataWidget* data_widget_ {nullptr};

    QPoint  p1_;
    QPointF p1_data_;

    QPoint  p2_;
    QPointF p2_data_;
};

}

#endif // HISTOGRAMVIEWCHARTVIEW_H
