#pragma once

#include <QDialog>

#include <memory>

class EvaluationCalculator;
class EvaluationMainTabWidget;
class EvaluationFilterTabWidget;
class EvaluationStandardTabWidget;
class EvaluationReportTabWidget;

class QLabel;

class EvaluationDialog : public QDialog
{
public:
    EvaluationDialog(EvaluationCalculator& calculator);
    virtual ~EvaluationDialog();

    void updateDataSources();
    void updateSectors();

    void updateButtons();

    void updateFilterWidget();

protected:
    EvaluationCalculator& calculator_;

    std::unique_ptr<EvaluationMainTabWidget> main_tab_widget_;
    std::unique_ptr<EvaluationFilterTabWidget> filter_widget_;
    std::unique_ptr<EvaluationStandardTabWidget> std_tab_widget_;
    std::unique_ptr<EvaluationReportTabWidget> report_tab_widget_;

    QLabel* not_eval_comment_label_ {nullptr};

    QPushButton* cancel_button_{nullptr};
    QPushButton* run_button_{nullptr};
};

