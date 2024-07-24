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

#include "momcorrect.h"

namespace EvaluationRequirement
{


MomLongAccCorrect::MomLongAccCorrect(const std::string& name, const std::string& short_name, const std::string& group_name,
                                     double prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man)
: GenericInteger(name, short_name, group_name, prob, prob_check_type, eval_man)
{
    result_type_ = "MomLongAccCorrect";

    value_name_ = "MoM Longitudinal Acceleration";
    value_name_short_ = "MoM Long";
    value_name_plural_ = "MoM Longitudinal Acceleration";

    value_compare_func_ = [this] (const dbContent::TargetReport::Chain::DataID& id,
                             const EvaluationTargetData& target_data,
                             boost::posix_time::time_duration max_ref_time_diff)
    { return compareMomLongAcc(id, target_data, max_ref_time_diff); };
}


MomTransAccCorrect::MomTransAccCorrect(const std::string& name, const std::string& short_name, const std::string& group_name,
                                     double prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man)
    : GenericInteger(name, short_name, group_name, prob, prob_check_type, eval_man)
{
    result_type_ = "MomTransAccCorrect";

    value_name_ = "MoM Transversal Acceleration";
    value_name_short_ = "MoM Trans";
    value_name_plural_ = "MoM Transversal Acceleration";

    value_compare_func_ = [this] (const dbContent::TargetReport::Chain::DataID& id,
                                const EvaluationTargetData& target_data,
                                boost::posix_time::time_duration max_ref_time_diff)
    { return compareMomTransAcc(id, target_data, max_ref_time_diff); };
}

MomVertRateCorrect::MomVertRateCorrect(const std::string& name, const std::string& short_name, const std::string& group_name,
                                       double prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man)
    : GenericInteger(name, short_name, group_name, prob, prob_check_type, eval_man)
{
    result_type_ = "MomVertRateCorrect";

    value_name_ = "MoM Vertical Rate";
    value_name_short_ = "MoM Vert";
    value_name_plural_ = "MoM Vertical Rate";

    value_compare_func_ = [this] (const dbContent::TargetReport::Chain::DataID& id,
                                const EvaluationTargetData& target_data,
                                boost::posix_time::time_duration max_ref_time_diff)
    { return compareMomVertRate(id, target_data, max_ref_time_diff); };
}

}
