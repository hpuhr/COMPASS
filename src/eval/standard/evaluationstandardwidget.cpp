#include "evaluationstandardwidget.h"
#include "evaluationstandard.h"
#include "evaluationrequirementgroup.h"
#include "evaluationrequirementconfig.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QStackedWidget>

using namespace std;

EvaluationStandardWidget::EvaluationStandardWidget(EvaluationStandard& standard)
    : QWidget(), standard_(standard), standard_model_(standard)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    QHBoxLayout* req_layout = new QHBoxLayout();

    tree_view_.reset(new QTreeView());
    tree_view_->setModel(&standard_model_);
    tree_view_->setRootIsDecorated(false);
    tree_view_->expandAll();

    connect (tree_view_.get(), &QTreeView::clicked, this, &EvaluationStandardWidget::itemClickedSlot);

    req_layout->addWidget(tree_view_.get());

    // requirements stack
    requirements_widget_ = new QStackedWidget();
    req_layout->addWidget(requirements_widget_);

    main_layout->addLayout(req_layout);

    setLayout(main_layout);
}

EvaluationStandardTreeModel& EvaluationStandardWidget::model()
{
    return standard_model_;
}

void EvaluationStandardWidget::itemClickedSlot(const QModelIndex& index)
{
    EvaluationStandardTreeItem* item = static_cast<EvaluationStandardTreeItem*>(index.internalPointer());
    assert (item);

    if (dynamic_cast<EvaluationStandard*>(item))
    {
        loginf << "EvaluationStandardWidget: itemClickedSlot: got standard";

        EvaluationStandard* std = dynamic_cast<EvaluationStandard*>(item);

        std->showMenu();
    }
    else if (dynamic_cast<EvaluationRequirementGroup*>(item))
    {
        loginf << "EvaluationStandardWidget: itemClickedSlot: got group";

        EvaluationRequirementGroup* group = dynamic_cast<EvaluationRequirementGroup*>(item);

        group->showMenu();
    }
    else if (dynamic_cast<EvaluationRequirementConfig*>(item))
    {
        loginf << "EvaluationStandardWidget: itemClickedSlot: got config";

        EvaluationRequirementConfig* config = dynamic_cast<EvaluationRequirementConfig*>(item);
    }
    else
        assert (false);
}

void EvaluationStandardWidget::expandAll()
{
    tree_view_->expandAll();
}
