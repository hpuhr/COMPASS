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
