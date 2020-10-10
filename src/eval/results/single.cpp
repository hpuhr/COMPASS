#include "eval/results/single.h"
#include "evaluationtargetdata.h"

namespace EvaluationRequirementResult
{

Single::Single(
        const std::string& type, const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
        unsigned int utn, const EvaluationTargetData* target,
        EvaluationManager& eval_man)
    : Base(type, result_id, requirement, eval_man), utn_(utn), target_(target)
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

void Single::updateUseFromTarget ()
{
    use_ = result_usable_ && target_->use();
}

}
