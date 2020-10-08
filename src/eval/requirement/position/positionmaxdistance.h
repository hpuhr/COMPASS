#ifndef EVALUATIONREQUIREMENPOSITIONMAXDISTANCE_H
#define EVALUATIONREQUIREMENPOSITIONMAXDISTANCE_H

#include "eval/requirement/base.h"

namespace EvaluationRequirement
{

class PositionMaxDistance : public Base
{
public:
    PositionMaxDistance(
            const std::string& name, const std::string& short_name, const std::string& group_name,
            EvaluationManager& eval_man,
            float max_distance, float maximum_probability);

    float maxDistance() const;
    float maximumProbability() const;

    virtual std::shared_ptr<EvaluationRequirementResult::Single> evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<Base> instance) override;

protected:
    float max_distance_ {0};
    float maximum_probability_{0};
};

}

#endif // EVALUATIONREQUIREMENPOSITIONMAXDISTANCE_H
