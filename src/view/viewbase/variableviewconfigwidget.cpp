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

#include "variableviewconfigwidget.h"
#include "variableview.h"
#include "viewvariable.h"

#include "viewevaldatawidget.h"

#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variableselectionwidget.h"

#include "logger.h"
#include "variable.h"
#include "metavariable.h"
#include "ui_test_common.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <QRadioButton>

/**
*/
VariableViewConfigWidget::VariableViewConfigWidget(ViewWidget* view_widget, 
                                                   VariableView* view,
                                                   QWidget* parent)
:   TabStyleViewConfigWidget(view_widget, parent)
,   var_view_(view)
{
    assert(var_view_);

    auto addVariableUI = [ & ] (QVBoxLayout* layout, int idx)
    {
        const auto& var = var_view_->variable(idx);

        std::string label       = var.settings().display_name + " Variable";
        std::string object_name = "variable_selection_" + var.settings().var_name;

        layout->addWidget(new QLabel(QString::fromStdString(label)));

        auto sel_widget = new dbContent::VariableSelectionWidget();
        sel_widget->setObjectName(QString::fromStdString(object_name));

        var_view_->variable(idx).configureWidget(*sel_widget);

        var_selection_widgets_.push_back(sel_widget);
        updateSelectedVariables(idx);

        connect(sel_widget, &dbContent::VariableSelectionWidget::selectionChanged, 
            [ this, idx ] () { this->selectedVariableChangedSlot(idx); } );

        layout->addWidget(sel_widget);
    };

    bool show_result = var_view_->showResults() && var_view_->hasResultID();

    QWidget*     cfg_widget = new QWidget();
    QVBoxLayout* cfg_layout = new QVBoxLayout();

    // variables
    {
        show_variables_box_ = new QRadioButton("Show Variable Data");
        show_variables_box_->setChecked(!show_result);
        UI_TEST_OBJ_NAME(show_variables_box_, show_variables_box_->text())

        connect(show_variables_box_, &QRadioButton::toggled, this, &VariableViewConfigWidget::dataSourceToggled);

        cfg_layout->addWidget(show_variables_box_);

        variables_widget_ = new QWidget;

        QVBoxLayout* var_layout = new QVBoxLayout;
        variables_widget_->setLayout(var_layout);

        for (size_t i = 0; i < var_view_->numVariables(); ++i)
            addVariableUI(var_layout, i);

        cfg_layout->addWidget(variables_widget_);
    }

    //eval results
    {
        show_eval_results_box_ = new QRadioButton("Show Evaluation Result Data");
        show_eval_results_box_->setChecked(!show_result);
        UI_TEST_OBJ_NAME(show_eval_results_box_, show_eval_results_box_->text())

        connect(show_eval_results_box_, &QRadioButton::toggled, this, &VariableViewConfigWidget::dataSourceToggled);

        cfg_layout->addWidget(show_eval_results_box_);

        eval_id_widget_ = new ViewEvalDataIDWidget;

        cfg_layout->addWidget(eval_id_widget_);
    }

    //deactivate eval result related ui if not supported by view
    show_variables_box_->setVisible(var_view_->canShowResults());
    show_eval_results_box_->setVisible(var_view_->canShowResults());
    eval_id_widget_->setVisible(var_view_->canShowResults());

    config_layout_ = new QVBoxLayout;

    cfg_layout->addLayout(config_layout_);
    cfg_layout->addStretch();

    cfg_widget->setLayout(cfg_layout);

    getTabWidget()->addTab(cfg_widget, "Config");

    //update ui
    updateConfig();
}

/**
*/
VariableViewConfigWidget::~VariableViewConfigWidget() = default;

/**
*/
void VariableViewConfigWidget::selectedVariableChangedSlot(int idx)
{
    loginf << "VariableViewConfigWidget: selectedVariableChangedSlot: idx = " << idx;

    auto selection = var_selection_widgets_.at(idx);
    assert(selection);

    var_view_->variable(idx).setVariable(*selection, true);
}

/**
*/
void VariableViewConfigWidget::configChanged()
{
    updateSelectedVariables();

    //invoke derived
    configChanged_impl();
}

/**
*/
void VariableViewConfigWidget::updateSelectedVariables()
{
    size_t n = var_view_->numVariables();
    
    for (size_t i = 0; i < n; ++i)
        updateSelectedVariables(i);
}

/**
*/
void VariableViewConfigWidget::updateSelectedVariables(size_t idx)
{
    auto selection = var_selection_widgets_.at(idx);
    assert(selection);

    var_view_->variable(idx).updateWidget(*selection);
}

/**
*/
void VariableViewConfigWidget::viewInfoJSON_impl(nlohmann::json& info) const
{
    //variable related
    for (size_t i = 0; i < var_selection_widgets_.size(); ++i)
    {
        auto w = var_selection_widgets_[ i ];

        const auto& var = var_view_->variable(i);

        std::string tag = "selected_var_" + var.settings().var_name;

        if (w->hasMetaVariable())
            info[ tag ] = "Meta - " + w->selectedMetaVariable().name();
        else
            info[ tag ] = w->selectedVariable().dbContentName() + " - " + w->selectedVariable().name();
    }

    //eval result related
    info[ "eval_result_active" ] = show_eval_results_box_->isChecked();
    info[ "eval_result_id"     ] = eval_id_widget_->getID().toString();
}

/**
*/
void VariableViewConfigWidget::updateEvalConfig()
{
    loginf << "VariableViewConfigWidget: updateEvalConfig";

    assert(eval_id_widget_);
    eval_id_widget_->setID(var_view_->resultID());
}

/**
 */
void VariableViewConfigWidget::updateConfig()
{
    bool has_result  = var_view_->hasResultID();
    bool show_result = var_view_->showResults() && has_result;

    show_eval_results_box_->blockSignals(true);
    show_eval_results_box_->setChecked(show_result);
    show_eval_results_box_->blockSignals(false);

    show_variables_box_->blockSignals(true);
    show_variables_box_->setChecked(!show_result);
    show_variables_box_->blockSignals(false);

    updateEvalConfig();

    show_eval_results_box_->setEnabled(has_result);

    eval_id_widget_->setEnabled(show_result);
    variables_widget_->setEnabled(!show_result);
}

/**
 */
void VariableViewConfigWidget::dataSourceToggled()
{
    //modify state in view based on selected radio button
    bool show_results  = show_eval_results_box_->isChecked();
    var_view_->showResults(show_results);

    //then update config
    updateConfig();
}