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
#include "eval/results/position/along.h"
#include "evaluationmanager.h"
#include "logger.h"
#include "util/timeconv.h"
#include "sectorlayer.h"

#include <algorithm>

using namespace std;
using namespace Utils;
using namespace boost::posix_time;

namespace EvaluationRequirement
{

PositionAlong::PositionAlong(const std::string& name, const std::string& short_name, const std::string& group_name,
                             double prob, COMPARISON_TYPE prob_check_type, EvaluationCalculator& calculator,
                             float max_abs_value)
    : ProbabilityBase(name, short_name, group_name, prob, prob_check_type, false, calculator), 
      max_abs_value_(max_abs_value)
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

    time_duration max_ref_time_diff = Time::partialSeconds(calculator_.settings().max_ref_time_diff_);

    const auto& tst_data = target_data.tstChain().timestampIndexes();

    unsigned int num_pos {0};
    unsigned int num_no_ref {0};
    unsigned int num_pos_outside {0};
    unsigned int num_pos_inside {0};
    unsigned int num_pos_calc_errors {0};
    unsigned int num_value_ok {0};
    unsigned int num_value_nok {0};

    typedef EvaluationRequirementResult::SinglePositionAlong Result;
    typedef EvaluationDetail                                 Detail;
    typedef Result::EvaluationDetails                        Details;
    Details details;

    ptime timestamp;

    Transformation ogr_geo2cart;

    dbContent::TargetPosition tst_pos;

    double d_along;

    bool is_inside;
    boost::optional<dbContent::TargetPosition> ref_pos;
    boost::optional<dbContent::TargetVelocity> ref_spd;

    bool along_ok;

    unsigned int num_distances {0};
    string comment;

    bool skip_no_data_details = calculator_.settings().report_skip_no_data_details_;

    auto addDetail = [ & ] (const ptime& ts,
                            const dbContent::TargetPosition& tst_pos,
                            const boost::optional<dbContent::TargetPosition>& ref_pos,
                            const QVariant& pos_inside,
                            const QVariant& offset,
                            const QVariant& check_passed,
                            const QVariant& num_pos,
                            const QVariant& num_no_ref,
                            const QVariant& num_pos_inside,
                            const QVariant& num_pos_outside,
                            const QVariant& num_value_ok,
                            const QVariant& num_value_nok,
                            const std::string& comment)
    {
        details.push_back(Detail(ts, tst_pos).setValue(Result::DetailKey::PosInside, pos_inside.isValid() ? pos_inside : "false")
                                             .setValue(Result::DetailKey::Value, offset)
                                             .setValue(Result::DetailKey::CheckPassed, check_passed)
                                             .setValue(Result::DetailKey::NumPos, num_pos)
                                             .setValue(Result::DetailKey::NumNoRef, num_no_ref)
                                             .setValue(Result::DetailKey::NumInside, num_pos_inside)
                                             .setValue(Result::DetailKey::NumOutside, num_pos_outside)
                                             .setValue(Result::DetailKey::NumCheckPassed, num_value_ok)
                                             .setValue(Result::DetailKey::NumCheckFailed, num_value_nok)
                                             .addPosition(ref_pos)
                                             .generalComment(comment));
    };

    for (const auto& tst_id : tst_data)
    {
        ++num_pos;

        timestamp = tst_id.first;
        tst_pos = target_data.tstChain().pos(tst_id);

        along_ok = true;

        if (!target_data.hasMappedRefData(tst_id, max_ref_time_diff))
        {
            if (!skip_no_data_details)
                addDetail(timestamp, tst_pos,
                            {}, // ref_pos
                            {}, {}, along_ok, // pos_inside, value, value_ok,
                            num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                            num_value_ok, num_value_nok,
                            "No reference data");

            ++num_no_ref;
            continue;
        }

        ref_pos = target_data.mappedRefPos(tst_id, max_ref_time_diff);

        if (!ref_pos.has_value())
        {
            if (!skip_no_data_details)
                addDetail(timestamp, tst_pos,
                            {}, // ref_pos
                            {}, {}, along_ok, // pos_inside, value, value_ok
                            num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                            num_value_ok, num_value_nok,
                            "No reference position");

            ++num_no_ref;
            continue;
        }

        ref_spd = target_data.mappedRefSpeed(tst_id, max_ref_time_diff);

        if (!ref_spd.has_value())
        {
            if (!skip_no_data_details)
                addDetail(timestamp, tst_pos,
                            {}, // ref_pos
                            {}, {}, along_ok, // pos_inside, value, value_ok
                            num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                            num_value_ok, num_value_nok,
                            "No reference speed");

            ++num_no_ref;
            continue;
        }

        is_inside = target_data.isTimeStampNotExcluded(timestamp)
                    && target_data.mappedRefPosInside(sector_layer, tst_id);

        if (!is_inside)
        {
            if (!skip_no_data_details)
                addDetail(timestamp, tst_pos,
                            ref_pos, // ref_pos
                            is_inside, {}, along_ok, // pos_inside, value, value_ok
                            num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                            num_value_ok, num_value_nok,
                            "Outside sector");
            ++num_pos_outside;
            continue;
        }
        ++num_pos_inside;

        bool   transform_ok;
        double distance, angle;

        std::tie(transform_ok, distance, angle) = ogr_geo2cart.distanceAngleCart(ref_pos->latitude_, ref_pos->longitude_, tst_pos.latitude_, tst_pos.longitude_);
        assert(transform_ok);

        angle = ref_spd->track_angle_ - angle;

        if (std::isnan(distance) || std::isinf(distance))
        {
            addDetail(timestamp, tst_pos,
                        ref_pos, // ref_pos
                        is_inside, {}, along_ok, // pos_inside, value, value_ok
                        num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                        num_value_ok, num_value_nok,
                        "Distance Invalid");
            ++num_pos_calc_errors;
            continue;
        }

        if (std::isnan(angle) || std::isinf(angle))
        {
            addDetail(timestamp, tst_pos,
                        ref_pos, // ref_pos
                        is_inside, {}, along_ok, // pos_inside, value, value_ok
                        num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                        num_value_ok, num_value_nok,
                        "Angle Invalid");
            ++num_pos_calc_errors;
            continue;
        }

        d_along = distance * cos(angle);
        assert (!std::isnan(d_along) && !std::isinf(d_along));

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

        addDetail(timestamp, tst_pos,
                    ref_pos,
                    is_inside, d_along, along_ok, // pos_inside, value, value_ok
                    num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                    num_value_ok, num_value_nok,
                    comment);
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
    assert (num_distances == num_value_ok + num_value_nok);

    return std::make_shared<EvaluationRequirementResult::SinglePositionAlong>(
                "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                calculator_, details, num_pos, num_no_ref, num_pos_outside, num_pos_inside, num_value_ok, num_value_nok);
}

}
