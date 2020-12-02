#ifndef EVALUATIONREQUIREMENPOSITIONDETAIL_H
#define EVALUATIONREQUIREMENPOSITIONDETAIL_H

#include "evaluationtargetposition.h"

#include <QVariant>

namespace EvaluationRequirement
{
class PositionDetail
{
public:
    PositionDetail(
            float tod, EvaluationTargetPosition tst_pos,
            bool has_ref_pos, EvaluationTargetPosition ref_pos,
            QVariant pos_inside, QVariant value, bool value_ok,
            unsigned int num_pos, unsigned int num_no_ref,
            unsigned int num_inside, unsigned int num_outside,
            unsigned int num_value_ok, unsigned int num_value_nok,
            const std::string& comment)
        : tod_(tod), tst_pos_(tst_pos), has_ref_pos_(has_ref_pos), ref_pos_(ref_pos),
          value_(value), value_ok_(value_ok),
          pos_inside_(pos_inside), num_pos_(num_pos), num_no_ref_(num_no_ref),
          num_inside_(num_inside), num_outside_(num_outside),
          num_value_ok_(num_value_ok), num_value_nok_(num_value_nok),
          comment_(comment)
    {
    }

    float tod_ {0};

    EvaluationTargetPosition tst_pos_;

    bool has_ref_pos_ {false};
    EvaluationTargetPosition ref_pos_;

    QVariant value_ {0};
    bool value_ok_ {false};

    QVariant pos_inside_ {false};

    unsigned int num_pos_ {0};
    unsigned int num_no_ref_ {0};
    unsigned int num_inside_ {0};
    unsigned int num_outside_ {0};

    unsigned int num_value_ok_ {0};
    unsigned int num_value_nok_ {0};

    std::string comment_;
};
}

#endif // EVALUATIONREQUIREMENPOSITIONDETAIL_H
