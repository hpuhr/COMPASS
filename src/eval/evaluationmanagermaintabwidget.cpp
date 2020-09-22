#include "evaluationmanagermaintabwidget.h"
#include "evaluationmanagerwidget.h"
#include "evaluationdatasourcewidget.h"
#include "evaluationmanager.h"
#include "evaluationstandardcombobox.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

EvaluationManagerMainTabWidget::EvaluationManagerMainTabWidget(EvaluationManager& eval_man,
                                                               EvaluationManagerWidget& man_widget)
    : QWidget(nullptr), eval_man_(eval_man), man_widget_(man_widget)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    QFont font_bold;
    font_bold.setBold(true);

    // data sources
    QLabel* main_label = new QLabel("Data Selection");
    main_label->setFont(font_bold);
    main_layout->addWidget(main_label);

    QHBoxLayout* data_sources_layout = new QHBoxLayout();

    data_source_ref_widget_.reset(new EvaluationDataSourceWidget("Reference Data", eval_man_.dboNameRef(),
                                                             eval_man_.dataSourcesRef()));
    connect (data_source_ref_widget_.get(), &EvaluationDataSourceWidget::dboNameChangedSignal,
             this, &EvaluationManagerMainTabWidget::dboRefNameChangedSlot);
    data_sources_layout->addWidget(data_source_ref_widget_.get());

    data_source_tst_widget_.reset(new EvaluationDataSourceWidget("Test Data", eval_man_.dboNameTst(),
                                                             eval_man_.dataSourcesTst()));
    connect (data_source_tst_widget_.get(), &EvaluationDataSourceWidget::dboNameChangedSignal,
             this, &EvaluationManagerMainTabWidget::dboTstNameChangedSlot);
    data_sources_layout->addWidget(data_source_tst_widget_.get());

    main_layout->addLayout(data_sources_layout);

    // standard

    standard_box_.reset(new EvaluationStandardComboBox(eval_man_));
    main_layout->addWidget(standard_box_.get());

    if (eval_man_.hasCurrentStandard())
        standard_box_->setStandardName(eval_man_.currentStandard());

    main_layout->addStretch();

    // connections

    connect (&eval_man_, &EvaluationManager::standardsChangedSignal,
             this, &EvaluationManagerMainTabWidget::changedStandardsSlot);
    connect (&eval_man_, &EvaluationManager::currentStandardChangedSignal,
             this, &EvaluationManagerMainTabWidget::changedCurrentStandardSlot);

    setContentsMargins(0, 0, 0, 0);
    setLayout(main_layout);
}

void EvaluationManagerMainTabWidget::dboRefNameChangedSlot(const std::string& dbo_name)
{
    loginf << "EvaluationManagerMainTabWidget: dboRefNameChangedSlot: name " << dbo_name;

    eval_man_.dboNameRef(dbo_name);
}

void EvaluationManagerMainTabWidget::dboTstNameChangedSlot(const std::string& dbo_name)
{
    loginf << "EvaluationManagerMainTabWidget: dboTstNameChangedSlot: name " << dbo_name;

    eval_man_.dboNameTst(dbo_name);
}

void EvaluationManagerMainTabWidget::changedStandardSlot(const QString& standard_name)
{
    loginf << "EvaluationManagerMainTabWidget: changedStandardSlot: name " << standard_name.toStdString();

    eval_man_.currentStandard(standard_name.toStdString());
}

void EvaluationManagerMainTabWidget::changedStandardsSlot()
{
    loginf << "EvaluationManagerMainTabWidget: changedStandardsSlot";
}

void EvaluationManagerMainTabWidget::changedCurrentStandardSlot()
{
    loginf << "EvaluationManagerMainTabWidget: changedCurrentStandardSlot";

    assert (standard_box_);
    standard_box_->setStandardName(eval_man_.currentStandard());
}
