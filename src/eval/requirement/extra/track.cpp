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

#include "eval/requirement/extra/track.h"
#include "eval/results/extra/tracksingle.h"
#include "evaluationdata.h"
#include "evaluationmanager.h"
#include "logger.h"
#include "stringconv.h"
#include "sectorlayer.h"
#include "timeperiod.h"

#include <boost/algorithm/string/join.hpp>

using namespace std;
using namespace Utils;
using namespace boost::posix_time;

namespace EvaluationRequirement
{

ExtraTrack::ExtraTrack(
        const std::string& name, const std::string& short_name, const std::string& group_name,
        float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man,
        float min_duration, unsigned int min_num_updates, bool ignore_primary_only)
    : Base(name, short_name, group_name, prob, prob_check_type, eval_man),
      min_duration_(Time::partialSeconds(min_duration)),
      min_num_updates_(min_num_updates), ignore_primary_only_(ignore_primary_only)
{

}

float ExtraTrack::minDuration() const
{
    return Time::partialSeconds(min_duration_);
}

unsigned int ExtraTrack::minNumUpdates() const
{
    return min_num_updates_;
}

bool ExtraTrack::ignorePrimaryOnly() const
{
    return ignore_primary_only_;
}

std::shared_ptr<EvaluationRequirementResult::Single> ExtraTrack::evaluate (
        const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
        const SectorLayer& sector_layer)
{
    logdbg << "EvaluationRequirementResultTrack '" << name_ << "': evaluate: utn " << target_data.utn_
           << " min_duration " << min_duration_ << " min_num_updates " << min_num_updates_
           << " ignore_primary_only " << ignore_primary_only_;

    time_duration max_ref_time_diff = Time::partialSeconds(eval_man_.maxRefTimeDiff());

    const auto& tst_data = target_data.tstData();

    ptime timestamp;
    EvaluationTargetPosition tst_pos;

    bool is_inside;

    bool skip_no_data_details = eval_man_.reportSkipNoDataDetails();

    bool has_ground_bit;
    bool ground_bit_set;
    unsigned int track_num;

    // collect track numbers with time periods
    vector<pair<unsigned int, TimePeriod>> finished_tracks;
    map<unsigned int, TimePeriod> active_tracks;

    for (const auto& tst_id : tst_data)
    {
        timestamp = tst_id.first;
        tst_pos = target_data.tstPos(tst_id);

        if (!target_data.hasMappedRefData(tst_id, max_ref_time_diff))
            continue;

        has_ground_bit = target_data.hasTstGroundBit(tst_id);

        if (has_ground_bit)
            ground_bit_set = target_data.tstGroundBit(tst_id);
        else
            ground_bit_set = false;

        if (!ground_bit_set)
            tie(has_ground_bit, ground_bit_set) = target_data.mappedRefGroundBit(tst_id, seconds(15));

        is_inside = target_data.tstPosInside(sector_layer, tst_id, tst_pos, has_ground_bit, ground_bit_set);

        if (!is_inside)
            continue;

        if (!target_data.hasTstTrackNum(tst_id))
            continue;

        track_num = target_data.tstTrackNum(tst_id);

        if (!active_tracks.count(track_num)) // not yet existing
        {
            //active_tracks[track_num] = {tod, tod};
            active_tracks.emplace(std::piecewise_construct,
                                  std::forward_as_tuple(track_num),  // args for key
                                  std::forward_as_tuple(timestamp, timestamp));
            continue;
        }

        // track num exists in active tracks
        TimePeriod& period = active_tracks.at(track_num);
        if (!period.isCloseToEnd(timestamp, seconds(300))) // gap, finish old, create new track
        {
            finished_tracks.emplace_back(track_num, period);
            active_tracks.emplace(std::piecewise_construct,
                                  std::forward_as_tuple(track_num),  // args for key
                                  std::forward_as_tuple(timestamp, timestamp));
            continue;
        }

        // extend active track
        period.extend(timestamp);
    }

    // finish active tracks
    while (active_tracks.size())
    {
        auto at_it = active_tracks.begin();
        finished_tracks.emplace_back(at_it->first, at_it->second);
        active_tracks.erase(at_it);
    }

    unsigned int num_pos {0};
    unsigned int num_pos_outside {0};
    unsigned int num_pos_inside {0};
    unsigned int num_no_track_num {0};
    unsigned int num_extra {0};
    unsigned int num_ok {0};

    typedef EvaluationRequirementResult::SingleExtraTrack Result;
    typedef EvaluationDetail                              Detail;
    typedef Result::EvaluationDetails                     Details;
    Details details;

    //unsigned int extra_time_period_cnt;
    vector<string> extra_track_nums;

    bool has_track_num = false;

    bool has_tod {false};
    ptime tod_min, tod_max;

    auto addDetail = [ & ] (const ptime& ts,
                            const EvaluationTargetPosition& tst_pos,
                            const QVariant& inside,
                            const QVariant& track_num,
                            const QVariant& extra,
                            const std::string& comment)
    {
        details.push_back(Detail(ts, tst_pos).setValue(Result::DetailInside, inside)
                                             .setValue(Result::DetailTrackNum, track_num)
                                             .setValue(Result::DetailExtra, extra)
                                             .generalComment(comment));
    };

    for (const auto& tst_id : tst_data)
    {
        ++num_pos;

        timestamp = tst_id.first;
        tst_pos = target_data.tstPos(tst_id);

        has_track_num = target_data.hasTstTrackNum(tst_id);

        if (has_track_num)
            track_num = target_data.tstTrackNum(tst_id);

        has_ground_bit = target_data.hasTstGroundBit(tst_id);

        if (has_ground_bit)
            ground_bit_set = target_data.tstGroundBit(tst_id);
        else
            ground_bit_set = false;

        //@TODO: check hasMappedRefDataForTime() ???

        if (!ground_bit_set)
            tie(has_ground_bit, ground_bit_set) = target_data.mappedRefGroundBit(tst_id, seconds(15));

        is_inside = target_data.tstPosInside(sector_layer, tst_id, tst_pos, has_ground_bit, ground_bit_set);

        if (!is_inside)
        {
            if (!skip_no_data_details)
                addDetail(timestamp, tst_pos, false, // inside
                            {has_track_num ? track_num : QVariant::Invalid}, // track_num
                            false, "Tst outside"); // extra

            ++num_pos_outside;
            continue;
        }
        ++num_pos_inside;

        if (!has_track_num)
        {
            addDetail(timestamp, tst_pos, true, // inside
                        {has_track_num ? track_num : QVariant::Invalid}, // track_num
                        false, "No track num"); // extra
            ++num_no_track_num;
            continue;
        }

        //extra_time_period_cnt = 0;
        extra_track_nums.clear();

        for (auto& per_it : finished_tracks)
        {
            if (per_it.second.isInside(timestamp) && per_it.first != track_num)
                extra_track_nums.push_back(to_string(per_it.first));
        }

        if (extra_track_nums.size())
        {
            ++num_extra;

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

            addDetail(timestamp, tst_pos, true, // inside
                        {has_track_num ? track_num : QVariant::Invalid}, // track_num
                        true, // extra
                        "Extra tracks: "+ boost::algorithm::join(extra_track_nums, ","));
        }
        else
        {
            ++num_ok;
            addDetail(timestamp, tst_pos, true, // inside
                        {has_track_num ? track_num : QVariant::Invalid}, // track_num
                        false, "OK"); // extra
        }

    }

    bool ignore = false;

    if (num_extra && num_extra < min_num_updates_)
        ignore = true;

    if (!ignore && has_tod && (tod_max-tod_min) < min_duration_)
        ignore = true;

    if (!ignore && ignore_primary_only_ && target_data.isPrimaryOnly())
        ignore = true;

    assert (num_pos == num_pos_inside + num_pos_outside);
    assert (num_pos_inside == num_no_track_num + num_extra + num_ok);

    return make_shared<EvaluationRequirementResult::SingleExtraTrack>(
                "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                eval_man_, details, ignore, num_pos_inside, num_extra, num_ok);
}

}
