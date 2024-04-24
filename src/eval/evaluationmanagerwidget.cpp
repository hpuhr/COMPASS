/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "evaluationmanagerwidget.h"
#include "evaluationmaintabwidget.h"
#include "evaluationfiltertabwidget.h"
#include "evaluationtargetstabwidget.h"
#include "evaluationstandardtabwidget.h"
#include "evaluationresultstabwidget.h"
#include "evaluationstandardcombobox.h"
#include "evaluationmanager.h"
#include "evaluationresultsgenerator.h"
#include "evaluationresultsgeneratorwidget.h"
//#include "evaluationdata.h"
//#include "evaluationdatawidget.h"
#include "evaluationdatasourcewidget.h"
#include "evaluationsectorwidget.h"
#include "logger.h"

#include <QCheckBox>
#include <QComboBox>
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
#include <QTimer>

EvaluationManagerWidget::EvaluationManagerWidget(EvaluationManager& eval_man, EvaluationManagerSettings& eval_settings)
    : QWidget(nullptr), eval_man_(eval_man), eval_settings_(eval_settings)
{
    main_layout_ = new QVBoxLayout();

    tab_widget_ = new QTabWidget();

    main_tab_widget_.reset(new EvaluationMainTabWidget(eval_man_, eval_settings_, *this));
    tab_widget_->addTab(main_tab_widget_.get(), "Main");

    targets_tab_widget_.reset(new EvaluationTargetsTabWidget(eval_man_, *this));
    tab_widget_->addTab(targets_tab_widget_.get(), "Targets");

    filter_widget_.reset(new EvaluationFilterTabWidget(eval_man_, eval_settings_, *this));
    tab_widget_->addTab(filter_widget_.get(), "Filter");

    std_tab_widget_.reset(new EvaluationStandardTabWidget(eval_man_, eval_settings_, *this));
    tab_widget_->addTab(std_tab_widget_.get(), "Standard");

    tab_widget_->addTab(&eval_man_.resultsGenerator().widget(), "Results Config");

    results_tab_widget_.reset(new EvaluationResultsTabWidget(eval_man_, *this));
    tab_widget_->addTab(results_tab_widget_.get(), "Results");

    main_layout_->addWidget(tab_widget_);

    // not evaluate comment
    not_eval_comment_label_ = new QLabel();
    QPalette palette = not_eval_comment_label_->palette();
    palette.setColor(not_eval_comment_label_->foregroundRole(), Qt::red);
    not_eval_comment_label_->setPalette(palette);

    main_layout_->addWidget(not_eval_comment_label_);

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

EvaluationManagerWidget::~EvaluationManagerWidget() = default;

void EvaluationManagerWidget::updateDataSources()
{
    if (main_tab_widget_)
        main_tab_widget_->updateDataSources();
}

void EvaluationManagerWidget::updateSectors()
{
    if (main_tab_widget_)
        main_tab_widget_->updateSectors();

    updateButtons();
}

void EvaluationManagerWidget::updateButtons()
{
    load_button_->setEnabled(eval_man_.anySectorsWithReq()
                             && eval_man_.hasSelectedReferenceDataSources()
                             && eval_man_.hasSelectedTestDataSources());

    evaluate_button_->setEnabled(eval_man_.canEvaluate());
    gen_report_button_->setEnabled(eval_man_.canGenerateReport());

    if (eval_man_.canEvaluate())
    {
        not_eval_comment_label_->setText("");
        not_eval_comment_label_->setHidden(true);
    }
    else
    {
        not_eval_comment_label_->setText(eval_man_.getCannotEvaluateComment().c_str());
        not_eval_comment_label_->setHidden(false);
    }
}

void EvaluationManagerWidget::updateFilterWidget()
{
    assert (filter_widget_);
    filter_widget_->update();
}

void EvaluationManagerWidget::updateResultsConfig()
{
    eval_man_.resultsGenerator().widget().updateFromSettings();
}

void EvaluationManagerWidget::updateFromSettings()
{
    updateDataSources();
    updateSectors();
    updateButtons();
    updateFilterWidget();
    updateResultsConfig();
}

void EvaluationManagerWidget::expandResults()
{
    assert (results_tab_widget_);

    results_tab_widget_->expand();
}

void EvaluationManagerWidget::showResultId (const std::string& id, 
                                            bool select_tab,
                                            bool show_figure)
{
    assert(tab_widget_);
    assert (results_tab_widget_);

    if (select_tab) 
        tab_widget_->setCurrentWidget(results_tab_widget_.get());

    results_tab_widget_->selectId(id, show_figure);
}

void EvaluationManagerWidget::reshowLastResultId()
{
    assert (results_tab_widget_);
    results_tab_widget_->reshowLastId();
}

void EvaluationManagerWidget::loadDataSlot()
{
    loginf << "EvaluationManagerWidget: loadDataSlot";

    if (!eval_settings_.warning_shown_)
    {
        QMessageBox* mbox = new QMessageBox;
        mbox->setWindowTitle(tr("Warning"));
        mbox->setText("Please note that the Evaluation feature is currently not verified and should be used"
                      " for testing/validation purposes only.");
        mbox->exec();

        eval_settings_.warning_shown_ = true;
    }

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

boost::optional<nlohmann::json> EvaluationManagerWidget::getTableData(const std::string& result_id, const std::string& table_id) const
{
    return results_tab_widget_->getTableData(result_id, table_id);
}
