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

#include "eval/requirement/extra/data.h"
#include "eval/results/extra/datasingle.h"
#include "evaluationdata.h"
#include "evaluationmanager.h"
#include "logger.h"
#include "stringconv.h"
#include "sectorlayer.h"
#include "timeperiod.h"

using namespace std;
using namespace Utils;

namespace EvaluationRequirement
{

ExtraData::ExtraData(
        const std::string& name, const std::string& short_name, const std::string& group_name,
        float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man,
        float min_duration, unsigned int min_num_updates, bool ignore_primary_only)
    : Base(name, short_name, group_name, prob, prob_check_type, eval_man), min_duration_(min_duration),
      min_num_updates_(min_num_updates), ignore_primary_only_(ignore_primary_only)
{

}

float ExtraData::minDuration() const
{
    return min_duration_;
}

unsigned int ExtraData::minNumUpdates() const
{
    return min_num_updates_;
}

bool ExtraData::ignorePrimaryOnly() const
{
    return ignore_primary_only_;
}

std::shared_ptr<EvaluationRequirementResult::Single> ExtraData::evaluate (
        const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
        const SectorLayer& sector_layer)
{
    logdbg << "EvaluationRequirementResultExtraData '" << name_ << "': evaluate: utn " << target_data.utn_
           << " min_duration " << min_duration_ << " min_num_updates " << min_num_updates_
           << " ignore_primary_only " << ignore_primary_only_ << " prob " << prob_;

    float max_ref_time_diff = eval_man_.maxRefTimeDiff();
    bool ignore = false;

    // create ref time periods, irrespective of inside
    TimePeriodCollection ref_periods;

    unsigned int num_ref_inside = 0;
    float tod {0};
    bool has_ground_bit;
    bool ground_bit_set;
    bool inside;

    {
        const std::multimap<float, unsigned int>& ref_data = target_data.refData();

        bool first {true};

        for (auto& ref_it : ref_data)
        {
            tod = ref_it.first;

            // for ref
            tie (has_ground_bit, ground_bit_set) = target_data.tstGroundBitForTimeInterpolated(tod);

            inside = target_data.hasRefPosForTime(tod)
                    && sector_layer.isInside(target_data.refPosForTime(tod), has_ground_bit, ground_bit_set);

            if (inside)
                ++num_ref_inside;

            if (first)
            {
                ref_periods.add({tod, tod});

                first = false;

                continue;
            }

            // not first, was_inside is valid

            // extend last time period, if possible, or finish last and create new one
            if (ref_periods.lastPeriod().isCloseToEnd(tod, max_ref_time_diff)) // 4.9
                ref_periods.lastPeriod().extend(tod);
            else
                ref_periods.add({tod, tod});
        }
    }
    ref_periods.removeSmallPeriods(1);

    bool is_inside_ref_time_period {false};
    bool has_tod{false};
    float tod_min, tod_max;
    unsigned int num_ok = 0;
    unsigned int num_extra = 0;
    EvaluationTargetPosition tst_pos;

    vector<ExtraDataDetail> details;
    bool skip_no_data_details = eval_man_.resultsGenerator().skipNoDataDetails();

    {
        const std::multimap<float, unsigned int>& tst_data = target_data.tstData();

        for (auto& tst_it : tst_data)
        {
            tod = tst_it.first;

            assert (target_data.hasTstPosForTime(tod));
            tst_pos = target_data.tstPosForTime(tod);

            is_inside_ref_time_period = ref_periods.isInside(tod);

            if (is_inside_ref_time_period)
            {
                ++num_ok;
                details.push_back({tod, tst_pos, true, false, true, "OK"}); // inside, extra, ref
                continue;
            }

            // no ref

            has_ground_bit = target_data.hasTstGroundBitForTime(tod);

            if (has_ground_bit)
                ground_bit_set = target_data.tstGroundBitForTime(tod);
            else
                ground_bit_set = false;

            if (!ground_bit_set)
                tie(has_ground_bit, ground_bit_set) = target_data.interpolatedRefGroundBitForTime(tod, 15.0);

            inside = sector_layer.isInside(tst_pos, has_ground_bit, ground_bit_set);

            if (inside)
            {
                ++num_extra;
                details.push_back({tod, tst_pos, true, true, false, "Extra"}); // inside, extra, ref

                if (!has_tod)
                {
                    tod_min = tod;
                    tod_max = tod;
                    has_tod = true;
                }
                else
                {
                    tod_min = min (tod, tod_min);
                    tod_max = max (tod, tod_max);
                }
            }
            else if (skip_no_data_details)
                details.push_back({tod, tst_pos, false, false, false, "Tst outside"}); // inside, extra, ref
        }
    }

    if (num_extra && num_extra < min_num_updates_)
        ignore = true;

    if (!ignore && has_tod && (tod_max-tod_min) < min_duration_)
        ignore = true;

    if (!ignore && ignore_primary_only_ && target_data.isPrimaryOnly())
        ignore = true;

    bool has_extra_test_data = num_extra;

//    if (!ignore && test_data_only)
//        loginf << "EvaluationRequirementResultExtraData '" << name_ << "': evaluate: utn " << target_data.utn_
//               << " not ignored tdo, ref " << num_ref_inside << " num_ok " << num_ok << " num_extra " << num_extra;

    return make_shared<EvaluationRequirementResult::SingleExtraData>(
                "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                eval_man_, ignore, num_extra, num_ok, has_extra_test_data, details);
}
}
