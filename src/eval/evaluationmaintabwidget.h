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

#ifndef EVALUATIONMANAGERMAINTABWIDGET_H
#define EVALUATIONMANAGERMAINTABWIDGET_H

#include <QWidget>

#include <memory>

class EvaluationManager;
class EvaluationManagerSettings;
class EvaluationManagerWidget;
class EvaluationDataSourceWidget;
class EvaluationStandardComboBox;
class EvaluationSectorWidget;

class QComboBox;

/**
*/
class EvaluationMainTabWidget : public QWidget
{
    Q_OBJECT

private slots:
    void dboRefNameChangedSlot(const std::string& dbcontent_name);
    void lineRefChangedSlot(unsigned int line_id);
    void dboTstNameChangedSlot(const std::string& dbcontent_name);
    void lineTstChangedSlot(unsigned int line_id);
    void minHeightFilterChangedSlot(int idx);

    void changedStandardsSlot(); // eval man
    void changedCurrentStandardSlot(); // eval man

public:
    EvaluationMainTabWidget(EvaluationManager& eval_man, EvaluationManagerSettings& eval_settings,
                            EvaluationManagerWidget& man_widget);

    void updateDataSources();
    void updateSectors();

protected:
    void updateMinHeightFilterCombo();

    EvaluationManager& eval_man_;
    EvaluationManagerSettings& eval_settings_;
    EvaluationManagerWidget& man_widget_;

    std::unique_ptr<EvaluationDataSourceWidget> data_source_ref_widget_ {nullptr};
    std::unique_ptr<EvaluationDataSourceWidget> data_source_tst_widget_ {nullptr};

    std::unique_ptr<EvaluationStandardComboBox> standard_box_ {nullptr};
    std::unique_ptr<EvaluationSectorWidget> sector_widget_ {nullptr};

    QComboBox* min_height_filter_combo_ {nullptr};
};

#endif // EVALUATIONMANAGERMAINTABWIDGET_H
