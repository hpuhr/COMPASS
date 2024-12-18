#include "targetlistwidget.h"
#include "targetmodel.h"
#include "dbcontentmanager.h"
#include "targetfilterdialog.h"
#include "logger.h"
#include "compass.h"
#include "taskmanager.h"
#include "evaluationmanager.h"

#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QToolBar>
#include <QApplication>
#include <QThread>

using namespace std;

namespace dbContent {


TargetListWidget::TargetListWidget(TargetModel& model, DBContentManager& dbcont_manager)
    : QWidget(), model_(model), dbcont_manager_(dbcont_manager)
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
    table_view_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    table_view_->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    table_view_->horizontalHeader()->setMaximumSectionSize(300);
    //table_view_->setIconSize(QSize(24, 24));
    table_view_->setContextMenuPolicy(Qt::CustomContextMenu);
    table_view_->setWordWrap(true);
    table_view_->reset();
    // update done later

    connect(table_view_, &QTableView::customContextMenuRequested,
            this, &TargetListWidget::customContextMenuSlot);

    // connect(table_view_->selectionModel(), &QItemSelectionModel::currentRowChanged,
    //         this, &TargetListWidget::currentRowChanged);

    connect(table_view_->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &TargetListWidget::selectionChanged);

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
    model_.setUseAllTargetData(true);
}

void TargetListWidget::useNoneSlot()
{
    model_.setUseAllTargetData(false);
}

void TargetListWidget::clearCommentsSlot()
{
    model_.clearComments();
}

void TargetListWidget::filterSlot()
{
    loginf << "TargetListWidget: filterSlot";

    TargetFilterDialog dialog (model_);
    dialog.exec();
}

void TargetListWidget::customContextMenuSlot(const QPoint& p)
{
    logdbg << "TargetListWidget: customContextMenuSlot";

    assert (table_view_);

    QModelIndex index = table_view_->indexAt(p);
    if (!index.isValid())
        return;

    auto const source_index = proxy_model_->mapToSource(index);
    assert (source_index.isValid());

    const dbContent::Target& target = model_.getTargetOf(source_index);

    QMenu menu;

    QAction* action2 = new QAction("Show Surrounding Data", this);
    connect (action2, &QAction::triggered, this, &TargetListWidget::showSurroundingDataSlot);
    action2->setData(target.utn_);
    menu.addAction(action2);

    menu.exec(table_view_->viewport()->mapToGlobal(p));
}

// void TargetListWidget::showFullUTNSlot ()
// {
//     QAction* action = dynamic_cast<QAction*> (QObject::sender());
//     assert (action);

//     unsigned int utn = action->data().toUInt();

//     loginf << "TargetListWidget: showFullUTNSlot: utn " << utn;

//     //eval_man_.showFullUTN(utn);
// }

void TargetListWidget::showSurroundingDataSlot ()
{
    auto& dbcont_man = COMPASS::instance().dbContentManager();

    while (dbcont_man.loadInProgress())
    {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep(10);
    }

    QAction* action = dynamic_cast<QAction*> (QObject::sender());
    assert (action);

    unsigned int utn = action->data().toUInt();

    loginf << "TargetListWidget: showSurroundingDataSlot: utn " << utn;

    dbcont_man.showSurroundingData(utn);
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

    const dbContent::Target& target = model_.getTargetOf(source_index);

    loginf << "TargetListWidget: currentRowChanged: current target " << target.utn_;

    dbcont_manager_.showUTN(target.utn_);
}

void TargetListWidget::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    vector<unsigned int> selected_utns;

    QModelIndex current_index;
    const QModelIndexList items = table_view_->selectionModel()->selectedRows();

    logdbg << "TargetListWidget: selectionChanged: list size " << items.count();

    foreach (current_index, items)
    {
        auto const source_index = proxy_model_->mapToSource(current_index);
        assert (source_index.isValid());

        const dbContent::Target& target = model_.getTargetOf(source_index);

        logdbg << "TargetListWidget: selectionChanged: utn " << target.utn_;

        selected_utns.push_back(target.utn_);
    }

    logdbg << "TargetListWidget: selectionChanged: num targets " << selected_utns.size();

    dbcont_manager_.showUTNs(selected_utns);
}

}
