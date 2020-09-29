#include "evaluationresultstabwidget.h"

#include "evaluationmanagerwidget.h"
#include "evaluationmanager.h"
#include "evaluationresultsgenerator.h"
#include "eval/results/report/treemodel.h"
#include "eval/results/report/treeitem.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>

using namespace EvaluationResultsReport;

EvaluationResultsTabWidget::EvaluationResultsTabWidget(EvaluationManager& eval_man, EvaluationManagerWidget& man_widget)
    : QWidget(nullptr), eval_man_(eval_man), man_widget_(man_widget)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    QHBoxLayout* res_layout = new QHBoxLayout();

    tree_view_.reset(new QTreeView());
    tree_view_->setModel(&eval_man_.resultsGenerator().resultsModel());
    tree_view_->setRootIsDecorated(false);
    tree_view_->expandAll();

    connect (tree_view_.get(), &QTreeView::clicked, this, &EvaluationResultsTabWidget::itemClickedSlot);

    res_layout->addWidget(tree_view_.get());

    // results stack

    QScrollArea* scroll_area = new QScrollArea();
    scroll_area->setWidgetResizable(true);

    results_widget_ = new QStackedWidget();

    scroll_area->setWidget(results_widget_);
    res_layout->addWidget(scroll_area, 1);

    main_layout->addLayout(res_layout);

    setContentsMargins(0, 0, 0, 0);
    setLayout(main_layout);
}

void EvaluationResultsTabWidget::expandAll()
{
    loginf << "EvaluationResultsTabWidget: expandAll";

    tree_view_->expandAll();
}


void EvaluationResultsTabWidget::itemClickedSlot(const QModelIndex& index)
{
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    assert (item);

    loginf << "EvaluationResultsTabWidget: itemClickedSlot: name " << item->name();

//    if (dynamic_cast<EvaluationStandard*>(item))
//    {
}
