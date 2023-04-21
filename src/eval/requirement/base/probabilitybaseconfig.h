#ifndef EVALUATIONREQUIREMENT_PROBABILITYBASECONFIG_H
#define EVALUATIONREQUIREMENT_PROBABILITYBASECONFIG_H

#include "eval/requirement/base/baseconfig.h"

namespace EvaluationRequirement {

class ProbabilityBaseConfig : public BaseConfig
{
public:
    ProbabilityBaseConfig(const std::string& class_id, const std::string& instance_id,
                          Group& group, EvaluationStandard& standard,
                          EvaluationManager& eval_man);
    virtual ~ProbabilityBaseConfig() {}

    float prob() const;
    void prob(float value);

    COMPARISON_TYPE probCheckType() const;
    void probCheckType(const COMPARISON_TYPE& prob_type);

protected:
    float prob_ {0};
    COMPARISON_TYPE prob_check_type_ {COMPARISON_TYPE::GREATER_THAN_OR_EUQAL};

};

} // namespace EvaluationRequirement

#endif // EVALUATIONREQUIREMENT_PROBABILITYBASECONFIG_H
