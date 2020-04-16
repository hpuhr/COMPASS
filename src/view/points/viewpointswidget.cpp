#include "viewpointswidget.h"
#include "viewmanager.h"
#include "viewpoint.h"
#include "logger.h"
#include "viewpointstablemodel.h"

#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSortFilterProxyModel>



ViewPointsWidget::ViewPointsWidget(ViewManager& view_manager)
    : QWidget(), view_manager_(view_manager)
{
    QHBoxLayout* main_layout = new QHBoxLayout();

    table_model_ = new ViewPointsTableModel(view_manager_);

    QSortFilterProxyModel* proxy_model = new QSortFilterProxyModel();
    proxy_model->setSourceModel(table_model_);

    table_view_ = new QTableView();
    table_view_->setModel(proxy_model);
    table_view_->setSortingEnabled(true);
    table_view_->sortByColumn(0, Qt::AscendingOrder);
    table_view_->reset();
    table_view_->show();
//    table_->setEditTriggers(QAbstractItemView::AllEditTriggers);
//    table_->setColumnCount(table_columns_.size());
//    table_->setHorizontalHeaderLabels(table_columns_);
//    //table_->verticalHeader()->setVisible(false);
//    connect(table_, &QTableWidget::itemChanged, this,
//            &DBOEditDataSourcesWidget::configItemChangedSlot);
    // update done later

    table_view_->resizeColumnsToContents();
    main_layout->addWidget(table_view_);


    setLayout(main_layout);
}

ViewPointsWidget::~ViewPointsWidget()
{

}


void ViewPointsWidget::update()
{

}




