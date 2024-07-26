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

#include "eval/requirement/position/radarrange.h"
#include "eval/results/position/radarrange.h"
//#include "evaluationdata.h"
#include "evaluationmanager.h"
#include "logger.h"
#include "util/timeconv.h"
#include "sectorlayer.h"

#include "ogrprojection.h"
#include "projectionmanager.h"

#include <algorithm>

using namespace std;
using namespace Utils;
using namespace boost::posix_time;

namespace EvaluationRequirement
{

PositionRadarRange::PositionRadarRange(
        const std::string& name, const std::string& short_name, const std::string& group_name,
        EvaluationManager& eval_man, double threshold_value)
    : Base(name, short_name, group_name, threshold_value, COMPARISON_TYPE::LESS_THAN_OR_EQUAL, eval_man)
{
}

std::shared_ptr<EvaluationRequirementResult::Single> PositionRadarRange::evaluate (
        const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
        const SectorLayer& sector_layer)
{
    logdbg << "EvaluationRequirementPositionRadarRange '" << name_ << "': evaluate: utn " << target_data.utn_
           << " threshold_value " << threshold();

    time_duration max_ref_time_diff = Time::partialSeconds(eval_man_.settings().max_ref_time_diff_);

    const auto& tst_data = target_data.tstChain().timestampIndexes();

    unsigned int num_pos {0};
    unsigned int num_no_ref {0};
    unsigned int num_pos_outside {0};
    unsigned int num_pos_inside {0};
    unsigned int num_pos_calc_errors {0};
    unsigned int num_comp_failed {0};
    unsigned int num_comp_passed {0};

    typedef EvaluationRequirementResult::SinglePositionRadarRange Result;
    typedef EvaluationDetail                                    Detail;
    typedef Result::EvaluationDetails                           Details;
    Details details;

    ptime timestamp;

    dbContent::TargetPosition tst_pos;

    bool is_inside;
    boost::optional<dbContent::TargetPosition> ref_pos;
    bool ok;

    bool comp_passed;

    unsigned int num_distances {0};
    string comment;

    bool skip_no_data_details = eval_man_.settings().report_skip_no_data_details_;

    auto addDetail = [ & ] (const ptime& ts,
                            const dbContent::TargetPosition& tst_pos,
                            const boost::optional<dbContent::TargetPosition>& ref_pos,
                            const QVariant& pos_inside,
                            const QVariant& offset,
                            const QVariant& RangeRef,
                            const QVariant& RangeTst,
                            const QVariant& check_passed,
                            const QVariant& num_pos,
                            const QVariant& num_no_ref,
                            const QVariant& num_pos_inside,
                            const QVariant& num_pos_outside,
                            const QVariant& num_comp_passed,
                            const QVariant& num_comp_failed,
                            const std::string& comment)
    {
        details.push_back(Detail(ts, tst_pos).setValue(Result::DetailKey::PosInside, pos_inside.isValid() ? pos_inside : "false")
                                             .setValue(Result::DetailKey::Value, offset)
                                             .setValue(Result::DetailKeyAdditional::RangeRef, RangeRef)
                                             .setValue(Result::DetailKeyAdditional::RangeTst, RangeTst)
                                             .setValue(Result::DetailKey::CheckPassed, check_passed)
                                             .setValue(Result::DetailKey::NumPos, num_pos)
                                             .setValue(Result::DetailKey::NumNoRef, num_no_ref)
                                             .setValue(Result::DetailKey::NumInside, num_pos_inside)
                                             .setValue(Result::DetailKey::NumOutside, num_pos_outside)
                                             .setValue(Result::DetailKey::NumCheckPassed, num_comp_passed)
                                             .setValue(Result::DetailKey::NumCheckFailed, num_comp_failed)
                                             .addPosition(ref_pos)
                                             .generalComment(comment));
    };

    ProjectionManager& proj_man = ProjectionManager::instance();

    OGRProjection& projection = proj_man.ogrProjection();

    if (!projection.radarCoordinateSystemsAdded())
        projection.addAllRadarCoordinateSystems();

    unsigned int tst_ds_id;

    double ref_range_m, ref_azm_deg, tst_range_m, tst_azm_deg;
    double range_m_diff;

    for (const auto& tst_id : tst_data)
    {
        ++num_pos;

        tst_ds_id = target_data.tstChain().dsID(tst_id);

        if (!projection.hasCoordinateSystem(tst_ds_id))
        {
            if (!skip_no_data_details)
                addDetail(timestamp, tst_pos,
                            {}, // ref_pos
                            {}, {}, {}, {}, comp_passed, // pos_inside, value, range_ref, range_tst, check_passed
                            num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                            num_comp_passed, num_comp_failed,
                            "No data source info");

            continue;
        }

        timestamp = tst_id.first;
        tst_pos = target_data.tstChain().pos(tst_id);

        comp_passed = false;

        if (!target_data.hasMappedRefData(tst_id, max_ref_time_diff))
        {
            if (!skip_no_data_details)
                addDetail(timestamp, tst_pos,
                            {}, // ref_pos
                            {}, {}, {}, {}, comp_passed, // pos_inside, value, range_ref, range_tst, check_passed
                            num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                            num_comp_passed, num_comp_failed,
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
                            {}, {}, {}, {}, comp_passed, // pos_inside, value, range_ref, range_tst, check_passed
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
                            is_inside, {}, {}, {}, comp_passed, // pos_inside, value, range_ref, range_tst, check_passed
                            num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                            num_comp_passed, num_comp_failed, 
                            "Outside sector");
            ++num_pos_outside;
            continue;
        }
        ++num_pos_inside;

        ok = projection.wgs842PolarHorizontal(tst_ds_id, ref_pos->latitude_, ref_pos->longitude_, ref_azm_deg, ref_range_m);
        if (!ok)
        {
            addDetail(timestamp, tst_pos,
                      ref_pos, // ref_pos
                      is_inside, {}, {}, {}, comp_passed, // pos_inside, value, range_ref, range_tst, check_passed
                      num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                      num_comp_passed, num_comp_failed,
                      "Ref. position transformation error");
            ++num_pos_calc_errors;
            continue;
        }

        ok = projection.wgs842PolarHorizontal(tst_ds_id, tst_pos.latitude_, tst_pos.longitude_, tst_azm_deg, tst_range_m);
        if (!ok)
        {
            addDetail(timestamp, tst_pos,
                      ref_pos, // ref_pos
                      is_inside, {}, {}, {}, comp_passed, // pos_inside, value, range_ref, range_tst, check_passed
                      num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                      num_comp_passed, num_comp_failed,
                      "Tst. position transformation error");
            ++num_pos_calc_errors;
            continue;
        }

        range_m_diff = ref_range_m - tst_range_m;

        if (std::isnan(range_m_diff) || std::isinf(range_m_diff))
        {
            addDetail(timestamp, tst_pos,
                        ref_pos, // ref_pos
                        is_inside, {}, {}, {}, comp_passed, // pos_inside, value, range_ref, range_tst, check_passed
                        num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                        num_comp_passed, num_comp_failed, 
                        "Range Invalid");
            ++num_pos_calc_errors;
            continue;
        }

        ++num_distances;

        if (fabs(range_m_diff) <= threshold()) // for single value
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
                    is_inside, range_m_diff, ref_range_m, tst_range_m, comp_passed, // pos_inside, value, range_ref, range_tst, check_passed
                    num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                    num_comp_passed, num_comp_failed,
                    comment);
    }

