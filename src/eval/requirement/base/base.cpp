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
#include "compass.h"
#include "util/timeconv.h"

using namespace std;
using namespace Utils;
using namespace boost::posix_time;

namespace EvaluationRequirement
{

//bool Base::in_appimage_ = true; //COMPASS::isAppImage();

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

unsigned int Base::getNumProbDecimals() const
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

COMPARISON_TYPE Base::probCheckType() const
{
    return prob_check_type_;
}

std::string Base::getConditionStr () const
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
        const dbContent::TargetReport::Chain::DataID& id, const EvaluationTargetData& target_data,
        time_duration max_ref_time_diff)
{
    ptime ref_lower, ref_upper;
    tie(ref_lower, ref_upper) = target_data.mappedRefTimes(id, max_ref_time_diff);

    bool has_ref_data = (!ref_lower.is_not_a_date_time() || !ref_upper.is_not_a_date_time())
            && ((!ref_lower.is_not_a_date_time() && target_data.refChain().hasACID(ref_lower))
                || (!ref_upper.is_not_a_date_time() && target_data.refChain().hasACID(ref_upper)));

    bool has_tst_data = target_data.tstChain().hasACID(id);

    if (!has_ref_data)
    {
        if (has_tst_data)
            return {ValueComparisonResult::Different, "Tst data without ref value"};
        else
            return {ValueComparisonResult::Unknown_NoRefData, "No ref value"};
    }

    if (has_tst_data)
    {
        string value = target_data.tstChain().acid(id);

        bool value_ok;
        bool lower_nok, upper_nok;

        assert (has_ref_data); // ref times possible

        value_ok = false;

        lower_nok = false;
        upper_nok = false;

        if (!ref_lower.is_not_a_date_time() && target_data.refChain().hasACID((ref_lower)))
        {
            value_ok = target_data.refChain().acid(ref_lower) == value;
            lower_nok = !value_ok;
        }

        if (!value_ok && !ref_upper.is_not_a_date_time() && target_data.refChain().hasACID((ref_upper)))
        {
            value_ok = target_data.refChain().acid(ref_upper) == value;
            upper_nok = !value_ok;
        }

        if (value_ok)
            return {ValueComparisonResult::Same, "OK"};
        else
        {
            string comment = "Not OK:";

            if (lower_nok)
            {
                comment += " tst '" + target_data.tstChain().acid(id)
                        +"' ref at " + Time::toString(ref_lower)
                        + " '" + target_data.refChain().acid(ref_lower)
                        + "'";
            }
            else
            {
                assert (upper_nok);
                comment += " tst '" + target_data.tstChain().acid(id)
                        +"' ref at " + Time::toString(ref_upper)
                        + " '" + target_data.refChain().acid(ref_upper)
                        + "'";
            }

            return {ValueComparisonResult::Different, comment};
        }

    }
    else
        return {ValueComparisonResult::Unknown_NoTstData, "No test value"};
}

std::pair<ValueComparisonResult, std::string> Base::compareTa (
        const dbContent::TargetReport::Chain::DataID& id, const EvaluationTargetData& target_data,
        time_duration max_ref_time_diff)
{
    ptime ref_lower, ref_upper;

    tie(ref_lower, ref_upper) = target_data.mappedRefTimes(id, max_ref_time_diff);

    bool has_ref_data = (!ref_lower.is_not_a_date_time() || !ref_upper.is_not_a_date_time())
            && ((!ref_lower.is_not_a_date_time() && target_data.refChain().hasACAD(ref_lower))
                || (!ref_upper.is_not_a_date_time() && target_data.refChain().hasACAD(ref_upper)));

    bool has_tst_data = target_data.tstChain().hasACAD(id);

    if (!has_ref_data)
    {
        if (has_tst_data)
            return {ValueComparisonResult::Different, "Tst data without ref value"};
        else
            return {ValueComparisonResult::Unknown_NoRefData, "No ref value"};

    }

    if (has_tst_data)
    {
        unsigned int value = target_data.tstChain().acad(id);

        bool value_ok;
        bool lower_nok, upper_nok;

        assert (has_ref_data); // ref times possible

        value_ok = false;

        lower_nok = false;
        upper_nok = false;

        if (!ref_lower.is_not_a_date_time() && target_data.refChain().hasACAD(ref_lower))
        {
            value_ok = target_data.refChain().acad(ref_lower) == value;
            lower_nok = !value_ok;
        }

        if (!value_ok && !ref_upper.is_not_a_date_time() && target_data.refChain().hasACAD(ref_upper))
        {
            value_ok = target_data.refChain().acad(ref_upper) == value;
            upper_nok = !value_ok;
        }

        if (value_ok)
            return {ValueComparisonResult::Same, "OK"};
        else
        {
            string comment = "Not OK:";

            if (lower_nok)
            {
                comment += " tst value '" + String::hexStringFromInt(target_data.tstChain().acad(id))
                        +"' ref value at " + Time::toString(ref_lower)
                        + "  '" + String::hexStringFromInt(target_data.refChain().acad(ref_lower))
                        + "'";
            }
            else
            {
                assert (upper_nok);
                comment += " tst value '" + String::hexStringFromInt(target_data.tstChain().acad(id))
                        +"' ref value at " + Time::toString(ref_upper)
                        + "  '" + String::hexStringFromInt(target_data.refChain().acad(ref_upper))
                        + "'";
            }

            return {ValueComparisonResult::Different, comment};
        }
    }
    else
        return {ValueComparisonResult::Unknown_NoTstData, "No test value"};
}

