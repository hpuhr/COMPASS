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

#ifndef HISTOGRAMVIEWDATAWIDGET_H_
#define HISTOGRAMVIEWDATAWIDGET_H_

#include "global.h"
#include "nullablevector.h"
#include "dbcontent/variable/variable.h"
#include "histogramviewchartview.h"
#include "viewdatawidget.h"
#include "histogram.h"
#include "results/base.h"

#include <QVariant>

#include <memory>

class HistogramView;
class HistogramViewDataSource;
class QTabWidget;
class QHBoxLayout;
class Buffer;
class DBContent;
class HistogramGenerator;

enum HistogramViewDataTool
{
    HG_DEFAULT_TOOL = 0,
    HG_SELECT_TOOL,
    HG_ZOOM_TOOL
};

/**
 * @brief Widget with tab containing BufferTableWidgets in HistogramView
 *
 */
class HistogramViewDataWidget : public ViewDataWidget
{
    Q_OBJECT
public:
    /// @brief Constructor
    HistogramViewDataWidget(HistogramView* view, HistogramViewDataSource* data_source,
                          QWidget* parent = nullptr, Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~HistogramViewDataWidget();

    unsigned int numBins() const;

    HistogramViewDataTool selectedTool() const;
    QCursor currentCursor() const;

    bool dataNotInBuffer() const;

    QPixmap renderPixmap();

    struct ViewInfo
    {
        QString  min;
        QString  max;
        uint32_t out_of_range = 0;
        bool     has_result   = false;
        bool     zoom_active  = false;
    };

    ViewInfo getViewInfo() const;

signals:
    void exportDoneSignal(bool cancelled);

public slots:
    void exportDataSlot(bool overwrite);
    void exportDoneSlot(bool cancelled);

    void resetZoomSlot();

    void rectangleSelectedSlot (unsigned int index1, unsigned int index2);

    void invertSelectionSlot();
    void clearSelectionSlot();

protected:
    virtual void toolChanged_impl(int mode) override;
    virtual void loadingStarted_impl() override;
    virtual void loadingDone_impl() override;
    virtual void updateData_impl(bool requires_reset) override;
    virtual void clearData_impl() override;
    virtual bool redrawData_impl(bool recompute) override;
    virtual void liveReload_impl() override;

    void resetCounts();

    void updateGenerator();
    void updateGeneratorFromData();
    void updateGeneratorFromResults();

    bool updateChart();

    void selectData(unsigned int index1, unsigned int index2);
    void zoomToSubrange(unsigned int index1, unsigned int index2);

    static const unsigned int NumBins     = 20;
    static const int          LabelAngleX = 85;

    static const QColor       ColorSelected;
    static const QColor       ColorCAT001;
    static const QColor       ColorCAT010;
    static const QColor       ColorCAT020;
    static const QColor       ColorCAT021;
    static const QColor       ColorCAT048;
    static const QColor       ColorCAT062;
    static const QColor       ColorRefTraj;

    QHBoxLayout*             main_layout_{nullptr};
    HistogramView*           view_       {nullptr};
    HistogramViewDataSource* data_source_{nullptr};

    std::map<std::string, QColor>                  colors_;
    QCursor                                        current_cursor_{Qt::CrossCursor};
    HistogramViewDataTool                          selected_tool_ {HG_DEFAULT_TOOL};

    std::unique_ptr<QtCharts::HistogramViewChartView> chart_view_;
    std::unique_ptr<HistogramGenerator>               histogram_generator_;

    bool data_not_in_buffer_ {false};
};

#endif /* HISTOGRAMVIEWDATAWIDGET_H_ */
