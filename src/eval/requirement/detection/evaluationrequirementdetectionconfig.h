#ifndef EVALUATIONREQUIREMENTDETECTIONCONFIG_H
#define EVALUATIONREQUIREMENTDETECTIONCONFIG_H

#include "configurable.h"
#include "evaluationrequirementconfig.h"
#include "evaluationrequirementdetectionconfigwidget.h"

#include <QObject>

#include <memory>

class EvaluationRequirementGroup;
class EvaluationRequirementStandard;

class EvaluationRequirementDetectionConfig : EvaluationRequirementConfig
{
    Q_OBJECT

signals:

public slots:

public:
    EvaluationRequirementDetectionConfig(const std::string& class_id, const std::string& instance_id,
                                         EvaluationRequirementGroup& group, EvaluationRequirementStandard& standard);
    virtual ~EvaluationRequirementDetectionConfig();

    virtual void addGUIElements(QFormLayout* layout) override;
    EvaluationRequirementDetectionConfigWidget* widget() override;

protected:
    std::unique_ptr<EvaluationRequirementDetectionConfigWidget> widget_;
};

#endif // EVALUATIONREQUIREMENTDETECTIONCONFIG_H
