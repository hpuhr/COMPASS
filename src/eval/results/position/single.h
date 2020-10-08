#ifndef EVALUATIONREQUIREMENPOSITIONMAXDISTANCERESULT_H
#define EVALUATIONREQUIREMENPOSITIONMAXDISTANCERESULT_H

#include "eval/results/single.h"
#include "eval/requirement/position/positionmaxdistance.h"

namespace EvaluationRequirementResult
{

class SinglePositionMaxDistance : public Single
{
public:
    SinglePositionMaxDistance(
            const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
            unsigned int utn, const EvaluationTargetData* target, EvaluationManager& eval_man,
            float numPos, float numPosOk, float numPosNOk);

    virtual void print() override;
    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

    virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) override;

    float numPos() const;
    float numPosOk() const;
    float numPosNOk() const;

protected:
    float num_pos_ {0};
    float num_pos_ok_ {0};
    float num_pos_nok_ {0};

    bool has_p_max_pos_ {false};
    float p_max_pos_{0};

    void updatePMaxPos();
};

}

#endif // EVALUATIONREQUIREMENPOSITIONMAXDISTANCERESULT_H
