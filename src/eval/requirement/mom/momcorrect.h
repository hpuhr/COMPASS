#pragma once

#include <requirement/generic/generic.h>

namespace EvaluationRequirement
{

class MomLongAccCorrect : public GenericInteger
{
  public:
    MomLongAccCorrect(const std::string& name, const std::string& short_name, const std::string& group_name,
                      float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man);
};

class MomTransAccCorrect : public GenericInteger
{
  public:
    MomTransAccCorrect(const std::string& name, const std::string& short_name, const std::string& group_name,
                      float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man);
};


class MomVertRateCorrect : public GenericInteger
{
  public:
    MomVertRateCorrect(const std::string& name, const std::string& short_name, const std::string& group_name,
                       float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man);
};


}


