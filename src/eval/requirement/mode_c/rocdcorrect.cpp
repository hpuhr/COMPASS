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

#include "rocdcorrect.h"

namespace EvaluationRequirement 
{

ROCDCorrect::ROCDCorrect(const std::string& name, const std::string& short_name, const std::string& group_name,
                         double prob, COMPARISON_TYPE prob_check_type, double threshold, EvaluationManager& eval_man)
: GenericDouble(name, short_name, group_name, prob, prob_check_type, threshold, eval_man)
{
    result_type_ = "ROCDCorrect";

    value_name_ = "Rate of Climb/Descent";
    value_name_short_ = "ROCD";
    value_name_plural_ = "Rate of Climb/Descent";

    value_compare_func_ = [this] (const dbContent::TargetReport::Chain::DataID& id,
                                 const EvaluationTargetData& target_data,
                                 boost::posix_time::time_duration max_ref_time_diff, double threshold)
    { return compareROCD(id, target_data, max_ref_time_diff, threshold); };
}

} // namespace EvaluationRequirement
