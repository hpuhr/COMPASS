#include "evaluationrequirementdetection.h"
#include "evaluationdata.h"

using namespace std;


EvaluationRequirementDetection::EvaluationRequirementDetection(const std::string& name, const std::string& short_name,
                                                               const std::string& group_name, float update_interval_s,
                                                               float minimum_probability, bool use_max_gap_interval,
                                                               float max_gap_interval_s, bool use_miss_tolerance,
                                                               float miss_tolerance_s)
    : EvaluationRequirement(name, short_name, group_name), update_interval_s_(update_interval_s),
      minimum_probability_(minimum_probability), use_max_gap_interval_(use_max_gap_interval),
      max_gap_interval_s_(max_gap_interval_s), use_miss_tolerance_(use_miss_tolerance),
      miss_tolerance_s_(miss_tolerance_s)
{

}

float EvaluationRequirementDetection::updateInterval() const
{
    return update_interval_s_;
}

float EvaluationRequirementDetection::minimumProbability() const
{
    return minimum_probability_;
}

bool EvaluationRequirementDetection::useMaxGapInterval() const
{
    return use_max_gap_interval_;
}

float EvaluationRequirementDetection::maxGapInterval() const
{
    return max_gap_interval_s_;
}

bool EvaluationRequirementDetection::useMissTolerance() const
{
    return use_miss_tolerance_;
}

float EvaluationRequirementDetection::missTolerance() const
{
    return miss_tolerance_s_;
}

void EvaluationRequirementDetection::evaluate (EvaluationTargetData& target_data)
{

}
