#include "evaluationrequirement.h"

namespace EvaluationRequirement
{

Base::Base(const std::string& name, const std::string& short_name,
                                             const std::string& group_name, EvaluationManager& eval_man)
    : name_(name), short_name_(short_name), group_name_(group_name), eval_man_(eval_man)
{

}

std::string Base::name() const
{
    return name_;
}

std::string Base::shortname() const
{
    return short_name_;
}

std::string Base::groupName() const
{
    return group_name_;
}

}
