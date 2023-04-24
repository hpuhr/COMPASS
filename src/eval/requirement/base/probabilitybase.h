#ifndef EVALUATIONREQUIREMENT_PROBABILITYBASE_H
#define EVALUATIONREQUIREMENT_PROBABILITYBASE_H

#include "eval/requirement/base/comparisontype.h"
#include "eval/requirement/base/base.h"

#include <string>
#include <memory>

class EvaluationManager;

namespace EvaluationRequirement {

class ProbabilityBase : public Base
{
public:
    ProbabilityBase(const std::string& name, const std::string& short_name, const std::string& group_name,
                    float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man);
    virtual ~ProbabilityBase();

    virtual std::shared_ptr<EvaluationRequirementResult::Single> evaluate (
            const EvaluationTargetData& target_data,
            std::shared_ptr<Base> instance,
            const SectorLayer& sector_layer) override = 0 ;
    // instance is the self-reference for the result (we want to pass the shared pointer to the result)

    float prob() const;
    unsigned int getNumProbDecimals() const;

    COMPARISON_TYPE probCheckType() const;

    std::string getConditionStr () const override;
    bool getConditionResult (float prob) const; //  true if passed
    std::string getConditionResultStr (float prob) const;

protected:
    float prob_ {0};
    COMPARISON_TYPE prob_check_type_ {COMPARISON_TYPE::GREATER_THAN_OR_EUQAL};


    bool compareValue (double val, double threshold, COMPARISON_TYPE check_type);
};

} // namespace EvaluationRequirement

#endif // EVALUATIONREQUIREMENT_PROBABILITYBASE_H
