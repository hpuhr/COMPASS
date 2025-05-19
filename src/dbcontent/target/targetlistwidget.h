
#pragma once

#include "toolboxwidget.h"

#include <QItemSelection>

#include <set>

class DBContentManager;

class QTableView;
class QSortFilterProxyModel;

namespace dbContent {

class TargetModel;

/**
 */
class TargetListWidget : public ToolBoxWidget
{
    Q_OBJECT

public slots:
    void evalUseAllSlot();
    void evalUseNoneSlot();
    void evalExcludeTimeWindowsSlot();
    void evalFilterSlot();

    void clearCommentsSlot();

    void customContextMenuSlot(const QPoint& p);
    void showSurroundingDataSlot ();

    void evalUseTargetsSlot();
    void evalDisableUseTargetsSlot();
    void evalExcludeTimeWindowsTargetSlot();

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

protected:
    void showMainColumns(bool show);
    void showEvalColumns(bool show);
    void showDurationColumns(bool show);
    void showModeACColumns(bool show);
    void showModeSColumns(bool show);

    TargetModel& model_;
    DBContentManager& dbcont_manager_;

    QTableView* table_view_{nullptr};
    QSortFilterProxyModel* proxy_model_{nullptr};

    std::set<unsigned int> selectedUTNs() const;
};

};
