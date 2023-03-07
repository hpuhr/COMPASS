#ifndef EVALUATIONREQUIREMENTRACKANGLEDETAIL_H
#define EVALUATIONREQUIREMENTRACKANGLEDETAIL_H

#include "evaluationtargetposition.h"

#include <QVariant>

#include "boost/date_time/posix_time/ptime.hpp"

namespace EvaluationRequirement
{
class TrackAngleDetail
{
public:
    TrackAngleDetail(
            boost::posix_time::ptime timestmap, EvaluationTargetPosition tst_pos,
            bool has_ref_pos, EvaluationTargetPosition ref_pos,
            QVariant pos_inside, QVariant offset, bool check_passed,
            QVariant value_ref, QVariant value_tst, QVariant speed_ref,
            unsigned int num_pos, unsigned int num_no_ref,
            unsigned int num_inside, unsigned int num_outside,
            unsigned int num_check_failed, unsigned int num_check_passed,
            const std::string& comment)
        : timestamp_(timestmap), tst_pos_(tst_pos), has_ref_pos_(has_ref_pos), ref_pos_(ref_pos),
          offset_(offset), check_passed_(check_passed),
          value_ref_(value_ref), value_tst_(value_tst), speed_ref_(speed_ref),
          pos_inside_(pos_inside), num_pos_(num_pos), num_no_ref_(num_no_ref),
          num_inside_(num_inside), num_outside_(num_outside),
          num_check_failed_(num_check_failed), num_check_passed_(num_check_passed),
          comment_(comment)
    {
    }

    boost::posix_time::ptime timestamp_;

    EvaluationTargetPosition tst_pos_;

    bool has_ref_pos_ {false};
    EvaluationTargetPosition ref_pos_;

    QVariant offset_ {0};
    bool check_passed_ {false};

    QVariant value_ref_;
    QVariant value_tst_;
    QVariant speed_ref_;

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

#endif // EVALUATIONREQUIREMENTRACKANGLEDETAIL_H
