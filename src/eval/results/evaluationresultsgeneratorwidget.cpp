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
#include "evaluationmanager.h"
#include "textfielddoublevalidator.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include <QFormLayout>
#include <QCheckBox>
#include <QLineEdit>

EvaluationResultsGeneratorWidget::EvaluationResultsGeneratorWidget(
        EvaluationResultsGenerator& results_gen, EvaluationManager& eval_man)
    : results_gen_(results_gen), eval_man_(eval_man)
{
    QHBoxLayout* main_layout = new QHBoxLayout();

    QFormLayout* layout = new QFormLayout();

    //int row = 0;

    // skip no data details
    //++row;

    skip_no_data_details_check_ = new QCheckBox ();
    skip_no_data_details_check_->setChecked(eval_man_.reportSkipNoDataDetails());
    connect(skip_no_data_details_check_, &QCheckBox::clicked,
            this, &EvaluationResultsGeneratorWidget::toggleSkipNoDataDetailsSlot);
    layout->addRow("Skip No Data Details", skip_no_data_details_check_);

    // split results by MOPS
    //++row;

    split_results_by_mops_check_ = new QCheckBox ();
    split_results_by_mops_check_->setChecked(eval_man_.reportSplitResultsByMOPS());
    connect(split_results_by_mops_check_, &QCheckBox::clicked,
            this, &EvaluationResultsGeneratorWidget::toggleSplitResultsByMOPSSlot);
    layout->addRow("Split Results by MOPS Version", split_results_by_mops_check_);

    // split results by mode a/c only and mode s

    split_results_by_mac_ms_check_ = new QCheckBox ();
    split_results_by_mac_ms_check_->setChecked(eval_man_.reportSplitResultsByACOnlyMS());
    connect(split_results_by_mac_ms_check_, &QCheckBox::clicked,
            this, &EvaluationResultsGeneratorWidget::toggleSplitResultsByMACMSSlot);
    layout->addRow("Split Results by Mode A/C Only and Mode S", split_results_by_mac_ms_check_);

    // show adsb info
    //++row;

    show_adsb_info_check_ = new QCheckBox ();
    show_adsb_info_check_->setChecked(eval_man_.reportShowAdsbInfo());
    connect(show_adsb_info_check_, &QCheckBox::clicked,
            this, &EvaluationResultsGeneratorWidget::toggleShowAdsbInfoSlot);
    layout->addRow("Show ADS-B Info", show_adsb_info_check_);


    // show ok

    show_ok_joined_target_reports_check_ = new QCheckBox ();
    show_ok_joined_target_reports_check_->setChecked(eval_man_.showJoinedOkTargetReports());
    connect(show_ok_joined_target_reports_check_, &QCheckBox::clicked,
            this, &EvaluationResultsGeneratorWidget::toggleShowOKJoinedSlot);
    layout->addRow("Show OK data in Sector Results", show_ok_joined_target_reports_check_);

    result_detail_zoom_edit_ = new QLineEdit(QString::number(eval_man_.resultDetailZoom()));
    result_detail_zoom_edit_->setValidator(new QDoubleValidator(0.000001, 1.0, 7, this));
    connect(result_detail_zoom_edit_, &QLineEdit::textEdited,
            this, &EvaluationResultsGeneratorWidget::resultDetailZoomEditSlot);
    layout->addRow("Result Detail WGS84 Zoom Factor [deg]", result_detail_zoom_edit_);


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
    eval_man_.reportSplitResultsByMOPS(split_results_by_mops_check_->checkState() == Qt::Checked);
}

void EvaluationResultsGeneratorWidget::toggleSplitResultsByMACMSSlot()
{
    assert (split_results_by_mac_ms_check_);
    eval_man_.reportSplitResultsByACOnlyMS(split_results_by_mac_ms_check_->checkState() == Qt::Checked);
}

void EvaluationResultsGeneratorWidget::toggleShowAdsbInfoSlot()
{
    assert (show_adsb_info_check_);
    eval_man_.reportShowAdsbInfo(show_adsb_info_check_->checkState() == Qt::Checked);
}

void EvaluationResultsGeneratorWidget::toggleShowOKJoinedSlot()
{
    assert (show_ok_joined_target_reports_check_);
    eval_man_.showJoinedOkTargetReports(show_ok_joined_target_reports_check_->checkState() == Qt::Checked);
}

void EvaluationResultsGeneratorWidget::toggleSkipNoDataDetailsSlot()
{
    assert (skip_no_data_details_check_);
    eval_man_.reportSkipNoDataDetails(skip_no_data_details_check_->checkState() == Qt::Checked);
}

void EvaluationResultsGeneratorWidget::resultDetailZoomEditSlot(QString value)
{
    loginf << "EvaluationResultsGeneratorWidget: resultDetailZoomEditSlot: value " << value.toStdString();

    bool ok;
    float val = value.toFloat(&ok);

    if (ok)
        eval_man_.resultDetailZoom(val);
    else
        loginf << "EvaluationResultsGeneratorWidget: resultDetailZoomEditSlot: axvalid value";
}
