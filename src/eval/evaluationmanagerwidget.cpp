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

#include "evaluationtargetstabwidget.h"

#include "evaluationresultstabwidget.h"
//#include "evaluationstandardcombobox.h"
#include "evaluationmanager.h"

//#include "evaluationdata.h"
//#include "evaluationdatawidget.h"
//#include "evaluationdatasourcewidget.h"
//#include "evaluationsectorwidget.h"
#include "logger.h"
#include "files.h"

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
    : ToolBoxWidget(nullptr), eval_man_(eval_man), eval_settings_(eval_settings)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    tab_widget_ = new QTabWidget();

    targets_tab_widget_.reset(new EvaluationTargetsTabWidget(eval_man_, *this));
    tab_widget_->addTab(targets_tab_widget_.get(), "Targets");


    results_tab_widget_.reset(new EvaluationResultsTabWidget(eval_man_, *this));
    tab_widget_->addTab(results_tab_widget_.get(), "Results");

    main_layout->addWidget(tab_widget_);

    // // not evaluate comment
    // not_eval_comment_label_ = new QLabel();
    // QPalette palette = not_eval_comment_label_->palette();
    // palette.setColor(not_eval_comment_label_->foregroundRole(), Qt::red);
    // not_eval_comment_label_->setPalette(palette);

    // main_layout_->addWidget(not_eval_comment_label_);

    // buttons
    QHBoxLayout* button_layout = new QHBoxLayout();

    // load_button_ = new QPushButton("Load Data");
    // connect (load_button_, &QPushButton::clicked, this, &EvaluationManagerWidget::loadDataSlot);
    // button_layout->addWidget(load_button_);

    // evaluate_button_ = new QPushButton("Evaluate");
    // connect (evaluate_button_, &QPushButton::clicked, this, &EvaluationManagerWidget::evaluateSlot);
    // button_layout->addWidget(evaluate_button_);

    gen_report_button_ = new QPushButton("Generate Report");
    connect (gen_report_button_, &QPushButton::clicked, this, &EvaluationManagerWidget::generateReportSlot);
    button_layout->addWidget(gen_report_button_);

    main_layout->addLayout(button_layout);

    updateButtons();

    setLayout(main_layout);
}

EvaluationManagerWidget::~EvaluationManagerWidget() = default;

/**
 */
QIcon EvaluationManagerWidget::toolIcon() const
{
    return QIcon(Utils::Files::getIconFilepath("scale.png").c_str());
}

/**
 */
std::string EvaluationManagerWidget::toolName() const 
{
    return "Evaluation Results";
}

/**
 */
std::string EvaluationManagerWidget::toolInfo() const 
{
    return "Evaluation Results";
}

/**
 */
std::vector<std::string> EvaluationManagerWidget::toolLabels() const 
{
    return { "Evaluation", "Results" };
}

/**
 */
toolbox::ScreenRatio EvaluationManagerWidget::defaultScreenRatio() const 
{
    return ToolBoxWidget::defaultScreenRatio();
}

/**
 */
void EvaluationManagerWidget::addToConfigMenu(QMenu* menu) const 
{
}

void EvaluationManagerWidget::updateButtons()
{
    gen_report_button_->setEnabled(eval_man_.canGenerateReport());
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

void EvaluationManagerWidget::generateReportSlot()
{
    loginf << "EvaluationManagerWidget: generateReportSlot";

    eval_man_.generateReport();
}

boost::optional<nlohmann::json> EvaluationManagerWidget::getTableData(const std::string& result_id, 
                                                                      const std::string& table_id,
                                                                      bool rowwise,
                                                                      const std::vector<int>& cols) const
{
    //retrieve special tables
    if (table_id == "Targets")
        return eval_man_.getData().getTableData(rowwise, cols);

    //retrieve result table
    return results_tab_widget_->getTableData(result_id, table_id, rowwise, cols);
}
