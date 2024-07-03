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

#pragma once

#include <QWidget>

class QLineEdit;
class QCheckBox;
class QSpinBox;

class EvaluationResultsGenerator;
class EvaluationManager;
class EvaluationManagerSettings;

class EvaluationResultsGeneratorWidget : public QWidget
{
    Q_OBJECT
private slots:
    void toggleSplitResultsByMOPSSlot();
    void toggleSplitResultsByMACMSSlot();
    void toggleShowOKJoinedSlot();
    void toggleSkipNoDataDetailsSlot();

    void resultDetailZoomEditSlot(QString value);

public:
    EvaluationResultsGeneratorWidget(EvaluationResultsGenerator& results_gen,
                                     EvaluationManager& eval_man, EvaluationManagerSettings& eval_settings);
    virtual ~EvaluationResultsGeneratorWidget();

    void updateFromSettings();

protected:
    EvaluationResultsGenerator& results_gen_;
    EvaluationManager& eval_man_;
    EvaluationManagerSettings& eval_settings_;

    QCheckBox* skip_no_data_details_check_ {nullptr};
    QCheckBox* split_results_by_mops_check_ {nullptr};
    QCheckBox* split_results_by_mac_ms_check_ {nullptr};
    QCheckBox* show_ok_joined_target_reports_check_ {nullptr};

    QLineEdit* result_detail_zoom_edit_{nullptr};

    QSpinBox*  grid_num_cells_x_box_  = nullptr;
    QSpinBox*  grid_num_cells_y_box_  = nullptr;
    QSpinBox*  grid_pix_per_cell_box_ = nullptr;
};
