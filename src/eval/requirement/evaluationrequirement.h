#ifndef EVALUATIONREQUIREMENT_H
#define EVALUATIONREQUIREMENT_H

#include <string>

class EvaluationTargetData;

class EvaluationRequirement
{
public:
    EvaluationRequirement(const std::string& name, const std::string& short_name, const std::string& group_name);

    virtual void evaluate (const EvaluationTargetData& target_data) = 0;

    std::string name() const;
    std::string shortname() const;
    std::string groupName() const;

protected:
    std::string name_;
    std::string short_name_;
    std::string group_name_;
};

#endif // EVALUATIONREQUIREMENT_H
