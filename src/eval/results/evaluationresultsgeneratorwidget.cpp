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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include <QFormLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QSpinBox>

EvaluationResultsGeneratorWidget::EvaluationResultsGeneratorWidget(
        EvaluationResultsGenerator& results_gen, EvaluationManager& eval_man,
        EvaluationManagerSettings& eval_settings)
    : results_gen_(results_gen), eval_man_(eval_man), eval_settings_(eval_settings)
{
    QHBoxLayout* main_layout = new QHBoxLayout();

    QFormLayout* layout = new QFormLayout();

    //int row = 0;

    // skip no data details
    //++row;

    skip_no_data_details_check_ = new QCheckBox ();
    skip_no_data_details_check_->setChecked(eval_settings_.report_skip_no_data_details_);
    connect(skip_no_data_details_check_, &QCheckBox::clicked,
            this, &EvaluationResultsGeneratorWidget::toggleSkipNoDataDetailsSlot);
    layout->addRow("Skip No Data Details", skip_no_data_details_check_);

    // split results by MOPS
    //++row;

    split_results_by_mops_check_ = new QCheckBox ();
    split_results_by_mops_check_->setChecked(eval_settings_.report_split_results_by_mops_);
    connect(split_results_by_mops_check_, &QCheckBox::clicked,
            this, &EvaluationResultsGeneratorWidget::toggleSplitResultsByMOPSSlot);
    layout->addRow("Split Results by MOPS Version", split_results_by_mops_check_);

    // split results by mode a/c only and mode s

    split_results_by_mac_ms_check_ = new QCheckBox ();
    split_results_by_mac_ms_check_->setChecked(eval_settings_.report_split_results_by_aconly_ms_);
    connect(split_results_by_mac_ms_check_, &QCheckBox::clicked,
            this, &EvaluationResultsGeneratorWidget::toggleSplitResultsByMACMSSlot);
    layout->addRow("Split Results by Mode A/C Only and Mode S", split_results_by_mac_ms_check_);

    // show adsb info
    //++row;

    show_adsb_info_check_ = new QCheckBox ();
    show_adsb_info_check_->setChecked(eval_settings_.report_show_adsb_info_);
    connect(show_adsb_info_check_, &QCheckBox::clicked,
            this, &EvaluationResultsGeneratorWidget::toggleShowAdsbInfoSlot);
    layout->addRow("Show ADS-B Info", show_adsb_info_check_);

    // show ok

    show_ok_joined_target_reports_check_ = new QCheckBox ();
    show_ok_joined_target_reports_check_->setChecked(eval_settings_.show_ok_joined_target_reports_);
    connect(show_ok_joined_target_reports_check_, &QCheckBox::clicked,
            this, &EvaluationResultsGeneratorWidget::toggleShowOKJoinedSlot);
    layout->addRow("Show OK data in Sector Results", show_ok_joined_target_reports_check_);

    result_detail_zoom_edit_ = new QLineEdit(QString::number(eval_settings_.result_detail_zoom_));
    result_detail_zoom_edit_->setValidator(new QDoubleValidator(0.000001, 1.0, 7, this));
    connect(result_detail_zoom_edit_, &QLineEdit::textEdited,
            this, &EvaluationResultsGeneratorWidget::resultDetailZoomEditSlot);
    layout->addRow("Result Detail WGS84 Zoom Factor [deg]", result_detail_zoom_edit_);

    // grid generation
    grid_num_cells_x_box_  = new QSpinBox;
    grid_num_cells_x_box_->setMinimum(1);
    grid_num_cells_x_box_->setMaximum(2000);
    grid_num_cells_x_box_->setValue(eval_settings_.grid_num_cells_x);
    connect(grid_num_cells_x_box_, QOverload<int>::of(&QSpinBox::valueChanged), [ = ] (int v) { this->eval_settings_.grid_num_cells_x = v; });
    layout->addRow("#Grid Cells X", grid_num_cells_x_box_);

    grid_num_cells_y_box_  = new QSpinBox;
    grid_num_cells_y_box_->setMinimum(1);
    grid_num_cells_y_box_->setMaximum(2000);
    grid_num_cells_y_box_->setValue(eval_settings_.grid_num_cells_y);
    connect(grid_num_cells_y_box_, QOverload<int>::of(&QSpinBox::valueChanged), [ = ] (int v) { this->eval_settings_.grid_num_cells_y = v; });
    layout->addRow("#Grid Cells Y", grid_num_cells_y_box_);

    grid_pix_per_cell_box_ = new QSpinBox;
    grid_pix_per_cell_box_->setMinimum(1);
    grid_pix_per_cell_box_->setMaximum(10);
    grid_pix_per_cell_box_->setValue(eval_settings_.grid_pixels_per_cell);
    connect(grid_pix_per_cell_box_, QOverload<int>::of(&QSpinBox::valueChanged), [ = ] (int v) { this->eval_settings_.grid_pixels_per_cell = v; });
    layout->addRow("#Pixels Per Grid Cell", grid_pix_per_cell_box_);

    setContentsMargins(0, 0, 0, 0);

    main_layout->addLayout(layout);
    main_layout->addStretch();

    setLayout(main_layout);
}

