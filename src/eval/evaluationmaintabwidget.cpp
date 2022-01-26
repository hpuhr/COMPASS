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

#include "evaluationmaintabwidget.h"
#include "evaluationmanagerwidget.h"
#include "evaluationdatasourcewidget.h"
#include "evaluationmanager.h"
#include "evaluationstandardcombobox.h"
#include "evaluationsectorwidget.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QCheckBox>
#include <QLineEdit>

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

    // titles define which data sources to display
    data_source_ref_widget_.reset(new EvaluationDataSourceWidget("Reference Data", eval_man_.dboNameRef()));
    connect (data_source_ref_widget_.get(), &EvaluationDataSourceWidget::dboNameChangedSignal,
             this, &EvaluationMainTabWidget::dboRefNameChangedSlot);
    data_sources_layout->addWidget(data_source_ref_widget_.get());

    data_source_tst_widget_.reset(new EvaluationDataSourceWidget("Test Data", eval_man_.dboNameTst()));
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
    std_layout->addWidget(standard_box_.get());

    main_layout->addLayout(std_layout);

    // sector requirement mapping

    QVBoxLayout* sec_layout = new QVBoxLayout();

    QLabel* sec_label = new QLabel("Sector Layers: Requirement Groups Usage");
    sec_label->setFont(font_bold);
    sec_layout->addWidget(sec_label);

    sector_widget_.reset(new EvaluationSectorWidget(eval_man_));
    sec_layout->addWidget(sector_widget_.get());

    main_layout->addLayout(sec_layout);

    // additional config
//    QFormLayout* cfg_layout = new QFormLayout();

//    main_layout->addLayout(cfg_layout);

    main_layout->addStretch();

    // connections

    connect (&eval_man_, &EvaluationManager::standardsChangedSignal,
             this, &EvaluationMainTabWidget::changedStandardsSlot);
    connect (&eval_man_, &EvaluationManager::currentStandardChangedSignal,
             this, &EvaluationMainTabWidget::changedCurrentStandardSlot);

    setContentsMargins(0, 0, 0, 0);
    setLayout(main_layout);
}

void EvaluationMainTabWidget::updateDataSources()
{
    if (data_source_ref_widget_)
        data_source_ref_widget_->updateDataSources();

    if (data_source_tst_widget_)
        data_source_tst_widget_->updateDataSources();
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

    logdbg << "EvaluationMainTabWidget: changedCurrentStandardSlot: sectors";

    assert (sector_widget_);
    sector_widget_->update();

    logdbg << "EvaluationMainTabWidget: changedCurrentStandardSlot: done";
}

