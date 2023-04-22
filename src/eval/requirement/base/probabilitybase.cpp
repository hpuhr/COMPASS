#include "probabilitybase.h"
#include "stringconv.h"

using namespace std;
using namespace Utils;

namespace EvaluationRequirement {

ProbabilityBase::ProbabilityBase(const std::string& name, const std::string& short_name, const std::string& group_name,
                                 float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man)
    : Base(name, short_name, group_name, eval_man), prob_(prob), prob_check_type_(prob_check_type)
{

}

ProbabilityBase::~ProbabilityBase()
{

}

float ProbabilityBase::prob() const
{
    return prob_;
}

unsigned int ProbabilityBase::getNumProbDecimals() const
{
    assert (prob_ <= 1);

    float tmp=1;
    unsigned int decimals=1;

    while (tmp > prob_ && decimals < 6)
    {
        tmp /= 10.0;
        ++decimals;
    }

    //loginf << "Requirement::Base: getNumProbDecimals: prob " << prob_ << " dec " << decimals;
    return decimals;
}

COMPARISON_TYPE ProbabilityBase::probCheckType() const
{
    return prob_check_type_;
}

std::string ProbabilityBase::getConditionStr () const
{
    if (prob_check_type_ == COMPARISON_TYPE::LESS_THAN)
        return "< "+String::percentToString(prob_ * 100.0, getNumProbDecimals());
    else if (prob_check_type_ == COMPARISON_TYPE::LESS_THAN_OR_EQUAL)
        return "<= "+String::percentToString(prob_ * 100.0, getNumProbDecimals());
    else if (prob_check_type_ == COMPARISON_TYPE::GREATER_THAN)
        return "> "+String::percentToString(prob_ * 100.0, getNumProbDecimals());
    else if (prob_check_type_ == COMPARISON_TYPE::GREATER_THAN_OR_EUQAL)
        return ">= "+String::percentToString(prob_ * 100.0, getNumProbDecimals());
    else
        throw std::runtime_error("ProbabilityBase: getConditionStr: unknown type '"
                                 +to_string(prob_check_type_)+"'");
}

std::string ProbabilityBase::getResultConditionStr (float prob) const
{
    bool result;

    if (prob_check_type_ == COMPARISON_TYPE::LESS_THAN)
        result = prob < prob_;
    else if (prob_check_type_ == COMPARISON_TYPE::LESS_THAN_OR_EQUAL)
        result = prob <= prob_;
    else if (prob_check_type_ == COMPARISON_TYPE::GREATER_THAN)
        result = prob > prob_;
    else if (prob_check_type_ == COMPARISON_TYPE::GREATER_THAN_OR_EUQAL)
        result = prob >= prob_;
    else
        throw std::runtime_error("ProbabilityBase: getResultConditionStr: unknown type '"
                                 +to_string(prob_check_type_)+"'");

    return result ? "Passed" : "Failed";
}

bool ProbabilityBase::compareValue (double val, double threshold, COMPARISON_TYPE check_type)
{
    if (check_type == COMPARISON_TYPE::LESS_THAN)
        return val < threshold;
    else if (check_type == COMPARISON_TYPE::LESS_THAN_OR_EQUAL)
        return val <= threshold;
    else if (check_type == COMPARISON_TYPE::GREATER_THAN)
        return val > threshold;
    else if (check_type == COMPARISON_TYPE::GREATER_THAN_OR_EUQAL)
        return val >= threshold;
    else
        throw std::runtime_error("ProbabilityBase: compareValue: unknown type '"
                                 +to_string(check_type)+"'");
}

} // namespace EvaluationRequirement
