#include "eval/requirement/position/positionmaxdistance.h"

namespace EvaluationRequirement
{

PositionMaxDistance::PositionMaxDistance(const std::string& name, const std::string& short_name, const std::string& group_name,
                                         EvaluationManager& eval_man,
                                         float max_distance, float maximum_probability)
    : Base(name, short_name, group_name, eval_man),
      max_distance_(max_distance), maximum_probability_(maximum_probability)
{

}

float PositionMaxDistance::maxDistance() const
{
    return max_distance_;
}


float PositionMaxDistance::maximumProbability() const
{
    return maximum_probability_;
}

std::shared_ptr<EvaluationRequirementResult::Single> PositionMaxDistance::evaluate (
        const EvaluationTargetData& target_data, std::shared_ptr<Base> instance)
{
    // TODO
}


}
