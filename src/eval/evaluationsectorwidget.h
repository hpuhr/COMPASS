#ifndef EVALUATIONSECTORWIDGET_H
#define EVALUATIONSECTORWIDGET_H

#include <QWidget>

class EvaluationManager;

class QGridLayout;

class EvaluationSectorWidget : public QWidget
{
    Q_OBJECT

public slots:
    void toggleUseGroupSlot();

public:
    EvaluationSectorWidget(EvaluationManager& eval_man);

    void update();

protected:
    EvaluationManager& eval_man_;
    QGridLayout* grid_layout_ {nullptr};
};

#endif // EVALUATIONSECTORWIDGET_H