std::pair<ValueComparisonResult, std::string> Base::compareModeA (
        const dbContent::TargetReport::Chain::DataID& id, const EvaluationTargetData& target_data,
        time_duration max_ref_time_diff)
{
    ptime ref_lower, ref_upper;
    tie(ref_lower, ref_upper) = target_data.mappedRefTimes(id, max_ref_time_diff);

    bool has_ref_data = (!ref_lower.is_not_a_date_time() || !ref_upper.is_not_a_date_time())
            && ((!ref_lower.is_not_a_date_time() && target_data.refChain().hasModeA(ref_lower))
                || (!ref_upper.is_not_a_date_time() && target_data.refChain().hasModeA(ref_upper)));

    bool has_tst_data = target_data.tstChain().hasModeA(id);

    if (!has_ref_data)
    {
        if (has_tst_data)
            return {ValueComparisonResult::Different, "Tst data without ref value"};
        else
            return {ValueComparisonResult::Unknown_NoRefData, "No ref value"};
    }

    if (has_tst_data)
    {
        unsigned int code = target_data.tstChain().modeA(id);

        bool value_ok;
        bool lower_nok, upper_nok;

        assert (has_ref_data); // ref times possible

        value_ok = false;

        lower_nok = false;
        upper_nok = false;

        if (!ref_lower.is_not_a_date_time() && target_data.refChain().hasModeA(ref_lower))
        {
            value_ok = target_data.refChain().modeA(ref_lower) == code;
            lower_nok = !value_ok;
        }

        if (!value_ok && !ref_upper.is_not_a_date_time() && target_data.refChain().hasModeA(ref_upper))
        {
            value_ok = target_data.refChain().modeA(ref_upper) == code;
            upper_nok = !value_ok;
        }

        if (value_ok)
            return {ValueComparisonResult::Same, "OK"};
        else
        {
            string comment = "Not OK:";

            if (lower_nok)
            {
                comment += " tst value '" + String::octStringFromInt(code, 4, '0')
                        +"' ref value at " + Time::toString(ref_lower)+ "  '"
                        + String::octStringFromInt(target_data.refChain().modeA(ref_lower), 4, '0')
                        + "'";
            }
            else
            {
                assert (upper_nok);
                comment += " tst value '" + String::octStringFromInt(code, 4, '0')
                        +"' ref value at " + Time::toString(ref_upper)+ "  '"
                        + String::octStringFromInt(target_data.refChain().modeA(ref_upper), 4, '0')
                        + "'";
            }

            return {ValueComparisonResult::Different, comment};
        }
    }
    else
        return {ValueComparisonResult::Unknown_NoTstData, "No test value"};
}

std::pair<ValueComparisonResult, std::string> Base::compareModeC (
        const dbContent::TargetReport::Chain::DataID& id, const EvaluationTargetData& target_data,
        time_duration max_ref_time_diff, float max_val_diff)
{
    if (target_data.tstChain().hasModeC(id))
    {
        int code = target_data.tstChain().modeC(id);

        ptime ref_lower, ref_upper;
        tie(ref_lower, ref_upper) = target_data.mappedRefTimes(id, max_ref_time_diff);

        bool value_ok;
        bool lower_nok, upper_nok;

        if ((!ref_lower.is_not_a_date_time() || !ref_upper.is_not_a_date_time())) // ref times possible
        {
            if ((!ref_lower.is_not_a_date_time() && target_data.refChain().hasModeC(ref_lower))
                    || (!ref_upper.is_not_a_date_time() && target_data.refChain().hasModeC(ref_upper))) // ref value(s) exist
            {
                value_ok = false;

                lower_nok = false;
                upper_nok = false;

                if (!ref_lower.is_not_a_date_time() && target_data.refChain().hasModeC(ref_lower))
                {
                    value_ok = fabs(target_data.refChain().modeC(ref_lower) - code) <= max_val_diff;
                    lower_nok = !value_ok;
                }

                if (!value_ok && !ref_upper.is_not_a_date_time() && target_data.refChain().hasModeC(ref_upper))
                {
                    value_ok = fabs(target_data.refChain().modeC(ref_upper) - code) <= max_val_diff;
                    upper_nok = !value_ok;
                }

                if (value_ok)
                    return {ValueComparisonResult::Same, "OK"};
                else
                {
                    string comment = "Not OK:";

                    if (lower_nok)
                    {
                        comment += " tst value '" + to_string(code)
                                +"' ref value at " + Time::toString(ref_lower)+ "  '"
                                + to_string(target_data.refChain().modeC(ref_lower))
                                + "'";
                    }
                    else
                    {
                        assert (upper_nok);
                        comment += " tst value '" + to_string(code)
                                +"' ref value at " + Time::toString(ref_upper)+ "  '"
                                + to_string(target_data.refChain().modeC(ref_upper))
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
