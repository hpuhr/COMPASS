#include "evaluationrequirementconfig.h"
#include "evaluationrequirementgroup.h"

EvaluationRequirementConfig::EvaluationRequirementConfig(const std::string& class_id, const std::string& instance_id,
                                                         EvaluationRequirementGroup& group)
    : Configurable(class_id, instance_id, &group), group_(group)
{
    registerParameter("name", &name_, "");

    assert (name_.size());

    createSubConfigurables();
}

EvaluationRequirementConfig::~EvaluationRequirementConfig()
{

}


void EvaluationRequirementConfig::generateSubConfigurable(const std::string& class_id,
                                                          const std::string& instance_id)
{

}

std::string EvaluationRequirementConfig::name() const
{
    return name_;
}

void EvaluationRequirementConfig::checkSubConfigurables()
{

}
