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

#include "eval/requirement/mode_c/present.h"
#include "eval/results/mode_c/presentsingle.h"
#include "eval/requirement/checkdetail.h"
#include "evaluationdata.h"
#include "evaluationmanager.h"
#include "logger.h"
#include "stringconv.h"
#include "sectorlayer.h"

using namespace std;
using namespace Utils;

namespace EvaluationRequirement
{

    ModeCPresent::ModeCPresent(const std::string& name, const std::string& short_name, const std::string& group_name,
                 EvaluationManager& eval_man, float max_ref_time_diff, float minimum_probability_present)
        : Base(name, short_name, group_name, eval_man),
          max_ref_time_diff_(max_ref_time_diff),
          minimum_probability_present_(minimum_probability_present)
    {

    }

    std::shared_ptr<EvaluationRequirementResult::Single> ModeCPresent::evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
            const SectorLayer& sector_layer)
    {
        logdbg << "EvaluationRequirementModeC '" << name_ << "': evaluate: utn " << target_data.utn_
               << " min prob " << minimum_probability_present_;

        const std::multimap<float, unsigned int>& tst_data = target_data.tstData();

        float tod{0};

        float ref_lower{0}, ref_upper{0};

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

        bool skip_no_data_details = eval_man_.resultsGenerator().skipNoDataDetails();
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

            tod = tst_id.first;
            pos_current = target_data.tstPosForTime(tod);

            if (!target_data.hasRefDataForTime (tod, max_ref_time_diff_))
            {
                if (!skip_no_data_details)
                    details.push_back({tod, pos_current,
                                       false, {}, false, // ref_exists, pos_inside, is_not_ok
                                       num_updates, num_no_ref_pos, num_pos_inside, num_pos_outside,
                                       num_no_ref_id, num_present_id, num_missing_id, "No reference data"});

                ++num_no_ref_pos;
                continue;
            }

            ret_pos = target_data.interpolatedRefPosForTime(tod, max_ref_time_diff_);

            ref_pos = ret_pos.first;
            ok = ret_pos.second;

            if (!ok)
            {
                if (!skip_no_data_details)
                    details.push_back({tod, pos_current,
                                       false, {}, false, // ref_exists, pos_inside, is_not_ok
                                       num_updates, num_no_ref_pos, num_pos_inside, num_pos_outside,
                                       num_no_ref_id, num_present_id, num_missing_id, "No reference position"});

                ++num_no_ref_pos;
                continue;
            }
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
                                       true, is_inside, false, // ref_exists, pos_inside, is_not_ok
                                       num_updates, num_no_ref_pos, num_pos_inside, num_pos_outside,
                                       num_no_ref_id, num_present_id, num_missing_id, "Outside sector"});

                ++num_pos_outside;
                continue;
            }

            ++num_pos_inside;

            // check if ref code exists
            code_present_ref = false;

            tie(ref_lower, ref_upper) = target_data.refTimesFor(tod, max_ref_time_diff_);

            if ((ref_lower != -1 || ref_upper != -1)) // ref times possible
            {
                if ((ref_lower != -1 && target_data.hasRefModeCForTime(ref_lower))
                        || (ref_upper != -1 && target_data.hasRefModeCForTime(ref_upper))) // ref value(s) exist
                {
                    code_present_ref = true;
                }
            }

            code_present_tst = target_data.hasTstModeCForTime(tod);

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
                details.push_back({tod, pos_current,
                                   true, is_inside, code_missing, // ref_exists, pos_inside, is_not_ok
                                   num_updates, num_no_ref_pos, num_pos_inside, num_pos_outside,
                                   num_no_ref_id, num_present_id, num_missing_id, comment});
        }

        logdbg << "EvaluationRequirementModeC '" << name_ << "': evaluate: utn " << target_data.utn_
               << " num_updates " << num_updates << " num_no_ref_pos " << num_no_ref_pos
               //<< " num_no_ref_val " << num_no_ref_val
               << " num_pos_outside " << num_pos_outside << " num_pos_inside " << num_pos_inside
               << " num_no_ref_id " << num_no_ref_id << " num_present_id " << num_present_id
               << " num_missing_id " << num_missing_id;

        assert (num_updates - num_no_ref_pos == num_pos_inside + num_pos_outside);
        assert (num_pos_inside == num_no_ref_id+num_present_id+num_missing_id);

        //assert (details.size() == tst_data.size());

        return make_shared<EvaluationRequirementResult::SingleModeCPresent>(
                    "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                    eval_man_, num_updates, num_no_ref_pos, num_pos_outside, num_pos_inside,
                    num_no_ref_id, num_present_id, num_missing_id, details);
    }

    float ModeCPresent::maxRefTimeDiff() const
    {
        return max_ref_time_diff_;
    }

    float ModeCPresent::minimumProbabilityPresent() const
    {
        return minimum_probability_present_;
    }
}
