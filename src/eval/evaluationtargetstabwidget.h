#ifndef EVALUATIONTARGETSTABWIDGET_H
#define EVALUATIONTARGETSTABWIDGET_H

#include <QWidget>

#include <memory>

class EvaluationManager;
class EvaluationManagerWidget;

class EvaluationTargetsTabWidget : public QWidget
{
    Q_OBJECT

private slots:

public:
    EvaluationTargetsTabWidget(EvaluationManager& eval_man, EvaluationManagerWidget& man_widget);

protected:
    EvaluationManager& eval_man_;
    EvaluationManagerWidget& man_widget_;
};

#endif // EVALUATIONTARGETSTABWIDGET_H
