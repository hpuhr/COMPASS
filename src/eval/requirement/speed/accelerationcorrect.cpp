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

#include "accelerationcorrect.h"

namespace EvaluationRequirement 
{

/**
*/
AccelerationCorrect::AccelerationCorrect(
    const std::string& name, const std::string& short_name, const std::string& group_name,
    double prob, COMPARISON_TYPE prob_check_type, double threshold, EvaluationCalculator& calculator)
:   GenericDouble(name, short_name, group_name, prob, prob_check_type, threshold, calculator)
{
    result_type_ = "AccelerationCorrect";

    value_name_ = "Acceleration";
    value_name_short_ = "Acc";
    value_name_plural_ = "Acceleration";

    value_compare_func_ = [this] (const dbContent::TargetReport::Chain::DataID& id,
                                 const EvaluationTargetData& target_data,
                                 boost::posix_time::time_duration max_ref_time_diff, double threshold)
    { return compareAcceleration(id, target_data, max_ref_time_diff, threshold); };
}

} // namespace EvaluationRequirement
