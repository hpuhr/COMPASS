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

#include "eval/requirement/mode_c/correct.h"
#include "eval/results/mode_c/correct.h"

#include "evaluationmanager.h"
#include "sectorlayer.h"

#include "util/timeconv.h"
#include "logger.h"

using namespace std;
using namespace Utils;
using namespace boost::posix_time;

namespace EvaluationRequirement
{

ModeCCorrect::ModeCCorrect(
        const std::string& name, const std::string& short_name, const std::string& group_name,
        double prob, COMPARISON_TYPE prob_check_type, EvaluationCalculator& calculator,
        float max_distance_ft)
    : ProbabilityBase(name, short_name, group_name, prob, prob_check_type, false, calculator),
      max_distance_ft_(max_distance_ft)
{

}

std::shared_ptr<EvaluationRequirementResult::Single> ModeCCorrect::evaluate (
        const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
        const SectorLayer& sector_layer)
{
    logdbg << "EvaluationRequirementModeC '" << name_ << "': evaluate: utn " << target_data.utn_;

    typedef EvaluationRequirementResult::SingleModeCCorrect Result;
    typedef EvaluationDetail                                         Detail;
    typedef Result::EvaluationDetails                                Details;

    if (target_data.isPrimaryOnly())
    {
        logdbg << "EvaluationRequirementModeC '" << name_ << "': evaluate: utn " << target_data.utn_
               << " ignored since primary only";

        return make_shared<EvaluationRequirementResult::SingleModeCCorrect>(
                    "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                    calculator_, Details(), 0, 0, 0, 0, 0, 0, 0);
    }

    time_duration max_ref_time_diff = Time::partialSeconds(calculator_.settings().max_ref_time_diff_);

    const auto& tst_data = target_data.tstChain().timestampIndexes();

    ptime timestamp;

    unsigned int num_updates {0};
    unsigned int num_no_ref_pos {0};
    unsigned int num_no_ref_id {0};
    unsigned int num_pos_outside {0};
    unsigned int num_pos_inside {0};
    unsigned int num_correct {0};
    unsigned int num_not_correct {0};

    Details details;

    dbContent::TargetPosition pos_current;

    bool ref_exists;
    bool is_inside;
    //pair<dbContent::TargetPosition, bool> ret_pos;
    boost::optional<dbContent::TargetPosition> ref_pos;
    bool ok;

    string comment;

    bool skip_no_data_details = calculator_.settings().report_skip_no_data_details_;
    bool skip_detail;

    ValueComparisonResult cmp_res_mc;
    string cmp_res_mc_comment;
    bool mc_no_ref;
    bool mc_correct_failed;

    bool result_ok;

    auto addDetail = [ & ] (const ptime& ts,
                            const dbContent::TargetPosition& tst_pos,
                            const boost::optional<dbContent::TargetPosition>& ref_pos,
                            const QVariant& ref_exists,
                            const QVariant& pos_inside,
                            const QVariant& is_not_correct,
                            const QVariant& num_updates,
                            const QVariant& num_no_ref,
                            const QVariant& num_pos_inside,
                            const QVariant& num_pos_outside,
                            const QVariant& num_correct,
                            const QVariant& num_not_correct,
                            const std::string& comment)
    {
        details.push_back(Detail(ts, tst_pos).setValue(Result::DetailKey::RefExists, ref_exists)
                                             .setValue(Result::DetailKey::PosInside, pos_inside.isValid() ? pos_inside : "false")
                                             .setValue(Result::DetailKey::IsNotCorrect, is_not_correct)
                                             .setValue(Result::DetailKey::NumUpdates, num_updates)
                                             .setValue(Result::DetailKey::NumNoRef, num_no_ref)
                                             .setValue(Result::DetailKey::NumInside, num_pos_inside)
                                             .setValue(Result::DetailKey::NumOutside, num_pos_outside)
                                             .setValue(Result::DetailKey::NumCorrect, num_correct)
                                             .setValue(Result::DetailKey::NumNotCorrect, num_not_correct)
                                             .addPosition(ref_pos)
                                             .generalComment(comment));
    };

    for (const auto& tst_id : tst_data)
    {
        ref_exists = false;
        is_inside = false;
        comment = "";

        skip_detail = false;

        ++num_updates;

        timestamp = tst_id.first;
        pos_current = target_data.tstChain().pos(tst_id);

        if (!target_data.hasMappedRefData(tst_id, max_ref_time_diff))
        {
            if (!skip_no_data_details)
                addDetail(timestamp, pos_current, {},
                            false, {}, false, // ref_exists, pos_inside, is_not_correct
                            num_updates, num_no_ref_pos, num_pos_inside, num_pos_outside,
                            num_correct, num_not_correct, "No reference data");

            ++num_no_ref_pos;
            continue;
        }

        ref_pos = target_data.mappedRefPos(tst_id, max_ref_time_diff);

//        ref_pos = ret_pos.first;
//        ok = ret_pos.second;

        if (!ref_pos.has_value())
        {
            if (!skip_no_data_details)
                addDetail(timestamp, pos_current, {},
                            false, {}, false, // ref_exists, pos_inside, is_not_correct
                            num_updates, num_no_ref_pos, num_pos_inside, num_pos_outside,
                            num_correct, num_not_correct, "No reference position");

            ++num_no_ref_pos;
            continue;
        }
        ref_exists = true;

        is_inside = target_data.mappedRefPosInside(sector_layer, tst_id);

        if (!is_inside)
        {
            if (!skip_no_data_details)
                addDetail(timestamp, pos_current, ref_pos,
                            ref_exists, is_inside, false, // ref_exists, pos_inside, is_not_correct
                            num_updates, num_no_ref_pos, num_pos_inside, num_pos_outside,
                            num_correct, num_not_correct, "Outside sector");

            ++num_pos_outside;
            continue;
        }
        ++num_pos_inside;

        tie(cmp_res_mc, cmp_res_mc_comment) = compareModeC(tst_id, target_data, max_ref_time_diff, max_distance_ft_);

        mc_no_ref = cmp_res_mc == ValueComparisonResult::Unknown_NoRefData;

        if (mc_no_ref) // none has a reference
        {
            if (!skip_no_data_details)
                addDetail(timestamp, pos_current, ref_pos,
                            ref_exists, is_inside, false, // ref_exists, pos_inside, is_not_correct
                            num_updates, num_no_ref_pos, num_pos_inside, num_pos_outside,
                            num_correct, num_not_correct, "No reference Mode C");

            ++num_no_ref_id;

            continue;
        }

        mc_correct_failed = cmp_res_mc == ValueComparisonResult::Unknown_NoTstData
                || cmp_res_mc == ValueComparisonResult::Different;

        if (mc_correct_failed)
        {
            if (comment.size())
                comment += ", ";
            comment += "MC failed (" + cmp_res_mc_comment + ")";
        }

        result_ok = !mc_correct_failed;

        if (result_ok)
            ++num_correct;
        else
            ++num_not_correct;

        if (!skip_detail)
            addDetail(timestamp, pos_current, ref_pos,
                        ref_exists, is_inside, !result_ok,
                        num_updates, num_no_ref_pos, num_pos_inside, num_pos_outside,
                        num_correct, num_not_correct, comment);
    }

    logdbg << "EvaluationRequirementModeC '" << name_ << "': evaluate: utn " << target_data.utn_
           << " num_updates " << num_updates << " num_no_ref_pos " << num_no_ref_pos
           << " num_no_ref_id " << num_no_ref_id
           << " num_pos_outside " << num_pos_outside << " num_pos_inside " << num_pos_inside
           << " num_correct " << num_correct << " num_not_correct " << num_not_correct;

    assert (num_updates - num_no_ref_pos == num_pos_inside + num_pos_outside);
    assert (num_pos_inside == num_no_ref_id+num_correct+num_not_correct);

    return make_shared<EvaluationRequirementResult::SingleModeCCorrect>(
                "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                calculator_, details, num_updates, num_no_ref_pos, num_no_ref_id, num_pos_outside, num_pos_inside,
                num_correct, num_not_correct);
}

float ModeCCorrect::maxDistanceFt() const
{
    return max_distance_ft_;
}

}
