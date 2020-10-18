#ifndef EVALUATIONREQUIREMENPOSITIONMAXDISTANCE_H
#define EVALUATIONREQUIREMENPOSITIONMAXDISTANCE_H

#include "eval/requirement/base.h"
#include "evaluationtargetposition.h"

#include <QVariant>

namespace EvaluationRequirement
{

    class PositionMaxDistanceDetail
    {
    public:
        PositionMaxDistanceDetail(
                float tod, EvaluationTargetPosition tst_pos,
                bool has_ref_pos, EvaluationTargetPosition ref_pos,
                QVariant pos_inside, QVariant distance, bool pos_ok,
                int num_pos, int num_no_ref, int num_outside, int num_pos_ok, int num_pos_nok)
            : tod_(tod), tst_pos_(tst_pos), has_ref_pos_(has_ref_pos), ref_pos_(ref_pos),
              distance_(distance), pos_ok_(pos_ok), pos_inside_(pos_inside),
              num_pos_(num_pos), num_no_ref_(num_no_ref),
              num_outside_(num_outside), num_pos_ok_(num_pos_ok), num_pos_nok_(num_pos_nok)
        {
        }

        float tod_ {0};

        EvaluationTargetPosition tst_pos_;

        bool has_ref_pos_ {false};
        EvaluationTargetPosition ref_pos_;

        QVariant distance_ {0}; // only set if has_ref_pos_
        bool pos_ok_ {false};

        QVariant pos_inside_ {false};

        int num_pos_ {0};
        int num_no_ref_ {0};
        int num_outside_ {0};
        int num_pos_ok_ {0};
        int num_pos_nok_ {0};
    };


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
                const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
                const SectorLayer& sector_layer) override;

    protected:
        float max_distance_ {0};
        float maximum_probability_{0};
    };

}

#endif // EVALUATIONREQUIREMENPOSITIONMAXDISTANCE_H
