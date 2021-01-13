#ifndef EVALUATIONREQUIREMENSPEEDDETAIL_H
#define EVALUATIONREQUIREMENSPEEDDETAIL_H

#include "evaluationtargetposition.h"

#include <QVariant>

namespace EvaluationRequirement
{
class SpeedDetail
{
public:
    SpeedDetail(
            float tod, EvaluationTargetPosition tst_pos,
            bool has_ref_pos, EvaluationTargetPosition ref_pos,
            QVariant pos_inside, QVariant offset, bool check_passed,
            unsigned int num_pos, unsigned int num_no_ref,
            unsigned int num_inside, unsigned int num_outside,
            unsigned int num_check_failed, unsigned int num_check_passed,
            const std::string& comment)
        : tod_(tod), tst_pos_(tst_pos), has_ref_pos_(has_ref_pos), ref_pos_(ref_pos),
          offset_(offset), check_passed_(check_passed),
          pos_inside_(pos_inside), num_pos_(num_pos), num_no_ref_(num_no_ref),
          num_inside_(num_inside), num_outside_(num_outside),
          num_check_failed_(num_check_failed), num_check_passed_(num_check_passed),
          comment_(comment)
    {
    }

    float tod_ {0};

    EvaluationTargetPosition tst_pos_;

    bool has_ref_pos_ {false};
    EvaluationTargetPosition ref_pos_;

    QVariant offset_ {0};
    bool check_passed_ {false};

    QVariant pos_inside_ {false};

    unsigned int num_pos_ {0};
    unsigned int num_no_ref_ {0};
    unsigned int num_inside_ {0};
    unsigned int num_outside_ {0};

    unsigned int num_check_failed_ {0};
    unsigned int num_check_passed_ {0};

    std::string comment_;
};
}

#endif // EVALUATIONREQUIREMENSPEEDDETAIL_H
