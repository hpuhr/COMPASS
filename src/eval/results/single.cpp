#include "eval/results/single.h"

namespace EvaluationRequirementResult
{

Single::Single(
        const std::string& type, std::shared_ptr<EvaluationRequirement> requirement,
        unsigned int utn, const EvaluationTargetData* target,
        EvaluationManager& eval_man)
    : Base(type, requirement, eval_man), utn_(utn), target_(target)
{
}

unsigned int Single::utn() const
{
    return utn_;
}

const EvaluationTargetData* Single::target() const
{
    return target_;
}

}
