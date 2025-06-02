
#pragma once

#include "toolboxwidget.h"

#include <QItemSelection>

#include <set>

class DBContentManager;

class QTableView;
class QSortFilterProxyModel;

class QMenu;

namespace dbContent 
{

class TargetModel;

/**
 */
class TargetListWidget : public ToolBoxWidget
{
    Q_OBJECT

public slots:
    void evalUseAllSlot();
    void evalUseNoneSlot();
    void evalFilterSlot();

    void evalEditGlobalExcludeTimeWindowsSlot();

    void clearAllCommentsSlot();
    void evalClearAllExcludeTimeWindowsSlot();
    void evalClearAllExcludeRequirementsSlot();

    void customContextMenuSlot(const QPoint& p);
    void showSurroundingDataSlot ();

    //per-target
    void clearSelectedTargetsCommentsSlot();
    void evalUseSelectedTargetsSlot();
    void evalDisableSelectedTargetsSlot();
    void evalClearTargetsExcludeTimeWindowsSlot();
    void evalClearTargetsExcludeRequirementsSlot();
    void evalExcludeTimeWindowsTargetSlot();
    void evalExcludeRequirementsTargetSlot();

    void currentRowChanged(const QModelIndex& current, const QModelIndex& previous);
    void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

public:
    TargetListWidget(TargetModel& model, DBContentManager& dbcont_manager);
    virtual ~TargetListWidget() {};

    //ToolBoxWidget
    QIcon toolIcon() const override final;
    std::string toolName() const override final;
    std::string toolInfo() const override final;
    std::vector<std::string> toolLabels() const override final;
    toolbox::ScreenRatio defaultScreenRatio() const override final;
    void addToConfigMenu(QMenu* menu) override final;
    void addToToolBar(QToolBar* tool_bar) override final;
    void loadingStarted() override final;
    void loadingDone() override final;

    void resizeColumnsToContents();

    void createTargetEvalMenu(QMenu& menu, const std::set<unsigned int>& utns);

protected:
    void showMainColumns(bool show);
    void showEvalColumns(bool show);
    void showDurationColumns(bool show);
    void showModeACColumns(bool show);
    void showModeSColumns(bool show);

    void clearSelectedTargetsComments(const std::set<unsigned int>& utns);
    void evalUseSelectedTargets(const std::set<unsigned int>& utns);
    void evalDisableSelectedTargets(const std::set<unsigned int>& utns);
    void evalClearTargetsExcludeTimeWindows(const std::set<unsigned int>& utns);
    void evalClearTargetsExcludeRequirements(const std::set<unsigned int>& utns);
    void evalExcludeTimeWindowsTarget(const std::set<unsigned int>& utns);
    void evalExcludeRequirementsTarget(const std::set<unsigned int>& utns);

    TargetModel& model_;
    DBContentManager& dbcont_manager_;

    QTableView* table_view_{nullptr};
    QSortFilterProxyModel* proxy_model_{nullptr};

    std::set<unsigned int> selectedUTNs() const;
};

};
