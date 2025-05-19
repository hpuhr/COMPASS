
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
#include "stringconv.h"

#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QApplication>
#include <QThread>
#include <QToolBar>

using namespace std;
using namespace Utils;

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

    COMPASS::instance().evaluationManager().saveTimeConstraints();
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

    QMenu menu;

    QAction* show_action = menu.addAction("Show Surrounding Data");
    connect (show_action, &QAction::triggered, this, &TargetListWidget::showSurroundingDataSlot);

    QMenu* eval_menu = menu.addMenu("Evaluation");

    QAction* use_action = eval_menu->addAction("Use Target(s)");
    connect (use_action, &QAction::triggered, this, &TargetListWidget::evalUseTargetsSlot);

    QAction* nouse_action = eval_menu->addAction("Disable Use Target(s)");
    connect (nouse_action, &QAction::triggered, this, &TargetListWidget::evalDisableUseTargetsSlot);

    menu.exec(table_view_->viewport()->mapToGlobal(p));
}

void TargetListWidget::showSurroundingDataSlot ()
{
    auto& dbcont_man = COMPASS::instance().dbContentManager();

    while (dbcont_man.loadInProgress())
    {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep(10);
    }

    std::set<unsigned int> selected_utns = selectedUTNs();

    logdbg << "TargetListWidget: showSurroundingDataSlot: utns " << String::compress(selected_utns,',');

    dbcont_man.showSurroundingData(selected_utns);
}

void TargetListWidget::evalUseTargetsSlot()
{
    loginf << "TargetListWidget: evalUseTargetsSlot";

    auto& dbcont_man = COMPASS::instance().dbContentManager();

    std::set<unsigned int> selected_utns = selectedUTNs();

    for (auto utn : selected_utns)
    {
        assert (dbcont_man.existsTarget(utn));

        auto& target = dbcont_man.target(utn);

        target.useInEval(true);
    }

    model_.updateEvalItems();
    dbcont_man.storeTargetsEvalInfo();
}

void TargetListWidget::evalDisableUseTargetsSlot()
{
    loginf << "TargetListWidget: evalDisableUseTargetsSlot";

    auto& dbcont_man = COMPASS::instance().dbContentManager();

    std::set<unsigned int> selected_utns = selectedUTNs();

    for (auto utn : selected_utns)
    {
        assert (dbcont_man.existsTarget(utn));

        auto& target = dbcont_man.target(utn);

        target.useInEval(false);
    }

    model_.updateEvalItems();
    dbcont_man.storeTargetsEvalInfo();
}

void TargetListWidget::evalExcludeTimeWindowsTargetSlot()
{
    loginf << "TargetListWidget: evalExcludeTimeWindowsTargetSlot";

    auto& dbcont_man = COMPASS::instance().dbContentManager();

    std::set<unsigned int> selected_utns = selectedUTNs();

    Utils::TimeWindowCollection filtered_time_windows;

    // collect all time windows from all targets
    for (auto utn : selected_utns)
    {
        assert (dbcont_man.existsTarget(utn));

        auto& target = dbcont_man.target(utn);


    }

    model_.updateEvalItems();
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
    std::set<unsigned int> selected_utns = selectedUTNs();

    dbcont_manager_.showUTNs(selected_utns);
}

void TargetListWidget::showMainColumns(bool show)
{
    model_.showMainColumns(show);

    for (int c : model_.mainColumns())
        table_view_->setColumnHidden(c, !show);

    table_view_->resizeColumnsToContents();
    table_view_->resizeRowsToContents();
}

void TargetListWidget::showEvalColumns(bool show)
{
    model_.showEvalColumns(show);

    for (int c : model_.evalColumns())
        table_view_->setColumnHidden(c, !show);

    table_view_->resizeColumnsToContents();
    table_view_->resizeRowsToContents();
}

void TargetListWidget::showDurationColumns(bool show)
{
    model_.showDurationColumns(show);

    for (int c : model_.durationColumns())
        table_view_->setColumnHidden(c, !show);

    table_view_->resizeColumnsToContents();
    table_view_->resizeRowsToContents();
}

void TargetListWidget::showModeSColumns(bool show)
{
    model_.showModeSColumns(show);

    for (int c : model_.modeSColumns())
        table_view_->setColumnHidden(c, !show);

    table_view_->resizeColumnsToContents();
    table_view_->resizeRowsToContents();
}

void TargetListWidget::showModeACColumns(bool show)
{
    model_.showModeACColumns(show);

    for (int c : model_.modeACColumns())
        table_view_->setColumnHidden(c, !show);

    table_view_->resizeColumnsToContents();
    table_view_->resizeRowsToContents();
}

std::set<unsigned int> TargetListWidget::selectedUTNs() const
{
    assert (table_view_);

    std::set<unsigned int> selected_indexes;

    QItemSelectionModel* selection_model = table_view_->selectionModel();

    QModelIndexList selected_indexes_list = selection_model->selectedIndexes();

    for (const QModelIndex& index : selected_indexes_list)
    {
        if (!index.isValid())
            continue;

        auto const source_index = proxy_model_->mapToSource(index);
        assert (source_index.isValid());

        const dbContent::Target& target = model_.getTargetOf(source_index);

        selected_indexes.insert(target.utn_);
    }

    loginf << "TargetListWidget: selectedUTNs: " << String::compress(selected_indexes, ',');

    return selected_indexes;
}

}
