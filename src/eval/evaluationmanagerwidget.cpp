#include "evaluationmanagerwidget.h"
#include "evaluationmaintabwidget.h"
#include "evaluationtargetstabwidget.h"
#include "evaluationstandardtabwidget.h"
#include "evaluationresultstabwidget.h"
#include "evaluationstandardcombobox.h"
#include "evaluationmanager.h"
#include "evaluationdata.h"
#include "evaluationdatawidget.h"
#include "evaluationdatasourcewidget.h"
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
    evaluate_button_->setEnabled(eval_man_.dataLoaded() && eval_man_.hasCurrentStandard());
    gen_report_button_->setEnabled(eval_man_.evaluated());
}

void EvaluationManagerWidget::addMainWidget ()
{
    main_tab_widget_.reset(new EvaluationMainTabWidget(eval_man_, *this));

    tab_widget_->addTab(main_tab_widget_.get(), "Main");
}

void EvaluationManagerWidget::addTargetsWidget ()
{
    targets_tab_widget_.reset(new EvaluationTargetsTabWidget(eval_man_, *this));

    tab_widget_->addTab(targets_tab_widget_.get(), "Targets");
}

void EvaluationManagerWidget::addStandardWidget ()
{
    std_tab_widget_.reset(new EvaluationStandardTabWidget(eval_man_, *this));

    tab_widget_->addTab(std_tab_widget_.get(), "Standard");
}

void EvaluationManagerWidget::addResultsWidget ()
{
    results_tab_widget_.reset(new EvaluationResultsTabWidget(eval_man_, *this));

    tab_widget_->addTab(results_tab_widget_.get(), "Results");
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
