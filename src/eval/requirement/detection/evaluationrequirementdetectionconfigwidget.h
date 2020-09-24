#ifndef EVALUATIONREQUIREMENTDETECTIONCONFIGWIDGET_H
#define EVALUATIONREQUIREMENTDETECTIONCONFIGWIDGET_H

#include <QWidget>

class EvaluationRequirementDetectionConfig;

class QFormLayout;

class EvaluationRequirementDetectionConfigWidget : public QWidget
{
    Q_OBJECT

public slots:

public:
    EvaluationRequirementDetectionConfigWidget(EvaluationRequirementDetectionConfig& config);

protected:
    EvaluationRequirementDetectionConfig& config_;

    QFormLayout* form_layout_ {nullptr};
};

#endif // EVALUATIONREQUIREMENTDETECTIONCONFIGWIDGET_H
