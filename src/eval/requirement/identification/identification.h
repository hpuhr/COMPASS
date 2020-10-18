#ifndef EVALUATIONREQUIREMENTIDENTIFICATION_H
#define EVALUATIONREQUIREMENTIDENTIFICATION_H

#include "eval/requirement/base.h"
#include "evaluationtargetposition.h"

#include <QVariant>

namespace EvaluationRequirement
{

class IdentificationDetail
{
public:
    IdentificationDetail(
            float tod, EvaluationTargetPosition pos_tst,
            bool ref_exists, QVariant pos_inside,
            int num_updates, int num_no_ref, int num_inside, int num_outside,
            int num_unknown_id, int num_correct_id, int num_false_id,
            const std::string& comment)
        : tod_(tod), pos_tst_(pos_tst), ref_exists_(ref_exists), pos_inside_(pos_inside),
          num_updates_(num_updates), num_no_ref_(num_no_ref), num_inside_(num_inside), num_outside_(num_outside),
          num_unknown_id_(num_unknown_id), num_correct_id_(num_correct_id), num_false_id_(num_false_id),
          comment_(comment)
    {
    }

    float tod_ {0};

    EvaluationTargetPosition pos_tst_;

    bool ref_exists_ {false};
    QVariant pos_inside_ {false};

    int num_updates_ {0};
    int num_no_ref_ {0};
    int num_inside_ {0};
    int num_outside_ {0};
    int num_unknown_id_ {0};
    int num_correct_id_ {0};
    int num_false_id_ {0};

    std::string comment_;
};

class Identification : public Base
{
public:
    Identification(
            const std::string& name, const std::string& short_name, const std::string& group_name,
            EvaluationManager& eval_man, float max_time_diff, float minimum_probability);

    float maxRefTimeDiff() const;
    float minimumProbability() const;

    virtual std::shared_ptr<EvaluationRequirementResult::Single> evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
            const SectorLayer& sector_layer) override;

protected:
    float max_ref_time_diff_ {0};
    float minimum_probability_{0};
};

}
#endif // EVALUATIONREQUIREMENTDETECTION_H
