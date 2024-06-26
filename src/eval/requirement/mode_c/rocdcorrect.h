#pragma once

#include <requirement/generic/generic.h>


namespace EvaluationRequirement {

class ROCDCorrect : public GenericDouble
{
  public:
    ROCDCorrect(const std::string& name, const std::string& short_name, const std::string& group_name,
                float prob, COMPARISON_TYPE prob_check_type, double threshold, EvaluationManager& eval_man);

    std::string probabilityNameShort() const override final { return "PCROCD"; }
    std::string probabilityName() const override final { return "Probability of Correct ROCD"; }
};

} // namespace EvaluationRequirement
