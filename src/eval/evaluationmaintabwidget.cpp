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
#include "airspace.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>

EvaluationMainTabWidget::EvaluationMainTabWidget(EvaluationManager& eval_man, EvaluationManagerSettings& eval_settings,
                                                 EvaluationManagerWidget& man_widget)
    : QWidget(nullptr), eval_man_(eval_man),  eval_settings_(eval_settings), man_widget_(man_widget)
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
    data_source_ref_widget_.reset(
                new EvaluationDataSourceWidget("Reference Data", eval_man_.dbContentNameRef(), eval_settings_.line_id_ref_));
    connect (data_source_ref_widget_.get(), &EvaluationDataSourceWidget::dbContentNameChangedSignal,
             this, &EvaluationMainTabWidget::dboRefNameChangedSlot);
    connect (data_source_ref_widget_.get(), &EvaluationDataSourceWidget::lineChangedSignal,
             this, &EvaluationMainTabWidget::lineRefChangedSlot);
    data_sources_layout->addWidget(data_source_ref_widget_.get());

    data_source_tst_widget_.reset(
                new EvaluationDataSourceWidget("Test Data", eval_man_.dbContentNameTst(), eval_settings_.line_id_tst_));
    connect (data_source_tst_widget_.get(), &EvaluationDataSourceWidget::dbContentNameChangedSignal,
             this, &EvaluationMainTabWidget::dboTstNameChangedSlot);
    connect (data_source_tst_widget_.get(), &EvaluationDataSourceWidget::lineChangedSignal,
             this, &EvaluationMainTabWidget::lineTstChangedSlot);
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

    // minimum height filter
    QHBoxLayout* height_filter_layout = new QHBoxLayout();

    QLabel* height_filter_label = new QLabel("Sector Layers: Minimum Height Filter");
    height_filter_label->setFont(font_bold);
    height_filter_layout->addWidget(height_filter_label);

    min_height_filter_combo_ = new QComboBox;
    height_filter_layout->addWidget(min_height_filter_combo_);

    connect(min_height_filter_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EvaluationMainTabWidget::minHeightFilterChangedSlot);

    main_layout->addLayout(height_filter_layout);

    // sector requirement mapping
    QVBoxLayout* sec_layout = new QVBoxLayout();

    QLabel* sec_label = new QLabel("Sector Layers: Requirement Groups Usage");
    sec_label->setFont(font_bold);
    sec_layout->addWidget(sec_label);

    sector_widget_.reset(new EvaluationSectorWidget(eval_man_));
    sector_widget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sec_layout->addWidget(sector_widget_.get());

    main_layout->addLayout(sec_layout);

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

void EvaluationMainTabWidget::updateSectors()
{
    assert (sector_widget_);
    assert (min_height_filter_combo_);

    sector_widget_->update();

    updateMinHeightFilterCombo();
}

void EvaluationMainTabWidget::updateMinHeightFilterCombo()
{
    assert (min_height_filter_combo_);

    min_height_filter_combo_->blockSignals(true);

    min_height_filter_combo_->clear();
    min_height_filter_combo_->addItem("None");

    if (eval_man_.hasCurrentStandard() && eval_man_.sectorsLoaded())
    {
        for (const auto& sec_lay_it : eval_man_.sectorsLayers())
            min_height_filter_combo_->addItem(QString::fromStdString(sec_lay_it->name()));
    }

    min_height_filter_combo_->setCurrentIndex(0);

    if (eval_man_.filterMinimumHeight())
    {
        const auto& layer_name = eval_man_.minHeightFilterLayerName();
        int idx = min_height_filter_combo_->findText(QString::fromStdString(layer_name));

        //assert(idx >= 0);

        if (idx >= 0)
            min_height_filter_combo_->setCurrentIndex(idx);
    }

    min_height_filter_combo_->blockSignals(false);
}

void EvaluationMainTabWidget::minHeightFilterChangedSlot(int idx)
{
    if (idx <= 0)
    {
        eval_man_.minHeightFilterLayerName("");
        return;
    }

    eval_man_.minHeightFilterLayerName(min_height_filter_combo_->itemText(idx).toStdString());
}

void EvaluationMainTabWidget::dboRefNameChangedSlot(const std::string& dbcontent_name)
{
    loginf << "EvaluationMainTabWidget: dboRefNameChangedSlot: name " << dbcontent_name;

    eval_man_.dbContentNameRef(dbcontent_name);
}

void EvaluationMainTabWidget::lineRefChangedSlot(unsigned int line_id)
{
    loginf << "EvaluationMainTabWidget: lineRefChangedSlot: value " << line_id;

    eval_settings_.line_id_ref_ = line_id;
}

void EvaluationMainTabWidget::dboTstNameChangedSlot(const std::string& dbcontent_name)
{
    loginf << "EvaluationMainTabWidget: dboTstNameChangedSlot: name " << dbcontent_name;

    eval_man_.dbContentNameTst(dbcontent_name);
}

void EvaluationMainTabWidget::lineTstChangedSlot(unsigned int line_id)
{
    loginf << "EvaluationMainTabWidget: lineTstChangedSlot: value " << line_id;

    eval_settings_.line_id_tst_ = line_id;
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
