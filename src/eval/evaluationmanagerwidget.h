#ifndef EVALUATIONMANAGERWIDGET_H
#define EVALUATIONMANAGERWIDGET_H

#include <QWidget>

#include <memory>

class EvaluationManager;
class EvaluationManagerMainTabWidget;

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

protected:
    EvaluationManager& eval_man_;

    QVBoxLayout* main_layout_{nullptr};

    QTabWidget* tab_widget_{nullptr};

    std::unique_ptr<EvaluationManagerMainTabWidget> main_tab_widget_;

    QPushButton* load_button_ {nullptr};
    QPushButton* evaluate_button_ {nullptr};
    QPushButton* gen_report_button_ {nullptr};

    void addMainWidget ();
    void addTargetsWidget ();
    void addStandardWidget ();
    void addResultsWidget ();
};

#endif // EVALUATIONMANAGERWIDGET_H
