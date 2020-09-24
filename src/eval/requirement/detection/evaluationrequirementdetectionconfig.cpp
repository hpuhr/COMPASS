#include "evaluationrequirementdetectionconfig.h"
#include "evaluationrequirementdetectionconfigwidget.h"

EvaluationRequirementDetectionConfig::EvaluationRequirementDetectionConfig(
        const std::string& class_id, const std::string& instance_id,
        EvaluationRequirementGroup& group, EvaluationRequirementStandard& standard)
    : EvaluationRequirementConfig(class_id, instance_id, group, standard)
{

}

EvaluationRequirementDetectionConfig::~EvaluationRequirementDetectionConfig()
{

}

void EvaluationRequirementDetectionConfig::addGUIElements(QFormLayout* layout)
{
    assert (layout);

    EvaluationRequirementConfig::addGUIElements(layout);
}

EvaluationRequirementDetectionConfigWidget* EvaluationRequirementDetectionConfig::widget()
{
    if (!widget_)
        widget_.reset(new EvaluationRequirementDetectionConfigWidget(*this));

    return widget_.get();
}
