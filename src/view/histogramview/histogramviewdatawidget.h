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

#include <QWidget>
#include <memory>

#include "global.h"

class HistogramView;
class HistogramViewDataSource;
class QTabWidget;
class Buffer;
class DBObject;

namespace QtCharts {
    class QChart;
    class QBarSeries;
    class QChartView;
    class QBarCategoryAxis;
    class QValueAxis;
}


/**
 * @brief Widget with tab containing BufferTableWidgets in HistogramView
 *
 */
class HistogramViewDataWidget : public QWidget
{
    Q_OBJECT

  signals:
    void exportDoneSignal(bool cancelled);
//    void showOnlySelectedSignal(bool value);
//    void usePresentationSignal(bool use_presentation);
//    void showAssociationsSignal(bool value);

  public slots:
    void loadingStartedSlot();
    /// @brief Called when new result Buffer was delivered
    void updateDataSlot(DBObject& object, std::shared_ptr<Buffer> buffer);

    void exportDataSlot(bool overwrite);
    void exportDoneSlot(bool cancelled);

//    void showOnlySelectedSlot(bool value);
//    void usePresentationSlot(bool use_presentation);
//    void showAssociationsSlot(bool value);

  public:
    /// @brief Constructor
    HistogramViewDataWidget(HistogramView* view, HistogramViewDataSource* data_source,
                          QWidget* parent = nullptr, Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~HistogramViewDataWidget();

    /// @brief Clears the table contents
    void clearTables();
    void resetModels();
    void updateToSelection();

    //void selectFirstSelectedRow();

    //AllBufferTableWidget* getAllBufferTableWidget ();

  protected:
    HistogramView* view_{nullptr};
    /// Data source
    HistogramViewDataSource* data_source_{nullptr};

    QtCharts::QBarSeries* chart_series_ {nullptr};
    QtCharts::QChart* chart_ {nullptr};
    QtCharts::QBarCategoryAxis* chart_x_axis_ {nullptr};
    QtCharts::QValueAxis* chart_y_axis_ {nullptr};
    QtCharts::QChartView* chart_view_ {nullptr};
};

#endif /* HISTOGRAMVIEWDATAWIDGET_H_ */
