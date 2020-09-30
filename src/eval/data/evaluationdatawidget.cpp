#include "evaluationdatawidget.h"
#include "evaluationdata.h"
#include "evaluationmanager.h"
#include "logger.h"

#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QSortFilterProxyModel>

EvaluationDataWidget::EvaluationDataWidget(EvaluationData& eval_data, EvaluationManager& eval_man)
    : QWidget(), eval_data_(eval_data), eval_man_(eval_man)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    proxy_model_ = new QSortFilterProxyModel();
    proxy_model_->setSourceModel(&eval_data_);

    table_view_ = new QTableView();
    table_view_->setModel(proxy_model_);
    table_view_->setSortingEnabled(true);
    table_view_->sortByColumn(1, Qt::AscendingOrder);
    table_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_view_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_view_->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    //table_view_->setIconSize(QSize(24, 24));
    table_view_->setWordWrap(true);
    table_view_->reset();
    // update done later

    connect(table_view_->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &EvaluationDataWidget::currentRowChanged);

    table_view_->resizeColumnsToContents();
    table_view_->resizeRowsToContents();
    main_layout->addWidget(table_view_);

    setLayout(main_layout);
}

void EvaluationDataWidget::resizeColumnsToContents()
{
    loginf << "EvaluationDataWidget: resizeColumnsToContents";
    //table_model_->update();
    table_view_->resizeColumnsToContents();
}

void EvaluationDataWidget::currentRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
    if (!current.isValid())
    {
        loginf << "EvaluationDataWidget: currentRowChanged: invalid index";
        return;
    }

    auto const source_index = proxy_model_->mapToSource(current);
    assert (source_index.isValid());

    const EvaluationTargetData& target = eval_data_.getTargetOf(source_index);

    loginf << "EvaluationDataWidget: currentRowChanged: current target " << target.utn_;
    //restore_focus_ = true;

    eval_man_.showUTN(target.utn_);
}
