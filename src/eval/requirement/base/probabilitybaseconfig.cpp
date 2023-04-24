#include "probabilitybaseconfig.h"

namespace EvaluationRequirement {

ProbabilityBaseConfig::ProbabilityBaseConfig(
        const std::string& class_id, const std::string& instance_id,
        Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
    : BaseConfig(class_id, instance_id, group, standard, eval_man)
{
    registerParameter("prob", &prob_, 0.9);
    registerParameter("prob_check_type", (unsigned int*)&prob_check_type_,
                      (unsigned int)COMPARISON_TYPE::GREATER_THAN_OR_EUQAL);

}

float ProbabilityBaseConfig::prob() const
{
    return prob_;
}

void ProbabilityBaseConfig::prob(float value)
{
    prob_ = value;
}

COMPARISON_TYPE ProbabilityBaseConfig::probCheckType() const
{
    return prob_check_type_;
}

void ProbabilityBaseConfig::probCheckType(const COMPARISON_TYPE& prob_type)
{
    prob_check_type_ = prob_type;
}

} // namespace EvaluationRequirement
