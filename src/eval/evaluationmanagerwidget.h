#ifndef EVALUATIONMANAGERWIDGET_H
#define EVALUATIONMANAGERWIDGET_H

#include <QWidget>

class EvaluationManager;

class EvaluationManagerWidget : public QWidget
{
    Q_OBJECT

private slots:

public:
    EvaluationManagerWidget(EvaluationManager& eval_man);
    virtual ~EvaluationManagerWidget();

protected:
    EvaluationManager& eval_man_;
};

#endif // EVALUATIONMANAGERWIDGET_H
