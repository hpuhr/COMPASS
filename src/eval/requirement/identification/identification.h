#ifndef EVALUATIONREQUIREMENTIDENTIFICATION_H
#define EVALUATIONREQUIREMENTIDENTIFICATION_H

#include "eval/requirement/base.h"
#include "evaluationtargetposition.h"

namespace EvaluationRequirement
{

//class DetectionDetail
//{
//public:
//    DetectionDetail(
//            float tod, bool has_d_tod, float d_tod,
//            bool miss_occurred, EvaluationTargetPosition pos_current,
//            bool ref_exists, float missed_uis, float max_gap_uis,
//            float no_ref_uis, const std::string& comment)
//        : tod_(tod), has_d_tod_(has_d_tod), d_tod_(d_tod), miss_occurred_(miss_occurred), pos_current_(pos_current),
//          ref_exists_(ref_exists), missed_uis_(missed_uis), max_gap_uis_(max_gap_uis),
//          no_ref_uis_(no_ref_uis), comment_(comment)
//    {
//    }

//    float tod_ {0};
//    bool has_d_tod_ {false};
//    float d_tod_ {0};
//    bool miss_occurred_ {false};

//    EvaluationTargetPosition pos_current_;

//    bool ref_exists_ {false};

//    float missed_uis_ {0};
//    float max_gap_uis_ {0};
//    float no_ref_uis_ {0};

//    std::string comment_;

//    bool has_last_position_ {false};
//    EvaluationTargetPosition pos_last;
//};

class Identification : public Base
{
public:
    Identification(
            const std::string& name, const std::string& short_name, const std::string& group_name,
            EvaluationManager& eval_man, float minimum_probability);

    float minimumProbability() const;

    virtual std::shared_ptr<EvaluationRequirementResult::Single> evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<Base> instance) override;

protected:
    float minimum_probability_{0};
};

}
#endif // EVALUATIONREQUIREMENTDETECTION_H
