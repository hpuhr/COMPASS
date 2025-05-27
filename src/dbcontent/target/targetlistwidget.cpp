
#include "targetlistwidget.h"
#include "targetmodel.h"
#include "evaluationtargetfilterdialog.h"

#include "compass.h"
#include "taskmanager.h"
#include "evaluationmanager.h"
#include "dbcontentmanager.h"
#include "evaluationtimestampconditionsdialog.h"
#include "evaluationtargetexcludedtimewindowsdialog.h"
#include "evaluationtargetexcludedrequirementsdialog.h"
#include "evaluationcalculator.h"
#include "evaluationstandard.h"

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
#include <QMessageBox>

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
    QAction* clear_action = menu->addAction("Clear All Comments");
    connect (clear_action, &QAction::triggered, this, &TargetListWidget::clearAllCommentsSlot);

    QMenu* column_menu = menu->addMenu("Edit Columns");

    // auto main_cols_action = column_menu->addAction("Main");
    // main_cols_action->setCheckable(true);
    // main_cols_action->setChecked(model_.showMainColumns());

    // connect(main_cols_action, &QAction::toggled, this, &TargetListWidget::showMainColumns);
    // connect(main_cols_action, &QAction::toggled, this, &TargetListWidget::toolsChangedSignal);

    auto eval_cols_action = column_menu->addAction("Eval Details");
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

    QAction* all_action = eval_menu->addAction("Use All Targets");
    connect (all_action, &QAction::triggered, this, &TargetListWidget::evalUseAllSlot);

    QAction* none_action = eval_menu->addAction("Disable All Targets");
    connect (none_action, &QAction::triggered, this, &TargetListWidget::evalUseNoneSlot);

    QAction* filter_action = eval_menu->addAction("Filter Usage ...");
    connect (filter_action, &QAction::triggered, this, &TargetListWidget::evalFilterSlot);

    eval_menu->addSeparator();

    QAction* tw_action = eval_menu->addAction("Clear All Targets Excluded Time Windows");
    connect (tw_action, &QAction::triggered, this, &TargetListWidget::evalClearAllExcludeTimeWindowsSlot);

    QAction* req_action = eval_menu->addAction("Clear All Targets Excluded Requirements");
    connect (req_action, &QAction::triggered, this, &TargetListWidget::evalClearAllExcludeRequirementsSlot);

    eval_menu->addSeparator();

    QAction* exlcude_tw_action = eval_menu->addAction("Edit Global Excluded Time Windows ...");
    connect (exlcude_tw_action, &QAction::triggered, this, &TargetListWidget::evalEditGlobalExcludeTimeWindowsSlot);

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

    auto eval_cols_action = tool_bar->addAction("Eval Details");
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
    model_.setAllUseTargets(true);
}

void TargetListWidget::evalUseNoneSlot()
{
    model_.setAllUseTargets(false);
}

void TargetListWidget::evalUseSelectedTargetsSlot()
{
    loginf << "TargetListWidget: evalUseTargetsSlot";

    std::set<unsigned int> selected_utns = selectedUTNs();

    model_.setEvalUseTarget(selected_utns, true);
}

void TargetListWidget::evalDisableSelectedTargetsSlot()
{
    loginf << "TargetListWidget: evalDisableUseTargetsSlot";

    std::set<unsigned int> selected_utns = selectedUTNs();

    model_.setEvalUseTarget(selected_utns, false);
}

void TargetListWidget::evalFilterSlot()
{
    loginf << "TargetListWidget: evalFilterSlot";

    EvaluationTargetFilterDialog dialog (COMPASS::instance().evaluationManager().targetFilter(), model_);
    dialog.exec();
}

void TargetListWidget::clearAllCommentsSlot()
{
    model_.clearAllTargetComments();

    resizeColumnsToContents();
}

void TargetListWidget::clearSelectedTargetsCommentsSlot()
{
    loginf << "TargetListWidget: clearTargetsCommentsSlot";

    std::set<unsigned int> selected_utns = selectedUTNs();

    model_.setTargetComment(selected_utns, "");

    resizeColumnsToContents();
}

void TargetListWidget::evalClearAllExcludeTimeWindowsSlot()
{
    loginf << "TargetListWidget: evalClearAllExcludeTimeWindowsSlot";

    model_.clearAllEvalExcludeTimeWindows();

    resizeColumnsToContents();
}

void TargetListWidget::evalClearAllExcludeRequirementsSlot()
{
    loginf << "TargetListWidget: evalClearAllExcludeTimeWindowsSlot";

    model_.clearAllEvalExcludeRequirements();

    resizeColumnsToContents();
}

void TargetListWidget::evalEditGlobalExcludeTimeWindowsSlot()
{
    loginf << "TargetListWidget: filterSlot";

    EvaluationTimestampConditionsDialog dialog (COMPASS::instance().evaluationManager());
    dialog.exec();

    COMPASS::instance().evaluationManager().saveTimeConstraints();

    model_.updateEvalUseColumn();
}


