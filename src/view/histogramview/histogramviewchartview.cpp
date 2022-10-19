#include "histogramviewchartview.h"
#include "histogramviewdatawidget.h"
#include "logger.h"

#include <QApplication>
#include <QRubberBand>
#include <QBarSet>

namespace QtCharts
{

/**
 */
HistogramViewChartView::HistogramViewChartView(HistogramViewDataWidget* data_widget, 
                                               QChart* chart, 
                                               QWidget* parent)
:   ChartView   (chart, parent)
,   data_widget_(data_widget  )
{
    assert (data_widget_);
}

/**
 */
HistogramViewChartView::~HistogramViewChartView() = default;

/**
 */
bool HistogramViewChartView::handleMousePress(Qt::MouseButtons buttons, const QPointF& widget_pos)
{
    if (buttons & Qt::LeftButton)
    {
        auto tool = data_widget_->selectedTool();

        if (tool == HG_SELECT_TOOL ||
            tool == HG_ZOOM_TOOL)
        {
            logdbg << "HistogramViewChartView: handleMousePress: RECT x " << widget_pos.x() << " y " << widget_pos.y();

            // view widget coordinates to chart coordinates
            QPointF p = widgetToChart(widget_pos);

            p1_      = widget_pos.toPoint();
            p1_data_ = p;

            p2_      = widget_pos.toPoint();
            p2_data_ = p;

            beginSelection(p1_, p2_, p1_data_, p2_data_);

            return true;
        }
    }
    return false;
}

/**
 */
bool HistogramViewChartView::handleMouseMove(Qt::MouseButtons buttons, const QPointF& widget_pos)
{
    if (buttons & Qt::LeftButton)
    {
        auto tool = data_widget_->selectedTool();

        if (tool == HG_SELECT_TOOL ||
            tool == HG_ZOOM_TOOL)
        {
            if (isSelectionEnabled())
            {
                logdbg << "HistogramViewChartView: handleMouseMove: RECT x " << widget_pos.x() << " y " << widget_pos.y();

                // view widget coordinates to chart coordinates
                QPointF p = widgetToChart(widget_pos);

                p2_      = widget_pos.toPoint();
                p2_data_ = p;

                updateSelection(p1_, p2_, p1_data_, p2_data_);

                return true;
            }
        }
    }
    return false;
}

/**
 */
bool HistogramViewChartView::handleMouseRelease(Qt::MouseButtons buttons, const QPointF& widget_pos, bool update_pos)
{
    if (buttons & Qt::LeftButton)
    {
        auto tool = data_widget_->selectedTool();

        if (tool == HG_SELECT_TOOL ||
            tool == HG_ZOOM_TOOL)
        {
            if (isSelectionEnabled())
            {
                logdbg << "HistogramViewChartView: handleMouseRelease: RECT x " << widget_pos.x() << " y " << widget_pos.y();

                // view widget coordinates to chart coordinates
                QPointF p = widgetToChart(widget_pos);

                if (update_pos)
                {
                    p2_      = widget_pos.toPoint();
                    p2_data_ = p;
                }
                
                logdbg << "ScatterPlotViewChartView: handleMouseRelease: REGION p1 " << p1_data_.x() << "," << p1_data_.y() << " p2 " << p2_data_.x() << "," << p2_data_.y();

                updateSelection(p1_, p2_, p1_data_, p2_data_);
                sendSelectedBins();
                endSelection();

                return true;
            }  
        }
    }
    return false;
}

/**
 */
void HistogramViewChartView::sendSelectedBins()
{
    const int bins = (int)data_widget_->numBins();

    if (bins > 0)
    {
        auto clampBinToRange = [ & ] (int& bin) 
        {
            //bin == binmax is the zero value bin, so clamp to [0, numbins]
            bin = std::max(0, std::min(bin, bins));
        };

        auto region = selectedChartRegion();

        //determine which bin centers are inside the numeric range (bin centers are at full integers, 0, 1, 2, and so on)
        int bin_min = (int)std::ceil(region.left());
        int bin_max = (int)std::floor(region.right());

        //clamp bin ids for safety
        clampBinToRange(bin_min);
        clampBinToRange(bin_max);

        logdbg << "HistogramViewChartView: sendSelectedBins: bin range [" << bin_min << "," << bin_max << "] bins = " << bins;

        //only send range when valid
        if (bin_min <= bin_max)
        {
            // do stuff
            emit rectangleSelectedSignal(bin_min, bin_max);
        }
    } 
}

/**
 */
void HistogramViewChartView::wheelEvent(QWheelEvent* event)
{
    //zooming using the wheel only makes sense if some kind of navigation in the chart plane is supported
    //qreal factor = event->angleDelta().y() > 0 ? 1.1 : 0.9;
    //chart()->zoom(factor);
    //event->accept();
    
    QChartView::wheelEvent(event);
}

}
