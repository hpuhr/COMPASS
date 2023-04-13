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

#include "eval/results/presentbase.h"

namespace EvaluationRequirementResult
{

/***************************************************************************
 * SinglePresentBase
 ***************************************************************************/

SinglePresentBase::SinglePresentBase(const std::string& result_type,
                                     const std::string& result_id, 
                                     std::shared_ptr<EvaluationRequirement::Base> requirement,
                                     const SectorLayer& sector_layer,
                                     unsigned int utn, 
                                     const EvaluationTargetData* target, 
                                     EvaluationManager& eval_man,
                                     const EvaluationDetails& details,
                                     int num_updates, 
                                     int num_no_ref_pos, 
                                     int num_pos_outside, 
                                     int num_pos_inside,
                                     int num_no_ref_val, 
                                     int num_present, 
                                     int num_missing)
:   Single(result_type, result_id, requirement, sector_layer, utn, target, eval_man, details)
,   num_updates_    (num_updates)
,   num_no_ref_pos_ (num_no_ref_pos)
,   num_pos_outside_(num_pos_outside)
,   num_pos_inside_ (num_pos_inside)
,   num_no_ref_val_ (num_no_ref_val)
,   num_present_    (num_present)
,   num_missing_    (num_missing)
{
}

int SinglePresentBase::numUpdates() const
{
    return num_updates_;
}

int SinglePresentBase::numNoRefPos() const
{
    return num_no_ref_pos_;
}

int SinglePresentBase::numPosOutside() const
{
    return num_pos_outside_;
}

int SinglePresentBase::numPosInside() const
{
    return num_pos_inside_;
}

int SinglePresentBase::numNoRefValue() const
{
    return num_no_ref_val_;
}

int SinglePresentBase::numPresent() const
{
    return num_present_;
}

int SinglePresentBase::numMissing() const
{
    return num_missing_;
}

/***************************************************************************
 * JoinedPresentBase
 ***************************************************************************/

JoinedPresentBase::JoinedPresentBase(const std::string& result_type,
                                     const std::string& result_id, 
                                     std::shared_ptr<EvaluationRequirement::Base> requirement,
                                     const SectorLayer& sector_layer, 
                                     EvaluationManager& eval_man)
:   Joined(result_type, result_id, requirement, sector_layer, eval_man)
{
}

}
