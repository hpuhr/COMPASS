#ifndef JOINEDEVALUATIONREQUIREMENTDETECTIONRESULT_H
#define JOINEDEVALUATIONREQUIREMENTDETECTIONRESULT_H

#include "eval/results/joined.h"

namespace EvaluationRequirementResult
{
    class SingleDetection;

    class JoinedDetection : public Joined
    {
    public:
        JoinedDetection(const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
                        EvaluationManager& eval_man);

        virtual void join(std::shared_ptr<Base> other) override;

        virtual void print() override;
        virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

    protected:
        float sum_uis_ {0};
        float missed_uis_ {0};
        float max_gap_uis_ {0};
        float no_ref_uis_ {0};

        bool has_pd_ {false};
        float pd_{0};

        void updatePD();
    };

}

#endif // JOINEDEVALUATIONREQUIREMENTDETECTIONRESULT_H
