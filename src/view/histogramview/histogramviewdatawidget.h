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
#include "histogramviewdatatoolwidget.h"
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

/**
 * @brief Widget with tab containing BufferTableWidgets in HistogramView
 *
 */
class HistogramViewDataWidget : public ViewDataWidget
{
    Q_OBJECT

  signals:
    void exportDoneSignal(bool cancelled);

  public slots:
    void loadingStartedSlot();
    /// @brief Called when new result Buffer was delivered
    void updateDataSlot(const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset);
    void loadingDoneSlot();

    void exportDataSlot(bool overwrite);
    void exportDoneSlot(bool cancelled);

    void resetZoomSlot();

    void rectangleSelectedSlot (unsigned int index1, unsigned int index2);

    void invertSelectionSlot();
    void clearSelectionSlot();

  public:
    /// @brief Constructor
    HistogramViewDataWidget(HistogramView* view, HistogramViewDataSource* data_source,
                          QWidget* parent = nullptr, Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~HistogramViewDataWidget();

    void clear();
    void updateView();
    void updateChart();

    unsigned int numBins() const;

    HistogramViewDataTool selectedTool() const;
    QCursor currentCursor() const;

    bool showsData() const;
    bool dataNotInBuffer() const;

    QPixmap renderPixmap();

protected:
    virtual void toolChanged_impl(int mode) override;

    void updateFromData();
    void updateFromResults();

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

    std::map<std::string, std::shared_ptr<Buffer>> buffers_;

    std::map<std::string, QColor>                  colors_;
    QCursor                                        current_cursor_{Qt::CrossCursor};
    HistogramViewDataTool                          selected_tool_ {HG_DEFAULT_TOOL};

    std::unique_ptr<QtCharts::HistogramViewChartView> chart_view_;
    std::unique_ptr<HistogramGenerator>               histogram_generator_;

    bool shows_data_         {false};
    bool data_not_in_buffer_ {false};
};

#endif /* HISTOGRAMVIEWDATAWIDGET_H_ */
