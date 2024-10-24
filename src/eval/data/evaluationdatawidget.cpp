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

#include "evaluationdatawidget.h"
#include "evaluationdata.h"
#include "evaluationmanager.h"
#include "logger.h"

#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QToolBar>

EvaluationDataWidget::EvaluationDataWidget(EvaluationData& eval_data, EvaluationManager& eval_man)
    : QWidget(), eval_data_(eval_data), eval_man_(eval_man)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    // toolbar
//    toolbar_ = new QToolBar("Tools");

//    QWidget* spacer = new QWidget();
//    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    toolbar_->addWidget(spacer);

//    toolbar_->addAction("Change Usage");

//    connect(toolbar_, &QToolBar::actionTriggered, this, &EvaluationDataWidget::actionTriggeredSlot);

//    main_layout->addWidget(toolbar_);

    // table
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
    table_view_->setContextMenuPolicy(Qt::CustomContextMenu);
    table_view_->setWordWrap(true);
    table_view_->reset();
    // update done later

    connect(table_view_, &QTableView::customContextMenuRequested,
            this, &EvaluationDataWidget::customContextMenuSlot);

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

//void EvaluationDataWidget::actionTriggeredSlot(QAction* action)
//{
//    QMenu menu;

//    QAction* all_action = new QAction("Use All", this);
//    connect (all_action, &QAction::triggered, this, &EvaluationDataWidget::useAllSlot);
//    menu.addAction(all_action);

//    QAction* none_action = new QAction("Use None", this);
//    connect (none_action, &QAction::triggered, this, &EvaluationDataWidget::useNoneSlot);
//    menu.addAction(none_action);

//    QAction* clear_action = new QAction("Clear Comments", this);
//    connect (clear_action, &QAction::triggered, this, &EvaluationDataWidget::clearCommentsSlot);
//    menu.addAction(clear_action);

////    QAction* filter_action = new QAction("Filter", this);
////    connect (filter_action, &QAction::triggered, this, &EvaluationDataWidget::filterSlot);
////    menu.addAction(filter_action);

//    menu.exec(QCursor::pos());
//}

//void EvaluationDataWidget::useAllSlot()
//{
//    eval_data_.setUseAllTargetData(true);
//}

//void EvaluationDataWidget::useNoneSlot()
//{
//    eval_data_.setUseAllTargetData(false);
//}

//void EvaluationDataWidget::clearCommentsSlot()
//{
//    eval_data_.clearComments();
//}

//void EvaluationDataWidget::filterSlot()
//{
//    EvaluationDataFilterDialog& dialog = eval_data_.dialog();
//    dialog.show();
//}

void EvaluationDataWidget::customContextMenuSlot(const QPoint& p)
{
    logdbg << "EvaluationDataWidget: customContextMenuSlot";

    assert (table_view_);

    QModelIndex index = table_view_->indexAt(p);
    assert (index.isValid());

    auto const source_index = proxy_model_->mapToSource(index);
    assert (source_index.isValid());

    const EvaluationTargetData& target = eval_data_.getTargetOf(source_index);

    unsigned int utn = target.utn_;
    loginf << "EvaluationDataWidget: customContextMenuSlot: row " << index.row() << " utn " << utn;
    assert (eval_man_.getData().hasTargetData(utn));

    QMenu menu;

    QAction* action = new QAction("Show Full UTN", this);
    connect (action, &QAction::triggered, this, &EvaluationDataWidget::showFullUTNSlot);
    action->setData(utn);
    menu.addAction(action);

    QAction* action2 = new QAction("Show Surrounding Data", this);
    connect (action2, &QAction::triggered, this, &EvaluationDataWidget::showSurroundingDataSlot);
    action2->setData(utn);
    menu.addAction(action2);

    menu.exec(table_view_->viewport()->mapToGlobal(p));
}

void EvaluationDataWidget::showFullUTNSlot ()
{
    QAction* action = dynamic_cast<QAction*> (QObject::sender());
    assert (action);

    unsigned int utn = action->data().toUInt();

    loginf << "EvaluationDataWidget: showFullUTNSlot: utn " << utn;

    eval_man_.showFullUTN(utn);
}

void EvaluationDataWidget::showSurroundingDataSlot ()
{
    QAction* action = dynamic_cast<QAction*> (QObject::sender());
    assert (action);

    unsigned int utn = action->data().toUInt();

    loginf << "EvaluationDataWidget: showSurroundingDataSlot: utn " << utn;

    eval_man_.showSurroundingData(utn);
}

void EvaluationDataWidget::currentRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
    itemClicked(current);
}

void EvaluationDataWidget::itemClicked(const QModelIndex& index)
{
    if (!index.isValid())
    {
        loginf << "EvaluationDataWidget: itemClicked: invalid index";
        return;
    }

    auto const source_index = proxy_model_->mapToSource(index);
    assert (source_index.isValid());

    const EvaluationTargetData& target = eval_data_.getTargetOf(source_index);

    loginf << "EvaluationDataWidget: itemClicked: current target " << target.utn_;
    //restore_focus_ = true;

    eval_man_.showUTN(target.utn_);
}
