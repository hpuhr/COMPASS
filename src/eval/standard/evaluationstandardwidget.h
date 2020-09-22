#ifndef EVALUATIONSTANDARDWIDGET_H
#define EVALUATIONSTANDARDWIDGET_H

#include <QWidget>

class EvaluationStandard;

class EvaluationStandardWidget : public QWidget
{
public:
    EvaluationStandardWidget(EvaluationStandard& standard);

protected:
    EvaluationStandard& standard_;
};

#endif // EVALUATIONSTANDARDWIDGET_H
