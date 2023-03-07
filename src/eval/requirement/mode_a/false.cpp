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

#include "eval/requirement/mode_a/false.h"
#include "eval/results/mode_a/falsesingle.h"
#include "eval/requirement/checkdetail.h"
#include "evaluationdata.h"
#include "evaluationmanager.h"
#include "logger.h"
#include "util/stringconv.h"
#include "util/timeconv.h"
#include "sectorlayer.h"

using namespace std;
using namespace Utils;
using namespace boost::posix_time;

namespace EvaluationRequirement
{

    ModeAFalse::ModeAFalse(const std::string& name, const std::string& short_name, const std::string& group_name,
                 float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man)
        : Base(name, short_name, group_name, prob, prob_check_type, eval_man)
    {
    }

    std::shared_ptr<EvaluationRequirementResult::Single> ModeAFalse::evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
            const SectorLayer& sector_layer)
    {
        logdbg << "EvaluationRequirementModeAFalse '" << name_ << "': evaluate: utn " << target_data.utn_;

        time_duration max_ref_time_diff = Time::partialSeconds(eval_man_.maxRefTimeDiff());

        const std::multimap<ptime, unsigned int>& tst_data = target_data.tstData();

        ptime timestamp;

        int num_updates {0};
        int num_no_ref_pos {0};
        int num_no_ref_val {0};
        int num_pos_outside {0};
        int num_pos_inside {0};
        int num_unknown {0};
        int num_correct {0};
        int num_false {0};

        vector<CheckDetail> details;
        EvaluationTargetPosition pos_current;
        bool code_ok;

        bool ref_exists;
        bool is_inside;
        pair<EvaluationTargetPosition, bool> ret_pos;
        EvaluationTargetPosition ref_pos;
        bool ok;

        ValueComparisonResult cmp_res;
        string comment;

        bool skip_no_data_details = eval_man_.reportSkipNoDataDetails();
        bool skip_detail;

        bool has_ground_bit;
        bool ground_bit_set;

        for (const auto& tst_id : tst_data)
        {
            ref_exists = false;
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
                                       false, {}, false, // ref_exists, pos_inside,
                                       num_updates, num_no_ref_pos+num_no_ref_val, num_pos_inside, num_pos_outside,
                                       num_unknown, num_correct, num_false, "No reference data"});

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
                                       false, {}, false, // ref_exists, pos_inside,
                                       num_updates, num_no_ref_pos+num_no_ref_val, num_pos_inside, num_pos_outside,
                                       num_unknown, num_correct, num_false, "No reference position"});

                ++num_no_ref_pos;
                continue;
            }
            ref_exists = true;

            has_ground_bit = target_data.hasTstGroundBitForTime(timestamp);

            if (has_ground_bit)
                ground_bit_set = target_data.tstGroundBitForTime(timestamp);
            else
                ground_bit_set = false;

            if (!ground_bit_set)
                tie(has_ground_bit, ground_bit_set) = target_data.interpolatedRefGroundBitForTime(
                            timestamp, seconds(15));

            is_inside = sector_layer.isInside(ref_pos, has_ground_bit, ground_bit_set);

            if (!is_inside)
            {
                if (!skip_no_data_details)
                    details.push_back({timestamp, pos_current,
                                       ref_exists, is_inside, false, // ref_exists, pos_inside,
                                       num_updates, num_no_ref_pos+num_no_ref_val, num_pos_inside, num_pos_outside,
                                       num_unknown, num_correct, num_false, "Outside sector"});

                ++num_pos_outside;
                continue;
            }
            ++num_pos_inside;

            tie(cmp_res, comment) = compareModeA(timestamp, target_data, max_ref_time_diff);

            code_ok = true;
            if (cmp_res == ValueComparisonResult::Unknown_NoRefData)
            {
                if (skip_no_data_details)
                    skip_detail = true;

                ++num_no_ref_val;
            }
            else if (cmp_res == ValueComparisonResult::Unknown_NoTstData)
            {
                if (skip_no_data_details)
                    skip_detail = true;

                ++num_unknown;
            }
            else if (cmp_res == ValueComparisonResult::Same)
            {
                ++num_correct;
            }
            else if (cmp_res == ValueComparisonResult::Different)
            {
                code_ok = false;
                ++num_false;
            }
            else
                throw runtime_error("EvaluationRequirementModeAFalse: evaluate: unknown compare result "
                                    +to_string(cmp_res));

            if (!skip_detail)
                details.push_back({timestamp, pos_current,
                                   ref_exists, is_inside, !code_ok,
                                   num_updates, num_no_ref_pos+num_no_ref_val, num_pos_inside, num_pos_outside,
                                   num_unknown, num_correct, num_false, comment});
        }

        logdbg << "EvaluationRequirementModeAFalse '" << name_ << "': evaluate: utn " << target_data.utn_
               << " num_updates " << num_updates << " num_no_ref_pos " << num_no_ref_pos
               << " num_no_ref_val " << num_no_ref_val
               << " num_pos_outside " << num_pos_outside << " num_pos_inside " << num_pos_inside
               << " num_unknown " << num_unknown << " num_correct " << num_correct
               << " num_false " << num_false;

        assert (num_updates - num_no_ref_pos == num_pos_inside + num_pos_outside);
        assert (num_pos_inside == num_no_ref_val+num_unknown+num_correct+num_false);

        //assert (details.size() == tst_data.size());

        return make_shared<EvaluationRequirementResult::SingleModeAFalse>(
                    "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                    eval_man_, num_updates, num_no_ref_pos, num_no_ref_val, num_pos_outside, num_pos_inside,
                    num_unknown, num_correct, num_false, details);
    }

}
