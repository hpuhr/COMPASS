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


#include "eval/requirement/dubious/dubioustarget.h"
#include "eval/results/dubious/dubioustargetsingle.h"
#include "evaluationmanager.h"
#include "evaluationdata.h"
#include "stringconv.h"
#include "util/number.h"

#include <cmath>

using namespace std;
using namespace Utils;
using namespace boost::posix_time;

#include "boost/date_time/posix_time/ptime.hpp"
#include "boost/date_time/time_duration.hpp"

namespace EvaluationRequirement
{

DubiousTarget::DubiousTarget(
        const std::string& name, const std::string& short_name, const std::string& group_name,
        float minimum_comparison_time, float maximum_comparison_time,
        bool mark_primary_only, bool use_min_updates, unsigned int min_updates,
        bool use_min_duration, float min_duration,
        bool use_max_groundspeed, float max_groundspeed_kts,
        bool use_max_acceleration, float max_acceleration,
        bool use_max_turnrate, float max_turnrate,
        bool use_rocd, float max_rocd, float dubious_prob,
        float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man)
    : Base(name, short_name, group_name, prob, prob_check_type, eval_man),
      minimum_comparison_time_(minimum_comparison_time), maximum_comparison_time_(maximum_comparison_time),
      mark_primary_only_(mark_primary_only), use_min_updates_(use_min_updates), min_updates_(min_updates),
      use_min_duration_(use_min_duration), min_duration_(Time::partialSeconds(min_duration)),
      use_max_groundspeed_(use_max_groundspeed), max_groundspeed_kts_(max_groundspeed_kts),
      use_max_acceleration_(use_max_acceleration), max_acceleration_(max_acceleration),
      use_max_turnrate_(use_max_turnrate), max_turnrate_(max_turnrate),
      use_rocd_(use_rocd), max_rocd_(max_rocd), dubious_prob_(dubious_prob)
{
}


std::shared_ptr<EvaluationRequirementResult::Single> DubiousTarget::evaluate (
        const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
        const SectorLayer& sector_layer)
{
    logdbg << "EvaluationRequirementDubiousTarget '" << name_ << "': evaluate: utn " << target_data.utn_
           << " mark_primary_only " << mark_primary_only_ << " prob " << prob_
           << " use_min_updates " << use_min_updates_ << " min_updates " << min_updates_
           << " use_min_duration " << use_min_duration_ << " min_duration " << min_duration_;

    const std::multimap<ptime, unsigned int>& tst_data = target_data.tstData();

    EvaluationTargetPosition tst_pos;
    bool has_ground_bit;
    bool ground_bit_set;

    bool is_inside;

    ptime timestamp;

    unsigned int num_updates {0};
    unsigned int num_pos_outside {0};
    unsigned int num_pos_inside {0};
    unsigned int num_pos_inside_dubious {0};

    DubiousTargetDetail detail_ (target_data.utn_);

    bool do_not_evaluate_target = false;

    for (const auto& tst_id : tst_data)
    {
        timestamp = tst_id.first;

        ++num_updates;

        // check if inside based on test position only

        tst_pos = target_data.tstPosForTime(timestamp);

        has_ground_bit = target_data.hasTstGroundBitForTime(timestamp);

        if (has_ground_bit)
            ground_bit_set = target_data.tstGroundBitForTime(timestamp);
        else
            ground_bit_set = false;

        is_inside = sector_layer.isInside(tst_pos, has_ground_bit, ground_bit_set);

        if (!is_inside)
        {
            ++num_pos_outside;

            detail_.left_sector_ = true;

            continue;
        }

        detail_.left_sector_ = false;
        ++num_pos_inside;

        if (!detail_.first_inside_ && timestamp - detail_.tod_end_ > seconds(300)) // not first time and time gap too large, skip
            continue;

        ++detail_.num_pos_inside_;

        detail_.updates_.emplace_back(timestamp, tst_pos);

        if (detail_.first_inside_) // do detail time & pos
        {
            detail_.tod_begin_ = timestamp;
            detail_.tod_end_ = timestamp;

            detail_.pos_begin_ = tst_pos;
            detail_.pos_last_ = tst_pos;

            detail_.first_inside_ = false;
        }
        else
        {
            detail_.tod_end_ = timestamp;
            assert (detail_.tod_end_ >= detail_.tod_begin_);
            detail_.duration_ = detail_.tod_end_ - detail_.tod_begin_;

            detail_.pos_last_ = tst_pos;
        }

        // do stats
        if (!detail_.has_mode_ac_
                && (target_data.hasTstModeAForTime(timestamp) || target_data.hasTstModeCForTime(timestamp)))
            detail_.has_mode_ac_  = true;

        if (!detail_.has_mode_s_
                && (target_data.hasTstTAForTime(timestamp) || target_data.hasTstCallsignForTime(timestamp)))
            detail_.has_mode_s_  = true;
    }

    if (!num_pos_inside)
    {
        return make_shared<EvaluationRequirementResult::SingleDubiousTarget>(
                    "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                    eval_man_, num_updates, num_pos_outside, num_pos_inside, num_pos_inside_dubious,
                    detail_);
    }

    // have data, calculate

    unsigned int dubious_groundspeed_found;
    unsigned int dubious_acceleration_found;
    unsigned int dubious_turnrate_found;
    unsigned int dubious_rocd_found;

    bool all_updates_dubious; // for short or primary track
    std::map<std::string, std::string> all_updates_dubious_reasons;

    bool has_last_tod;
    ptime last_timestamp;
    float time_diff;
    float acceleration;
    float track_angle1, track_angle2, turnrate;
    float rocd;

    all_updates_dubious = false;
    all_updates_dubious_reasons.clear();

    dubious_groundspeed_found = 0;
    dubious_acceleration_found = 0;
    dubious_turnrate_found = 0;
    dubious_rocd_found = 0;

    if (!do_not_evaluate_target && mark_primary_only_ && !detail_.has_mode_ac_ && !detail_.has_mode_s_)
    {
        detail_.dubious_reasons_["Pri."] = "";

        all_updates_dubious = true;
        all_updates_dubious_reasons["Pri."] = "";
    }

    if (!do_not_evaluate_target && use_min_updates_ && !detail_.left_sector_
            && detail_.num_pos_inside_ < min_updates_)
    {
        detail_.dubious_reasons_["#Up"] = to_string(detail_.num_pos_inside_);

        all_updates_dubious = true;
        all_updates_dubious_reasons["#Up"] = to_string(detail_.num_pos_inside_);
    }

    if (!do_not_evaluate_target && use_min_duration_ && !detail_.left_sector_
            && detail_.duration_ < min_duration_)
    {
        detail_.dubious_reasons_["Dur."] = Time::toString(detail_.duration_, 1);

        all_updates_dubious = true;
        all_updates_dubious_reasons["Dur."] = Time::toString(detail_.duration_, 1);
    }

    has_last_tod = false;

    DubiousTargetUpdateDetail* last_update {nullptr};
    Transformation trafo_;

    bool ok;
    double x_pos, y_pos;
    double distance, t_diff, spd;

    for (DubiousTargetUpdateDetail& update : detail_.updates_)
    {
        if (!do_not_evaluate_target && all_updates_dubious) // mark was primarty/short track if required
            update.dubious_comments_ = all_updates_dubious_reasons;

        if (!do_not_evaluate_target && use_max_groundspeed_)
        {
            if (target_data.hasTstMeasuredSpeedForTime(update.timestamp_)
                    && target_data.tstMeasuredSpeedForTime(update.timestamp_) > max_groundspeed_kts_)
            {
                update.dubious_comments_["MSpd"] =
                        String::doubleToStringPrecision(target_data.tstMeasuredSpeedForTime(update.timestamp_), 1);

                ++dubious_groundspeed_found;
            }
            else if (last_update) // check last speed to last update
            {
                tie(ok, x_pos, y_pos) = trafo_.distanceCart(
                            last_update->pos_.latitude_, last_update->pos_.longitude_,
                            update.pos_.latitude_, update.pos_.longitude_);
                distance = sqrt(pow(x_pos, 2) + pow(y_pos, 2));
                t_diff = Time::partialSeconds(update.timestamp_ - last_update->timestamp_);
                assert (t_diff >= 0);

                if (t_diff > 0)
                {
                    spd = M_S2KNOTS * distance / t_diff;

                    if(spd > max_groundspeed_kts_)
                    {
                        update.dubious_comments_["CSpd"] =
                                String::doubleToStringPrecision(spd, 1);
                        ++dubious_groundspeed_found;
                    }
                }
            }
        }

        if (has_last_tod)
        {
            assert (update.timestamp_ >= last_timestamp);
            time_diff = Time::partialSeconds(update.timestamp_ - last_timestamp);

            if (!do_not_evaluate_target && time_diff >= minimum_comparison_time_
                    && time_diff <= maximum_comparison_time_)
            {
                if (use_max_acceleration_ && target_data.hasTstMeasuredSpeedForTime(update.timestamp_)
                        && target_data.hasTstMeasuredSpeedForTime(last_timestamp))
                {

                    acceleration = fabs(target_data.tstMeasuredSpeedForTime(update.timestamp_)
                                        - target_data.tstMeasuredSpeedForTime(last_timestamp)) * KNOTS2M_S / time_diff;

                    if (acceleration > max_acceleration_)
                    {
                        update.dubious_comments_["Acc"] =
                                String::doubleToStringPrecision(acceleration, 1);

                        ++dubious_acceleration_found;
                    }
                }

                if (!do_not_evaluate_target && use_max_turnrate_
                        && target_data.hasTstMeasuredTrackAngleForTime(update.timestamp_)
                        && target_data.hasTstMeasuredTrackAngleForTime(last_timestamp))
                {
                    track_angle1 = target_data.tstMeasuredTrackAngleForTime(update.timestamp_);
                    track_angle2 = target_data.tstMeasuredTrackAngleForTime(last_timestamp);

                    turnrate = Number::calculateMinAngleDifference(track_angle2, track_angle1) / time_diff;

//                    turnrate = fabs(RAD2DEG*atan2(sin(DEG2RAD*(track_angle1-track_angle2)),
//                                                  cos(DEG2RAD*(track_angle1-track_angle2)))) / time_diff; // turn angle rate

                    if (fabs(turnrate) > max_turnrate_)
                    {
                        update.dubious_comments_["TR"] =
                                String::doubleToStringPrecision(turnrate, 1);

                        ++dubious_turnrate_found;
                    }
                }

                if (!do_not_evaluate_target && use_rocd_ && target_data.hasTstModeCForTime(update.timestamp_)
                        && target_data.hasTstModeCForTime(last_timestamp))
                {

                    rocd = fabs(target_data.tstModeCForTime(update.timestamp_)
                                - target_data.tstModeCForTime(last_timestamp)) / time_diff;

                    if (rocd > max_rocd_)
                    {
                        update.dubious_comments_["ROCD"] =
                                String::doubleToStringPrecision(rocd, 1);

                        ++dubious_rocd_found;
                    }
                }
            }

            last_update = &update;
        }

        // done
        last_timestamp = update.timestamp_;
        has_last_tod = true;
    }

    if (!do_not_evaluate_target && use_max_groundspeed_ && dubious_groundspeed_found > 0)
    {
        detail_.dubious_reasons_["Spd"] = to_string(dubious_groundspeed_found);
    }

    if (!do_not_evaluate_target && use_max_acceleration_ && dubious_acceleration_found > 0)
    {
        detail_.dubious_reasons_["Acc"] = to_string(dubious_acceleration_found);
    }

    if (!do_not_evaluate_target && use_max_turnrate_ && dubious_turnrate_found > 0)
    {
        detail_.dubious_reasons_["TR"] = to_string(dubious_turnrate_found);
    }

    if (!do_not_evaluate_target && use_rocd_ && dubious_rocd_found > 0)
    {
        detail_.dubious_reasons_["ROCD"] = to_string(dubious_rocd_found);
    }

    detail_.num_pos_inside_dubious_ = detail_.getNumUpdatesDubious(); // num of tods with issues

    if (detail_.num_pos_inside_
            && ((float) detail_.num_pos_inside_dubious_ /(float)(detail_.num_pos_inside_) > dubious_prob_))
    {
        detail_.is_dubious_ = true;
    }
    else
        detail_.is_dubious_ = false;

    //        loginf << "EvaluationRequirementDubiousTarget '" << name_ << "': evaluate: utn " << target_data.utn_
    //               << " is_dubious_ " << track_detail.is_dubious_
    //               << " num_pos_inside_dubious_ " << track_detail.num_pos_inside_dubious_
    //               << " num_pos_inside_ " << track_detail.num_pos_inside_;


    num_pos_inside_dubious += detail_.num_pos_inside_dubious_;

    return make_shared<EvaluationRequirementResult::SingleDubiousTarget>(
                "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                eval_man_, num_updates, num_pos_outside, num_pos_inside, num_pos_inside_dubious,
                detail_);
}

bool DubiousTarget::markPrimaryOnly() const
{
    return mark_primary_only_;
}

bool DubiousTarget::useMinUpdates() const
{
    return use_min_updates_;
}

unsigned int DubiousTarget::minUpdates() const
{
    return min_updates_;
}

bool DubiousTarget::useMinDuration() const
{
    return use_min_duration_;
}

float DubiousTarget::minDuration() const
{
    return Time::partialSeconds(min_duration_);
}

bool DubiousTarget::useMaxAcceleration() const
{
    return use_max_acceleration_;
}
float DubiousTarget::maxAcceleration() const
{
    return max_acceleration_;
}

bool DubiousTarget::useMaxTurnrate() const
{
    return use_max_turnrate_;
}
float DubiousTarget::maxTurnrate() const
{
    return max_turnrate_;
}

bool DubiousTarget::useROCD() const
{
    return use_rocd_;
}
float DubiousTarget::maxROCD() const
{
    return max_rocd_;
}

}
