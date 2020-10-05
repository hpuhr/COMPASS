#ifndef EVALUATIONREQUIREMENTDETECTIONRESULT_H
#define EVALUATIONREQUIREMENTDETECTIONRESULT_H

#include "eval/results/single.h"
#include "evaluationrequirementdetection.h"

namespace EvaluationRequirementResult
{

class SingleDetection : public Single
{
public:
    SingleDetection(
            std::shared_ptr<EvaluationRequirement> requirement,
            unsigned int utn, const EvaluationTargetData* target, EvaluationManager& eval_man,
            float sum_uis, float missed_uis, float max_gap_uis, float no_ref_uis,
            std::vector<EvaluationRequirementDetectionDetail> details);

//    virtual void join(const std::shared_ptr<EvaluationRequirementResult> other_base) override;
//    // joins other result to this one

//    virtual std::shared_ptr<EvaluationRequirementResult> copy() override; // copies this instance

    virtual void print() override;
    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

    virtual std::shared_ptr<Joined> createEmptyJoined();

    float sumUIs() const;
    float missedUIs() const;
    float maxGapUIs() const;
    float noRefUIs() const;

protected:
    float sum_uis_ {0};
    float missed_uis_ {0};
    float max_gap_uis_ {0};
    float no_ref_uis_ {0};

    std::vector<EvaluationRequirementDetectionDetail> details_;

    bool has_pd_ {false};
    float pd_{0};

    void updatePD();
};

}

#endif // EVALUATIONREQUIREMENTDETECTIONRESULT_H
