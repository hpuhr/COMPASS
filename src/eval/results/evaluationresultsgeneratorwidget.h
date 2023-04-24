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

#ifndef EVALUATIONRESULTSGENERATORWIDGET_H
#define EVALUATIONRESULTSGENERATORWIDGET_H

#include <QWidget>

class QLineEdit;
class QCheckBox;

class EvaluationResultsGenerator;
class EvaluationManager;

class EvaluationResultsGeneratorWidget : public QWidget
{
    Q_OBJECT
private slots:
    void toggleSplitResultsByMOPSSlot();
    void toggleSplitResultsByMACMSSlot();
    void toggleShowAdsbInfoSlot();
    void toggleSkipNoDataDetailsSlot();

    void resultDetailZoomEditSlot(QString value);

public:
    EvaluationResultsGeneratorWidget(EvaluationResultsGenerator& results_gen, EvaluationManager& eval_man);
    virtual ~EvaluationResultsGeneratorWidget();

protected:
    EvaluationResultsGenerator& results_gen_;
    EvaluationManager& eval_man_;

    QCheckBox* skip_no_data_details_check_ {nullptr};
    QCheckBox* split_results_by_mops_check_ {nullptr};
    QCheckBox* split_results_by_mac_ms_check_ {nullptr};
    QCheckBox* show_adsb_info_check_ {nullptr};

    QLineEdit* result_detail_zoom_edit_{nullptr};
};

#endif // EVALUATIONRESULTSGENERATORWIDGET_H
