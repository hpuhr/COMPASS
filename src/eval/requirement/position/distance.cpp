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

#include "eval/requirement/position/distance.h"
#include "eval/results/position/distancesingle.h"
#include "evaluationdata.h"
#include "evaluationmanager.h"
#include "logger.h"
#include "util/stringconv.h"
#include "util/timeconv.h"
#include "sectorlayer.h"

#include <ogr_spatialref.h>

#include <algorithm>

using namespace std;
using namespace Utils;
using namespace boost::posix_time;

namespace EvaluationRequirement
{

PositionDistance::PositionDistance(
        const std::string& name, const std::string& short_name, const std::string& group_name,
        float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man,
        float threshold_value, COMPARISON_TYPE threshold_value_check_type,
        bool failed_values_of_interest)
    : Base(name, short_name, group_name, prob, prob_check_type, eval_man),
      threshold_value_(threshold_value), threshold_value_check_type_(threshold_value_check_type),
      failed_values_of_interest_(failed_values_of_interest)
{

}

float PositionDistance::thresholdValue() const
{
    return threshold_value_;
}

COMPARISON_TYPE PositionDistance::thresholdValueCheckType() const
{
    return threshold_value_check_type_;
}

bool PositionDistance::failedValuesOfInterest() const
{
    return failed_values_of_interest_;
}

std::shared_ptr<EvaluationRequirementResult::Single> PositionDistance::evaluate (
        const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
        const SectorLayer& sector_layer)
{
    logdbg << "EvaluationRequirementPositionDistance '" << name_ << "': evaluate: utn " << target_data.utn_
           << " threshold_value " << threshold_value_ << " threshold_value_check_type " << threshold_value_check_type_;

    time_duration max_ref_time_diff = Time::partialSeconds(eval_man_.maxRefTimeDiff());

    const auto& tst_data = target_data.tstData();

    unsigned int num_pos {0};
    unsigned int num_no_ref {0};
    unsigned int num_pos_outside {0};
    unsigned int num_pos_inside {0};
    unsigned int num_pos_calc_errors {0};
    unsigned int num_comp_failed {0};
    unsigned int num_comp_passed {0};

    typedef EvaluationRequirementResult::SinglePositionDistance Result;
    typedef EvaluationDetail                                    Detail;
    typedef Result::EvaluationDetails                           Details;
    Details details;

    ptime timestamp;

    OGRSpatialReference wgs84;
    wgs84.SetWellKnownGeogCS("WGS84");

    OGRSpatialReference local;

    std::unique_ptr<OGRCoordinateTransformation> ogr_geo2cart;

    EvaluationTargetPosition tst_pos;

    double x_pos, y_pos;
    double distance;

    bool is_inside;
    pair<EvaluationTargetPosition, bool> ret_pos;
    EvaluationTargetPosition ref_pos;
    bool ok;

    bool comp_passed;

    unsigned int num_distances {0};
    string comment;

    vector<double> values;

    bool skip_no_data_details = eval_man_.reportSkipNoDataDetails();

    auto addDetail = [ & ] (const ptime& ts,
                            const EvaluationTargetPosition& tst_pos,
                            const boost::optional<EvaluationTargetPosition>& ref_pos,
                            const QVariant& pos_inside,
                            const QVariant& offset,
                            const QVariant& check_passed,
                            const QVariant& num_pos,
                            const QVariant& num_no_ref,
                            const QVariant& num_pos_inside,
                            const QVariant& num_pos_outside,
                            const QVariant& num_comp_passed,
                            const QVariant& num_comp_failed,
                            const std::string& comment)
    {
        details.push_back(Detail(ts, tst_pos).setValue(Result::DetailPosInside, pos_inside.isValid() ? pos_inside : "false")
                                             .setValue(Result::DetailValue, offset.isValid() ? offset : 0.0f)
                                             .setValue(Result::DetailCheckPassed, check_passed)
                                             .setValue(Result::DetailNumPos, num_pos)
                                             .setValue(Result::DetailNumNoRef, num_no_ref)
                                             .setValue(Result::DetailNumInside, num_pos_inside)
                                             .setValue(Result::DetailNumOutside, num_pos_outside)
                                             .setValue(Result::DetailNumCheckPassed, num_comp_passed)
                                             .setValue(Result::DetailNumCheckFailed, num_comp_failed)
                                             .addPosition(ref_pos)
                                             .generalComment(comment));
    };

    for (const auto& tst_id : tst_data)
    {
        ++num_pos;

        timestamp = tst_id.first;
        tst_pos = target_data.tstPos(tst_id);

        comp_passed = false;

        if (!target_data.hasMappedRefData(tst_id, max_ref_time_diff))
        {
            if (!skip_no_data_details)
                addDetail(timestamp, tst_pos,
                            {}, // ref_pos
                            {}, {}, comp_passed, // pos_inside, value, check_passed
                            num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                            num_comp_passed, num_comp_failed,
                            "No reference data");

            ++num_no_ref;
            continue;
        }

        ret_pos = target_data.mappedRefPos(tst_id, max_ref_time_diff);

        ref_pos = ret_pos.first;
        ok = ret_pos.second;

        if (!ok)
        {
            if (!skip_no_data_details)
                addDetail(timestamp, tst_pos,
                            {}, // ref_pos
                            {}, {}, comp_passed, // pos_inside, value, check_passed
                            num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                            num_comp_passed, num_comp_failed,
                            "No reference position");

            ++num_no_ref;
            continue;
        }

        is_inside = target_data.mappedRefPosInside(sector_layer, tst_id);

        if (!is_inside)
        {
            if (!skip_no_data_details)
                addDetail(timestamp, tst_pos,
                            ref_pos, // ref_pos
                            is_inside, {}, comp_passed, // pos_inside, value, check_passed
                            num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                            num_comp_passed, num_comp_failed, 
                            "Outside sector");
            ++num_pos_outside;
            continue;
        }
        ++num_pos_inside;

        local.SetStereographic(ref_pos.latitude_, ref_pos.longitude_, 1.0, 0.0, 0.0);

        ogr_geo2cart.reset(OGRCreateCoordinateTransformation(&wgs84, &local));

//        if (in_appimage_) // inside appimage
//        {
            x_pos = tst_pos.longitude_;
            y_pos = tst_pos.latitude_;
//        }
//        else
//        {
//            x_pos = tst_pos.latitude_;
//            y_pos = tst_pos.longitude_;
//        }

        ok = ogr_geo2cart->Transform(1, &x_pos, &y_pos); // wgs84 to cartesian offsets
        if (!ok)
        {
            addDetail(timestamp, tst_pos,
                        ref_pos, // ref_pos
                        is_inside, {}, comp_passed, // pos_inside, value, check_passed
                        num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                        num_comp_passed, num_comp_failed, 
                        "Position transformation error");
            ++num_pos_calc_errors;
            continue;
        }

        distance = sqrt(pow(x_pos,2)+pow(y_pos,2));

        if (std::isnan(distance) || std::isinf(distance))
        {
            addDetail(timestamp, tst_pos,
                        ref_pos, // ref_pos
                        is_inside, {}, comp_passed, // pos_inside, value, check_passed
                        num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                        num_comp_passed, num_comp_failed, 
                        "Distance Invalid");
            ++num_pos_calc_errors;
            continue;
        }

        ++num_distances;

        if (compareValue(fabs(distance), threshold_value_, threshold_value_check_type_))
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

        addDetail(timestamp, tst_pos,
                    ref_pos,
                    is_inside, distance, comp_passed, // pos_inside, value, check_passed
                    num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                    num_comp_passed, num_comp_failed,
                    comment);

        values.push_back(distance);
    }

    //        logdbg << "EvaluationRequirementPositionDistance '" << name_ << "': evaluate: utn " << target_data.utn_
    //               << " num_pos " << num_pos << " num_no_ref " <<  num_no_ref
    //               << " num_pos_outside " << num_pos_outside << " num_pos_inside " << num_pos_inside
    //               << " num_pos_ok " << num_pos_ok << " num_pos_nok " << num_pos_nok
    //               << " num_distances " << num_distances;

    assert (num_no_ref <= num_pos);

    if (num_pos - num_no_ref != num_pos_inside + num_pos_outside)
        loginf << "EvaluationRequirementPositionDistance '" << name_ << "': evaluate: utn " << target_data.utn_
               << " num_pos " << num_pos << " num_no_ref " <<  num_no_ref
               << " num_pos_outside " << num_pos_outside << " num_pos_inside " << num_pos_inside
               << " num_pos_calc_errors " << num_pos_calc_errors
               << " num_distances " << num_distances;

    assert (num_pos - num_no_ref == num_pos_inside + num_pos_outside);

    assert (num_distances == num_comp_failed + num_comp_passed);
    assert (num_distances == values.size());

    //assert (details.size() == num_pos);

    return make_shared<EvaluationRequirementResult::SinglePositionDistance>(
                "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                eval_man_, details, num_pos, num_no_ref, num_pos_outside, num_pos_inside, num_comp_passed, num_comp_failed,
                values);
}

}
