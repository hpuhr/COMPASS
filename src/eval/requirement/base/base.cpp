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
    float ref_lower{0}, ref_upper{0};
    tie(ref_lower, ref_upper) = target_data.refTimesFor(tod, max_ref_time_diff);
    bool has_ref_data = (ref_lower != -1 || ref_upper != -1)
            && ((ref_lower != -1 && target_data.hasRefCallsignForTime(ref_lower))
                || (ref_upper != -1 && target_data.hasRefCallsignForTime(ref_upper)));

    bool has_tst_data = target_data.hasTstCallsignForTime(tod);

    if (!has_ref_data)
    {
        if (has_tst_data)
            return {ValueComparisonResult::Different, "Tst data without ref value"};
        else
            return {ValueComparisonResult::Unknown_NoRefData, "No ref value"};
    }

    if (has_tst_data)
    {
        string value = target_data.tstCallsignForTime(tod);

        bool value_ok;
        bool lower_nok, upper_nok;

        assert (has_ref_data); // ref times possible

        value_ok = false;

        lower_nok = false;
        upper_nok = false;

        if (ref_lower != -1 && target_data.hasRefCallsignForTime(ref_lower))
        {
            value_ok = target_data.refCallsignForTime(ref_lower) == value;
            lower_nok = !value_ok;
        }

        if (!value_ok && ref_upper != -1 && target_data.hasRefCallsignForTime(ref_upper))
        {
            value_ok = target_data.refCallsignForTime(ref_upper) == value;
            upper_nok = !value_ok;
        }

        if (value_ok)
            return {ValueComparisonResult::Same, "OK"};
        else
        {
            string comment = "Not OK:";

            if (lower_nok)
            {
                comment += " tst value '"+target_data.tstCallsignForTime(tod)
                        +"' ref value at "+String::timeStringFromDouble(ref_lower)
                        + "  '"+target_data.refCallsignForTime(ref_lower)
                        + "'";
            }
            else
            {
                assert (upper_nok);
                comment += " tst value '"+target_data.tstCallsignForTime(tod)
                        +"' ref value at "+String::timeStringFromDouble(ref_upper)
                        + "  '"+target_data.refCallsignForTime(ref_upper)
                        + "'";
            }

            return {ValueComparisonResult::Different, comment};
        }

    }
    else
        return {ValueComparisonResult::Unknown_NoTstData, "No test value"};
}

std::pair<ValueComparisonResult, std::string> Base::compareTa (
        float tod, const EvaluationTargetData& target_data, float max_ref_time_diff)
{
    float ref_lower{0}, ref_upper{0};

    tie(ref_lower, ref_upper) = target_data.refTimesFor(tod, max_ref_time_diff);

    bool has_ref_data = (ref_lower != -1 || ref_upper != -1)
            && ((ref_lower != -1 && target_data.hasRefTAForTime(ref_lower))
                || (ref_upper != -1 && target_data.hasRefTAForTime(ref_upper)));

    bool has_tst_data = target_data.hasTstTAForTime(tod);

    if (!has_ref_data)
    {
        if (has_tst_data)
            return {ValueComparisonResult::Different, "Tst data without ref value"};
        else
            return {ValueComparisonResult::Unknown_NoRefData, "No ref value"};

    }

    if (has_tst_data)
    {
        unsigned int value = target_data.tstTAForTime(tod);

        bool value_ok;
        bool lower_nok, upper_nok;

        assert (has_ref_data); // ref times possible

        value_ok = false;

        lower_nok = false;
        upper_nok = false;

        if (ref_lower != -1 && target_data.hasRefTAForTime(ref_lower))
        {
            value_ok = target_data.refTAForTime(ref_lower) == value;
            lower_nok = !value_ok;
        }

        if (!value_ok && ref_upper != -1 && target_data.hasRefTAForTime(ref_upper))
        {
            value_ok = target_data.refTAForTime(ref_upper) == value;
            upper_nok = !value_ok;
        }

        if (value_ok)
            return {ValueComparisonResult::Same, "OK"};
        else
        {
            string comment = "Not OK:";

            if (lower_nok)
            {
                comment += " tst value '"+String::hexStringFromInt(target_data.tstTAForTime(tod))
                        +"' ref value at "+String::timeStringFromDouble(ref_lower)
                        + "  '"+String::hexStringFromInt(target_data.refTAForTime(ref_lower))
                        + "'";
            }
            else
            {
                assert (upper_nok);
                comment += " tst value '"+String::hexStringFromInt(target_data.tstTAForTime(tod))
                        +"' ref value at "+String::timeStringFromDouble(ref_upper)
                        + "  '"+String::hexStringFromInt(target_data.refTAForTime(ref_upper))
                        + "'";
            }

            return {ValueComparisonResult::Different, comment};
        }
    }
    else
        return {ValueComparisonResult::Unknown_NoTstData, "No test value"};
}

