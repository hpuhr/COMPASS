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

#ifndef SCATTERPLOTVIEWDATAWIDGET_H_
#define SCATTERPLOTVIEWDATAWIDGET_H_

#include "global.h"
#include "nullablevector.h"
#include "dbovariable.h"

#include <QWidget>
#include <QVariant>

#include <memory>
#include <limits>

class ScatterPlotView;
class ScatterPlotViewDataSource;
class QTabWidget;
class Buffer;
class DBObject;

namespace QtCharts {
    class QChart;
    class QScatterSeries;
    class QChartView;
    class QBarCategoryAxis;
    class QValueAxis;
}


/**
 * @brief Widget with tab containing BufferTableWidgets in ScatterPlotView
 *
 */
class ScatterPlotViewDataWidget : public QWidget
{
    Q_OBJECT

  signals:
//    void showOnlySelectedSignal(bool value);
//    void usePresentationSignal(bool use_presentation);
//    void showAssociationsSignal(bool value);

  public slots:
    void loadingStartedSlot();
    /// @brief Called when new result Buffer was delivered
    void updateDataSlot(DBObject& object, std::shared_ptr<Buffer> buffer);

//    void showOnlySelectedSlot(bool value);
//    void usePresentationSlot(bool use_presentation);
//    void showAssociationsSlot(bool value);

  public:
    /// @brief Constructor
    ScatterPlotViewDataWidget(ScatterPlotView* view, ScatterPlotViewDataSource* data_source,
                          QWidget* parent = nullptr, Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~ScatterPlotViewDataWidget();

    void update();
    void clear();

  protected:
    ScatterPlotView* view_{nullptr};
    /// Data source
    ScatterPlotViewDataSource* data_source_{nullptr};

    std::map<std::string, std::shared_ptr<Buffer>> buffers_;
    std::map<std::string, unsigned int> buffer_x_counts_;
    std::map<std::string, unsigned int> buffer_y_counts_;

    std::map<std::string, std::vector<double>> x_values_;
    std::map<std::string, std::vector<double>> y_values_;

    std::map<std::string, QColor> colors_;

    //QtCharts::QScatterSeries* chart_series_ {nullptr};
    QtCharts::QChart* chart_ {nullptr};
    QtCharts::QChartView* chart_view_ {nullptr};

    bool canUpdateFromDataX(std::string dbo_name);
    void updateFromDataX(std::string dbo_name, unsigned int current_size);
    bool canUpdateFromDataY(std::string dbo_name);
    void updateFromDataY(std::string dbo_name, unsigned int current_size);
    void updateFromAllData();
    void updateChart();

    template<typename T>
    void appendData(NullableVector<T>& data, std::vector<double>& target, unsigned int last_size,
                    unsigned int current_size)
    {
        for (unsigned int cnt=last_size; cnt < current_size; ++cnt)
        {
            if (data.isNull(cnt))
                target.push_back(std::numeric_limits<double>::signaling_NaN());
            else
                target.push_back(data.get(cnt));
        }
    }
};

#endif /* SCATTERPLOTVIEWDATAWIDGET_H_ */
