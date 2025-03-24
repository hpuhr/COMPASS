#include "evaluationdialog.h"
#include "evaluationmanager.h"
#include "evaluationmaintabwidget.h"
#include "evaluationfiltertabwidget.h"
#include "evaluationstandardtabwidget.h"
#include "evaluationresultsgenerator.h"
#include "evaluationresultsgeneratorwidget.h"
#include "evaluationdatasourcewidget.h"
#include "evaluationsectorwidget.h"
#include "evaluationstandardcombobox.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QFormLayout>
#include <QComboBox>
#include <QStandardItemModel>

EvaluationDialog::EvaluationDialog(EvaluationManager& eval_man, EvaluationManagerSettings& eval_settings)
    : eval_man_(eval_man), eval_settings_(eval_settings)
{
    setWindowTitle("Evaluation");
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    setModal(true);

    setMinimumSize(QSize(1000, 800));

    // QFont font_bold;
    // font_bold.setBold(true);

    // QFont font_big;
    // font_big.setPointSize(16);

    QVBoxLayout* main_layout = new QVBoxLayout();

    QTabWidget* tab_widget = new QTabWidget();

    main_tab_widget_.reset(new EvaluationMainTabWidget(eval_man_, eval_settings_, *this));
    tab_widget->addTab(main_tab_widget_.get(), "Main");

    filter_widget_.reset(new EvaluationFilterTabWidget(eval_man_, eval_settings_));
    tab_widget->addTab(filter_widget_.get(), "Filter");

    std_tab_widget_.reset(new EvaluationStandardTabWidget(eval_man_, eval_settings_));
    tab_widget->addTab(std_tab_widget_.get(), "Standard");

    tab_widget->addTab(&eval_man_.resultsGenerator().widget(), "Results Config");

    main_layout->addWidget(tab_widget);

    main_layout->addSpacing(20);

    // not evaluate comment
    not_eval_comment_label_ = new QLabel();
    QPalette palette = not_eval_comment_label_->palette();
    palette.setColor(not_eval_comment_label_->foregroundRole(), Qt::red);
    not_eval_comment_label_->setPalette(palette);

    main_layout->addWidget(not_eval_comment_label_);

    //main_layout->addWidget(reconstructor_widget_stack_);

    QHBoxLayout* button_layout = new QHBoxLayout();

    cancel_button_ = new QPushButton("Cancel");
    connect(cancel_button_, &QPushButton::clicked, this, &QDialog::reject);
    button_layout->addWidget(cancel_button_);

    button_layout->addStretch();

    run_button_ = new QPushButton("Run");
    connect(run_button_, &QPushButton::clicked, this, &QDialog::accept);
    button_layout->addWidget(run_button_);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);

    updateFromSettings();
    updateButtons();

}

EvaluationDialog::~EvaluationDialog()
{

}

void EvaluationDialog::updateDataSources()
{
    if (main_tab_widget_)
        main_tab_widget_->updateDataSources();
}

void EvaluationDialog::updateSectors()
{
    if (main_tab_widget_)
        main_tab_widget_->updateSectors();

    updateButtons();
}

void EvaluationDialog::updateFilterWidget()
{
    assert (filter_widget_);
    filter_widget_->update();
}

void EvaluationDialog::updateResultsConfig()
{
    eval_man_.resultsGenerator().widget().updateFromSettings();
}

void EvaluationDialog::updateFromSettings()
{
    updateDataSources();
    updateSectors();
    updateButtons();
    updateFilterWidget();
    updateResultsConfig();
}

void EvaluationDialog::updateButtons()
{
    assert (run_button_);

    if (eval_man_.canLoadDataAndEvaluate())
    {
        not_eval_comment_label_->setText("");
        not_eval_comment_label_->setHidden(true);
    }
    else
    {
        not_eval_comment_label_->setText(eval_man_.getCannotRunAndEvaluateComment().c_str());
        not_eval_comment_label_->setHidden(false);
    }

    run_button_->setEnabled(eval_man_.canLoadDataAndEvaluate());
}
