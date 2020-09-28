#ifndef EVALUATIONREQUIREMENTRESULT_H
#define EVALUATIONREQUIREMENTRESULT_H

#include <memory>
#include <vector>

class EvaluationRequirement;

class EvaluationRequirementResult
{
public:
    EvaluationRequirementResult(std::shared_ptr<EvaluationRequirement> requirement, std::vector<unsigned int> utns);

    std::shared_ptr<EvaluationRequirement> requirement() const;

    virtual void join(const std::shared_ptr<EvaluationRequirementResult> other_base); // joins other result to this one

    virtual std::shared_ptr<EvaluationRequirementResult> copy() = 0; // copies this instance

    virtual void print() = 0;

    std::vector<unsigned int> utns() const;
    std::string utnsString() const;

protected:
    std::shared_ptr<EvaluationRequirement> requirement_;

    std::vector<unsigned int> utns_; // utns used to generate result
};

#endif // EVALUATIONREQUIREMENTRESULT_H
