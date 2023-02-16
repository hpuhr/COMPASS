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

#ifndef EVALUATIONMANAGERWIDGET_H
#define EVALUATIONMANAGERWIDGET_H

#include <QWidget>

#include <memory>

class EvaluationManager;
class EvaluationMainTabWidget;
class EvaluationFilterTabWidget;
class EvaluationTargetsTabWidget;
class EvaluationStandardTabWidget;
class EvaluationResultsTabWidget;

class QVBoxLayout;
class QTabWidget;
class QPushButton;
class QLabel;

class EvaluationManagerWidget : public QWidget
{
    Q_OBJECT

private slots:
    void loadDataSlot();
    void evaluateSlot();
    void generateReportSlot();

public:
    EvaluationManagerWidget(EvaluationManager& eval_man);
    virtual ~EvaluationManagerWidget();

    void updateDataSources();
    void updateSectors();
    void updateButtons();
    void updateFilterWidget();
    void expandResults();

    void showResultId (const std::string& id);
    void reshowLastResultId();

protected:
    EvaluationManager& eval_man_;

    QVBoxLayout* main_layout_{nullptr};

    QTabWidget* tab_widget_{nullptr};

    std::unique_ptr<EvaluationMainTabWidget> main_tab_widget_;
    std::unique_ptr<EvaluationFilterTabWidget> filter_widget_;
    std::unique_ptr<EvaluationTargetsTabWidget> targets_tab_widget_;
    std::unique_ptr<EvaluationStandardTabWidget> std_tab_widget_;
    std::unique_ptr<EvaluationResultsTabWidget> results_tab_widget_;

    QLabel* not_eval_comment_label_ {nullptr};
    QPushButton* load_button_ {nullptr};
    QPushButton* evaluate_button_ {nullptr};
    QPushButton* gen_report_button_ {nullptr};
};

#endif // EVALUATIONMANAGERWIDGET_H
