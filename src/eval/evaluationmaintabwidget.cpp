#include "evaluationmaintabwidget.h"
#include "evaluationmanagerwidget.h"
#include "evaluationdatasourcewidget.h"
#include "evaluationmanager.h"
#include "evaluationstandardcombobox.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFormLayout>

EvaluationMainTabWidget::EvaluationMainTabWidget(EvaluationManager& eval_man,
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
             this, &EvaluationMainTabWidget::dboRefNameChangedSlot);
    data_sources_layout->addWidget(data_source_ref_widget_.get());

    data_source_tst_widget_.reset(new EvaluationDataSourceWidget("Test Data", eval_man_.dboNameTst(),
                                                             eval_man_.dataSourcesTst()));
    connect (data_source_tst_widget_.get(), &EvaluationDataSourceWidget::dboNameChangedSignal,
             this, &EvaluationMainTabWidget::dboTstNameChangedSlot);
    data_sources_layout->addWidget(data_source_tst_widget_.get());

    main_layout->addLayout(data_sources_layout);

    // standard
    QHBoxLayout* std_layout = new QHBoxLayout();

    QLabel* standard_label = new QLabel("Standard");
    standard_label->setFont(font_bold);
    std_layout->addWidget(standard_label);

    standard_box_.reset(new EvaluationStandardComboBox(eval_man_));

//    if (eval_man_.hasCurrentStandard())
//        standard_box_->setStandardName(eval_man_.currentStandardName());

    std_layout->addWidget(standard_box_.get());

    main_layout->addLayout(std_layout);

    main_layout->addStretch();

    // connections

    connect (&eval_man_, &EvaluationManager::standardsChangedSignal,
             this, &EvaluationMainTabWidget::changedStandardsSlot);
    connect (&eval_man_, &EvaluationManager::currentStandardChangedSignal,
             this, &EvaluationMainTabWidget::changedCurrentStandardSlot);

    setContentsMargins(0, 0, 0, 0);
    setLayout(main_layout);
}

void EvaluationMainTabWidget::dboRefNameChangedSlot(const std::string& dbo_name)
{
    loginf << "EvaluationMainTabWidget: dboRefNameChangedSlot: name " << dbo_name;

    eval_man_.dboNameRef(dbo_name);
}

void EvaluationMainTabWidget::dboTstNameChangedSlot(const std::string& dbo_name)
{
    loginf << "EvaluationMainTabWidget: dboTstNameChangedSlot: name " << dbo_name;

    eval_man_.dboNameTst(dbo_name);
}

void EvaluationMainTabWidget::changedStandardsSlot()
{
    loginf << "EvaluationMainTabWidget: changedStandardsSlot";

    assert (standard_box_);
    standard_box_->updateStandards();
}

void EvaluationMainTabWidget::changedCurrentStandardSlot()
{
    loginf << "EvaluationMainTabWidget: changedCurrentStandardSlot";

    assert (standard_box_);
    standard_box_->setStandardName(eval_man_.currentStandardName());
}
