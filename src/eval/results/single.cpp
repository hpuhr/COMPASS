#include "eval/results/single.h"
#include "eval/requirement/base.h"
#include "evaluationtargetdata.h"
#include "sectorlayer.h"

namespace EvaluationRequirementResult
{

Single::Single(
        const std::string& type, const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
        const SectorLayer& sector_layer, unsigned int utn, const EvaluationTargetData* target,
        EvaluationManager& eval_man)
    : Base(type, result_id, requirement, sector_layer, eval_man), utn_(utn), target_(target)
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

std::string Single::getTargetSectionID ()
{
    return "Targets:"+to_string(utn_)+":"+sector_layer_.name()+":"+requirement_->groupName()+":"+requirement_->name();
}

}
