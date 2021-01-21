#ifndef EVALUATIONREQUIREMENTCORRECTNESSDETAIL_H
#define EVALUATIONREQUIREMENTCORRECTNESSDETAIL_H

#include "evaluationtargetposition.h"

#include <QVariant>

namespace EvaluationRequirement
{
    class CorrectnessDetail
    {
    public:
        CorrectnessDetail(
                float tod, EvaluationTargetPosition pos_tst,
                bool ref_exists, QVariant pos_inside, bool is_not_correct,
                unsigned int num_updates, unsigned int num_no_ref, unsigned int num_inside, unsigned int num_outside,
                unsigned int num_correct, unsigned int num_not_correct,
                const std::string& comment)
            : tod_(tod), pos_tst_(pos_tst), ref_exists_(ref_exists), pos_inside_(pos_inside),
              is_not_correct_(is_not_correct),
              num_updates_(num_updates), num_no_ref_(num_no_ref),
              num_inside_(num_inside), num_outside_(num_outside),
              num_correct_(num_correct), num_not_correct_(num_not_correct),
              comment_(comment)
        {
        }

        float tod_ {0};

        EvaluationTargetPosition pos_tst_;

        bool ref_exists_ {false};
        QVariant pos_inside_ {false};

        bool is_not_correct_ {false};

        unsigned int num_updates_ {0};
        unsigned int num_no_ref_ {0};
        unsigned int num_inside_ {0};
        unsigned int num_outside_ {0};
        unsigned int num_correct_ {0};
        unsigned int num_not_correct_ {0};

        std::string comment_;
    };
}

#endif // EVALUATIONREQUIREMENTCORRECTNESSDETAIL_H
