#ifndef EVALUATIONMANAGERWIDGET_H
#define EVALUATIONMANAGERWIDGET_H

#include <QWidget>
#include <memory>

class EvaluationManager;
class EvaluationDataSourceWidget;

class QHBoxLayout;
class QTabWidget;

class EvaluationManagerWidget : public QWidget
{
    Q_OBJECT

private slots:

public:
    EvaluationManagerWidget(EvaluationManager& eval_man);
    virtual ~EvaluationManagerWidget();

protected:
    EvaluationManager& eval_man_;

    QHBoxLayout* main_layout_{nullptr};

    QTabWidget* tab_widget_{nullptr};

    std::unique_ptr<EvaluationDataSourceWidget> data_source_ref_widget_ {nullptr};
    std::unique_ptr<EvaluationDataSourceWidget> data_source_tst_widget_ {nullptr};

    void addMainWidget ();
    void addTargetsWidget ();
    void addStandardWidget ();
    void addResultsWidget ();
};

#endif // EVALUATIONMANAGERWIDGET_H
