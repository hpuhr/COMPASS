/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "eval/requirement/base/base.h"
#include "evaluationdata.h"
#include "stringconv.h"

using namespace std;
using namespace Utils;

namespace EvaluationRequirement
{

bool Base::in_appimage_ {getenv("APPDIR") != nullptr};

Base::Base(const std::string& name, const std::string& short_name, const std::string& group_name,
           float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man)
    : name_(name), short_name_(short_name), group_name_(group_name),
      prob_(prob), prob_check_type_(prob_check_type), eval_man_(eval_man)
{

}

Base::~Base()
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

float Base::prob() const
{
    return prob_;
}

COMPARISON_TYPE Base::probCheckType() const
{
    return prob_check_type_;
}

std::string Base::getConditionStr () const
{
    if (prob_check_type_ == COMPARISON_TYPE::LESS_THAN)
        return "< "+String::percentToString(prob_ * 100.0);
    else if (prob_check_type_ == COMPARISON_TYPE::LESS_THAN_OR_EQUAL)
        return "<= "+String::percentToString(prob_ * 100.0);
    else if (prob_check_type_ == COMPARISON_TYPE::GREATER_THAN)
        return "> "+String::percentToString(prob_ * 100.0);
    else if (prob_check_type_ == COMPARISON_TYPE::GREATER_THAN_OR_EUQAL)
        return ">= "+String::percentToString(prob_ * 100.0);
    else
        throw std::runtime_error("EvaluationRequiretBase: getConditionStr: unknown type '"
                                 +to_string(prob_check_type_)+"'");
}

std::string Base::getResultConditionStr (float prob) const
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
        throw std::runtime_error("EvaluationRequiretBase: getResultConditionStr: unknown type '"
                                 +to_string(prob_check_type_)+"'");

    return result ? "Passed" : "Failed";;
}

bool Base::compareValue (double val, double threshold, COMPARISON_TYPE check_type)
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
        throw std::runtime_error("EvaluationRequiretBase: compareValue: unknown type '"
                                 +to_string(check_type)+"'");
}

std::pair<ValueComparisonResult, std::string> Base::compareTi (
        float tod, const EvaluationTargetData& target_data, float max_ref_time_diff)
{
    if (target_data.hasTstCallsignForTime(tod))
    {
        string callsign = target_data.tstCallsignForTime(tod);

        float ref_lower{0}, ref_upper{0};
        tie(ref_lower, ref_upper) = target_data.refTimesFor(tod, max_ref_time_diff);

        bool ref_exists, callsign_ok;
        bool lower_nok, upper_nok;

        if ((ref_lower != -1 || ref_upper != -1)) // ref times possible
        {
            if ((ref_lower != -1 && target_data.hasRefCallsignForTime(ref_lower))
                    || (ref_upper != -1 && target_data.hasRefCallsignForTime(ref_upper))) // ref value(s) exist
            {
                ref_exists = true;
                callsign_ok = false;

                lower_nok = false;
                upper_nok = false;

                if (ref_lower != -1 && target_data.hasRefCallsignForTime(ref_lower))
                {
                    callsign_ok = target_data.refCallsignForTime(ref_lower) == callsign;
                    lower_nok = !callsign_ok;
                }

                if (!callsign_ok && ref_upper != -1 && target_data.hasRefCallsignForTime(ref_upper))
                {
                    callsign_ok = target_data.refCallsignForTime(ref_upper) == callsign;
                    upper_nok = !callsign_ok;
                }

                if (callsign_ok)
                    return {ValueComparisonResult::Same, "OK"};
                else
                {
                    string comment = "Not OK:";

                    if (lower_nok)
                    {
                        comment += " test id '"+target_data.tstCallsignForTime(tod)
                                +"' ref id at "+String::timeStringFromDouble(ref_lower)
                                + "  '"+target_data.refCallsignForTime(ref_lower)
                                + "'";
                    }
                    else
                    {
                        assert (upper_nok);
                        comment += " test id '"+target_data.tstCallsignForTime(tod)
                                +"' ref id at "+String::timeStringFromDouble(ref_upper)
                                + "  '"+target_data.refCallsignForTime(ref_upper)
                                + "'";
                    }

                    return {ValueComparisonResult::Different, comment};
                }
            }
            else
                return {ValueComparisonResult::Unknown_NoRefData, "No ref id"};

        }
        else
            return {ValueComparisonResult::Unknown_NoRefData, "No ref id"};
    }
    else
        return {ValueComparisonResult::Unknown_NoTstData, "No test id"};
}

}
