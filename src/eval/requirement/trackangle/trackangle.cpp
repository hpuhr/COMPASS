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

#include "eval/requirement/trackangle/trackangle.h"
#include "eval/results/trackangle/trackanglesingle.h"
#include "evaluationdata.h"
#include "evaluationmanager.h"
#include "logger.h"
#include "util/stringconv.h"
#include "util/timeconv.h"
#include "sectorlayer.h"
#include "util/number.h"
#include "global.h"

#include <ogr_spatialref.h>

#include <algorithm>
#include <cmath>

using namespace std;
using namespace Utils;
using namespace boost::posix_time;

namespace EvaluationRequirement
{

TrackAngle::TrackAngle(
        const std::string& name, const std::string& short_name, const std::string& group_name,
        float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man,
                    float threshold, bool use_minimum_speed, float minimum_speed,
        COMPARISON_TYPE threshold_value_check_type, bool failed_values_of_interest)
    : Base(name, short_name, group_name, prob, prob_check_type, eval_man),
      threshold_(threshold),
      use_minimum_speed_(use_minimum_speed), minimum_speed_(minimum_speed),
      threshold_value_check_type_(threshold_value_check_type),
      failed_values_of_interest_(failed_values_of_interest)
{

}
float TrackAngle::threshold() const
{
    return threshold_;
}

bool TrackAngle::useMinimumSpeed() const
{
    return use_minimum_speed_;
}

float TrackAngle::minimumSpeed() const
{
    return minimum_speed_;
}

COMPARISON_TYPE TrackAngle::thresholdValueCheckType() const
{
    return threshold_value_check_type_;
}

bool TrackAngle::failedValuesOfInterest() const
{
    return failed_values_of_interest_;
}

std::shared_ptr<EvaluationRequirementResult::Single> TrackAngle::evaluate (
        const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
        const SectorLayer& sector_layer)
{
    logdbg << "EvaluationRequirementTrackAngle '" << name_ << "': evaluate: utn " << target_data.utn_
           << " threshold_percent " << threshold_
           << " threshold_value_check_type " << threshold_value_check_type_;

    time_duration max_ref_time_diff = Time::partialSeconds(eval_man_.maxRefTimeDiff());

    const std::multimap<ptime, unsigned int>& tst_data = target_data.tstData();

    unsigned int num_pos {0};
    unsigned int num_no_ref {0};
    unsigned int num_pos_outside {0};
    unsigned int num_pos_inside {0};
    unsigned int num_pos_ref_spd_low {0};
    unsigned int num_no_tst_value {0};
    unsigned int num_comp_failed {0};
    unsigned int num_comp_passed {0};

    std::vector<EvaluationRequirement::TrackAngleDetail> details;

    ptime timestamp;

    OGRSpatialReference wgs84;
    wgs84.SetWellKnownGeogCS("WGS84");

    OGRSpatialReference local;

    std::unique_ptr<OGRCoordinateTransformation> ogr_geo2cart;

    EvaluationTargetPosition tst_pos;

    bool is_inside;
    EvaluationTargetPosition ref_pos;
    bool ok;

    EvaluationTargetVelocity ref_spd;
    double ref_trackangle_deg, tst_trackangle_deg;
    float trackangle_min_diff;

    bool comp_passed;

    unsigned int num_trackangle_comp {0};
    string comment;

    vector<double> values; // track angle diff percentage

    bool skip_no_data_details = eval_man_.reportSkipNoDataDetails();

    bool has_ground_bit;
    bool ground_bit_set;

    for (const auto& tst_id : tst_data)
    {
        ++num_pos;

        timestamp = tst_id.first;
        tst_pos = target_data.tstPosForTime(timestamp);

        comp_passed = false;

        if (!target_data.hasRefDataForTime (timestamp, max_ref_time_diff))
        {
            if (!skip_no_data_details)
                details.push_back({timestamp, tst_pos,
                                   false, {}, // has_ref_pos, ref_pos
                                   {}, {}, comp_passed, // pos_inside, value, check_passed
                                   {}, {}, {}, // value_ref, value_tst, speed_ref
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                                   num_comp_failed, num_comp_passed,
                                   "No reference data"});

            ++num_no_ref;
            continue;
        }

        tie(ref_pos, ok) = target_data.interpolatedRefPosForTime(timestamp, max_ref_time_diff);

        if (!ok)
        {
            if (!skip_no_data_details)
                details.push_back({timestamp, tst_pos,
                                   false, {}, // has_ref_pos, ref_pos
                                   {}, {}, comp_passed, // pos_inside, value, check_passed
                                   {}, {}, {}, // value_ref, value_tst, speed_ref
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                                   num_comp_failed, num_comp_passed,
                                   "No reference position"});

            ++num_no_ref;
            continue;
        }

        has_ground_bit = target_data.hasTstGroundBitForTime(timestamp);

        if (has_ground_bit)
            ground_bit_set = target_data.tstGroundBitForTime(timestamp);
        else
            ground_bit_set = false;

        if (!ground_bit_set)
            tie(has_ground_bit, ground_bit_set) = target_data.interpolatedRefGroundBitForTime(timestamp, seconds(15));

        is_inside = sector_layer.isInside(ref_pos, has_ground_bit, ground_bit_set);

        if (!is_inside)
        {
            if (!skip_no_data_details)
                details.push_back({timestamp, tst_pos,
                                   true, ref_pos, // has_ref_pos, ref_pos
                                   is_inside, {}, comp_passed, // pos_inside, value, check_passed
                                   {}, {}, {}, // value_ref, value_tst, speed_ref
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                                   num_comp_failed, num_comp_passed,
                                   "Outside sector"});
            ++num_pos_outside;
            continue;
        }

        tie (ref_spd, ok) = target_data.interpolatedRefSpdForTime(timestamp, max_ref_time_diff);

        if (!ok)
        {
            if (!skip_no_data_details)
                details.push_back({timestamp, tst_pos,
                                   false, {}, // has_ref_pos, ref_pos
                                   {}, {}, comp_passed, // pos_inside, value, check_passed
                                   {}, {}, {}, // value_ref, value_tst, speed_ref
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                                   num_comp_failed, num_comp_passed,
                                   "No reference speed"});

            ++num_no_ref;
            continue;
        }

        ++num_pos_inside;

        // ref_spd ok

        if (use_minimum_speed_ && ref_spd.speed_ < minimum_speed_)
        {
            if (!skip_no_data_details)
                details.push_back({timestamp, tst_pos,
                                   false, {}, // has_ref_pos, ref_pos
                                   {}, {}, comp_passed, // pos_inside, value, check_passed
                                   {}, {}, ref_spd.speed_, // value_ref, value_tst, speed_ref
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                                   num_comp_failed, num_comp_passed,
                                   "Reference speed too low"});

            ++num_pos_ref_spd_low;

            //loginf << Time::toString(timestamp) << " ref spd low "  << ref_spd.speed_;

            continue;
        }

        if (!target_data.hasTstMeasuredTrackAngleForTime(timestamp))
        {
            if (!skip_no_data_details)
                details.push_back({timestamp, tst_pos,
                                   false, {}, // has_ref_pos, ref_pos
                                   {}, {}, comp_passed, // pos_inside, value, check_passed
                                   {}, {}, ref_spd.speed_, // value_ref, value_tst, speed_ref
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                                   num_comp_failed, num_comp_passed,
                                   "No tst speed"});

            ++num_no_tst_value;
            continue;
        }

        ref_trackangle_deg = ref_spd.track_angle_;
        tst_trackangle_deg = target_data.tstMeasuredTrackAngleForTime (timestamp);

        trackangle_min_diff = Number::calculateMinAngleDifference(ref_trackangle_deg, tst_trackangle_deg);

//        loginf << Time::toString(timestamp) << " tst_track ref " << ref_trackangle_deg
//               << " tst " << tst_trackangle_deg << " diff " << trackangle_min_diff;

        ++num_trackangle_comp;

        if (compareValue(fabs(trackangle_min_diff), threshold_, threshold_value_check_type_))
        {
            comp_passed = true;
            ++num_comp_passed;
            comment = "Passed";
        }
        else
        {
            ++num_comp_failed;
            comment = "Failed";
        }

//        comment += " tst_track ref " +to_string(ref_trackangle_deg)
//                + " tst " + to_string(tst_trackangle_deg) + " diff " + to_string(trackangle_min_diff);

        details.push_back({timestamp, tst_pos,
                           true, ref_pos,
                           is_inside, trackangle_min_diff, comp_passed, // pos_inside, value, check_passed
                           ref_trackangle_deg, tst_trackangle_deg, ref_spd.speed_, // value_ref, value_tst, speed_ref
                           num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                           num_comp_failed, num_comp_passed,
                           comment});

        values.push_back(trackangle_min_diff);
    }

    //        logdbg << "EvaluationRequirementTrackAngle '" << name_ << "': evaluate: utn " << target_data.utn_
    //               << " num_pos " << num_pos << " num_no_ref " <<  num_no_ref
    //               << " num_pos_outside " << num_pos_outside << " num_pos_inside " << num_pos_inside
    //               << " num_pos_ok " << num_pos_ok << " num_pos_nok " << num_pos_nok
    //               << " num_speeds " << num_speeds;

    assert (num_no_ref <= num_pos);

    if (num_pos - num_no_ref - num_pos_ref_spd_low != num_pos_inside + num_pos_outside)
        logwrn << "EvaluationRequirementTrackAngle '" << name_ << "': evaluate: utn " << target_data.utn_
               << " num_pos " << num_pos << " num_no_ref " <<  num_no_ref
               << " num_pos_outside " << num_pos_outside << " num_pos_inside " << num_pos_inside;
    assert (num_pos - num_no_ref == num_pos_inside + num_pos_outside);


    if (num_trackangle_comp != num_comp_failed + num_comp_passed)
        logwrn << "EvaluationRequirementTrackAngle '" << name_ << "': evaluate: utn " << target_data.utn_
               << " num_speeds " << num_trackangle_comp
               << " num_comp_failed " <<  num_comp_failed << " num_comp_passed " << num_comp_passed;

    assert (num_trackangle_comp == num_comp_failed + num_comp_passed);
    assert (num_trackangle_comp == values.size());

    //assert (details.size() == num_pos);

    return make_shared<EvaluationRequirementResult::SingleTrackAngle>(
                "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                eval_man_, num_pos, num_no_ref, num_pos_outside, num_pos_inside, num_no_tst_value,
                num_comp_failed, num_comp_passed,
                values, details);
}

}
