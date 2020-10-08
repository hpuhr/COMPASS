#include "evaluationstandardwidget.h"
#include "evaluationstandard.h"
#include "evaluationrequirementgroup.h"
#include "evaluationrequirementconfig.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QScrollArea>

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
    QScrollArea* scroll_area = new QScrollArea();
    scroll_area->setWidgetResizable(true);

    requirements_widget_ = new QStackedWidget();

    scroll_area->setWidget(requirements_widget_);
    req_layout->addWidget(scroll_area, 1);

    main_layout->addLayout(req_layout);

    setContentsMargins(0, 0, 0, 0);
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

        showRequirementWidget(nullptr);

        std->showMenu();

    }
    else if (dynamic_cast<EvaluationRequirementGroup*>(item))
    {
        loginf << "EvaluationStandardWidget: itemClickedSlot: got group";

        EvaluationRequirementGroup* group = dynamic_cast<EvaluationRequirementGroup*>(item);

        showRequirementWidget(nullptr);

        group->showMenu();
    }
    else if (dynamic_cast<EvaluationRequirement::EvaluationRequirementConfig*>(item))
    {
        loginf << "EvaluationStandardWidget: itemClickedSlot: got config";

        EvaluationRequirement::EvaluationRequirementConfig* config =
                dynamic_cast<EvaluationRequirement::EvaluationRequirementConfig*>(item);

        showRequirementWidget(config->widget());
    }
    else
        assert (false);
}

void EvaluationStandardWidget::expandAll()
{
    tree_view_->expandAll();
}

void EvaluationStandardWidget::showRequirementWidget(QWidget* widget)
{
    assert(requirements_widget_);

    if (!widget)
    {
        while (requirements_widget_->count() > 0)  // remove all widgets
            requirements_widget_->removeWidget(requirements_widget_->widget(0));
        return;
    }

    if (requirements_widget_->indexOf(widget) < 0)
        requirements_widget_->addWidget(widget);

    requirements_widget_->setCurrentWidget(widget);
}
