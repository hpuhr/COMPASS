#include "evaluationmanagerwidget.h"
#include "evaluationmanager.h"
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
    main_layout_ = new QHBoxLayout();

    tab_widget_ = new QTabWidget();

    addMainWidget();
    addTargetsWidget();
    addStandardWidget();
    addResultsWidget();

    main_layout_->addWidget(tab_widget_);

    setLayout(main_layout_);
}

EvaluationManagerWidget::~EvaluationManagerWidget()
{

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

    data_source_ref_widget_.reset(new EvaluationDataSourceWidget("Reference Data", eval_man_.dboNameRefNonConst(),
                                                             eval_man_.dataSourcesRef()));
    data_sources_layout->addWidget(data_source_ref_widget_.get());

    data_source_tst_widget_.reset(new EvaluationDataSourceWidget("Test Data", eval_man_.dboNameTstNonConst(),
                                                             eval_man_.dataSourcesTst()));
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
