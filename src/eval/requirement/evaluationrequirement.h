#ifndef EVALUATIONREQUIREMENT_H
#define EVALUATIONREQUIREMENT_H

#include <string>
#include <memory>

class EvaluationTargetData;
class EvaluationRequirementResult;
class EvaluationManager;

class EvaluationRequirement
{
public:
    EvaluationRequirement(const std::string& name, const std::string& short_name, const std::string& group_name,
                          EvaluationManager& eval_man);

    virtual std::shared_ptr<EvaluationRequirementResult> evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<EvaluationRequirement> instance) = 0;
    // instance is the self-reference for the result

    std::string name() const;
    std::string shortname() const;
    std::string groupName() const;

protected:
    std::string name_;
    std::string short_name_;
    std::string group_name_;

    EvaluationManager& eval_man_;
};

#endif // EVALUATIONREQUIREMENT_H
