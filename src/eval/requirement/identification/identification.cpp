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

#include "eval/requirement/identification/identification.h"
#include "eval/results/identification/single.h"
#include "eval/requirement/correctnessdetail.h"
#include "evaluationdata.h"
#include "evaluationmanager.h"
#include "logger.h"
#include "stringconv.h"
#include "sectorlayer.h"

using namespace std;
using namespace Utils;

namespace EvaluationRequirement
{

Identification::Identification(
        const std::string& name, const std::string& short_name, const std::string& group_name,
        float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man,
        bool require_correctness_of_all, bool use_mode_a, bool use_ms_ta, bool use_ms_ti)
    : Base(name, short_name, group_name, prob, prob_check_type, eval_man),
      require_correctness_of_all_(require_correctness_of_all),
      use_mode_a_(use_mode_a), use_ms_ta_(use_ms_ta), use_ms_ti_(use_ms_ti)
{

}

std::shared_ptr<EvaluationRequirementResult::Single> Identification::evaluate (
        const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
        const SectorLayer& sector_layer)
{
    logdbg << "EvaluationRequirementIdentification '" << name_ << "': evaluate: utn " << target_data.utn_;

    float max_ref_time_diff = eval_man_.maxRefTimeDiff();

    const std::multimap<float, unsigned int>& tst_data = target_data.tstData();

    float tod{0};

    unsigned int num_updates {0};
    unsigned int num_no_ref_pos {0};
    unsigned int num_no_ref_id {0};
    unsigned int num_pos_outside {0};
    unsigned int num_pos_inside {0};
    unsigned int num_correct {0};
    unsigned int num_not_correct {0};

    vector<CorrectnessDetail> details;
    EvaluationTargetPosition pos_current;

    bool ref_exists;
    bool is_inside;
    pair<EvaluationTargetPosition, bool> ret_pos;
    EvaluationTargetPosition ref_pos;
    bool ok;

    string comment;

    bool skip_no_data_details = eval_man_.resultsGenerator().skipNoDataDetails();
    bool skip_detail;

    bool has_ground_bit;
    bool ground_bit_set;

    ValueComparisonResult cmp_res_ti;
    string cmp_res_ti_comment;
    bool ti_correct_failed;

    ValueComparisonResult cmp_res_ta;
    string cmp_res_ta_comment;
    bool ta_correct_failed;

    ValueComparisonResult cmp_res_ma;
    string cmp_res_ma_comment;
    bool ma_correct_failed;

    bool any_correct;
    bool all_correct;
    bool result_ok;

    for (const auto& tst_id : tst_data)
    {
        ref_exists = false;
        is_inside = false;
        comment = "";

        skip_detail = false;

        ++num_updates;

        tod = tst_id.first;
        pos_current = target_data.tstPosForTime(tod);

        if (!target_data.hasRefDataForTime (tod, max_ref_time_diff))
        {
            if (!skip_no_data_details)
                details.push_back({tod, pos_current,
                                   false, {}, false, // ref_exists, pos_inside, is_not_correct
                                   num_updates, num_no_ref_pos, num_pos_inside, num_pos_outside,
                                   num_correct, num_not_correct, "No reference data"});

            ++num_no_ref_pos;
            continue;
        }

        ret_pos = target_data.interpolatedRefPosForTime(tod, max_ref_time_diff);

        ref_pos = ret_pos.first;
        ok = ret_pos.second;

        if (!ok)
        {
            if (!skip_no_data_details)
                details.push_back({tod, pos_current,
                                   false, {}, false, // ref_exists, pos_inside, is_not_correct
                                   num_updates, num_no_ref_pos, num_pos_inside, num_pos_outside,
                                   num_correct, num_not_correct, "No reference position"});

            ++num_no_ref_pos;
            continue;
        }
        ref_exists = true;

        has_ground_bit = target_data.hasTstGroundBitForTime(tod);

        if (has_ground_bit)
            ground_bit_set = target_data.tstGroundBitForTime(tod);
        else
            ground_bit_set = false;

        is_inside = sector_layer.isInside(ref_pos, has_ground_bit, ground_bit_set);

        if (!is_inside)
        {
            if (!skip_no_data_details)
                details.push_back({tod, pos_current,
                                   ref_exists, is_inside, false, // ref_exists, pos_inside, is_not_correct
                                   num_updates, num_no_ref_pos+num_no_ref_id, num_pos_inside, num_pos_outside,
                                   num_correct, num_not_correct, "Outside sector"});

            ++num_pos_outside;
            continue;
        }
        ++num_pos_inside;


        tie(cmp_res_ti, cmp_res_ti_comment) = compareTi(tod, target_data, max_ref_time_diff);
        tie(cmp_res_ta, cmp_res_ta_comment) = compareTa(tod, target_data, max_ref_time_diff);
        tie(cmp_res_ma, cmp_res_ma_comment) = compareModeA(tod, target_data, max_ref_time_diff);

        any_correct = false;
        all_correct = true;

        if (use_ms_ti_)
        {
            ti_correct_failed = cmp_res_ti == ValueComparisonResult::Unknown_NoTstData
                    || cmp_res_ti == ValueComparisonResult::Different;

            any_correct |= !ti_correct_failed;
            all_correct &= !ti_correct_failed;
        }

        if (use_ms_ta_)
        {
            ta_correct_failed = cmp_res_ta == ValueComparisonResult::Unknown_NoTstData
                    || cmp_res_ta == ValueComparisonResult::Different;

            any_correct |= !ta_correct_failed;
            all_correct &= !ta_correct_failed;
        }

        if (use_mode_a_)
        {
            ma_correct_failed = cmp_res_ma == ValueComparisonResult::Unknown_NoTstData
                    || cmp_res_ma == ValueComparisonResult::Different;

            any_correct |= !ma_correct_failed;
            all_correct &= !ma_correct_failed;
        }

        if (!use_ms_ti_ && !use_ms_ta_ && !use_mode_a_)
            all_correct = false; // if none is used, set to false

        if (require_correctness_of_all_)
        {
            result_ok = all_correct;
        }
        else // one correct ok
        {
            result_ok = any_correct;
        }

        if (result_ok)
            ++num_correct;
        else
            ++num_not_correct;

        if (!skip_detail)
            details.push_back({tod, pos_current,
                               ref_exists, is_inside, !result_ok,
                               num_updates, num_no_ref_pos, num_pos_inside, num_pos_outside,
                               num_correct, num_not_correct, comment});
    }

    logdbg << "EvaluationRequirementIdentification '" << name_ << "': evaluate: utn " << target_data.utn_
           << " num_updates " << num_updates << " num_no_ref_pos " << num_no_ref_pos
           << " num_no_ref_id " << num_no_ref_id
           << " num_pos_outside " << num_pos_outside << " num_pos_inside " << num_pos_inside
           << " num_correct " << num_correct << " num_not_correct " << num_not_correct;

    assert (num_updates - num_no_ref_pos == num_pos_inside + num_pos_outside);
    assert (num_pos_inside == num_no_ref_id+num_correct+num_not_correct);

    //assert (details.size() == tst_data.size());

    //        if (num_correct_id+num_false_id)
    //        {
    //            float pid = (float)num_correct_id/(float)(num_correct_id+num_false_id);

    //            logdbg << "EvaluationRequirementIdentification '" << name_ << "': evaluate: utn " << target_data.utn_
    //                   << " pid " << String::percentToString(100.0 * pid) << " passed " << (pid >= minimum_probability_);
    //        }
    //        else
    //            logdbg << "EvaluationRequirementIdentification '" << name_ << "': evaluate: utn " << target_data.utn_
    //                   << " no data for pid";

    return make_shared<EvaluationRequirementResult::SingleIdentification>(
                "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                eval_man_, num_updates, num_no_ref_pos, num_no_ref_id, num_pos_outside, num_pos_inside,
                num_correct, num_not_correct, details);
}

bool Identification::requireCorrectnessOfAll() const
{
    return require_correctness_of_all_;
}

bool Identification::useModeA() const
{
    return use_mode_a_;
}

bool Identification::useMsTa() const
{
    return use_ms_ta_;
}

bool Identification::useMsTi() const
{
    return use_ms_ti_;
}

}
