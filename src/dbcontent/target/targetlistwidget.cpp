#include "targetlistwidget.h"
#include "targetmodel.h"
#include "logger.h"

#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QToolBar>

using namespace std;

namespace dbContent {


TargetListWidget::TargetListWidget(TargetModel& model)
    : QWidget(), model_(model)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    // toolbar
    toolbar_ = new QToolBar("Tools");

    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolbar_->addWidget(spacer);

    toolbar_->addAction("Change Usage");

    connect(toolbar_, &QToolBar::actionTriggered, this, &TargetListWidget::actionTriggeredSlot);

    main_layout->addWidget(toolbar_);

    // table
    proxy_model_ = new QSortFilterProxyModel();
    proxy_model_->setSourceModel(&model_);

    table_view_ = new QTableView();
    table_view_->setModel(proxy_model_);
    table_view_->setSortingEnabled(true);
    table_view_->sortByColumn(1, Qt::AscendingOrder);
    table_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_view_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_view_->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    //table_view_->setIconSize(QSize(24, 24));
    table_view_->setContextMenuPolicy(Qt::CustomContextMenu);
    table_view_->setWordWrap(true);
    table_view_->reset();
    // update done later

    connect(table_view_, &QTableView::customContextMenuRequested,
            this, &TargetListWidget::customContextMenuSlot);

    connect(table_view_->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &TargetListWidget::currentRowChanged);

    table_view_->resizeColumnsToContents();
    table_view_->resizeRowsToContents();
    main_layout->addWidget(table_view_);

    setLayout(main_layout);
}

void TargetListWidget::resizeColumnsToContents()
{
    loginf << "TargetListWidget: resizeColumnsToContents";
    //table_model_->update();
    table_view_->resizeColumnsToContents();
}

void TargetListWidget::actionTriggeredSlot(QAction* action)
{
    QMenu menu;

    QAction* all_action = new QAction("Use All", this);
    connect (all_action, &QAction::triggered, this, &TargetListWidget::useAllSlot);
    menu.addAction(all_action);

    QAction* none_action = new QAction("Use None", this);
    connect (none_action, &QAction::triggered, this, &TargetListWidget::useNoneSlot);
    menu.addAction(none_action);

    QAction* clear_action = new QAction("Clear Comments", this);
    connect (clear_action, &QAction::triggered, this, &TargetListWidget::clearCommentsSlot);
    menu.addAction(clear_action);

    QAction* filter_action = new QAction("Filter", this);
    connect (filter_action, &QAction::triggered, this, &TargetListWidget::filterSlot);
    menu.addAction(filter_action);

    menu.exec(QCursor::pos());
}

void TargetListWidget::useAllSlot()
{
    //eval_data_.setUseAllTargetData(true);
}

void TargetListWidget::useNoneSlot()
{
    //eval_data_.setUseAllTargetData(false);
}

void TargetListWidget::clearCommentsSlot()
{
    //eval_data_.clearComments();
}

void TargetListWidget::filterSlot()
{
//    EvaluationDataFilterDialog& dialog = eval_data_.dialog();
//    dialog.show();
}

void TargetListWidget::customContextMenuSlot(const QPoint& p)
{
    logdbg << "TargetListWidget: customContextMenuSlot";

    assert (table_view_);

    QModelIndex index = table_view_->indexAt(p);
    assert (index.isValid());

    auto const source_index = proxy_model_->mapToSource(index);
    assert (source_index.isValid());

//    const EvaluationTargetData& target = eval_data_.getTargetOf(source_index);

//    unsigned int utn = target.utn_;
//    loginf << "TargetListWidget: customContextMenuSlot: row " << index.row() << " utn " << utn;
//    assert (eval_man_.getData().hasTargetData(utn));

//    QMenu menu;

//    QAction* action = new QAction("Show Full UTN", this);
//    connect (action, &QAction::triggered, this, &TargetListWidget::showFullUTNSlot);
//    action->setData(utn);
//    menu.addAction(action);

//    QAction* action2 = new QAction("Show Surrounding Data", this);
//    connect (action2, &QAction::triggered, this, &TargetListWidget::showSurroundingDataSlot);
//    action2->setData(utn);
//    menu.addAction(action2);

//    menu.exec(table_view_->viewport()->mapToGlobal(p));
}

void TargetListWidget::showFullUTNSlot ()
{
    QAction* action = dynamic_cast<QAction*> (QObject::sender());
    assert (action);

    unsigned int utn = action->data().toUInt();

    loginf << "TargetListWidget: showFullUTNSlot: utn " << utn;

    //eval_man_.showFullUTN(utn);
}

void TargetListWidget::showSurroundingDataSlot ()
{
    QAction* action = dynamic_cast<QAction*> (QObject::sender());
    assert (action);

    unsigned int utn = action->data().toUInt();

    loginf << "TargetListWidget: showSurroundingDataSlot: utn " << utn;

    //eval_man_.showSurroundingData(utn);
}

void TargetListWidget::currentRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
    if (!current.isValid())
    {
        loginf << "TargetListWidget: currentRowChanged: invalid index";
        return;
    }

    auto const source_index = proxy_model_->mapToSource(current);
    assert (source_index.isValid());

    //    const EvaluationTargetData& target = eval_data_.getTargetOf(source_index);

    //    loginf << "TargetListWidget: currentRowChanged: current target " << target.utn_;

    //    eval_man_.showUTN(target.utn_);
}

}
