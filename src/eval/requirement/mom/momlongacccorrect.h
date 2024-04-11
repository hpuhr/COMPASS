#ifndef MOMLONGACCCORRECT_H
#define MOMLONGACCCORRECT_H

#include <requirement/generic/generic.h>

namespace EvaluationRequirement
{

class MomLongAccCorrect : public Generic
{
  public:
    MomLongAccCorrect(const std::string& name, const std::string& short_name, const std::string& group_name,
                      float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man);
};

}

#endif // MOMLONGACCCORRECT_H
