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

#ifndef EVALUATIONREQUIREMENT_H
#define EVALUATIONREQUIREMENT_H

#include "dbcontent/target/targetreportchain.h"
#include "evaluationdata.h"
#include "util/timeconv.h"

#include "boost/optional.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/time_duration.hpp"

#include <string>
#include <memory>

class EvaluationTargetData;
class EvaluationManager;
class SectorLayer;

namespace EvaluationRequirementResult {
class Single;
}

namespace Evaluation
{
class DataID;
}

namespace EvaluationRequirement
{

enum ValueComparisonResult
{
    Unknown_NoRefData=0,
    Unknown_NoTstData,
    Same,
    Different
};

class Base
{
  public:
    Base(const std::string& name, const std::string& short_name, const std::string& group_name,
         EvaluationManager& eval_man);
    virtual ~Base();

    virtual std::shared_ptr<EvaluationRequirementResult::Single> evaluate (
        const EvaluationTargetData& target_data,
        std::shared_ptr<Base> instance,
        const SectorLayer& sector_layer) = 0;
    // instance is the self-reference for the result (we want to pass the shared pointer to the result)

    std::string name() const;
    std::string shortname() const;
    std::string groupName() const;

    virtual std::string getConditionStr () const = 0;

  protected:

    std::string name_;
    std::string short_name_;
    std::string group_name_;

    EvaluationManager& eval_man_;

    static std::function<boost::optional<std::string>(const dbContent::TargetReport::Chain&,
                                                      const dbContent::TargetReport::Chain::DataID&)> getACID;
    static std::function<bool(const std::string&, const std::string&)> cmpACID;
    static std::function<std::string(const std::string&)> printACID;

    static std::function<boost::optional<unsigned int>(const dbContent::TargetReport::Chain&,
                                                       const dbContent::TargetReport::Chain::DataID&)> getACAD;
    static std::function<bool(const unsigned int&, const unsigned int&)> cmpACAD;
    static std::function<std::string(const unsigned int&)> printACAD;

    static std::function<boost::optional<unsigned int>(const dbContent::TargetReport::Chain&,
                                                       const dbContent::TargetReport::Chain::DataID&)> getModeA;
    static std::function<bool(const unsigned int&, const unsigned int&)> cmpModeA;
    static std::function<std::string(const unsigned int&)> printModeA;

    static std::function<boost::optional<float>(const dbContent::TargetReport::Chain&,
                                                const dbContent::TargetReport::Chain::DataID&)> getModeC;
    static std::function<bool(const float&, const float&, const float&)> cmpModeC;
    static std::function<std::string(const float&)> printModeC;

    // moms

    static std::function<boost::optional<unsigned char>(const dbContent::TargetReport::Chain&,
                                                       const dbContent::TargetReport::Chain::DataID&)> getMomLongAcc;

    static std::function<boost::optional<unsigned char>(const dbContent::TargetReport::Chain&,
                                                       const dbContent::TargetReport::Chain::DataID&)> getMomTransAcc;

    static std::function<boost::optional<unsigned char>(const dbContent::TargetReport::Chain&,
                                                       const dbContent::TargetReport::Chain::DataID&)> getMomVertRate;

    static std::function<bool(const unsigned char&, const unsigned char&)> cmpMomAny;
    static std::function<std::string(const unsigned char&)> printMomAny;

    template <typename T>
    std::pair<ValueComparisonResult, std::string> compare (
        const dbContent::TargetReport::Chain::DataID& id, const EvaluationTargetData& target_data,
        boost::posix_time::time_duration max_ref_time_diff,
        std::function<boost::optional<T>(const dbContent::TargetReport::Chain& chain,
                                                    const dbContent::TargetReport::Chain::DataID& id)>& getter,
        std::function<bool(const T& val1, const T& val2)>& cmp,
        std::function<std::string(const T& val)>& to_str
        ) const;

