#ifndef EVALUATIONMANAGERWIDGET_H
#define EVALUATIONMANAGERWIDGET_H

#include <QWidget>

#include <memory>

class EvaluationManager;
class EvaluationMainTabWidget;
class EvaluationTargetsTabWidget;
class EvaluationStandardTabWidget;
class EvaluationResultsTabWidget;

class QVBoxLayout;
class QTabWidget;
class QPushButton;

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

    void updateButtons();
    void expandResults();

    void showResultId (const std::string& id);
    void reshowLastResultId();

protected:
    EvaluationManager& eval_man_;

    QVBoxLayout* main_layout_{nullptr};

    QTabWidget* tab_widget_{nullptr};

    std::unique_ptr<EvaluationMainTabWidget> main_tab_widget_;
    std::unique_ptr<EvaluationTargetsTabWidget> targets_tab_widget_;
    std::unique_ptr<EvaluationStandardTabWidget> std_tab_widget_;
    std::unique_ptr<EvaluationResultsTabWidget> results_tab_widget_;

    QPushButton* load_button_ {nullptr};
    QPushButton* evaluate_button_ {nullptr};
    QPushButton* gen_report_button_ {nullptr};

    void addMainWidget ();
    void addTargetsWidget ();
    void addStandardWidget ();
    void addResultsWidget ();
};

#endif // EVALUATIONMANAGERWIDGET_H
