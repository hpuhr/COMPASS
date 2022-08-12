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

#include "eval/requirement/mode_a/present.h"
#include "eval/results/mode_a/presentsingle.h"
#include "eval/requirement/checkdetail.h"
#include "evaluationdata.h"
#include "evaluationmanager.h"
#include "logger.h"
#include "stringconv.h"
#include "sectorlayer.h"

using namespace std;
using namespace Utils;
using namespace boost::posix_time;

namespace EvaluationRequirement
{

ModeAPresent::ModeAPresent(const std::string& name, const std::string& short_name, const std::string& group_name,
                           float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man)
    : Base(name, short_name, group_name, prob, prob_check_type, eval_man)
{

}

std::shared_ptr<EvaluationRequirementResult::Single> ModeAPresent::evaluate (
        const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
        const SectorLayer& sector_layer)
{
    logdbg << "EvaluationRequirementModeA '" << name_ << "': evaluate: utn " << target_data.utn_;

    time_duration max_ref_time_diff = Time::partialSeconds(eval_man_.maxRefTimeDiff());

    const std::multimap<ptime, unsigned int>& tst_data = target_data.tstData();

    ptime timestamp;

    ptime ref_lower, ref_upper;

    int num_updates {0};
    int num_no_ref_pos {0};
    //int num_no_ref_val {0};
    int num_pos_outside {0};
    int num_pos_inside {0};

    int num_no_ref_id {0};
    int num_present_id {0};
    int num_missing_id {0};

    vector<PresentDetail> details;
    EvaluationTargetPosition pos_current;
    //unsigned int code;
    //bool code_ok;

    bool code_present_ref;
    bool code_present_tst;
    bool code_missing;

    //bool ref_exists;
    bool is_inside;
    pair<EvaluationTargetPosition, bool> ret_pos;
    EvaluationTargetPosition ref_pos;
    bool ok;

    string comment;
    //bool lower_nok, upper_nok;

    bool skip_no_data_details = eval_man_.reportSkipNoDataDetails();
    bool skip_detail;

    bool has_ground_bit;
    bool ground_bit_set;

    for (const auto& tst_id : tst_data)
    {
        //ref_exists = false;
        is_inside = false;
        comment = "";

        skip_detail = false;

        ++num_updates;

        timestamp = tst_id.first;
        pos_current = target_data.tstPosForTime(timestamp);

        if (!target_data.hasRefDataForTime (timestamp, max_ref_time_diff))
        {
            if (!skip_no_data_details)
                details.push_back({timestamp, pos_current,
                                   false, {}, false, // ref_exists, pos_inside, is_not_ok
                                   num_updates, num_no_ref_pos, num_pos_inside, num_pos_outside,
                                   num_no_ref_id, num_present_id, num_missing_id, "No reference data"});

            ++num_no_ref_pos;
            continue;
        }

        ret_pos = target_data.interpolatedRefPosForTime(timestamp, max_ref_time_diff);

        ref_pos = ret_pos.first;
        ok = ret_pos.second;

        if (!ok)
        {
            if (!skip_no_data_details)
                details.push_back({timestamp, pos_current,
                                   false, {}, false, // ref_exists, pos_inside, is_not_ok
                                   num_updates, num_no_ref_pos, num_pos_inside, num_pos_outside,
                                   num_no_ref_id, num_present_id, num_missing_id, "No reference position"});

            ++num_no_ref_pos;
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
                details.push_back({timestamp, pos_current,
                                   true, is_inside, false, // ref_exists, pos_inside, is_not_ok
                                   num_updates, num_no_ref_pos, num_pos_inside, num_pos_outside,
                                   num_no_ref_id, num_present_id, num_missing_id, "Outside sector"});

            ++num_pos_outside;
            continue;
        }

        ++num_pos_inside;

        // check if ref code exists
        code_present_ref = false;

        tie(ref_lower, ref_upper) = target_data.refTimesFor(timestamp, max_ref_time_diff);

        if ((!ref_lower.is_not_a_date_time() || !ref_upper.is_not_a_date_time())) // ref times possible
        {
            if ((!ref_lower.is_not_a_date_time() && target_data.hasRefModeAForTime(ref_lower))
                    || (!ref_upper.is_not_a_date_time() && target_data.hasRefModeAForTime(ref_upper))) // ref value(s) exist
            {
                code_present_ref = true;
            }
        }

        code_present_tst = target_data.hasTstModeAForTime(timestamp);

        code_missing = false;

        if (code_present_ref)
        {
            if (code_present_tst)
            {
                comment = "OK";
                ++num_present_id;
            }
            else
            {
                comment = "Id missing";
                ++num_missing_id;

                code_missing = true;
            }
        }
        else
        {
            comment = "No reference id";
            ++num_no_ref_id;

            // skip_detail?
        }

        if (!(skip_no_data_details && skip_detail))
            details.push_back({timestamp, pos_current,
                               true, is_inside, code_missing, // ref_exists, pos_inside, is_not_ok
                               num_updates, num_no_ref_pos, num_pos_inside, num_pos_outside,
                               num_no_ref_id, num_present_id, num_missing_id, comment});
    }

    logdbg << "EvaluationRequirementModeA '" << name_ << "': evaluate: utn " << target_data.utn_
           << " num_updates " << num_updates << " num_no_ref_pos " << num_no_ref_pos
              //<< " num_no_ref_val " << num_no_ref_val
           << " num_pos_outside " << num_pos_outside << " num_pos_inside " << num_pos_inside
           << " num_no_ref_id " << num_no_ref_id << " num_present_id " << num_present_id
           << " num_missing_id " << num_missing_id;

    assert (num_updates - num_no_ref_pos == num_pos_inside + num_pos_outside);
    assert (num_pos_inside == num_no_ref_id+num_present_id+num_missing_id);

    //assert (details.size() == tst_data.size());

    return make_shared<EvaluationRequirementResult::SingleModeAPresent>(
                "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                eval_man_, num_updates, num_no_ref_pos, num_pos_outside, num_pos_inside,
                num_no_ref_id, num_present_id, num_missing_id, details);
}

}
