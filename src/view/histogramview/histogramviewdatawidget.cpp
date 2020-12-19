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

#include "histogramviewdatawidget.h"
#include "compass.h"
#include "buffer.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "histogramviewdatasource.h"
#include "logger.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QTabWidget>

#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

QT_CHARTS_USE_NAMESPACE

HistogramViewDataWidget::HistogramViewDataWidget(HistogramView* view, HistogramViewDataSource* data_source,
                                                 QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f), view_(view), data_source_(data_source)
{
    assert(data_source_);

    QHBoxLayout* layout = new QHBoxLayout();

    //    tab_widget_ = new QTabWidget();
    //    layout->addWidget(tab_widget_);

    QBarSet *set0 = new QBarSet("Jane");
    QBarSet *set1 = new QBarSet("John");
    QBarSet *set2 = new QBarSet("Axel");
    QBarSet *set3 = new QBarSet("Mary");
    QBarSet *set4 = new QBarSet("Samantha");

    *set0 << 1 << 2 << 3 << 4 << 5 << 6;
    *set1 << 5 << 0 << 0 << 4 << 0 << 7;
    *set2 << 3 << 5 << 8 << 13 << 8 << 5;
    *set3 << 5 << 6 << 7 << 3 << 4 << 5;
    *set4 << 9 << 7 << 5 << 3 << 1 << 2;

    QBarSeries *series = new QBarSeries();
    series->setUseOpenGL(true);
    series->append(set0);
    series->append(set1);
    series->append(set2);
    series->append(set3);
    series->append(set4);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Simple barchart example");
    //chart->setAnimationOptions(QChart::SeriesAnimations);

    QStringList categories;
    categories << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun";

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    //QLogValueAxis *axisY = new QLogValueAxis();
//    axisY->setBase(8.0);
//    axisY->setMinorTickCount(-1);
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0,15);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setRubberBand(QChartView::RectangleRubberBand);

    layout->addWidget(chartView);

    setLayout(layout);
}

HistogramViewDataWidget::~HistogramViewDataWidget()
{
    // TODO
    // buffer_tables_.clear();
}

void HistogramViewDataWidget::clearTables()
{
    logdbg << "HistogramViewDataWidget: updateTables: start";
    // TODO
    //  std::map <DB_OBJECT_TYPE, BufferTableWidget*>::iterator it;

    //  for (it = buffer_tables_.begin(); it != buffer_tables_.end(); it++)
    //  {
    //    it->second->show (0, 0, false);
    //  }

    logdbg << "HistogramViewDataWidget: updateTables: end";
}

void HistogramViewDataWidget::loadingStartedSlot()
{
    // clear
}

void HistogramViewDataWidget::updateDataSlot(DBObject& object, std::shared_ptr<Buffer> buffer)
{
    logdbg << "HistogramViewDataWidget: updateTables: start";


    logdbg << "HistogramViewDataWidget: updateTables: end";
}

void HistogramViewDataWidget::exportDataSlot(bool overwrite)
{
    logdbg << "HistogramViewDataWidget: exportDataSlot";

}

void HistogramViewDataWidget::exportDoneSlot(bool cancelled) { emit exportDoneSignal(cancelled); }

//void HistogramViewDataWidget::showOnlySelectedSlot(bool value)
//{
//    loginf << "HistogramViewDataWidget: showOnlySelectedSlot: " << value;
//    emit showOnlySelectedSignal(value);
//}

//void HistogramViewDataWidget::usePresentationSlot(bool use_presentation)
//{
//    loginf << "HistogramViewDataWidget: usePresentationSlot";

//    emit usePresentationSignal(use_presentation);
//}

//void HistogramViewDataWidget::showAssociationsSlot(bool value)
//{
//    loginf << "HistogramViewDataWidget: showAssociationsSlot: " << value;
//    emit showAssociationsSignal(value);
//}

//void HistogramViewDataWidget::resetModels()
//{
//    if (all_buffer_table_widget_)
//        all_buffer_table_widget_->resetModel();

//    for (auto& table_widget_it : buffer_tables_)
//        table_widget_it.second->resetModel();
//}

//void HistogramViewDataWidget::updateToSelection()
//{
//    if (all_buffer_table_widget_)
//        all_buffer_table_widget_->updateToSelection();

//    for (auto& table_widget_it : buffer_tables_)
//        table_widget_it.second->updateToSelection();
//}

//void HistogramViewDataWidget::selectFirstSelectedRow()
//{
//    if (all_buffer_table_widget_)
//        all_buffer_table_widget_->selectSelectedRows();
//}

//AllBufferTableWidget* HistogramViewDataWidget::getAllBufferTableWidget ()
//{
//    assert (all_buffer_table_widget_);
//    return all_buffer_table_widget_;
//}
