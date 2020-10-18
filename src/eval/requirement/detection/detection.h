#ifndef EVALUATIONREQUIREMENTDETECTION_H
#define EVALUATIONREQUIREMENTDETECTION_H

#include "eval/requirement/base.h"
#include "evaluationtargetposition.h"

namespace EvaluationRequirement
{

class DetectionDetail
{
public:
    DetectionDetail(
            float tod, bool has_d_tod, float d_tod,
            bool miss_occurred, EvaluationTargetPosition pos_current,
            bool ref_exists, float missed_uis, float max_gap_uis,
            float no_ref_uis, const std::string& comment)
        : tod_(tod), has_d_tod_(has_d_tod), d_tod_(d_tod), miss_occurred_(miss_occurred), pos_current_(pos_current),
          ref_exists_(ref_exists), missed_uis_(missed_uis), max_gap_uis_(max_gap_uis),
          no_ref_uis_(no_ref_uis), comment_(comment)
    {
    }

    float tod_ {0};
    bool has_d_tod_ {false};
    float d_tod_ {0};
    bool miss_occurred_ {false};

    EvaluationTargetPosition pos_current_;

    bool ref_exists_ {false};

    float missed_uis_ {0};
    float max_gap_uis_ {0};
    float no_ref_uis_ {0};

    std::string comment_;

    bool has_last_position_ {false};
    EvaluationTargetPosition pos_last;
};

class Detection : public Base
{
public:
    Detection(
            const std::string& name, const std::string& short_name, const std::string& group_name,
            EvaluationManager& eval_man,
            float update_interval_s, float max_ref_time_diff, float minimum_probability, bool use_max_gap_interval,
            float max_gap_interval_s, bool use_miss_tolerance, float miss_tolerance_s);

    float updateInterval() const;

    float maxRefTimeDiff() const;

    float minimumProbability() const;

    bool useMaxGapInterval() const;

    float maxGapInterval() const;

    bool useMissTolerance() const;

    float missTolerance() const;

    virtual std::shared_ptr<EvaluationRequirementResult::Single> evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
            const SectorLayer& sector_layer) override;

protected:
    float update_interval_s_{0};

    float max_ref_time_diff_ {0};

    float minimum_probability_{0};

    bool use_max_gap_interval_{true};
    float max_gap_interval_s_{0};

    bool use_miss_tolerance_{false};
    float miss_tolerance_s_{0};

    bool isMiss (float d_tod);
    bool isMaxGap (float d_tod);
};

}
#endif // EVALUATIONREQUIREMENTDETECTION_H
