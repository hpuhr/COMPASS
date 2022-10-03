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

#include "eval/requirement/position/along.h"
#include "eval/results/position/alongsingle.h"
#include "evaluationdata.h"
#include "evaluationmanager.h"
#include "logger.h"
#include "stringconv.h"
#include "sectorlayer.h"

#include <ogr_spatialref.h>

#include <algorithm>

using namespace std;
using namespace Utils;
using namespace boost::posix_time;

namespace EvaluationRequirement
{

PositionAlong::PositionAlong(const std::string& name, const std::string& short_name, const std::string& group_name,
                             float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man,
                             float max_abs_value)
    : Base(name, short_name, group_name, prob, prob_check_type, eval_man), max_abs_value_(max_abs_value)
{
}

float PositionAlong::maxAbsValue() const
{
    return max_abs_value_;
}


std::shared_ptr<EvaluationRequirementResult::Single> PositionAlong::evaluate (
        const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
        const SectorLayer& sector_layer)
{
    logdbg << "EvaluationRequirementPositionAlong '" << name_ << "': evaluate: utn " << target_data.utn_
           << " max_abs_value " << max_abs_value_;

    time_duration max_ref_time_diff = Time::partialSeconds(eval_man_.maxRefTimeDiff());

    const std::multimap<ptime, unsigned int>& tst_data = target_data.tstData();

    unsigned int num_pos {0};
    unsigned int num_no_ref {0};
    unsigned int num_pos_outside {0};
    unsigned int num_pos_inside {0};
    unsigned int num_pos_calc_errors {0};
    unsigned int num_value_ok {0};
    unsigned int num_value_nok {0};

    std::vector<EvaluationRequirement::PositionDetail> details;

    ptime timestamp;

    OGRSpatialReference wgs84;
    wgs84.SetWellKnownGeogCS("WGS84");

    OGRSpatialReference local;

    std::unique_ptr<OGRCoordinateTransformation> ogr_geo2cart;

    EvaluationTargetPosition tst_pos;

    double x_pos, y_pos;
    double distance, angle, d_along;

    bool is_inside;
    pair<EvaluationTargetPosition, bool> ret_pos;
    EvaluationTargetPosition ref_pos;
    pair<EvaluationTargetVelocity, bool> ret_spd;
    EvaluationTargetVelocity ref_spd;
    bool ok;

    bool along_ok;

    unsigned int num_distances {0};
    string comment;

    vector<double> values;

    bool skip_no_data_details = eval_man_.reportSkipNoDataDetails();

    bool has_ground_bit;
    bool ground_bit_set;

    for (const auto& tst_id : tst_data)
    {
        ++num_pos;

        timestamp = tst_id.first;
        tst_pos = target_data.tstPosForTime(timestamp);

        along_ok = true;

        if (!target_data.hasRefDataForTime (timestamp, max_ref_time_diff))
        {
            if (!skip_no_data_details)
                details.push_back({timestamp, tst_pos,
                                   false, {}, // has_ref_pos, ref_pos
                                   {}, {}, along_ok, // pos_inside, value, value_ok,
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                                   num_value_ok, num_value_nok,
                                   "No reference data"});

            ++num_no_ref;
            continue;
        }

        ret_pos = target_data.interpolatedRefPosForTime(timestamp, max_ref_time_diff);

        ref_pos = ret_pos.first;
        ok = ret_pos.second;

        if (!ok)
        {
            if (!skip_no_data_details)
                details.push_back({timestamp, tst_pos,
                                   false, {}, // has_ref_pos, ref_pos
                                   {}, {}, along_ok, // pos_inside, value, value_ok
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                                   num_value_ok, num_value_nok,
                                   "No reference position"});

            ++num_no_ref;
            continue;
        }

        ret_spd = target_data.interpolatedRefPosBasedSpdForTime(timestamp, max_ref_time_diff);

        ref_spd = ret_spd.first;
        assert (ret_pos.second); // must be set of ref pos exists

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
                                   is_inside, {}, along_ok, // pos_inside, value, value_ok
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                                   num_value_ok, num_value_nok,
                                   "Outside sector"});
            ++num_pos_outside;
            continue;
        }
        ++num_pos_inside;

        local.SetStereographic(ref_pos.latitude_, ref_pos.longitude_, 1.0, 0.0, 0.0);

        ogr_geo2cart.reset(OGRCreateCoordinateTransformation(&wgs84, &local));

        if (in_appimage_) // inside appimage
        {
            x_pos = tst_pos.longitude_;
            y_pos = tst_pos.latitude_;
        }
        else
        {
            x_pos = tst_pos.latitude_;
            y_pos = tst_pos.longitude_;
        }

        ok = ogr_geo2cart->Transform(1, &x_pos, &y_pos); // wgs84 to cartesian offsets
        if (!ok)
        {
            details.push_back({timestamp, tst_pos,
                               true, ref_pos, // has_ref_pos, ref_pos
                               is_inside, {}, along_ok, // pos_inside, value, value_ok
                               num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                               num_value_ok, num_value_nok,
                               "Position transformation error"});
            ++num_pos_calc_errors;
            continue;
        }

        distance = sqrt(pow(x_pos,2)+pow(y_pos,2));
        angle = ref_spd.track_angle_ - atan2(y_pos, x_pos);

        if (std::isnan(angle))
        {
            details.push_back({timestamp, tst_pos,
                               true, ref_pos, // has_ref_pos, ref_pos
                               is_inside, {}, along_ok, // pos_inside, value, value_ok
                               num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                               num_value_ok, num_value_nok,
                               "Angle NaN"});
            ++num_pos_calc_errors;
            continue;
        }

        d_along = distance * cos(angle);

        ++num_distances;

        if (fabs(d_along) > max_abs_value_)
        {
            along_ok = false;
            ++num_value_nok;
            comment = "Along-track not OK";
        }
        else
        {
            ++num_value_ok;
            comment = "";
        }

        details.push_back({timestamp, tst_pos,
                           true, ref_pos,
                           is_inside, d_along, along_ok, // pos_inside, value, value_ok
                           num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                           num_value_ok, num_value_nok,
                           comment});

        values.push_back(d_along);
    }

    //        logdbg << "EvaluationRequirementPositionAlong '" << name_ << "': evaluate: utn " << target_data.utn_
    //               << " num_pos " << num_pos << " num_no_ref " <<  num_no_ref
    //               << " num_pos_outside " << num_pos_outside << " num_pos_inside " << num_pos_inside
    //               << " num_pos_ok " << num_pos_ok << " num_pos_nok " << num_pos_nok
    //               << " num_distances " << num_distances;

    assert (num_no_ref <= num_pos);

    if (num_pos - num_no_ref != num_pos_inside + num_pos_outside)
        loginf << "EvaluationRequirementPositionAlong '" << name_ << "': evaluate: utn " << target_data.utn_
               << " num_pos " << num_pos << " num_no_ref " <<  num_no_ref
               << " num_pos_outside " << num_pos_outside << " num_pos_inside " << num_pos_inside
               << " num_pos_calc_errors " << num_pos_calc_errors
               << " num_distances " << num_distances;

    assert (num_pos - num_no_ref == num_pos_inside + num_pos_outside);

    assert (num_distances == num_value_ok+num_value_nok);
    assert (num_distances == values.size());

    //assert (details.size() == num_pos);

    return make_shared<EvaluationRequirementResult::SinglePositionAlong>(
                "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                eval_man_, num_pos, num_no_ref, num_pos_outside, num_pos_inside, num_value_ok, num_value_nok,
                values, details);
}

}
