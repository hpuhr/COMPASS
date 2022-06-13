#ifndef HISTOGRAMVIEWCHARTVIEW_H
#define HISTOGRAMVIEWCHARTVIEW_H

#include <QChartView>
#include <QObject>
#include <QWidget>

class HistogramViewDataWidget;

class QRubberBand;

namespace QtCharts
{
class QBarSet;

class HistogramViewChartView : public QChartView
{
    Q_OBJECT

signals:
    void rectangleSelectedSignal (unsigned int index1, unsigned int index2);

//public slots:
//    void seriesPressedSlot(int index, QBarSet *barset);
//    void seriesReleasedSlot(int index, QBarSet *barset);

public:
    HistogramViewChartView(HistogramViewDataWidget* data_widget, QChart* chart, QWidget* parent = nullptr);
    virtual ~HistogramViewChartView();

protected:
    HistogramViewDataWidget* data_widget_ {nullptr};

    bool show_rect_ {false};
    QRubberBand* rectangle_ {nullptr};

    bool has_p1_ {false};
    QPoint p1_;
    QPointF p1_data_;
    //unsigned int p1_index_;

    QPoint p2_;
    QPointF p2_data_;
    //unsigned int p2_index_;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

    void paintEvent (QPaintEvent *e) override;
};

}

#endif // HISTOGRAMVIEWCHARTVIEW_H
