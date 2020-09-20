#include "evaluationmanagerwidget.h"
#include "evaluationmanager.h"
#include "evaluationdatasourcewidget.h"
#include "evaluationdata.h"
#include "evaluationdatawidget.h"
#include "logger.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

EvaluationManagerWidget::EvaluationManagerWidget(EvaluationManager& eval_man)
    : QWidget(nullptr), eval_man_(eval_man)
{
    main_layout_ = new QVBoxLayout();

    tab_widget_ = new QTabWidget();

    addMainWidget();
    addTargetsWidget();
    addStandardWidget();
    addResultsWidget();

    main_layout_->addWidget(tab_widget_);

    // buttons

    QHBoxLayout* button_layout = new QHBoxLayout();

    load_button_ = new QPushButton("Load Data");
    connect (load_button_, &QPushButton::clicked, this, &EvaluationManagerWidget::loadDataSlot);
    button_layout->addWidget(load_button_);

    //button_layout->addStretch();

    evaluate_button_ = new QPushButton("Evaluate");
    connect (evaluate_button_, &QPushButton::clicked, this, &EvaluationManagerWidget::evaluateSlot);
    button_layout->addWidget(evaluate_button_);

    gen_report_button_ = new QPushButton("Generate Report");
    connect (gen_report_button_, &QPushButton::clicked, this, &EvaluationManagerWidget::generateReportSlot);
    button_layout->addWidget(gen_report_button_);

    main_layout_->addLayout(button_layout);

    updateButtons();

    setLayout(main_layout_);
}

EvaluationManagerWidget::~EvaluationManagerWidget()
{

}

void EvaluationManagerWidget::updateButtons()
{
    evaluate_button_->setEnabled(eval_man_.dataLoaded());
    gen_report_button_->setEnabled(eval_man_.evaluated());
}

void EvaluationManagerWidget::addMainWidget ()
{
    QVBoxLayout* tab_layout = new QVBoxLayout();

    QFont font_bold;
    font_bold.setBold(true);

    QLabel* main_label = new QLabel("Data Selection");
    main_label->setFont(font_bold);
    tab_layout->addWidget(main_label);

    QHBoxLayout* data_sources_layout = new QHBoxLayout();

    data_source_ref_widget_.reset(new EvaluationDataSourceWidget("Reference Data", eval_man_.dboNameRef(),
                                                             eval_man_.dataSourcesRef()));
    connect (data_source_ref_widget_.get(), &EvaluationDataSourceWidget::dboNameChangedSignal,
             this, &EvaluationManagerWidget::dboRefNameChangedSlot);
    data_sources_layout->addWidget(data_source_ref_widget_.get());

    data_source_tst_widget_.reset(new EvaluationDataSourceWidget("Test Data", eval_man_.dboNameTst(),
                                                             eval_man_.dataSourcesTst()));
    connect (data_source_tst_widget_.get(), &EvaluationDataSourceWidget::dboNameChangedSignal,
             this, &EvaluationManagerWidget::dboTstNameChangedSlot);
    data_sources_layout->addWidget(data_source_tst_widget_.get());


    tab_layout->addLayout(data_sources_layout);

    tab_layout->addStretch();

    QWidget* tab_widget = new QWidget();
    tab_widget->setContentsMargins(0, 0, 0, 0);
    tab_widget->setLayout(tab_layout);
    tab_widget_->addTab(tab_widget, "Main");
}

void EvaluationManagerWidget::addTargetsWidget ()
{
    QVBoxLayout* tab_layout = new QVBoxLayout();

    tab_layout->addWidget(eval_man_.getData().widget());

    QWidget* tab_widget = new QWidget();
    tab_widget->setContentsMargins(0, 0, 0, 0);
    tab_widget->setLayout(tab_layout);
    tab_widget_->addTab(tab_widget, "Targets");
}

void EvaluationManagerWidget::addStandardWidget ()
{
    QVBoxLayout* tab_layout = new QVBoxLayout();

    QWidget* tab_widget = new QWidget();
    tab_widget->setContentsMargins(0, 0, 0, 0);
    tab_widget->setLayout(tab_layout);
    tab_widget_->addTab(tab_widget, "Standard");
}

void EvaluationManagerWidget::addResultsWidget ()
{
    QVBoxLayout* tab_layout = new QVBoxLayout();

    QWidget* tab_widget = new QWidget();
    tab_widget->setContentsMargins(0, 0, 0, 0);
    tab_widget->setLayout(tab_layout);
    tab_widget_->addTab(tab_widget, "Results");
}

void EvaluationManagerWidget::dboRefNameChangedSlot(const std::string& dbo_name)
{
    loginf << "EvaluationManagerWidget: dboRefNameChangedSlot: name " << dbo_name;

    eval_man_.dboNameRef(dbo_name);
}

void EvaluationManagerWidget::dboTstNameChangedSlot(const std::string& dbo_name)
{
    loginf << "EvaluationManagerWidget: dboTstNameChangedSlot: name " << dbo_name;

    eval_man_.dboNameTst(dbo_name);
}

void EvaluationManagerWidget::loadDataSlot()
{
    loginf << "EvaluationManagerWidget: loadDataSlot";

    eval_man_.loadData();
}

void EvaluationManagerWidget::evaluateSlot()
{
    loginf << "EvaluationManagerWidget: evaluateSlot";

    eval_man_.evaluate();
}


void EvaluationManagerWidget::generateReportSlot()
{
    loginf << "EvaluationManagerWidget: generateReportSlot";

    eval_man_.generateReport();
}
