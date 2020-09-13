#include "evaluationmanagerwidget.h"
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
