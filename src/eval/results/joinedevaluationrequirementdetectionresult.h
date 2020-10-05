#ifndef JOINEDEVALUATIONREQUIREMENTDETECTIONRESULT_H
#define JOINEDEVALUATIONREQUIREMENTDETECTIONRESULT_H

#include "evaluationrequirementresult.h"
#include "evaluationrequirementdetectionresult.h"

class JoinedEvaluationRequirementDetectionResult : public EvaluationRequirementResult
{
public:
    JoinedEvaluationRequirementDetectionResult(std::shared_ptr<EvaluationRequirement> requirement,
                                               EvaluationManager& eval_man);

    void join(std::shared_ptr<EvaluationRequirementResult> other);

    virtual void print() override;
    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

protected:
    float sum_uis_ {0};
    float missed_uis_ {0};
    float max_gap_uis_ {0};
    float no_ref_uis_ {0};

    bool has_pd_ {false};
    float pd_{0};

    std::vector<std::shared_ptr<EvaluationRequirementDetectionResult>> results_;

    void updatePD();
};

#endif // JOINEDEVALUATIONREQUIREMENTDETECTIONRESULT_H
