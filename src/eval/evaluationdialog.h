#pragma once

#include <QDialog>

#include <memory>

class EvaluationManager;
class EvaluationManagerSettings;
class EvaluationMainTabWidget;
class EvaluationFilterTabWidget;
class EvaluationStandardTabWidget;

class QLabel;

class EvaluationDialog : public QDialog
{
public:
    EvaluationDialog(EvaluationManager& eval_man, EvaluationManagerSettings& eval_settings);
    virtual ~EvaluationDialog();

    void updateDataSources();
    void updateSectors();

    void updateButtons();

    void updateFilterWidget();

protected:
    EvaluationManager& eval_man_;
    EvaluationManagerSettings& eval_settings_;

    std::unique_ptr<EvaluationMainTabWidget> main_tab_widget_;
    std::unique_ptr<EvaluationFilterTabWidget> filter_widget_;
    std::unique_ptr<EvaluationStandardTabWidget> std_tab_widget_;

    QLabel* not_eval_comment_label_ {nullptr};

    QPushButton* cancel_button_{nullptr};
    QPushButton* run_button_{nullptr};
};

