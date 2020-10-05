#include "eval/results/joined.h"

namespace EvaluationRequirementResult
{


Joined::Joined(std::shared_ptr<EvaluationRequirement> requirement, EvaluationManager& eval_man)
    : Base(requirement, eval_man)
{
}

void Joined::join(std::shared_ptr<Base> other)
{
    results_.push_back(other);
}

}
