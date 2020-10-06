#include "eval/results/joined.h"

namespace EvaluationRequirementResult
{


Joined::Joined(const std::string& type, std::shared_ptr<EvaluationRequirement> requirement, EvaluationManager& eval_man)
    : Base(type, requirement, eval_man)
{
}

void Joined::join(std::shared_ptr<Base> other)
{
    results_.push_back(other);
}

}
