#ifndef EVALUATIONREQUIREMENTDETECTION_H
#define EVALUATIONREQUIREMENTDETECTION_H

#include "evaluationrequirement.h"

class EvaluationRequirementDetection : public EvaluationRequirement
{
public:
    EvaluationRequirementDetection(const std::string& name, const std::string& short_name,
                                   const std::string& group_name, float update_interval_s,
                                   float minimum_probability, bool use_max_gap_interval,
                                   float max_gap_interval_s, bool use_miss_tolerance,
                                   float miss_tolerance_s);

    float updateInterval() const;

    float minimumProbability() const;

    bool useMaxGapInterval() const;

    float maxGapInterval() const;

    bool useMissTolerance() const;

    float missTolerance() const;

    virtual void evaluate (EvaluationTargetData& target_data) override;

protected:
    float update_interval_s_{0};

    float minimum_probability_{0};

    bool use_max_gap_interval_{true};
    float max_gap_interval_s_{0};

    bool use_miss_tolerance_{false};
    float miss_tolerance_s_{0};
};

#endif // EVALUATIONREQUIREMENTDETECTION_H
