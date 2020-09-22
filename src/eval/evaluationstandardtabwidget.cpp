#include "evaluationstandardtabwidget.h"
#include "evaluationmanagerwidget.h"
#include "evaluationmanager.h"
#include "evaluationstandardcombobox.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

EvaluationStandardTabWidget::EvaluationStandardTabWidget(EvaluationManager& eval_man,
                                                         EvaluationManagerWidget& man_widget)
    : QWidget(nullptr), eval_man_(eval_man), man_widget_(man_widget)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    QFont font_bold;
    font_bold.setBold(true);

    // standard
    standard_box_.reset(new EvaluationStandardComboBox(eval_man_));
    main_layout->addWidget(standard_box_.get());

    if (eval_man_.hasCurrentStandard())
        standard_box_->setStandardName(eval_man_.currentStandard());

    main_layout->addStretch();

    // connections
    connect (&eval_man_, &EvaluationManager::standardsChangedSignal,
             this, &EvaluationStandardTabWidget::changedStandardsSlot);
    connect (&eval_man_, &EvaluationManager::currentStandardChangedSignal,
             this, &EvaluationStandardTabWidget::changedCurrentStandardSlot);

    setContentsMargins(0, 0, 0, 0);
    setLayout(main_layout);
}

void EvaluationStandardTabWidget::changedStandardSlot(const QString& standard_name)
{
    loginf << "EvaluationStandardTabWidget: changedStandardSlot: name " << standard_name.toStdString();

    eval_man_.currentStandard(standard_name.toStdString());
}

void EvaluationStandardTabWidget::changedStandardsSlot()
{
    loginf << "EvaluationStandardTabWidget: changedStandardsSlot";
}

void EvaluationStandardTabWidget::changedCurrentStandardSlot()
{
    loginf << "EvaluationStandardTabWidget: changedCurrentStandardSlot";

    assert (standard_box_);
    standard_box_->setStandardName(eval_man_.currentStandard());
}
