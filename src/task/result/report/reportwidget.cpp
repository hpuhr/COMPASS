#include "reportwidget.h"
#include "report/treeitem.h"
#include "report/report.h"
#include "report/section.h"
#include "report/sectioncontentfigure.h"
#include "report/sectioncontenttable.h"
#include "files.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QPushButton>
#include <QSettings>
#include <QMenu>
#include <QWidgetAction>

using namespace Utils;

namespace ResultReport
{

ReportWidget::ReportWidget(TaskResultsWidget& task_result_widget)
    : QWidget(nullptr), task_result_widget_(task_result_widget)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    { // button layout
        QHBoxLayout* button_layout = new QHBoxLayout();

        QIcon left_icon(Files::getIconFilepath("arrow_to_left.png").c_str());

        back_button_ = new QPushButton ();
        back_button_->setIcon(left_icon);
        connect (back_button_, &QPushButton::clicked, this, &ReportWidget::stepBackSlot);
        button_layout->addWidget(back_button_);

        button_layout->addStretch();

        sections_button_ = new QPushButton("Sections");
        button_layout->addWidget(sections_button_);

        main_layout->addLayout(button_layout);

    }

    tree_view_ = new QTreeView;
    tree_view_->setModel(&tree_model_);
    tree_view_->setRootIsDecorated(false);
    tree_view_->expandToDepth(3);
    tree_view_->setFixedSize(500, 1000);

    connect (tree_view_, &QTreeView::clicked, this, &ReportWidget::itemClickedSlot);

    sections_menu_.reset(new QMenu);

    auto w_action = new QWidgetAction(sections_menu_.get());
    w_action->setDefaultWidget(tree_view_);

    sections_menu_->addAction(w_action);
    sections_button_->setMenu(sections_menu_.get());

    // results stack

    QScrollArea* scroll_area = new QScrollArea();
    scroll_area->setWidgetResizable(true);

    results_widget_ = new QStackedWidget();

    scroll_area->setWidget(results_widget_);

    main_layout->addWidget(scroll_area);

    setContentsMargins(0, 0, 0, 0);
    setLayout(main_layout);

    updateBackButton();
}

void ReportWidget::expand()
{
    loginf << "ReportWidget: expand";

    tree_view_->expandToDepth(3);
}

void ReportWidget::selectId (const std::string& id,
                                 bool show_figure)
{
    loginf << "ReportWidget: selectId: id '" << id << "'";

    QModelIndex index = tree_model_.findItem(id);

    if (!index.isValid())
    {
        logerr << "ReportWidget: selectId: id '" << id << "' not found";
        return;
    }

    assert (tree_view_);
    tree_view_->selectionModel()->clear();

    expandAllParents(index);

    tree_view_->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    tree_view_->scrollTo(index);

    itemClickedSlot(index);

    if (show_figure)
        showFigure(index);
}

void ReportWidget::reshowLastId ()
{
    if (id_history_.size() >= 1)
    {
        selectId(*id_history_.rbegin()); // select last one

        id_history_.pop_back(); // remove re-added id. slightly hacky
    }
}

void ReportWidget::itemClickedSlot(const QModelIndex& index)
{
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    assert (item);

    id_history_.push_back(item->id());

    loginf << "ReportWidget: itemClickedSlot: name " << item->name() << " id " << item->id();

    if (dynamic_cast<ResultReport::Report*>(item))
    {
        loginf << "ReportWidget: itemClickedSlot: root report";
        showResultWidget(nullptr);
    }
    else if (dynamic_cast<ResultReport::Section*>(item))
    {
        ResultReport::Section* section = dynamic_cast<ResultReport::Section*>(item);
        assert (section);

        loginf << "ReportWidget: itemClickedSlot: section " << section->name();
        showResultWidget(section->getContentWidget());
    }

    updateBackButton();
}

void ReportWidget::showFigure(const QModelIndex& index)
{
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    assert (item);

    loginf << "ReportWidget: showFigure: name " << item->name() << " id " << item->id();

    if (dynamic_cast<ResultReport::Report*>(item))
    {
        return;
    }
    else if (dynamic_cast<ResultReport::Section*>(item))
    {
        ResultReport::Section* section = dynamic_cast<ResultReport::Section*>(item);
        assert (section);

        loginf << "ReportWidget: showFigure: section " << section->name();

        auto figures = section->getFigures();
        if (!figures.empty())
            figures[ 0 ]->viewSlot();
    }
}

void ReportWidget::stepBackSlot()
{
    loginf << "ReportWidget: stepBackSlot";

    assert (id_history_.size() > 1);

    id_history_.pop_back(); // remove last entry
    reshowLastId(); // show last id

    updateBackButton();
}

void ReportWidget::showResultWidget(QWidget* widget)
{
    assert(results_widget_);

    if (!widget)
    {
        while (results_widget_->count() > 0)  // remove all widgets
            results_widget_->removeWidget(results_widget_->widget(0));
        return;
    }

    if (results_widget_->indexOf(widget) < 0)
        results_widget_->addWidget(widget);

    results_widget_->setCurrentWidget(widget);
}

void ReportWidget::expandAllParents (QModelIndex index)
{
    if (!index.isValid()) {
        return;
    }

    QModelIndex parent_index;

    for (parent_index = index.parent(); parent_index.isValid(); parent_index = parent_index.parent())
    {
        if (!tree_view_->isExpanded(parent_index))
            tree_view_->expand(parent_index);
    }
}

void ReportWidget::updateBackButton ()
{
    assert (back_button_);

    back_button_->setEnabled(id_history_.size() > 1);
}

boost::optional<nlohmann::json> ReportWidget::getTableData(const std::string& result_id,
                                                                const std::string& table_id,
                                                                bool rowwise,
                                                                const std::vector<int>& cols) const
{
    QString result_id_corr = QString::fromStdString(result_id);

    //this is just for convenience
    if (!result_id_corr.startsWith("Report:Results:"))
    {
        if (result_id_corr.startsWith("Results:"))
            result_id_corr = "Report:" + result_id_corr;
        else
            result_id_corr = "Report:Results:" + result_id_corr;
    }

    //get result section
    QModelIndex index = tree_model_.findItem(result_id_corr.toStdString());

    if (!index.isValid())
    {
        logerr << "ReportWidget: getTableData: id '" << result_id_corr.toStdString() << "' not found";
        return {};
    }

    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (!item)
    {
        logerr << "ReportWidget: getTableData: item null";
        return {};
    }

    ResultReport::Section* section = dynamic_cast<ResultReport::Section*>(item);
    if (!section)
    {
        logerr << "ReportWidget: getTableData: no section found";
        return {};
    }

    //check if table is available
    if (!section->hasTable(table_id))
    {
        logerr << "ReportWidget: getTableData: no table found";
        return {};
    }

    return section->getTable(table_id).toJSON(rowwise, cols);
}


}
