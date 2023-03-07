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

#include "eval/results/falsebase.h"

namespace EvaluationRequirementResult
{

/***************************************************************************
 * SingleFalseBase
 ***************************************************************************/

const std::string SingleFalseBase::DetailRefExists    = "RefExists";
const std::string SingleFalseBase::DetailPosInside    = "PosInside";
const std::string SingleFalseBase::DetailIsNotOk      = "IsNotOk";
const std::string SingleFalseBase::DetailNumUpdates   = "NumUpdates";
const std::string SingleFalseBase::DetailNumNoRef     = "NumNoRef";
const std::string SingleFalseBase::DetailNumInside    = "NumInside";
const std::string SingleFalseBase::DetailNumOutside   = "NumOutside";
const std::string SingleFalseBase::DetailNumUnknownID = "NumUnknownID";
const std::string SingleFalseBase::DetailNumCorrectID = "NumCorrectID";
const std::string SingleFalseBase::DetailNumFalseID   = "NumFalseID";

SingleFalseBase::SingleFalseBase(const std::string& result_type,
                                 const std::string& result_id, 
                                 std::shared_ptr<EvaluationRequirement::Base> requirement,
                                 const SectorLayer& sector_layer,
                                 unsigned int utn, 
                                 const EvaluationTargetData* target, 
                                 EvaluationManager& eval_man,
                                 const boost::optional<EvaluationDetails>& details,
                                 int num_updates, 
                                 int num_no_ref_pos, 
                                 int num_no_ref_val, 
                                 int num_pos_outside, 
                                 int num_pos_inside,
                                 int num_unknown, 
                                 int num_correct, 
                                 int num_false)
:   Single(result_type, result_id, requirement, sector_layer, utn, target, eval_man, details)
,   num_updates_    (num_updates)
,   num_no_ref_pos_ (num_no_ref_pos)
,   num_no_ref_val_ (num_no_ref_val)
,   num_pos_outside_(num_pos_outside)
,   num_pos_inside_ (num_pos_inside)
,   num_unknown_    (num_unknown)
,   num_correct_    (num_correct)
,   num_false_      (num_false)
{
}

int SingleFalseBase::numNoRefPos() const
{
    return num_no_ref_pos_;
}

int SingleFalseBase::numNoRefValue() const
{
    return num_no_ref_val_;
}

int SingleFalseBase::numPosOutside() const
{
    return num_pos_outside_;
}

int SingleFalseBase::numPosInside() const
{
    return num_pos_inside_;
}

int SingleFalseBase::numUpdates() const
{
    return num_updates_;
}

int SingleFalseBase::numUnknown() const
{
    return num_unknown_;
}

int SingleFalseBase::numCorrect() const
{
    return num_correct_;
}

int SingleFalseBase::numFalse() const
{
    return num_false_;
}

/***************************************************************************
 * JoinedFalseBase
 ***************************************************************************/

JoinedFalseBase::JoinedFalseBase(const std::string& result_type,
                                 const std::string& result_id, 
                                 std::shared_ptr<EvaluationRequirement::Base> requirement,
                                 const SectorLayer& sector_layer, 
                                 EvaluationManager& eval_man)
:   Joined(result_type, result_id, requirement, sector_layer, eval_man)
{
}

}
