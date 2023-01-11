
#pragma once

#include <QChartView>

#include <memory>

class QRubberBand;

namespace QtCharts
{
    class QChart;
    class QAreaSeries;
    class QLineSeries;
}

/**
 * Base chart view, encapsulating common features such as rectangle selection and reaction on tool changes.
 */
class ChartView : public QtCharts::QChartView
{
    Q_OBJECT
public:
    enum class SelectionStyle
    {
        RubberBand = 0,
        SeriesArea, 
        SeriesLines //gl accelerated lines
    };

    ChartView(QtCharts::QChart* chart, SelectionStyle sel_style, QWidget* parent = nullptr);
    virtual ~ChartView();

    void setDataBounds(const QRectF& r);

    virtual void onToolChanged();

public slots:
    virtual void seriesPressedSlot(const QPointF& point);
    virtual void seriesReleasedSlot(const QPointF& point);

protected:
    virtual void paintEvent(QPaintEvent *e) override final;
    virtual void mousePressEvent(QMouseEvent* event) override final;
    virtual void mouseMoveEvent(QMouseEvent* event) override final;
    virtual void mouseReleaseEvent(QMouseEvent* event) override final;

    virtual bool handleMousePress(Qt::MouseButtons buttons, const QPointF& widget_pos) = 0;
    virtual bool handleMouseRelease(Qt::MouseButtons buttons, const QPointF& widget_pos, bool update_pos) = 0;
    virtual bool handleMouseMove(Qt::MouseButtons buttons, const QPointF& widget_pos) = 0;

    QPointF widgetToChart(const QPointF& pos) const;
    QPointF widgetFromChart(const QPointF& pos) const;

    const QRectF& selectedChartRegion() const { return selected_region_chart_; }
    void beginSelection(const QRectF& r_widget,
                        const QRectF& r_chart);
    void beginSelection(const QPointF& p1_widget, 
                        const QPointF& p2_widget, 
                        const QPointF& p1_chart, 
                        const QPointF& p2_chart);
    void updateSelection(const QRectF& r_widget,
                         const QRectF& r_chart);
    void updateSelection(const QPointF& p1_widget, 
                         const QPointF& p2_widget, 
                         const QPointF& p1_chart, 
                         const QPointF& p2_chart);
    void endSelection();

    bool isSelectionEnabled() const;

private:
    void createDisplayElements(QtCharts::QChart* chart);

    void clearSelection();
    void updateSelectionBox(const QRectF& region);
    void updateSelectionLines(const QRectF& region);
    void updateRubberBand(const QRectF& region);

    static const QColor SelectionColor;

    SelectionStyle               sel_style_;
    QRectF                       data_bounds_;
    QRectF                       selected_region_widget_;
    QRectF                       selected_region_chart_;
    std::unique_ptr<QRubberBand> rubber_band_;
    QtCharts::QAreaSeries*       selection_box_    = nullptr;
    QtCharts::QLineSeries*       selection_lines_  = nullptr;
    bool                         enable_selection_ = false;
};
