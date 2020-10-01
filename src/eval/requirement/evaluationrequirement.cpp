#include "evaluationrequirement.h"

EvaluationRequirement::EvaluationRequirement(const std::string& name, const std::string& short_name,
                                             const std::string& group_name, EvaluationManager& eval_man)
    : name_(name), short_name_(short_name), group_name_(group_name), eval_man_(eval_man)
{

}

std::string EvaluationRequirement::name() const
{
    return name_;
}

std::string EvaluationRequirement::shortname() const
{
    return short_name_;
}

std::string EvaluationRequirement::groupName() const
{
    return group_name_;
}

