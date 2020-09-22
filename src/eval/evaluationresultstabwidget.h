#ifndef EVALUATIONRESULTSTABWIDGET_H
#define EVALUATIONRESULTSTABWIDGET_H

#include <QWidget>

#include <memory>

class EvaluationManager;
class EvaluationManagerWidget;

class EvaluationResultsTabWidget : public QWidget
{
    Q_OBJECT

private slots:

public:
    EvaluationResultsTabWidget(EvaluationManager& eval_man, EvaluationManagerWidget& man_widget);

protected:
    EvaluationManager& eval_man_;
    EvaluationManagerWidget& man_widget_;
};

#endif // EVALUATIONRESULTSTABWIDGET_H