    //        logdbg << "EvaluationRequirementPositionRange '" << name_ << "': evaluate: utn " << target_data.utn_
    //               << " num_pos " << num_pos << " num_no_ref " <<  num_no_ref
    //               << " num_pos_outside " << num_pos_outside << " num_pos_inside " << num_pos_inside
    //               << " num_pos_ok " << num_pos_ok << " num_pos_nok " << num_pos_nok
    //               << " num_distances " << num_distances;

    assert (num_no_ref <= num_pos);

    if (num_pos - num_no_ref != num_pos_inside + num_pos_outside)
        loginf << "EvaluationRequirementPositionRadarRange '" << name_ << "': evaluate: utn " << target_data.utn_
               << " num_pos " << num_pos << " num_no_ref " <<  num_no_ref
               << " num_pos_outside " << num_pos_outside << " num_pos_inside " << num_pos_inside
               << " num_pos_calc_errors " << num_pos_calc_errors
               << " num_distances " << num_distances;

    assert (num_pos - num_no_ref == num_pos_inside + num_pos_outside);

    assert (num_distances == num_comp_failed + num_comp_passed);
    
    //assert (tst_range_values.size() == ref_range_values.size());

    return make_shared<EvaluationRequirementResult::SinglePositionRadarRange>(
                "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                eval_man_, details, num_pos, num_no_ref, num_pos_outside, num_pos_inside, num_comp_passed, num_comp_failed);
}

}
