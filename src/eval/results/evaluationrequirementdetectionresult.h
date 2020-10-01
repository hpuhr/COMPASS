#ifndef EVALUATIONREQUIREMENTDETECTIONRESULT_H
#define EVALUATIONREQUIREMENTDETECTIONRESULT_H

#include "evaluationrequirementresult.h"

class EvaluationRequirementDetectionResult : public EvaluationRequirementResult
{
public:
    EvaluationRequirementDetectionResult(
            std::shared_ptr<EvaluationRequirement> requirement, std::vector<unsigned int> utns,
            std::vector<const EvaluationTargetData*> targets, EvaluationManager& eval_man,
            float sum_uis, float missed_uis, float max_gap_uis, float no_ref_uis);

    virtual void join(const std::shared_ptr<EvaluationRequirementResult> other_base) override;
    // joins other result to this one

    virtual std::shared_ptr<EvaluationRequirementResult> copy() override; // copies this instance

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

#endif // EVALUATIONREQUIREMENTDETECTIONRESULT_H
