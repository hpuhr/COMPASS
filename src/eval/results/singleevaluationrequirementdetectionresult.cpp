#include "singleevaluationrequirementdetectionresult.h"

namespace EvaluationRequirementResult
{

SingleEvaluationRequirementDetectionResult::SingleEvaluationRequirementDetectionResult(
        std::shared_ptr<EvaluationRequirement> requirement,
        unsigned int utn, const EvaluationTargetData* target,
        EvaluationManager& eval_man)
    : Base(requirement, eval_man), utn_(utn), target_(target)
{
}

unsigned int SingleEvaluationRequirementDetectionResult::utn() const
{
    return utn_;
}

const EvaluationTargetData* SingleEvaluationRequirementDetectionResult::target() const
{
    return target_;
}

}
