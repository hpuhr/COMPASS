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

namespace EvaluationRequirement
{

ExtraTrack::ExtraTrack(
        const std::string& name, const std::string& short_name, const std::string& group_name,
        float prob, CHECK_TYPE prob_check_type, EvaluationManager& eval_man,
        float min_duration, unsigned int min_num_updates, bool ignore_primary_only,
        float maximum_probability)
    : Base(name, short_name, group_name, prob, prob_check_type, eval_man), min_duration_(min_duration),
      min_num_updates_(min_num_updates), ignore_primary_only_(ignore_primary_only),
      maximum_probability_(maximum_probability)
{

}

float ExtraTrack::minDuration() const
{
    return min_duration_;
}

unsigned int ExtraTrack::minNumUpdates() const
{
    return min_num_updates_;
}

bool ExtraTrack::ignorePrimaryOnly() const
{
    return ignore_primary_only_;
}

float ExtraTrack::maximumProbability() const
{
    return maximum_probability_;
}

std::shared_ptr<EvaluationRequirementResult::Single> ExtraTrack::evaluate (
        const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
        const SectorLayer& sector_layer)
{
    logdbg << "EvaluationRequirementResultTrack '" << name_ << "': evaluate: utn " << target_data.utn_
           << " min_duration " << min_duration_ << " min_num_updates " << min_num_updates_
           << " ignore_primary_only " << ignore_primary_only_ << " maximum_probability " << maximum_probability_;

    float max_ref_time_diff = eval_man_.maxRefTimeDiff();

    const std::multimap<float, unsigned int>& tst_data = target_data.tstData();

    float tod{0};
    EvaluationTargetPosition tst_pos;

    bool is_inside;

    bool skip_no_data_details = eval_man_.resultsGenerator().skipNoDataDetails();

    bool has_ground_bit;
    bool ground_bit_set;
    unsigned int track_num;

    // collect track numbers with time periods
    vector<pair<unsigned int, TimePeriod>> finished_tracks;
    map<unsigned int, TimePeriod> active_tracks;

    for (const auto& tst_id : tst_data)
    {
        tod = tst_id.first;
        tst_pos = target_data.tstPosForTime(tod);

        if (!target_data.hasRefDataForTime (tod, max_ref_time_diff))
            continue;

        has_ground_bit = target_data.hasTstGroundBitForTime(tod);

        if (has_ground_bit)
            ground_bit_set = target_data.tstGroundBitForTime(tod);
        else
            ground_bit_set = false;

        is_inside = sector_layer.isInside(tst_pos, has_ground_bit, ground_bit_set);

        if (!is_inside)
            continue;

        if (!target_data.hasTstTrackNumForTime(tod))
            continue;

        track_num = target_data.tstTrackNumForTime(tod);

        if (!active_tracks.count(track_num)) // not yet existing
        {
            //active_tracks[track_num] = {tod, tod};
            active_tracks.emplace(std::piecewise_construct,
                                  std::forward_as_tuple(track_num),  // args for key
                                  std::forward_as_tuple(tod, tod));
            continue;
        }

        // track num exists in active tracks
        TimePeriod& period = active_tracks.at(track_num);
        if (!period.isCloseToEnd(tod, 300.0)) // gap, finish old, create new track
        {
            finished_tracks.emplace_back(track_num, period);
            active_tracks.emplace(std::piecewise_construct,
                                  std::forward_as_tuple(track_num),  // args for key
                                  std::forward_as_tuple(tod, tod));
            continue;
        }

        // extend active track
        period.extend(tod);
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

    std::vector<EvaluationRequirement::ExtraTrackDetail> details;

    unsigned int extra_time_period_cnt;
    vector<string> extra_track_nums;

    bool has_track_num = false;

    bool has_tod {false};
    float tod_min, tod_max;

    for (const auto& tst_id : tst_data)
    {
        ++num_pos;

        tod = tst_id.first;
        tst_pos = target_data.tstPosForTime(tod);

        has_track_num = target_data.hasTstTrackNumForTime(tod);

        if (has_track_num)
            track_num = target_data.tstTrackNumForTime(tod);

        has_ground_bit = target_data.hasTstGroundBitForTime(tod);

        if (has_ground_bit)
            ground_bit_set = target_data.tstGroundBitForTime(tod);
        else
            ground_bit_set = false;

        is_inside = sector_layer.isInside(tst_pos, has_ground_bit, ground_bit_set);


        if (!is_inside)
        {
            if (!skip_no_data_details)
                details.push_back({tod, tst_pos, false, // inside
                                   {has_track_num ? track_num : QVariant::Invalid}, // track_num
                                   false, "Tst outside"}); // extra

            ++num_pos_outside;
            continue;
        }
        ++num_pos_inside;

        if (!has_track_num)
        {
            details.push_back({tod, tst_pos, true, // inside
                               {has_track_num ? track_num : QVariant::Invalid}, // track_num
                               false, "No track num"}); // extra
            ++num_no_track_num;
            continue;
        }



        extra_time_period_cnt = 0;
        extra_track_nums.clear();

        for (auto& per_it : finished_tracks)
        {
            if (per_it.second.isInside(tod) && per_it.first != track_num)
                extra_track_nums.push_back(to_string(per_it.first));
        }

        if (extra_track_nums.size())
        {
            ++num_extra;

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

            details.push_back({tod, tst_pos, true, // inside
                               {has_track_num ? track_num : QVariant::Invalid}, // track_num
                               true, // extra
                               "Extra tracks: "+ boost::algorithm::join(extra_track_nums, ",")});
        }
        else
        {
            ++num_ok;
            details.push_back({tod, tst_pos, true, // inside
                               {has_track_num ? track_num : QVariant::Invalid}, // track_num
                               false, "OK"}); // extra
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
                eval_man_, ignore, num_pos_inside, num_extra, num_ok, details);
}
}
