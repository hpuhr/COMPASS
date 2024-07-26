#pragma once

#include <requirement/generic/generic.h>

namespace EvaluationRequirement
{

class CoastingCorrect : public GenericInteger
{
  public:
    CoastingCorrect(const std::string& name, const std::string& short_name, const std::string& group_name,
                      double prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man);

    std::string probabilityNameShort() const override final { return "PCCoast"; }
    std::string probabilityName() const override final { return "Probability of Correct Track Coasting"; }
};

}
