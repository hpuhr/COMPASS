#ifndef SINGLEEVALUATIONREQUIREMENTDETECTIONRESULT_H
#define SINGLEEVALUATIONREQUIREMENTDETECTIONRESULT_H

#include "eval/results/base.h"

namespace EvaluationRequirementResult
{
    class Joined;

    class Single : public Base
    {
    public:
        Single(const std::string& type, const std::string& result_id,
               std::shared_ptr<EvaluationRequirement::Base> requirement,
               unsigned int utn, const EvaluationTargetData* target, EvaluationManager& eval_man);

        virtual void print() = 0;
        virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) = 0;

        virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) = 0;

        unsigned int utn() const;
        const EvaluationTargetData* target() const;

    protected:
        unsigned int utn_; // used to generate result
        const EvaluationTargetData* target_; // used to generate result
    };

}

#endif // SINGLEEVALUATIONREQUIREMENTDETECTIONRESULT_H
