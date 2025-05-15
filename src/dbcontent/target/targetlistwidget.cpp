
#include "targetlistwidget.h"
#include "targetmodel.h"
#include "evaluationtargetfilterdialog.h"

#include "compass.h"
#include "taskmanager.h"
#include "evaluationmanager.h"
#include "dbcontentmanager.h"
#include "evaluationtimestampconditionsdialog.h"

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
    showEvalColumns(model_.showEvalColumns());
    showDurationColumns(model_.showDurationColumns());
    showModeSColumns(model_.showModeSColumns());
    showModeACColumns(model_.showModeACColumns());
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
    QAction* clear_action = menu->addAction("Clear Comments");
    connect (clear_action, &QAction::triggered, this, &TargetListWidget::clearCommentsSlot);

    QMenu* column_menu = menu->addMenu("Edit Columns");

    // auto main_cols_action = column_menu->addAction("Main");
    // main_cols_action->setCheckable(true);
    // main_cols_action->setChecked(model_.showMainColumns());

    // connect(main_cols_action, &QAction::toggled, this, &TargetListWidget::showMainColumns);
    // connect(main_cols_action, &QAction::toggled, this, &TargetListWidget::toolsChangedSignal);

    auto eval_cols_action = column_menu->addAction("Evaluation");
    eval_cols_action->setCheckable(true);
    eval_cols_action->setChecked(model_.showEvalColumns());

    connect(eval_cols_action, &QAction::toggled, this, &TargetListWidget::showEvalColumns);
    connect(eval_cols_action, &QAction::toggled, this, &TargetListWidget::toolsChangedSignal);

    auto dur_cols_action = column_menu->addAction("Duration");
    dur_cols_action->setCheckable(true);
    dur_cols_action->setChecked(model_.showDurationColumns());

    connect(dur_cols_action, &QAction::toggled, this, &TargetListWidget::showDurationColumns);
    connect(dur_cols_action, &QAction::toggled, this, &TargetListWidget::toolsChangedSignal);

    auto mode_s_cols_action = column_menu->addAction("Mode S");
    mode_s_cols_action->setCheckable(true);
    mode_s_cols_action->setChecked(model_.showModeSColumns());

    connect(mode_s_cols_action, &QAction::toggled, this, &TargetListWidget::showModeSColumns);
    connect(mode_s_cols_action, &QAction::toggled, this, &TargetListWidget::toolsChangedSignal);

    auto mode_ac_cols_action = column_menu->addAction("Mode A/C");
    mode_ac_cols_action->setCheckable(true);
    mode_ac_cols_action->setChecked(model_.showModeACColumns());

    connect(mode_ac_cols_action, &QAction::toggled, this, &TargetListWidget::showModeACColumns);
    connect(mode_ac_cols_action, &QAction::toggled, this, &TargetListWidget::toolsChangedSignal);

    menu->addSeparator();

    QMenu* eval_menu = menu->addMenu("Evaluation");

    QAction* all_action = eval_menu->addAction("Use All");
    connect (all_action, &QAction::triggered, this, &TargetListWidget::evalUseAllSlot);

    QAction* none_action = eval_menu->addAction("Use None");
    connect (none_action, &QAction::triggered, this, &TargetListWidget::evalUseNoneSlot);

    QAction* filter_action = eval_menu->addAction("Filter Usage ...");
    connect (filter_action, &QAction::triggered, this, &TargetListWidget::evalFilterSlot);

    eval_menu->addSeparator();

    QAction* exlcude_tw_action = eval_menu->addAction("Edit Excluded Time Windows ...");
    connect (exlcude_tw_action, &QAction::triggered, this, &TargetListWidget::evalExcludeTimeWindowsSlot);
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

    auto eval_cols_action = tool_bar->addAction("Evaluation");
    eval_cols_action->setCheckable(true);
    eval_cols_action->setChecked(model_.showEvalColumns());
    eval_cols_action->setToolTip("Show Evaluation Columns");

    connect(eval_cols_action, &QAction::toggled, this, &TargetListWidget::showEvalColumns);

    auto dur_cols_action = tool_bar->addAction("Duration");
    dur_cols_action->setCheckable(true);
    dur_cols_action->setChecked(model_.showDurationColumns());
    dur_cols_action->setToolTip("Show Duration Columns");

    connect(dur_cols_action, &QAction::toggled, this, &TargetListWidget::showDurationColumns);

    auto mode_s_cols_action = tool_bar->addAction("Mode S");
    mode_s_cols_action->setCheckable(true);
    mode_s_cols_action->setChecked(model_.showModeSColumns());
    mode_s_cols_action->setToolTip("Show Mode S Columns");

    connect(mode_s_cols_action, &QAction::toggled, this, &TargetListWidget::showModeSColumns);

    auto mode_ac_cols_action = tool_bar->addAction("Mode A/C");
    mode_ac_cols_action->setCheckable(true);
    mode_ac_cols_action->setChecked(model_.showModeACColumns());
    mode_ac_cols_action->setToolTip("Show Mode A/C Columns");

    connect(mode_ac_cols_action, &QAction::toggled, this, &TargetListWidget::showModeACColumns);
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

void TargetListWidget::evalUseAllSlot()
{
    model_.setUseAllTargetData(true);
}

void TargetListWidget::evalUseNoneSlot()
{
    model_.setUseAllTargetData(false);
}

void TargetListWidget::clearCommentsSlot()
{
    model_.clearComments();
}

void TargetListWidget::evalExcludeTimeWindowsSlot()
{
    loginf << "TargetListWidget: filterSlot";

    EvaluationTimestampConditionsDialog dialog (COMPASS::instance().evaluationManager());
    dialog.exec();
}

void TargetListWidget::evalFilterSlot()
{
    loginf << "TargetListWidget: evalFilterSlot";

    EvaluationTargetFilterDialog dialog (COMPASS::instance().evaluationManager().targetFilter(), model_);
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

void TargetListWidget::showEvalColumns(bool show)
{
    model_.showEvalColumns(show);

    for (int c : model_.evalColumns())
        table_view_->setColumnHidden(c, !show);
}

void TargetListWidget::showDurationColumns(bool show)
{
    model_.showDurationColumns(show);

    for (int c : model_.durationColumns())
        table_view_->setColumnHidden(c, !show);
}

void TargetListWidget::showModeSColumns(bool show)
{
    model_.showModeSColumns(show);

    for (int c : model_.modeSColumns())
        table_view_->setColumnHidden(c, !show);
}

void TargetListWidget::showModeACColumns(bool show)
{
    model_.showModeACColumns(show);

    for (int c : model_.modeACColumns())
        table_view_->setColumnHidden(c, !show);
}

}
