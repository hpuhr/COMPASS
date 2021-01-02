#include "histogramviewchartview.h"
#include "histogramviewdatawidget.h"
#include "logger.h"

#include <QApplication>
#include <QRubberBand>
#include <QBarSet>

namespace QtCharts
{

HistogramViewChartView::HistogramViewChartView (
        HistogramViewDataWidget* data_widget, QChart* chart, QWidget* parent)
    : QChartView(chart, parent), data_widget_(data_widget)
{
    assert (data_widget_);
}

//void HistogramViewChartView::seriesPressedSlot(int index, QBarSet* barset)
//{
//    loginf << "HistogramViewChartView: seriesPressedSlot: point index " << index
//           << " barset " << barset->label().toStdString();

//}

//void HistogramViewChartView::seriesReleasedSlot(int index, QBarSet *barset)
//{
//    loginf << "HistogramViewChartView: seriesReleasedSlot: point index " << index
//           << " barset " << barset->label().toStdString();

//    //loginf << "HistogramViewChartView: seriesReleasedSlot";
//}

void HistogramViewChartView::mousePressEvent(QMouseEvent* event)
{
    logdbg << "HistogramViewChartView: mousePressEvent: rect ev x " << event->pos().x()
           << " y " << event->pos().y() << " has_p1 " << has_p1_;

    QPointF widget_pos = event->localPos();

    auto point = mapFromGlobal(QCursor::pos());
    auto pointF = mapToScene(point);
    pointF = chart()->mapFromScene(point);
    auto p = chart()->mapToValue(point, chart()->series().at(0));

    int index = (int) (p.x()+0.5);

    unsigned int ui_index;

    if (index < 0)
        ui_index = 0;
    else if (index > data_widget_->numBins())
        ui_index = data_widget_->numBins();
    else
        ui_index = index;


    logdbg << "HistogramViewChartView: mousePressEvent: x " << p.x() << " y " << p.y() << " index " << index;

    if (!has_p1_) // first click
    {
        show_rect_ = true;
        has_p1_ = true;
        p1_ = widget_pos.toPoint();
        p1_data_ = p;
        p1_index_ = ui_index;

        p2_ = widget_pos.toPoint();
        p2_data_ = p;
        p2_index_ = ui_index;
    }
    else // second click
    {
        p2_ = widget_pos.toPoint();
        p2_data_ = p;
        p2_index_ = ui_index;

        // do stuff
        emit rectangleSelectedSignal(p1_index_, p2_index_);

        show_rect_ = false;
        has_p1_ = false;
        update();
        this->viewport()->repaint();
    }

    event->accept();
    return;

    //QChartView::mousePressEvent(event);
}



void HistogramViewChartView::mouseMoveEvent(QMouseEvent* event)
{
    if (has_p1_)
    {
        logdbg << "HistogramViewChartView: mouseMoveEvent: rect ev x " << event->pos().x()
               << " y " << event->pos().y();

        // Start with the view widget coordinates
        QPointF widget_pos = event->localPos();
        // view->mapToScene: widget (view) coordinates -> scene coordinates
        QPointF scene_pos = mapToScene(QPoint(static_cast<int>(widget_pos.x()), static_cast<int>(widget_pos.y())));
        // chart->mapFromScene: scene coordinates -> chart item coordinates
        QPointF chart_item_pos = chart()->mapFromScene(scene_pos);
        // chart->mapToValue: chart item coordinates -> value in a given series.
        QPointF p = chart()->mapToValue(chart_item_pos);

        logdbg << "HistogramViewChartView: mouseMoveEvent: x " << p.x() << " y " << p.y();

        p2_ = widget_pos.toPoint();
        p2_data_ = p;

        update();

        event->accept();

        this->viewport()->repaint();
        return;
    }

    setCursor(data_widget_->currentCursor());

    //QChartView::mouseMoveEvent(event);
    QChartView::mouseMoveEvent(event);
}

void HistogramViewChartView::mouseReleaseEvent(QMouseEvent* event)
{
    logdbg << "HistogramViewChartView: mouseReleaseEvent: ev x " << event->pos().x() << " y " << event->pos().y();

    QChartView::mouseReleaseEvent(event);
}

void HistogramViewChartView::wheelEvent(QWheelEvent* event)
{
    qreal factor = event->angleDelta().y() > 0 ? 1.1 : 0.9;
    chart()->zoom(factor);
    event->accept();
    QChartView::wheelEvent(event);
}

void HistogramViewChartView::paintEvent (QPaintEvent *e)
{
    logdbg << "HistogramViewChartView: paintEvent: show_rect " << show_rect_;

    QChartView::paintEvent(e);

    if (show_rect_)
    {
        logdbg << "HistogramViewChartView: paintEvent: rect";

        if (!rectangle_)
        {
            rectangle_ = new QRubberBand(QRubberBand::Rectangle, viewport());

            QPalette pal;
            pal.setBrush(QPalette::Highlight, QBrush(Qt::red));
            rectangle_->setPalette(pal);
        }

        QPoint p_min {std::min(p1_.x(), p2_.x()), std::min(p1_.y(), p2_.y())};
        QPoint p_max {std::max(p1_.x(), p2_.x()), std::max(p1_.y(), p2_.y())};

        rectangle_->setGeometry(QRect(p_min, p_max));
        rectangle_->show();
    }
    else if (rectangle_)
    {
        delete rectangle_;
        rectangle_ = nullptr;
    }
}

}