void TargetListWidget::customContextMenuSlot(const QPoint& p)
{
    logdbg << "TargetListWidget: customContextMenuSlot";

    assert (table_view_);

    QMenu menu;

    QAction* clear_action = menu.addAction("Clear Comment(s)");
    connect (clear_action, &QAction::triggered, this, &TargetListWidget::clearSelectedTargetsCommentsSlot);

    QAction* show_action = menu.addAction("Show Surrounding Data");
    connect (show_action, &QAction::triggered, this, &TargetListWidget::showSurroundingDataSlot);

    QMenu* eval_menu = menu.addMenu("Evaluation");

    QAction* use_action = eval_menu->addAction("Use Target(s)");
    connect (use_action, &QAction::triggered, this, &TargetListWidget::evalUseSelectedTargetsSlot);

    QAction* nouse_action = eval_menu->addAction("Disable Target(s)");
    connect (nouse_action, &QAction::triggered, this, &TargetListWidget::evalDisableSelectedTargetsSlot);

    eval_menu->addSeparator();

    QAction* tw_action = eval_menu->addAction("Edit Excluded Time Windows");
    connect (tw_action, &QAction::triggered, this, &TargetListWidget::evalExcludeTimeWindowsTargetSlot);

    QAction* tw_clear_action = eval_menu->addAction("Clear Excluded Time Windows");
    connect (tw_clear_action, &QAction::triggered, this, &TargetListWidget::evalClearTargetsExcludeTimeWindowsSlot);

    QAction* req_action = eval_menu->addAction("Edit Excluded Requirements");
    connect (req_action, &QAction::triggered, this, &TargetListWidget::evalExcludeRequirementsTargetSlot);

    QAction* req_clear_action = eval_menu->addAction("Clear Excluded Requirements");
    connect (req_clear_action, &QAction::triggered, this, &TargetListWidget::evalClearTargetsExcludeRequirementsSlot);


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



void TargetListWidget::evalClearTargetsExcludeTimeWindowsSlot()
{
    loginf << "TargetListWidget: evalClearTargetsExcludeTimeWindowsSlot";

    std::set<unsigned int> selected_utns = selectedUTNs();

    model_.clearEvalExcludeTimeWindows(selected_utns);

    resizeColumnsToContents();
}
void TargetListWidget::evalClearTargetsExcludeRequirementsSlot()
{
    loginf << "TargetListWidget: evalClearTargetsExcludeRequirementsSlot";

    std::set<unsigned int> selected_utns = selectedUTNs();

    model_.clearEvalExcludeRequirements(selected_utns);

    resizeColumnsToContents();
}

void TargetListWidget::evalExcludeTimeWindowsTargetSlot()
{
    loginf << "TargetListWidget: evalExcludeTimeWindowsTargetSlot";

    auto& dbcont_man = COMPASS::instance().dbContentManager();

    std::set<unsigned int> selected_utns = selectedUTNs();

    Utils::TimeWindowCollection filtered_time_windows;
    set<string> comments;

    // collect all time windows from all targets
    for (auto utn : selected_utns)
    {
        assert (dbcont_man.existsTarget(utn));

        auto& target = dbcont_man.target(utn);

        for (auto& tw : target.evalExcludedTimeWindows())
        {
            if (!filtered_time_windows.contains(tw))
                filtered_time_windows.add(tw);
        }

        if (target.comment().size() && !comments.count(target.comment()))
            comments.insert(target.comment());
    }

    EvaluationTargetExcludedTimeWindowsDialog dialog (String::compress(selected_utns, ','),
                                                     filtered_time_windows,
                                                     String::compress(comments, '\n'));
    int result = dialog.exec();

    if (result == QDialog::Rejected)
        return;

    string comment = dialog.comment();

    // set time windows for all targets
    model_.setTargetComment(selected_utns, comment);
    model_.setEvalExcludeTimeWindows(selected_utns, filtered_time_windows);

    resizeColumnsToContents();
}

void TargetListWidget::evalExcludeRequirementsTargetSlot()
{
    loginf << "TargetListWidget: evalExcludeRequirementsTargetSlot";

    auto& dbcont_man = COMPASS::instance().dbContentManager();
    auto& eval_man = COMPASS::instance().evaluationManager();

    if (!eval_man.calculator().hasCurrentStandard())
    {
        QMessageBox m_warning(QMessageBox::Information, "Exclude Requirements",
                              "Please select a current evaluation standard to disable requirements.",
                              QMessageBox::Ok);
        m_warning.exec();

        return;
    }

    std::set<unsigned int> selected_utns = selectedUTNs();

    set<string> selected_requirements;
    set<string> all_requirements = eval_man.calculator().currentStandard().getAllRequirementNames();
    set<string> comments;

    // collect all time windows from all targets
    for (auto utn : selected_utns)
    {
        assert (dbcont_man.existsTarget(utn));

        auto& target = dbcont_man.target(utn);

        for (auto& req_name : target.evalExcludedRequirements())
        {
            if (!selected_requirements.count(req_name))
                selected_requirements.insert(req_name);
        }

        if (target.comment().size() && !comments.count(target.comment()))
            comments.insert(target.comment());
    }

    EvaluationTargetExcludedRequirementsDialog dialog (String::compress(selected_utns, ','),
                                                      selected_requirements, all_requirements,
                                                      String::compress(comments, '\n'));
    int result = dialog.exec();

    if (result == QDialog::Rejected)
        return;

    selected_requirements = dialog.selectedRequirements();
    string comment = dialog.comment();

    // set reqs for all targets
    model_.setTargetComment(selected_utns, comment);
    model_.setEvalExcludeRequirements(selected_utns, selected_requirements);

    resizeColumnsToContents();
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
