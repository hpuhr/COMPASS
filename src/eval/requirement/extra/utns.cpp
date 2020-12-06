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

#include "eval/requirement/extra/utns.h"
#include "eval/results/extra/utnssingle.h"
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

ExtraUTNs::ExtraUTNs(
        const std::string& name, const std::string& short_name, const std::string& group_name,
        EvaluationManager& eval_man,
        float max_ref_time_diff, float min_duration, unsigned int min_num_updates, bool ignore_primary_only,
        float maximum_probability)
    : Base(name, short_name, group_name, eval_man), max_ref_time_diff_(max_ref_time_diff), min_duration_(min_duration),
      min_num_updates_(min_num_updates), ignore_primary_only_(ignore_primary_only),
      maximum_probability_(maximum_probability)
{

}

float ExtraUTNs::minDuration() const
{
    return min_duration_;
}

unsigned int ExtraUTNs::minNumUpdates() const
{
    return min_num_updates_;
}

bool ExtraUTNs::ignorePrimaryOnly() const
{
    return ignore_primary_only_;
}

float ExtraUTNs::maximumProbability() const
{
    return maximum_probability_;
}

std::shared_ptr<EvaluationRequirementResult::Single> ExtraUTNs::evaluate (
        const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
        const SectorLayer& sector_layer)
{
    logdbg << "EvaluationRequirementExtraUTNs '" << name_ << "': evaluate: utn " << target_data.utn_
           << " min_duration " << min_duration_ << " min_num_updates " << min_num_updates_
           << " ignore_primary_only " << ignore_primary_only_ << " maximum_probability " << maximum_probability_;

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
            if (ref_periods.lastPeriod().isCloseToEnd(tod, max_ref_time_diff_)) // 4.9
                ref_periods.lastPeriod().extend(tod);
            else
                ref_periods.add({tod, tod});
        }
    }
    ref_periods.removeSmallPeriods(1);

    bool is_inside_ref_time_period {false};
    bool has_tod{false};
    float tod_min, tod_max;
    unsigned int num_tst_inside = 0;
    EvaluationTargetPosition tst_pos;

    vector<ExtraUTNsDetail> details;
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
                details.push_back({tod, tst_pos, true, false, true, "OK"}); // inside, extra, ref
                continue;
            }

            // no ref

            has_ground_bit = target_data.hasTstGroundBitForTime(tod);

            if (has_ground_bit)
                ground_bit_set = target_data.tstGroundBitForTime(tod);
            else
                ground_bit_set = false;

            inside = sector_layer.isInside(tst_pos, has_ground_bit, ground_bit_set);

            if (inside)
            {
                ++num_tst_inside;
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

    unsigned int num_inside = num_ref_inside + num_tst_inside;

    if (num_inside < min_num_updates_)
        ignore = true;

    if (!ignore && has_tod && (tod_max-tod_min) < min_duration_)
        ignore = true;

    if (!ignore && ignore_primary_only_ && target_data.isPrimaryOnly())
        ignore = true;

    bool test_data_only = !num_ref_inside && num_tst_inside;

    if (!ignore && test_data_only)
        loginf << "EvaluationRequirementExtraUTNs '" << name_ << "': evaluate: utn " << target_data.utn_
               << " not ignored tdo, ref " << num_ref_inside << " tst " << num_tst_inside;

    return make_shared<EvaluationRequirementResult::SingleExtraUTNs>(
                "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                eval_man_, ignore, test_data_only, details);
}
}
