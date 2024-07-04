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

#include "probabilitybase.h"

#include "stringconv.h"

using namespace std;
using namespace Utils;

namespace EvaluationRequirement 
{

/**
*/
ProbabilityBase::ProbabilityBase(const std::string& name, 
                                 const std::string& short_name, 
                                 const std::string& group_name,
                                 double prob_threshold, 
                                 COMPARISON_TYPE prob_check_type, 
                                 bool invert_prob,
                                 EvaluationManager& eval_man,
                                 const boost::optional<bool>& must_hold_for_any_target)
:   Base(name, short_name, group_name, prob_threshold, prob_check_type, eval_man, must_hold_for_any_target)
,   invert_prob_(invert_prob)
{
}

/**
*/
ProbabilityBase::~ProbabilityBase() = default;

/**
*/
bool ProbabilityBase::invertProb() const 
{ 
    return invert_prob_; 
}

/**
*/
unsigned int ProbabilityBase::getNumThresholdDecimals() const
{
    const double thres = threshold();

    assert (thres <= 1);

    float        tmp      = 1;
    unsigned int decimals = 1;

    while (tmp > thres && decimals < NumThresholdDecimalsMax)
    {
        tmp /= 10.0;
        ++decimals;
    }

    //loginf << "ProbabilityBase: getNumThresholdDecimals: threshold " << thres << " dec " << decimals;
    return decimals;
}

/**
*/
std::string ProbabilityBase::getConditionResultNameShort() const
{
    return probabilityNameShort();
}

/**
*/
std::string ProbabilityBase::getConditionUnits() const
{
    return "%";
}

/**
*/
std::string ProbabilityBase::getConditionResultName() const
{
    return probabilityName();
}

/**
*/
std::string ProbabilityBase::getThresholdString(double thres) const
{
    return String::percentToString(thres * 100.0, getNumThresholdDecimals());
}

} // namespace EvaluationRequirement
