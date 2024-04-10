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

// acid
std::function<boost::optional<std::string>(const dbContent::TargetReport::Chain&,
                                           const dbContent::TargetReport::Chain::DataID&)> Base::getACID =
    [] (const dbContent::TargetReport::Chain& chain,
           const dbContent::TargetReport::Chain::DataID& id) { return chain.acid(id); };

std::function<bool(const std::string&, const std::string&)> Base::cmpACID =
    [] (const std::string& val1, const std::string& val2) { return val1 == val2; };

std::function<std::string(const std::string&)> Base::printACID =
    [] (const std::string& val) { return val; };

// acad
std::function<boost::optional<unsigned int>(const dbContent::TargetReport::Chain&,
                                           const dbContent::TargetReport::Chain::DataID&)> Base::getACAD =
    [] (const dbContent::TargetReport::Chain& chain,
       const dbContent::TargetReport::Chain::DataID& id) { return chain.acad(id); };

std::function<bool(const unsigned int&, const unsigned int&)> Base::cmpACAD =
    [] (const unsigned int& val1, const unsigned int& val2) { return val1 == val2; };

std::function<std::string(const unsigned int&)> Base::printACAD =
    [] (const unsigned int& val) { return String::hexStringFromInt(val); };

// mode a
std::function<boost::optional<unsigned int>(const dbContent::TargetReport::Chain&,
                                            const dbContent::TargetReport::Chain::DataID&)> Base::getModeA =
    [] (const dbContent::TargetReport::Chain& chain,
       const dbContent::TargetReport::Chain::DataID& id) { return chain.modeA(id); };

std::function<bool(const unsigned int&, const unsigned int&)> Base::cmpModeA =
    [] (const unsigned int& val1, const unsigned int& val2) { return val1 == val2; };

std::function<std::string(const unsigned int&)> Base::printModeA =
    [] (const unsigned int& val) { return String::octStringFromInt(val, 4, '0'); };

// mode c
std::function<boost::optional<float>(const dbContent::TargetReport::Chain&,
                                            const dbContent::TargetReport::Chain::DataID&)> Base::getModeC =
    [] (const dbContent::TargetReport::Chain& chain,
       const dbContent::TargetReport::Chain::DataID& id) { return chain.modeC(id); };

std::function<bool(const float&, const float&, const float&)> Base::cmpModeC =
    [] (const float& val1, const float& val2, const float& max_val_diff) { return fabs(val1 - val2) <= max_val_diff; };

std::function<std::string(const float&)> Base::printModeC =
    [] (const unsigned int& val) { return String::doubleToStringPrecision(val, 2); };

Base::Base(const std::string& name, const std::string& short_name, const std::string& group_name,
           EvaluationManager& eval_man)
    : name_(name), short_name_(short_name), group_name_(group_name),
      eval_man_(eval_man)
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



std::pair<ValueComparisonResult, std::string> Base::compareTi (
        const dbContent::TargetReport::Chain::DataID& id, const EvaluationTargetData& target_data,
        time_duration max_ref_time_diff) const
{
    return compare<std::string>(id, target_data, max_ref_time_diff, getACID, cmpACID, printACID);
}

std::pair<ValueComparisonResult, std::string> Base::compareTa (
        const dbContent::TargetReport::Chain::DataID& id, const EvaluationTargetData& target_data,
        time_duration max_ref_time_diff) const
{
    return compare<unsigned int>(id, target_data, max_ref_time_diff, getACAD, cmpACAD, printACAD);
}

std::pair<ValueComparisonResult, std::string> Base::compareModeA (
        const dbContent::TargetReport::Chain::DataID& id, const EvaluationTargetData& target_data,
        time_duration max_ref_time_diff) const
{
    return compare<unsigned int>(id, target_data, max_ref_time_diff, getModeA, cmpModeA, printModeA);
}

std::pair<ValueComparisonResult, std::string> Base::compareModeC (
        const dbContent::TargetReport::Chain::DataID& id, const EvaluationTargetData& target_data,
        time_duration max_ref_time_diff, float max_val_diff) const
{

    std::function<bool(const float&, const float&)> tmp_cmpModeC =
        [max_val_diff] (const float& val1, const float& val2) { return fabs(val1 - val2) <= max_val_diff; };

     return compare<float>(id, target_data, max_ref_time_diff, getModeC, tmp_cmpModeC, printModeC);
}

}
