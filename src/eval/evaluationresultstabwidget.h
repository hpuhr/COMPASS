#ifndef EVALUATIONRESULTSTABWIDGET_H
#define EVALUATIONRESULTSTABWIDGET_H

#include <QWidget>
#include <QTreeView>

#include <memory>

class EvaluationManager;
class EvaluationManagerWidget;

class EvaluationResultsTabWidget : public QWidget
{
    Q_OBJECT

private slots:

public:
    EvaluationResultsTabWidget(EvaluationManager& eval_man, EvaluationManagerWidget& man_widget);

    void expandAll();

protected:
    EvaluationManager& eval_man_;
    EvaluationManagerWidget& man_widget_;

    std::unique_ptr<QTreeView> tree_view_;

    //QStackedWidget* requirements_widget_{nullptr};
};

#endif // EVALUATIONRESULTSTABWIDGET_H
