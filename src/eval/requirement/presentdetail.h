#ifndef PRESENTDETAIL_H
#define PRESENTDETAIL_H

#include "evaluationtargetposition.h"

#include <QVariant>

namespace EvaluationRequirement
{
    class PresentDetail
    {
    public:
        PresentDetail(
                float tod, EvaluationTargetPosition pos_tst,
                bool ref_exists, QVariant pos_inside, bool is_not_ok,
                int num_updates, int num_no_ref, int num_inside, int num_outside,
                int num_no_ref_id, int num_present_id, int num_missing_id,
                const std::string& comment)
            : tod_(tod), pos_tst_(pos_tst), ref_exists_(ref_exists), pos_inside_(pos_inside), is_not_ok_(is_not_ok),
              num_updates_(num_updates), num_no_ref_(num_no_ref), num_inside_(num_inside), num_outside_(num_outside),
              num_no_ref_id_(num_no_ref_id), num_present_id_(num_present_id), num_missing_id_(num_missing_id),
              comment_(comment)
        {
        }

        float tod_ {0};

        EvaluationTargetPosition pos_tst_;

        bool ref_exists_ {false};
        QVariant pos_inside_ {false};

        bool is_not_ok_ {false}; // missing

        int num_updates_ {0};
        int num_no_ref_ {0}; // no ref pos
        int num_inside_ {0};
        int num_outside_ {0};
        int num_no_ref_id_ {0}; // !ref
        int num_present_id_ {0}; // ref + tst
        int num_missing_id_ {0}; // ref + !tst


        std::string comment_;
    };
}

#endif // PRESENTDETAIL_H
