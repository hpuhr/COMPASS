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

#include "eval/requirement/mode_a/correct_period.h"
#include "eval/results/mode_a/correct_period.h"
#include "eval/requirement/group.h"

#include "evaluationdata.h"
#include "evaluationmanager.h"
#include "logger.h"
#include "util/stringconv.h"
#include "util/timeconv.h"
#include "sectorlayer.h"

using namespace std;
using namespace Utils;
using namespace boost::posix_time;

namespace EvaluationRequirement
{

/********************************************************************************************************
 * ModeACorrectPeriod
 ********************************************************************************************************/

/**
 */
ModeACorrectPeriod::ModeACorrectPeriod(const std::string& name, 
                                       const std::string& short_name, 
                                       const std::string& group_name,
                                       float prob, 
                                       COMPARISON_TYPE prob_check_type, 
                                       EvaluationManager& eval_man,
                                       float update_interval_s, 
                                       bool  use_miss_tolerance,
                                       float miss_tolerance_s)
:   IntervalBase(name, 
                 short_name, 
                 group_name, 
                 prob, 
                 prob_check_type, 
                 eval_man, 
                 update_interval_s, 
                 {}, 
                 {},
                 use_miss_tolerance ? boost::optional<float>(miss_tolerance_s) : boost::optional<float>())
{
}

/**
 */
ModeACorrectPeriod::Validity ModeACorrectPeriod::isValid(const dbContent::TargetReport::DataID& data_id,
                                                         const EvaluationTargetData& target_data,
                                                         const SectorLayer& sector_layer,
                                                         const boost::posix_time::time_duration& max_ref_time_diff) const
{
    auto cmp_res = compareModeA(data_id, target_data, max_ref_time_diff);

    bool no_ref = cmp_res.first == ValueComparisonResult::Unknown_NoRefData;
    bool failed = cmp_res.first == ValueComparisonResult::Unknown_NoTstData || 
                  cmp_res.first == ValueComparisonResult::Different;

    Validity v;

    v.value   = no_ref ? Validity::Value::DataMissing : (failed ? Validity::Value::Invalid : Validity::Value::Valid);
    v.comment = no_ref ? "Reference Mode3A missing"   : (failed ? "M3A failed (" + cmp_res.second + ")" : "");

    return v;
}

/**
 */
std::shared_ptr<EvaluationRequirementResult::Single> ModeACorrectPeriod::ModeACorrectPeriod::createResult(
                                        const std::string& result_id,
                                        std::shared_ptr<Base> instance, 
                                        const EvaluationTargetData& target_data,
                                        const SectorLayer& sector_layer, 
                                        const std::vector<EvaluationDetail>& details,
                                        const TimePeriodCollection& periods,
                                        unsigned int sum_uis,
                                        unsigned int misses_total)
{
    return std::make_shared<EvaluationRequirementResult::SingleModeACorrectPeriod>(
                                        "SingleModeACorrectPeriod", 
                                        result_id, 
                                        instance, 
                                        sector_layer, 
                                        target_data.utn_, 
                                        &target_data, 
                                        eval_man_, 
                                        details, 
                                        sum_uis, 
                                        misses_total, 
                                        periods);
}

/********************************************************************************************************
 * ModeACorrectPeriodConfig
 ********************************************************************************************************/

/**
 */
ModeACorrectPeriodConfig::ModeACorrectPeriodConfig(const std::string& class_id, 
                                                   const std::string& instance_id,
                                                   Group& group, 
                                                   EvaluationStandard& standard,
                                                   EvaluationManager& eval_man)
:   IntervalBaseConfig(class_id, instance_id, group, standard, eval_man)
{
    configure(UseMissTol);
}

/**
*/
std::shared_ptr<Base> ModeACorrectPeriodConfig::createRequirement()
{
    shared_ptr<ModeACorrectPeriod> req = make_shared<ModeACorrectPeriod>(
                name_, 
                short_name_, 
                group_.name(), 
                prob_, 
                prob_check_type_, 
                eval_man_,
                update_interval_s_,
                use_miss_tolerance_, 
                miss_tolerance_s_);
    return req;
}

} // namespace EvaluationRequirement
