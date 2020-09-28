#include "evaluationresultstabwidget.h"

#include "evaluationmanagerwidget.h"
#include "evaluationmanager.h"
#include "evaluationresultsgenerator.h"
#include "eval/results/report/treemodel.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

EvaluationResultsTabWidget::EvaluationResultsTabWidget(EvaluationManager& eval_man, EvaluationManagerWidget& man_widget)
    : QWidget(nullptr), eval_man_(eval_man), man_widget_(man_widget)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    QHBoxLayout* res_layout = new QHBoxLayout();

    tree_view_.reset(new QTreeView());
    tree_view_->setModel(&eval_man_.resultsGenerator().resultsModel());
    tree_view_->setRootIsDecorated(false);
    tree_view_->expandAll();

    //connect (tree_view_.get(), &QTreeView::clicked, this, &EvaluationStandardWidget::itemClickedSlot);

    res_layout->addWidget(tree_view_.get());

//    // results stack
//    requirements_widget_ = new QStackedWidget();
//    req_layout->addWidget(requirements_widget_);

    main_layout->addLayout(res_layout);

    setContentsMargins(0, 0, 0, 0);
    setLayout(main_layout);
}

void EvaluationResultsTabWidget::expandAll()
{
    loginf << "EvaluationResultsTabWidget: expandAll";

    tree_view_->expandAll();
}
