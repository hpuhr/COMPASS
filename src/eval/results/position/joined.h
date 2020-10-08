#ifndef JOINEVALUATIONREQUIREMENPOSITIONMAXDISTANCERESULT_H
#define JOINEVALUATIONREQUIREMENPOSITIONMAXDISTANCERESULT_H

#include "eval/results/joined.h"

namespace EvaluationRequirementResult
{

class JoinedPositionMaxDistance : public Joined
{
public:
    JoinedPositionMaxDistance(const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
                    EvaluationManager& eval_man);

    virtual void join(std::shared_ptr<Base> other) override;

    virtual void print() override;
    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

protected:
    float num_pos_ {0};
    float num_pos_ok_ {0};
    float num_pos_nok_ {0};

    bool has_p_max_pos_ {false};
    float p_max_pos_{0};

    void updatePMaxPos();
};

}

#endif // JOINEVALUATIONREQUIREMENPOSITIONMAXDISTANCERESULT_H