    std::pair<ValueComparisonResult, std::string> compareTi (
        const dbContent::TargetReport::Chain::DataID& id, const EvaluationTargetData& target_data,
        boost::posix_time::time_duration max_ref_time_diff) const;
    std::pair<ValueComparisonResult, std::string> compareTa (
        const dbContent::TargetReport::Chain::DataID& id, const EvaluationTargetData& target_data,
        boost::posix_time::time_duration max_ref_time_diff) const;
    std::pair<ValueComparisonResult, std::string> compareModeA (
        const dbContent::TargetReport::Chain::DataID& id, const EvaluationTargetData& target_data,
        boost::posix_time::time_duration max_ref_time_diff) const;
    std::pair<ValueComparisonResult, std::string> compareModeC (
        const dbContent::TargetReport::Chain::DataID& id, const EvaluationTargetData& target_data,
        boost::posix_time::time_duration max_ref_time_diff, float max_val_diff) const; // tod tst

    std::pair<ValueComparisonResult, std::string> compareMomLongAcc (
        const dbContent::TargetReport::Chain::DataID& id, const EvaluationTargetData& target_data,
        boost::posix_time::time_duration max_ref_time_diff) const;
    std::pair<ValueComparisonResult, std::string> compareMomTransAcc (
        const dbContent::TargetReport::Chain::DataID& id, const EvaluationTargetData& target_data,
        boost::posix_time::time_duration max_ref_time_diff) const;
    std::pair<ValueComparisonResult, std::string> compareMomVertRate (
        const dbContent::TargetReport::Chain::DataID& id, const EvaluationTargetData& target_data,
        boost::posix_time::time_duration max_ref_time_diff) const;
};

template <typename T>
std::pair<ValueComparisonResult, std::string> Base::compare (
    const dbContent::TargetReport::Chain::DataID& id, const EvaluationTargetData& target_data,
    boost::posix_time::time_duration max_ref_time_diff,
    std::function<boost::optional<T>(const dbContent::TargetReport::Chain& chain,
                                     const dbContent::TargetReport::Chain::DataID& id)>& getter,
    std::function<bool(const T& val1, const T& val2)>& cmp,
    std::function<std::string(const T& val)>& to_str
    ) const // tst timestamp
{
    boost::posix_time::ptime ref_lower, ref_upper;
    std::tie(ref_lower, ref_upper) = target_data.mappedRefTimes(id, max_ref_time_diff);

    boost::optional<T> tst_value, ref_value_lower, ref_value_upper;

    tst_value = getter(target_data.tstChain(), id);

    if (!ref_lower.is_not_a_date_time())
        ref_value_lower = getter(target_data.refChain(), ref_lower);

    if (!ref_upper.is_not_a_date_time())
        ref_value_upper = getter(target_data.refChain(), ref_upper);

    bool has_ref_data = ref_value_lower.has_value() || ref_value_upper.has_value();

    if (!has_ref_data)
    {
        if (tst_value.has_value())
            return {ValueComparisonResult::Different, "Tst data without ref value"};
        else
            return {ValueComparisonResult::Unknown_NoRefData, "No ref value"};
    }

    if (tst_value.has_value())
    {
        bool value_ok;
        bool lower_nok, upper_nok;

        assert (has_ref_data); // ref times possible

        value_ok = false;

        lower_nok = false;
        upper_nok = false;

        if (ref_value_lower.has_value())
        {
            value_ok = cmp(*ref_value_lower, *tst_value);
            lower_nok = !value_ok;
        }

        if (!value_ok && ref_value_upper.has_value())
        {
            value_ok = cmp(*ref_value_upper, *tst_value);
            upper_nok = !value_ok;
        }

        if (value_ok)
            return {ValueComparisonResult::Same, "OK"};
        else
        {
            std::string comment = "Not OK:";

            if (lower_nok)
            {
                comment += " tst '" + to_str(*tst_value)
                           +"' ref at " + Utils::Time::toString(ref_lower)
                           + " '" + to_str(*ref_value_lower) + "'";
            }
            else
            {
                assert (upper_nok);
                comment += " tst '" + to_str(*tst_value)
                           +"' ref at " + Utils::Time::toString(ref_upper)
                           + " '" + to_str(*ref_value_upper)
                           + "'";
            }

            return {ValueComparisonResult::Different, comment};
        }

    }
    else
        return {ValueComparisonResult::Unknown_NoTstData, "No test value"};
}

}

#endif // EVALUATIONREQUIREMENT_H
