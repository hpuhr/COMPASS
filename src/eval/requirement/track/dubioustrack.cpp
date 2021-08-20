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


#include "dubioustrack.h"
#include "evaluationmanager.h"
#include "evaluationdata.h"
#include "stringconv.h"

using namespace std;
using namespace Utils;

namespace EvaluationRequirement
{

DubiousTrack::DubiousTrack(
        const std::string& name, const std::string& short_name, const std::string& group_name,
        bool mark_primary_only, bool use_min_updates, unsigned int min_updates,
        bool use_min_duration, float min_duration, float prob,
        COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man)
: Base(name, short_name, group_name, prob, prob_check_type, eval_man),
  mark_primary_only_(mark_primary_only), use_min_updates_(use_min_updates), min_updates_(min_updates),
  use_min_duration_(use_min_duration), min_duration_(min_duration)
{
}


std::shared_ptr<EvaluationRequirementResult::Single> DubiousTrack::evaluate (
        const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
        const SectorLayer& sector_layer)
{
    logdbg << "EvaluationRequirementDubiousTrack '" << name_ << "': evaluate: utn " << target_data.utn_
           << " mark_primary_only " << mark_primary_only_ << " prob " << prob_
           << " use_min_updates " << use_min_updates_ << " min_updates " << min_updates_
           << " use_min_duration " << use_min_duration_ << " min_duration " << min_duration_;

    const std::multimap<float, unsigned int>& tst_data = target_data.tstData();

    EvaluationTargetPosition tst_pos;
    bool has_ground_bit;
    bool ground_bit_set;

    bool is_inside;

    float tod{0};

    bool first_inside = true;
    float tod_first{0}, tod_last{0};

    unsigned int num_updates {0};
    unsigned int num_pos_outside {0};
    unsigned int num_pos_inside {0};

    for (const auto& tst_id : tst_data)
    {
        ++num_updates;

        // check if inside based on test position only

        tst_pos = target_data.tstPosForTime(tod);

        has_ground_bit = target_data.hasTstGroundBitForTime(tod);

        if (has_ground_bit)
            ground_bit_set = target_data.tstGroundBitForTime(tod);
        else
            ground_bit_set = false;

        is_inside = sector_layer.isInside(tst_pos, has_ground_bit, ground_bit_set);


        if (!is_inside)
        {
//            if (!skip_no_data_details)
//                details.push_back({tod, tst_pos,
//                                   true, ref_pos, // has_ref_pos, ref_pos
//                                   is_inside, {}, comp_passed, // pos_inside, value, check_passed
//                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside,
//                                   num_comp_failed, num_comp_passed,
//                                   "Outside sector"});
            ++num_pos_outside;
            continue;
        }
        ++num_pos_inside;

        tod = tst_id.first;

        if (first_inside)
        {
            tod_first = tod;
            tod_last = tod;

            first_inside = false;
        }
        else
        {
            tod_last = tod;
        }
    }

    bool is_dubious = false;
    string dubious_reason;

    if (mark_primary_only_)
    {
        if (target_data.isPrimaryOnly())
        {
            is_dubious = true;
            dubious_reason = "Primary-only";
        }
    }

    if (!is_dubious && use_min_updates_)
    {
        if (num_updates < min_updates_)
        {
            is_dubious = true;
            dubious_reason = "Too few updates ("+to_string(num_updates)+")";
        }
    }

    if (!is_dubious && use_min_duration_)
    {
        assert (tod_last >= tod_first);
        float duration = tod_last-tod_first;
        if (duration < min_duration_)
        {
            is_dubious = true;
            dubious_reason = "Too small duration ("+String::doubleToStringPrecision(duration,2)+")";
        }
    }

    return nullptr;
}

bool DubiousTrack::markPrimaryOnly() const
{
    return mark_primary_only_;
}

bool DubiousTrack::useMinUpdates() const
{
    return use_min_updates_;
}

unsigned int DubiousTrack::minUpdates() const
{
    return min_updates_;
}

bool DubiousTrack::useMinDuration() const
{
    return use_min_duration_;
}

float DubiousTrack::minDuration() const
{
    return min_duration_;
}

}
