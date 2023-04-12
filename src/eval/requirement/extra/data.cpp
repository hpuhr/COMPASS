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
using namespace boost::posix_time;

namespace EvaluationRequirement
{

ExtraData::ExtraData(
        const std::string& name, const std::string& short_name, const std::string& group_name,
        float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man,
        float min_duration, unsigned int min_num_updates, bool ignore_primary_only)
    : Base(name, short_name, group_name, prob, prob_check_type, eval_man),
      min_duration_(Time::partialSeconds(min_duration)),
      min_num_updates_(min_num_updates), ignore_primary_only_(ignore_primary_only)
{

}

float ExtraData::minDuration() const
{
    return Time::partialSeconds(min_duration_);
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

    time_duration max_ref_time_diff = Time::partialSeconds(eval_man_.maxRefTimeDiff());
    bool ignore = false;

    // create ref time periods, irrespective of inside
    TimePeriodCollection ref_periods;

    unsigned int num_ref_inside = 0;
    ptime timestamp;
    bool inside;

    {
        const auto& ref_data = target_data.refData();

        bool first {true};

        for (auto& ref_it : ref_data)
        {
            timestamp = ref_it.first;

            // for ref
            inside = target_data.refPosInside(sector_layer, ref_it);

            if (inside)
                ++num_ref_inside;

            if (first)
            {
                ref_periods.add({timestamp, timestamp});

                first = false;

                continue;
            }

            // not first, was_inside is valid

            // extend last time period, if possible, or finish last and create new one
            if (ref_periods.lastPeriod().isCloseToEnd(timestamp, max_ref_time_diff)) // 4.9
                ref_periods.lastPeriod().extend(timestamp);
            else
                ref_periods.add({timestamp, timestamp});
        }
    }
    ref_periods.removeSmallPeriods(seconds(1));

    bool is_inside_ref_time_period {false};
    bool has_tod{false};
    ptime tod_min, tod_max;
    unsigned int num_ok = 0;
    unsigned int num_extra = 0;
    EvaluationTargetPosition tst_pos;

    typedef EvaluationRequirementResult::SingleExtraData Result;
    typedef EvaluationDetail                             Detail;
    typedef Result::EvaluationDetails                    Details;
    Details details;

    bool skip_no_data_details = eval_man_.reportSkipNoDataDetails();

    auto addDetail = [ & ] (const ptime& ts,
                            const EvaluationTargetPosition& tst_pos,
                            const QVariant& inside,
                            const QVariant& extra,
                            const QVariant& ref_exists,
                            const std::string& comment)
    {
        details.push_back(Detail(ts, tst_pos).setValue(Result::DetailKey::Inside, inside)
                                             .setValue(Result::DetailKey::Extra, extra)
                                             .setValue(Result::DetailKey::RefExists, ref_exists)
                                             .generalComment(comment));
    };

    {
        const auto& tst_data = target_data.tstData();

        for (auto& tst_it : tst_data)
        {
            timestamp = tst_it.first;

            assert (target_data.hasTstPos(tst_it));
            tst_pos = target_data.tstPos(tst_it);

            is_inside_ref_time_period = ref_periods.isInside(timestamp);

            if (is_inside_ref_time_period)
            {
                ++num_ok;
                addDetail(timestamp, tst_pos, true, false, true, "OK"); // inside, extra, ref
                continue;
            }

            // no ref
            inside = target_data.tstPosInside(sector_layer, tst_it);

            if (inside)
            {
                ++num_extra;
                addDetail(timestamp, tst_pos, true, true, false, "Extra"); // inside, extra, ref

                if (!has_tod)
                {
                    tod_min = timestamp;
                    tod_max = timestamp;
                    has_tod = true;
                }
                else
                {
                    tod_min = min (timestamp, tod_min);
                    tod_max = max (timestamp, tod_max);
                }
            }
            else if (skip_no_data_details)
                addDetail(timestamp, tst_pos, false, false, false, "Tst outside"); // inside, extra, ref
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
                eval_man_, details, ignore, num_extra, num_ok, has_extra_test_data);
}
}