std::pair<ValueComparisonResult, std::string> Base::compareModeA (
        float tod, const EvaluationTargetData& target_data, float max_ref_time_diff)
{
    float ref_lower{0}, ref_upper{0};
    tie(ref_lower, ref_upper) = target_data.refTimesFor(tod, max_ref_time_diff);

    bool has_ref_data = (ref_lower != -1 || ref_upper != -1)
            && ((ref_lower != -1 && target_data.hasRefModeAForTime(ref_lower))
                || (ref_upper != -1 && target_data.hasRefModeAForTime(ref_upper)));

    bool has_tst_data = target_data.hasTstModeAForTime(tod);

    if (!has_ref_data)
    {
        if (has_tst_data)
            return {ValueComparisonResult::Different, "Tst data without ref value"};
        else
            return {ValueComparisonResult::Unknown_NoRefData, "No ref value"};
    }

    if (has_tst_data)
    {
        unsigned int code = target_data.tstModeAForTime(tod);

        bool value_ok;
        bool lower_nok, upper_nok;

        assert (has_ref_data); // ref times possible

        value_ok = false;

        lower_nok = false;
        upper_nok = false;

        if (ref_lower != -1 && target_data.hasRefModeAForTime(ref_lower))
        {
            value_ok = target_data.refModeAForTime(ref_lower) == code;
            lower_nok = !value_ok;
        }

        if (!value_ok && ref_upper != -1 && target_data.hasRefModeAForTime(ref_upper))
        {
            value_ok = target_data.refModeAForTime(ref_upper) == code;
            upper_nok = !value_ok;
        }

        if (value_ok)
            return {ValueComparisonResult::Same, "OK"};
        else
        {
            string comment = "Not OK:";

            if (lower_nok)
            {
                comment += " tst value '"+String::octStringFromInt(code, 4, '0')
                        +"' ref value at "+String::timeStringFromDouble(ref_lower)+ "  '"
                        +String::octStringFromInt(target_data.refModeAForTime(ref_lower), 4, '0')
                        + "'";
            }
            else
            {
                assert (upper_nok);
                comment += " tst value '"+String::octStringFromInt(code, 4, '0')
                        +"' ref value at "+String::timeStringFromDouble(ref_upper)+ "  '"
                        +String::octStringFromInt(target_data.refModeAForTime(ref_upper), 4, '0')
                        + "'";
            }

            return {ValueComparisonResult::Different, comment};
        }
    }
    else
        return {ValueComparisonResult::Unknown_NoTstData, "No test value"};
}

std::pair<ValueComparisonResult, std::string> Base::compareModeC (
        float tod, const EvaluationTargetData& target_data, float max_ref_time_diff, float max_val_diff)
{
    if (target_data.hasTstModeCForTime(tod))
    {
        int code = target_data.tstModeCForTime(tod);

        float ref_lower{0}, ref_upper{0};
        tie(ref_lower, ref_upper) = target_data.refTimesFor(tod, max_ref_time_diff);

        bool value_ok;
        bool lower_nok, upper_nok;

        if ((ref_lower != -1 || ref_upper != -1)) // ref times possible
        {
            if ((ref_lower != -1 && target_data.hasRefModeCForTime(ref_lower))
                    || (ref_upper != -1 && target_data.hasRefModeCForTime(ref_upper))) // ref value(s) exist
            {
                value_ok = false;

                lower_nok = false;
                upper_nok = false;

                if (ref_lower != -1 && target_data.hasRefModeCForTime(ref_lower))
                {
                    value_ok = fabs(target_data.refModeCForTime(ref_lower) - code) <= max_val_diff;
                    lower_nok = !value_ok;
                }

                if (!value_ok && ref_upper != -1 && target_data.hasRefModeCForTime(ref_upper))
                {
                    value_ok = fabs(target_data.refModeCForTime(ref_upper) - code) <= max_val_diff;
                    upper_nok = !value_ok;
                }

                if (value_ok)
                    return {ValueComparisonResult::Same, "OK"};
                else
                {
                    string comment = "Not OK:";

                    if (lower_nok)
                    {
                        comment += " tst value '"+to_string(code)
                                +"' ref value at "+String::timeStringFromDouble(ref_lower)+ "  '"
                                +to_string(target_data.refModeCForTime(ref_lower))
                                + "'";
                    }
                    else
                    {
                        assert (upper_nok);
                        comment += " tst value '"+to_string(code)
                                +"' ref value at "+String::timeStringFromDouble(ref_upper)+ "  '"
                                +to_string(target_data.refModeCForTime(ref_upper))
                                + "'";
                    }

                    return {ValueComparisonResult::Different, comment};
                }
            }
            else
                return {ValueComparisonResult::Unknown_NoRefData, "No ref value"};

        }
        else
            return {ValueComparisonResult::Unknown_NoRefData, "No ref value"};
    }
    else
        return {ValueComparisonResult::Unknown_NoTstData, "No test value"};
}

}
