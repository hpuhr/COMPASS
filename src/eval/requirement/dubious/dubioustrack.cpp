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


#include "eval/requirement/dubious/dubioustrack.h"
#include "eval/results/dubious/dubioustracksingle.h"
#include "evaluationmanager.h"
#include "evaluationdata.h"
#include "stringconv.h"

using namespace std;
using namespace Utils;

namespace EvaluationRequirement
{

DubiousTrack::DubiousTrack(
        const std::string& name, const std::string& short_name, const std::string& group_name,
        bool mark_primary_only, bool use_min_updates, unsigned int min_updates,
        bool use_min_duration, float min_duration,
        bool use_max_groundspeed, float max_groundspeed_kts,
        bool use_max_acceleration, float max_acceleration,
        bool use_max_turnrate, float max_turnrate,
        bool use_rocd, float max_rocd,
        float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man)
    : Base(name, short_name, group_name, prob, prob_check_type, eval_man),
      mark_primary_only_(mark_primary_only), use_min_updates_(use_min_updates), min_updates_(min_updates),
      use_min_duration_(use_min_duration), min_duration_(min_duration),
      use_max_groundspeed_(use_max_groundspeed), max_groundspeed_kts_(max_groundspeed_kts),
      use_max_acceleration_(use_max_acceleration), max_acceleration_(max_acceleration),
      use_max_turnrate_(use_max_turnrate), max_turnrate_(max_turnrate),
      use_rocd_(use_rocd), max_rocd_(max_rocd)
{
}


std::shared_ptr<EvaluationRequirementResult::Single> DubiousTrack::evaluate (
        const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
        const SectorLayer& sector_layer)
{
    logdbg << "EvaluationRequirementDubiousTrack '" << name_ << "': evaluate: utn " << target_data.utn_
           << " mark_primary_only " << mark_primary_only_ << " prob " << prob_
           << " use_min_updates " << use_min_updates_ << " min_updates " << min_updates_
           << " use_min_duration " << use_min_duration_ << " min_duration " << min_duration_;

    const std::multimap<float, unsigned int>& tst_data = target_data.tstData();

    unsigned int track_num;
    bool track_num_missing_reported {false};

    EvaluationTargetPosition tst_pos;
    bool has_ground_bit;
    bool ground_bit_set;

    bool is_inside;

    float tod{0};

    //bool general_first_inside = true;
    //float tod_first{0}, tod_last{0};
    //EvaluationTargetPosition pos_first, pos_last;

    unsigned int num_updates {0};
    unsigned int num_pos_outside {0};
    unsigned int num_pos_inside {0};
    unsigned int num_pos_inside_dubious {0};

    unsigned int num_tracks {0};
    unsigned int num_tracks_dubious {0};

    map<unsigned int, DubiousTrackDetail> tracks; // tn -> target
    vector<DubiousTrackDetail> finished_tracks; // tn -> target

    for (const auto& tst_id : tst_data)
    {
        tod = tst_id.first;

        if (!target_data.hasTstTrackNumForTime(tod))
        {
            if (!track_num_missing_reported)
            {
                logwrn << "EvaluationRequirementDubiousTrack '" << name_ << "': evaluate: utn " << target_data.utn_
                       << " has no track number at time " << String::timeStringFromDouble(tod);
                track_num_missing_reported = true;
            }
            continue;
        }

        track_num = target_data.tstTrackNumForTime(tod);

        ++num_updates;

        // check if inside based on test position only

        tst_pos = target_data.tstPosForTime(tod);

        has_ground_bit = target_data.hasTstGroundBitForTime(tod);

        if (has_ground_bit)
            ground_bit_set = target_data.tstGroundBitForTime(tod);
        else
            ground_bit_set = false;

        is_inside = sector_layer.isInside(tst_pos, has_ground_bit, ground_bit_set);

        if (!is_inside)
        {
            ++num_pos_outside;

            if (tracks.count(track_num)) // exists, left sector
            {
                tracks.at(track_num).left_sector_ = true;
            }

            continue;
        }

        // find corresponding track
        if (tracks.count(track_num)) // exists
        {
            assert (tod >= tracks.at(track_num).tod_end_);

            if (tod - tracks.at(track_num).tod_end_ > 300.0) // time gap too large, new track
            {
                finished_tracks.emplace_back(tracks.at(track_num));
                tracks.erase(track_num);
            }
        }

        if (!tracks.count(track_num))
        {
            tracks.emplace(std::piecewise_construct,
                           std::forward_as_tuple(track_num),  // args for key
                           std::forward_as_tuple(track_num, tod));
        }

        assert (tracks.count(track_num));

        DubiousTrackDetail& current_detail = tracks.at(track_num);

        ++num_pos_inside;
        ++current_detail.num_pos_inside_;

        current_detail.updates_.emplace_back(tod, tst_pos);

        //current_detail.tods_inside_.push_back(tod);

        if (current_detail.first_inside_) // do detail time & pos
        {
            current_detail.tod_begin_ = tod;
            current_detail.tod_end_ = tod;

            current_detail.pos_begin_ = tst_pos;
            current_detail.pos_last_ = tst_pos;

            current_detail.first_inside_ = false;
        }
        else
        {
            current_detail.tod_end_ = tod;
            assert (current_detail.tod_end_ >= current_detail.tod_begin_);
            current_detail.duration_ = current_detail.tod_end_ - current_detail.tod_begin_;

            current_detail.pos_last_ = tst_pos;
        }

        // do stats
        if (!current_detail.has_mode_ac_
                && (target_data.hasTstModeAForTime(tod) || target_data.hasTstModeCForTime(tod)))
            current_detail.has_mode_ac_  = true;

        if (!current_detail.has_mode_s_
                && (target_data.hasTstTAForTime(tod) || target_data.hasTstCallsignForTime(tod)))
            current_detail.has_mode_s_  = true;
    }

    while (tracks.size()) // move all to finished
    {
        finished_tracks.emplace_back(tracks.begin()->second);
        tracks.erase(tracks.begin());
    }

    if (!num_pos_inside)
    {
        return make_shared<EvaluationRequirementResult::SingleDubiousTrack>(
                    "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                    eval_man_, num_updates, num_pos_outside, num_pos_inside, num_pos_inside_dubious,
                    0, 0, finished_tracks);
    }

    unsigned int dubious_groundspeed_found;
    unsigned int dubious_acceleration_found;
    unsigned int dubious_turnrate_found;
    unsigned int dubious_rocd_found;

    bool all_updates_dubious; // for short or primary track
    std::map<std::string, std::string> all_updates_dubious_reasons;

    bool has_last_tod;
    float last_tod;
    float time_diff;
    float acceleration;
    float turnrate;
    float rocd;

    for (auto& track_detail : finished_tracks)
    {
        all_updates_dubious = false;
        all_updates_dubious_reasons.clear();

        dubious_groundspeed_found = 0;
        dubious_acceleration_found = 0;
        dubious_turnrate_found = 0;
        dubious_rocd_found = 0;

        if (mark_primary_only_ && !track_detail.has_mode_ac_ && !track_detail.has_mode_s_)
        {
            track_detail.dubious_reasons_["Pri."] = "";

            all_updates_dubious = true;
            all_updates_dubious_reasons["Pri."] = "";
        }

        if (use_min_updates_ && !track_detail.left_sector_ && track_detail.num_pos_inside_ < min_updates_)
        {
            track_detail.dubious_reasons_["#Up"] = to_string(track_detail.num_pos_inside_);

            all_updates_dubious = true;
            all_updates_dubious_reasons["#Up"] = to_string(track_detail.num_pos_inside_);
        }

        if (use_min_duration_ && !track_detail.left_sector_ && track_detail.duration_ < min_duration_)
        {
            track_detail.dubious_reasons_["Dur."] = String::doubleToStringPrecision(track_detail.duration_, 1);

            all_updates_dubious = true;
            all_updates_dubious_reasons["Dur."] = String::doubleToStringPrecision(track_detail.duration_, 1);
        }


        has_last_tod = false;
        for (DubiousTrackUpdateDetail& update : track_detail.updates_)
        {
            if (all_updates_dubious) // mark was primarty/short track if required
                update.dubious_comments_ = all_updates_dubious_reasons;

            if (use_max_groundspeed_ && target_data.hasTstMeasuredSpeedForTime(update.tod_)
                    && target_data.tstMeasuredSpeedForTime(update.tod_) > max_groundspeed_kts_)
            {
                update.dubious_comments_["Spd"] =
                        String::doubleToStringPrecision(target_data.tstMeasuredSpeedForTime(update.tod_), 1);

                ++dubious_groundspeed_found;
            }

            if (has_last_tod)
            {
                assert (update.tod_ >= last_tod);
                time_diff = update.tod_ - last_tod;

                if (time_diff <= 30.0)
                {
                    if (use_max_acceleration_ && target_data.hasTstMeasuredSpeedForTime(update.tod_)
                            && target_data.hasTstMeasuredSpeedForTime(last_tod))
                    {

                        acceleration = fabs(target_data.tstMeasuredSpeedForTime(update.tod_)
                                            - target_data.tstMeasuredSpeedForTime(last_tod)) * KNOTS2M_S / time_diff;

                        if (acceleration > max_acceleration_)
                        {
                            update.dubious_comments_["Acc"] =
                                    String::doubleToStringPrecision(acceleration, 1);

                            ++dubious_acceleration_found;
                        }
                    }

                    if (use_max_turnrate_ && target_data.hasTstMeasuredTrackAngleForTime(update.tod_)
                            && target_data.hasTstMeasuredTrackAngleForTime(last_tod))
                    {
                        turnrate = fabs(target_data.tstMeasuredTrackAngleForTime(update.tod_)
                                        - target_data.tstMeasuredTrackAngleForTime(last_tod)) / time_diff;

                        // move to correct period
                        while (turnrate < 0.0)
                            turnrate += 360.0;

                        while (turnrate > 360.0)
                            turnrate -= 360.0;

                        if (turnrate > max_turnrate_)
                        {
                            update.dubious_comments_["TR"] =
                                    String::doubleToStringPrecision(turnrate, 1);

                            ++dubious_turnrate_found;
                        }
                    }

                    if (use_rocd_ && target_data.hasTstModeCForTime(update.tod_)
                            && target_data.hasTstModeCForTime(last_tod))
                    {

                        rocd = fabs(target_data.tstModeCForTime(update.tod_)
                                            - target_data.tstModeCForTime(last_tod)) / time_diff;

                        if (rocd > max_rocd_)
                        {
                            update.dubious_comments_["ROCD"] =
                                    String::doubleToStringPrecision(rocd, 1);

                            ++dubious_rocd_found;
                        }
                    }
                }
            }

            // done
            last_tod = update.tod_;
            has_last_tod = true;
        }

        if (use_max_groundspeed_ && dubious_groundspeed_found > 0)
        {
            track_detail.dubious_reasons_["Spd"] = to_string(dubious_groundspeed_found);
        }

        if (use_max_acceleration_ && dubious_acceleration_found > 0)
        {
            track_detail.dubious_reasons_["Acc"] = to_string(dubious_acceleration_found);
        }

        if (use_max_turnrate_ && dubious_turnrate_found > 0)
        {
            track_detail.dubious_reasons_["TR"] = to_string(dubious_turnrate_found);
        }

        if (use_rocd_ && dubious_rocd_found > 0)
        {
            track_detail.dubious_reasons_["ROCD"] = to_string(dubious_rocd_found);
        }

        track_detail.num_pos_inside_dubious_ = track_detail.getNumUpdatesDubious(); // num of tods with issues
        //track_detail.is_dubious_ = track_detail.dubious_reasons_.size() != 0;

        if (track_detail.num_pos_inside_
                && ((float) track_detail.num_pos_inside_dubious_ /(float)(track_detail.num_pos_inside_) > 0.15))
        {
            track_detail.is_dubious_ = true;
            ++num_tracks_dubious;
        }
        else
            track_detail.is_dubious_ = false;

//        loginf << "EvaluationRequirementDubiousTrack '" << name_ << "': evaluate: utn " << target_data.utn_
//               << " is_dubious_ " << track_detail.is_dubious_
//               << " num_pos_inside_dubious_ " << track_detail.num_pos_inside_dubious_
//               << " num_pos_inside_ " << track_detail.num_pos_inside_;

        ++num_tracks;

        num_pos_inside_dubious += track_detail.num_pos_inside_dubious_;
    }


    return make_shared<EvaluationRequirementResult::SingleDubiousTrack>(
                "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                eval_man_, num_updates, num_pos_outside, num_pos_inside, num_pos_inside_dubious,
                num_tracks, num_tracks_dubious, finished_tracks);
}

bool DubiousTrack::markPrimaryOnly() const
{
    return mark_primary_only_;
}

bool DubiousTrack::useMinUpdates() const
{
    return use_min_updates_;
}

unsigned int DubiousTrack::minUpdates() const
{
    return min_updates_;
}

bool DubiousTrack::useMinDuration() const
{
    return use_min_duration_;
}

float DubiousTrack::minDuration() const
{
    return min_duration_;
}

bool DubiousTrack::useMaxAcceleration() const
{
    return use_max_acceleration_;
}
float DubiousTrack::maxAcceleration() const
{
    return max_acceleration_;
}

bool DubiousTrack::useMaxTurnrate() const
{
    return use_max_turnrate_;
}
float DubiousTrack::maxTurnrate() const
{
    return max_turnrate_;
}

bool DubiousTrack::useROCD() const
{
    return use_rocd_;
}
float DubiousTrack::maxROCD() const
{
    return max_rocd_;
}

}
