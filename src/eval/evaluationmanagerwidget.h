#ifndef EVALUATIONMANAGERWIDGET_H
#define EVALUATIONMANAGERWIDGET_H

#include <QWidget>
#include <memory>

class EvaluationManager;
class EvaluationDataSourceWidget;

class QVBoxLayout;
class QTabWidget;
class QPushButton;

class EvaluationManagerWidget : public QWidget
{
    Q_OBJECT

private slots:
    void dboRefNameChangedSlot(const std::string& dbo_name);
    void dboTstNameChangedSlot(const std::string& dbo_name);

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

    std::unique_ptr<EvaluationDataSourceWidget> data_source_ref_widget_ {nullptr};
    std::unique_ptr<EvaluationDataSourceWidget> data_source_tst_widget_ {nullptr};

    QPushButton* load_button_ {nullptr};
    QPushButton* evaluate_button_ {nullptr};
    QPushButton* gen_report_button_ {nullptr};

    void addMainWidget ();
    void addTargetsWidget ();
    void addStandardWidget ();
    void addResultsWidget ();
};

#endif // EVALUATIONMANAGERWIDGET_H
