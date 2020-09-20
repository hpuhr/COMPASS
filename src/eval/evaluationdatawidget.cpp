#include "evaluationdatawidget.h"
#include "evaluationdata.h"

#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QSortFilterProxyModel>

EvaluationDataWidget::EvaluationDataWidget(EvaluationData& eval_data)
    : QWidget(), eval_data_(eval_data)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    proxy_model_ = new QSortFilterProxyModel();
    proxy_model_->setSourceModel(&eval_data);

    table_view_ = new QTableView();
    table_view_->setModel(proxy_model_);
    table_view_->setSortingEnabled(true);
    table_view_->sortByColumn(0, Qt::AscendingOrder);
    table_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_view_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_view_->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    //table_view_->setIconSize(QSize(24, 24));
    table_view_->setWordWrap(true);
    table_view_->reset();
    // update done later

    table_view_->resizeColumnsToContents();
    table_view_->resizeRowsToContents();
    main_layout->addWidget(table_view_);

    setLayout(main_layout);
}
