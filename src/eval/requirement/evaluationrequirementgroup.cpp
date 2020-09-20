#include "evaluationrequirementgroup.h"
#include "evaluationstandard.h"

EvaluationRequirementGroup::EvaluationRequirementGroup(const std::string& class_id, const std::string& instance_id,
                                                       EvaluationStandard& standard)
    : Configurable(class_id, instance_id, &standard), standard_(standard)
{
    registerParameter("name_", &name_, "");

    assert (name_.size());

    createSubConfigurables();
}

EvaluationRequirementGroup::~EvaluationRequirementGroup()
{

}


void EvaluationRequirementGroup::generateSubConfigurable(const std::string& class_id,
                                                         const std::string& instance_id)
{

}


void EvaluationRequirementGroup::checkSubConfigurables()
{

}
