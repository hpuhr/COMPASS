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

#include "evaluationresultsgeneratorwidget.h"
#include "evaluationresultsgenerator.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include <QCheckBox>
#include <QLineEdit>

EvaluationResultsGeneratorWidget::EvaluationResultsGeneratorWidget(EvaluationResultsGenerator& results_gen)
    : results_gen_(results_gen)
{
    QHBoxLayout* main_layout = new QHBoxLayout();

    QGridLayout* layout = new QGridLayout();

    int row = 0;

    // skip no data details
    ++row;

    skip_no_data_details_check_ = new QCheckBox ("Skip No Data Details");
    skip_no_data_details_check_->setChecked(results_gen_.skipNoDataDetails());
    connect(skip_no_data_details_check_, &QCheckBox::clicked,
            this, &EvaluationResultsGeneratorWidget::toggleSkipNoDataDetailsSlot);
    layout->addWidget(skip_no_data_details_check_, row, 0);

    // split results by MOPS
    ++row;

    split_results_by_mops_check_ = new QCheckBox ("Split Results by MOPS Version");
    split_results_by_mops_check_->setChecked(results_gen_.splitResultsByMOPS());
    connect(split_results_by_mops_check_, &QCheckBox::clicked,
            this, &EvaluationResultsGeneratorWidget::toggleSplitResultsByMOPSSlot);
    layout->addWidget(split_results_by_mops_check_, row, 0);

    // show adsb info
    ++row;

    show_adsb_info_check_ = new QCheckBox ("Show ADS-B Info");
    show_adsb_info_check_->setChecked(results_gen_.showAdsbInfo());
    connect(show_adsb_info_check_, &QCheckBox::clicked,
            this, &EvaluationResultsGeneratorWidget::toggleShowAdsbInfoSlot);
    layout->addWidget(show_adsb_info_check_, row, 0);

    setContentsMargins(0, 0, 0, 0);

    main_layout->addLayout(layout);
    main_layout->addStretch();

    setLayout(main_layout);
}

EvaluationResultsGeneratorWidget::~EvaluationResultsGeneratorWidget()
{
}

void EvaluationResultsGeneratorWidget::toggleSplitResultsByMOPSSlot()
{
    assert (split_results_by_mops_check_);
    results_gen_.splitResultsByMOPS(split_results_by_mops_check_->checkState() == Qt::Checked);
}

void EvaluationResultsGeneratorWidget::toggleShowAdsbInfoSlot()
{
    assert (show_adsb_info_check_);
    results_gen_.showAdsbInfo(show_adsb_info_check_->checkState() == Qt::Checked);
}

void EvaluationResultsGeneratorWidget::toggleSkipNoDataDetailsSlot()
{
    assert (skip_no_data_details_check_);
    results_gen_.skipNoDataDetails(skip_no_data_details_check_->checkState() == Qt::Checked);
}

