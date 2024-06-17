/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "chartview.h"

#include <QLabel>
#include <QPixmap>

class GridViewDataWidget;

/**
 */
class SimpleGridViewChart : public QLabel
{
public:
    SimpleGridViewChart(QWidget* parent = nullptr);
    virtual ~SimpleGridViewChart();

    void setGrid(const QImage& grid_rendering);

protected:
    virtual void resizeEvent(QResizeEvent* evt) override;

private:
    void updateChart();

    QPixmap grid_rendering_;
};

namespace QtCharts
{

/**
 */
class GridViewChart : public ChartView
{
    Q_OBJECT
public:
    GridViewChart (GridViewDataWidget* data_widget,
                   QChart* chart, 
                   QWidget* parent = nullptr);
    virtual ~GridViewChart();

    virtual void onToolChanged() override;

    void resetZoom();
    void zoom(const QPointF& p1, const QPointF& p2);

signals:
    void rectangleSelectedSignal (QPointF p1, QPointF p2);

protected:
    void sendSelectedRegion();

    virtual void wheelEvent(QWheelEvent* event) override;

    virtual bool handleMousePress(Qt::MouseButtons buttons, const QPointF& widget_pos) override;
    virtual bool handleMouseRelease(Qt::MouseButtons buttons, const QPointF& widget_pos, bool update_pos) override;
    virtual bool handleMouseMove(Qt::MouseButtons buttons, const QPointF& widget_pos) override;

    virtual void paintCustomItems(QPaintEvent* e, QPainter& painter) override;

    GridViewDataWidget* data_widget_ {nullptr};

    QPoint  p1_;
    QPointF p1_data_;

    QPoint  p2_;
    QPointF p2_data_;

    bool    drag_data_init_ {false};
    QPointF drag_data_;
};

}