EvaluationResultsGeneratorWidget::~EvaluationResultsGeneratorWidget() = default;

void EvaluationResultsGeneratorWidget::updateFromSettings()
{
    skip_no_data_details_check_->setChecked(eval_settings_.report_skip_no_data_details_);
    split_results_by_mops_check_->setChecked(eval_settings_.report_split_results_by_mops_);
    split_results_by_mac_ms_check_->setChecked(eval_settings_.report_split_results_by_aconly_ms_);
    show_adsb_info_check_->setChecked(eval_settings_.report_show_adsb_info_);
    show_ok_joined_target_reports_check_->setChecked(eval_settings_.show_ok_joined_target_reports_);
    result_detail_zoom_edit_->setText(QString::number(eval_settings_.result_detail_zoom_));

    grid_num_cells_x_box_->setValue(eval_settings_.grid_num_cells_x);
    grid_num_cells_y_box_->setValue(eval_settings_.grid_num_cells_y);
    grid_pix_per_cell_box_->setValue(eval_settings_.grid_pixels_per_cell);
}

void EvaluationResultsGeneratorWidget::toggleSplitResultsByMOPSSlot()
{
    assert (split_results_by_mops_check_);
    eval_settings_.report_split_results_by_mops_ = split_results_by_mops_check_->checkState() == Qt::Checked;
}

void EvaluationResultsGeneratorWidget::toggleSplitResultsByMACMSSlot()
{
    assert (split_results_by_mac_ms_check_);
    eval_settings_.report_split_results_by_aconly_ms_ = split_results_by_mac_ms_check_->checkState() == Qt::Checked;
}

void EvaluationResultsGeneratorWidget::toggleShowAdsbInfoSlot()
{
    assert (show_adsb_info_check_);
    eval_settings_.report_show_adsb_info_ = show_adsb_info_check_->checkState() == Qt::Checked;
}

void EvaluationResultsGeneratorWidget::toggleShowOKJoinedSlot()
{
    assert (show_ok_joined_target_reports_check_);
    eval_settings_.show_ok_joined_target_reports_ = show_ok_joined_target_reports_check_->checkState() == Qt::Checked;
}

void EvaluationResultsGeneratorWidget::toggleSkipNoDataDetailsSlot()
{
    assert (skip_no_data_details_check_);
    eval_settings_.report_skip_no_data_details_ = skip_no_data_details_check_->checkState() == Qt::Checked;
}

void EvaluationResultsGeneratorWidget::resultDetailZoomEditSlot(QString value)
{
    loginf << "EvaluationResultsGeneratorWidget: resultDetailZoomEditSlot: value " << value.toStdString();

    bool ok;
    float val = value.toFloat(&ok);

    if (ok)
        eval_settings_.result_detail_zoom_ = val;
    else
        loginf << "EvaluationResultsGeneratorWidget: resultDetailZoomEditSlot: axvalid value";
}
