#pragma once

#include "report/treemodel.h"
#include "json.h"

#include <boost/optional.hpp>

#include <QWidget>
#include <QTreeView>
#include <QStackedWidget>

class TaskResultsWidget;

class QPushButton;
class QMenu;

namespace ResultReport
{


class ReportWidget : public QWidget
{
    Q_OBJECT

public slots:
    void itemClickedSlot(const QModelIndex& index);

    void stepBackSlot();

public:
    ReportWidget(TaskResultsWidget& task_result_widget);
    virtual ~ReportWidget()= default;

    void setReport(const std::shared_ptr<Report>& report);
    void clear();

    void expand();

    void showResultWidget(QWidget* widget); // can be nullptr

    void selectId (const std::string& id, bool show_figure = false);
    void reshowLastId ();

    boost::optional<nlohmann::json> getTableData(const std::string& result_id,
                                                 const std::string& table_id,
                                                 bool rowwise = true,
                                                 const std::vector<int>& cols = std::vector<int>()) const;

    void showFigure(const QModelIndex& index);

protected:
    TaskResultsWidget& task_result_widget_;

    ResultReport::TreeModel tree_model_;
    QTreeView* tree_view_;

    QStackedWidget* results_widget_{nullptr};

    QPushButton* sections_button_{nullptr};
    std::unique_ptr<QMenu> sections_menu_;

    QPushButton* back_button_ {nullptr};
    std::vector<std::string> id_history_;

    void expandAllParents (QModelIndex index);
    void updateBackButton ();
};

}
