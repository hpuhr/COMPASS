
#include "targetlistwidget.h"
#include "targetmodel.h"
#include "targetfilterdialog.h"

#include "compass.h"
#include "taskmanager.h"
#include "evaluationmanager.h"
#include "dbcontentmanager.h"

#include "files.h"
#include "logger.h"

#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QApplication>
#include <QThread>
#include <QToolBar>

using namespace std;

namespace dbContent 
{

TargetListWidget::TargetListWidget(TargetModel& model, DBContentManager& dbcont_manager)
    : ToolBoxWidget(), model_(model), dbcont_manager_(dbcont_manager)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

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

    showMainColumns(model_.showMainColumns());
    showDurationColumns(model_.showDurationColumns());
    showSecondaryColumns(model_.showSecondaryColumns());
}

/**
 */
QIcon TargetListWidget::toolIcon() const
{
    return QIcon(Utils::Files::getIconFilepath("targets.png").c_str());
}

/**
 */
std::string TargetListWidget::toolName() const 
{
    return "Targets";
}

/**
 */
std::string TargetListWidget::toolInfo() const 
{
    return "Targets";
}

/**
 */
std::vector<std::string> TargetListWidget::toolLabels() const 
{
    return { "Targets" };
}

/**
 */
toolbox::ScreenRatio TargetListWidget::defaultScreenRatio() const 
{
    return ToolBoxWidget::defaultScreenRatio();
}

/**
 */
void TargetListWidget::addToConfigMenu(QMenu* menu) 
{
    QAction* all_action = menu->addAction("Use All");
    connect (all_action, &QAction::triggered, this, &TargetListWidget::useAllSlot);

    QAction* none_action = menu->addAction("Use None");
    connect (none_action, &QAction::triggered, this, &TargetListWidget::useNoneSlot);

    QAction* clear_action = menu->addAction("Clear Comments");
    connect (clear_action, &QAction::triggered, this, &TargetListWidget::clearCommentsSlot);

    QAction* filter_action = menu->addAction("Filter Targets...");
    connect (filter_action, &QAction::triggered, this, &TargetListWidget::filterSlot);

    menu->addSeparator();

    QMenu* column_menu = menu->addMenu("Columns");

    // auto main_cols_action = column_menu->addAction("Main");
    // main_cols_action->setCheckable(true);
    // main_cols_action->setChecked(model_.showMainColumns());

    // connect(main_cols_action, &QAction::toggled, this, &TargetListWidget::showMainColumns);
    // connect(main_cols_action, &QAction::toggled, this, &TargetListWidget::toolsChangedSignal);

    auto dur_cols_action = column_menu->addAction("Durations");
    dur_cols_action->setCheckable(true);
    dur_cols_action->setChecked(model_.showDurationColumns());

    connect(dur_cols_action, &QAction::toggled, this, &TargetListWidget::showDurationColumns);
    connect(dur_cols_action, &QAction::toggled, this, &TargetListWidget::toolsChangedSignal);

    auto sec_cols_action = column_menu->addAction("Secondary Attributes");
    sec_cols_action->setCheckable(true);
    sec_cols_action->setChecked(model_.showSecondaryColumns());

    connect(sec_cols_action, &QAction::toggled, this, &TargetListWidget::showSecondaryColumns);
    connect(sec_cols_action, &QAction::toggled, this, &TargetListWidget::toolsChangedSignal);
}

/**
 */
void TargetListWidget::addToToolBar(QToolBar* tool_bar)
{
    // auto main_cols_action = tool_bar->addAction("M");
    // main_cols_action->setCheckable(true);
    // main_cols_action->setChecked(model_.showMainColumns());
    // main_cols_action->setToolTip("Show Main Information");

    // connect(main_cols_action, &QAction::toggled, this, &TargetListWidget::showMainColumns);

    auto dur_cols_action = tool_bar->addAction("D");
    dur_cols_action->setCheckable(true);
    dur_cols_action->setChecked(model_.showDurationColumns());
    dur_cols_action->setToolTip("Show Durations");

    connect(dur_cols_action, &QAction::toggled, this, &TargetListWidget::showDurationColumns);

    auto sec_cols_action = tool_bar->addAction("SA");
    sec_cols_action->setCheckable(true);
    sec_cols_action->setChecked(model_.showSecondaryColumns());
    sec_cols_action->setToolTip("Show Secondary Attributes");

    connect(sec_cols_action, &QAction::toggled, this, &TargetListWidget::showSecondaryColumns);
}

/**
 */
void TargetListWidget::loadingStarted()
{
    table_view_->setEnabled(false);
}

/**
 */
void TargetListWidget::loadingDone()
{
    table_view_->setEnabled(true);
}

void TargetListWidget::resizeColumnsToContents()
{
    loginf << "TargetListWidget: resizeColumnsToContents";
    //table_model_->update();
    table_view_->resizeColumnsToContents();
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

void TargetListWidget::showMainColumns(bool show)
{
    model_.showMainColumns(show);

    for (int c : model_.mainColumns())
        table_view_->setColumnHidden(c, !show);
}

void TargetListWidget::showDurationColumns(bool show)
{
    model_.showDurationColumns(show);

    for (int c : model_.durationColumns())
        table_view_->setColumnHidden(c, !show);
}

void TargetListWidget::showSecondaryColumns(bool show)
{
    model_.showSecondaryColumns(show);

    for (int c : model_.secondaryColumns())
        table_view_->setColumnHidden(c, !show);
}

}
