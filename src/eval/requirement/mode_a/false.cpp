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
#include "stringconv.h"
#include "sectorlayer.h"

using namespace std;
using namespace Utils;

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
        logdbg << "EvaluationRequirementModeA '" << name_ << "': evaluate: utn " << target_data.utn_;

        float max_ref_time_diff = eval_man_.maxRefTimeDiff();

        const std::multimap<float, unsigned int>& tst_data = target_data.tstData();

        float tod{0};

        float ref_lower{0}, ref_upper{0};

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
        unsigned int code;
        bool code_ok;

        bool ref_exists;
        bool is_inside;
        pair<EvaluationTargetPosition, bool> ret_pos;
        EvaluationTargetPosition ref_pos;
        bool ok;

        string comment;
        bool lower_nok, upper_nok;

        bool skip_no_data_details = eval_man_.resultsGenerator().skipNoDataDetails();
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

            tod = tst_id.first;
            pos_current = target_data.tstPosForTime(tod);

            if (!target_data.hasRefDataForTime (tod, max_ref_time_diff))
            {
                if (!skip_no_data_details)
                    details.push_back({tod, pos_current,
                                       false, {}, false, // ref_exists, pos_inside,
                                       num_updates, num_no_ref_pos+num_no_ref_val, num_pos_inside, num_pos_outside,
                                       num_unknown, num_correct, num_false, "No reference data"});

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
                                       false, {}, false, // ref_exists, pos_inside,
                                       num_updates, num_no_ref_pos+num_no_ref_val, num_pos_inside, num_pos_outside,
                                       num_unknown, num_correct, num_false, "No reference position"});

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
                                       ref_exists, is_inside, false, // ref_exists, pos_inside,
                                       num_updates, num_no_ref_pos+num_no_ref_val, num_pos_inside, num_pos_outside,
                                       num_unknown, num_correct, num_false, "Outside sector"});

                ++num_pos_outside;
                continue;
            }
            ++num_pos_inside;

            if (target_data.hasTstModeAForTime(tod))
            {
                code = target_data.tstModeAForTime(tod);

                tie(ref_lower, ref_upper) = target_data.refTimesFor(tod, max_ref_time_diff);

                if ((ref_lower != -1 || ref_upper != -1)) // ref times possible
                {
                    if ((ref_lower != -1 && target_data.hasRefModeAForTime(ref_lower))
                            || (ref_upper != -1 && target_data.hasRefModeAForTime(ref_upper))) // ref value(s) exist
                    {
                        ref_exists = true;
                        code_ok = false;

                        lower_nok = false;
                        upper_nok = false;

                        if (ref_lower != -1 && target_data.hasRefModeAForTime(ref_lower))
                        {
                            code_ok = target_data.refModeAForTime(ref_lower) == code;
                            lower_nok = !code_ok;
                        }

                        if (!code_ok && ref_upper != -1 && target_data.hasRefModeAForTime(ref_upper))
                        {
                            code_ok = target_data.refModeAForTime(ref_upper) == code;
                            upper_nok = !code_ok;
                        }

                        if (code_ok)
                        {
                            ++num_correct;
                            comment = "OK";
                        }
                        else
                        {
                            ++num_false;
                            comment = "Not OK:";

                            if (lower_nok)
                            {
                                comment += " test code '"+String::octStringFromInt(code, 4, '0')
                                        +"' reference code at "+String::timeStringFromDouble(ref_lower)+ "  '"
                                        +String::octStringFromInt(target_data.refModeAForTime(ref_lower), 4, '0')
                                        + "'";
                            }
                            else
                            {
                                assert (upper_nok);
                                comment += " test code '"+String::octStringFromInt(code, 4, '0')
                                        +"' reference code at "+String::timeStringFromDouble(ref_upper)+ "  '"
                                        +String::octStringFromInt(target_data.refModeAForTime(ref_upper), 4, '0')
                                        + "'";
                            }
                        }
                    }
                    else
                    {
                        comment = "No reference data";

                        if (skip_no_data_details)
                            skip_detail = true;

                        ++num_no_ref_val;
                    }
                }
                else
                {
                    comment = "No reference code";

                    if (skip_no_data_details)
                        skip_detail = true;

                    ++num_no_ref_val;
                }
            }
            else
            {
                comment = "No test code";

                if (skip_no_data_details)
                    skip_detail = true;

                ++num_unknown;
            }

            if (!skip_detail)
                details.push_back({tod, pos_current,
                                   ref_exists, is_inside, !code_ok,
                                   num_updates, num_no_ref_pos+num_no_ref_val, num_pos_inside, num_pos_outside,
                                   num_unknown, num_correct, num_false, comment});
        }

        logdbg << "EvaluationRequirementModeA '" << name_ << "': evaluate: utn " << target_data.utn_
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
