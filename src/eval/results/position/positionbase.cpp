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

#include "eval/results/position/positionbase.h"

using namespace std;
using namespace Utils;

namespace EvaluationRequirementResult
{

/****************************************************************************
 * SinglePositionBase
 ****************************************************************************/

const std::string SinglePositionBase::DetailValue           = "Value";
const std::string SinglePositionBase::DetailCheckPassed     = "CheckPassed";
const std::string SinglePositionBase::DetailPosInside       = "PosInside";
const std::string SinglePositionBase::DetailNumPos          = "NumPos";
const std::string SinglePositionBase::DetailNumNoRef        = "NumNoRef";
const std::string SinglePositionBase::DetailNumInside       = "NumInside";
const std::string SinglePositionBase::DetailNumOutside      = "NumOutside";
const std::string SinglePositionBase::DetailNumCheckPassed  = "NumCheckPassed";
const std::string SinglePositionBase::DetailNumCheckFailed  = "NumCheckFailed";

/**
*/
SinglePositionBase::SinglePositionBase(const std::string& result_type,
                                       const std::string& result_id, 
                                       std::shared_ptr<EvaluationRequirement::Base> requirement,
                                       const SectorLayer& sector_layer,
                                       unsigned int utn, 
                                       const EvaluationTargetData* target, 
                                       EvaluationManager& eval_man,
                                       const boost::optional<EvaluationDetails>& details,
                                       unsigned int num_pos, 
                                       unsigned int num_no_ref,
                                       unsigned int num_pos_outside, 
                                       unsigned int num_pos_inside,
                                       unsigned int num_passed, 
                                       unsigned int num_failed,
                                       vector<double> values)
:   Single("SinglePositionAcross", result_id, requirement, sector_layer, utn, target, eval_man, details)
,   num_pos_        (num_pos)
,   num_no_ref_     (num_no_ref)
,   num_pos_outside_(num_pos_outside)
,   num_pos_inside_ (num_pos_inside)
,   num_passed_     (num_passed)
,   num_failed_     (num_failed)
,   values_         (values)
{
}

/**
*/
unsigned int SinglePositionBase::numPassed() const
{
    return num_passed_;
}

/**
*/
unsigned int SinglePositionBase::numFailed() const
{
    return num_failed_;
}

/**
*/
unsigned int SinglePositionBase::numPosOutside() const
{
    return num_pos_outside_;
}

/**
*/
unsigned int SinglePositionBase::numPosInside() const
{
    return num_pos_inside_;
}

/**
*/
unsigned int SinglePositionBase::numPos() const
{
    return num_pos_;
}

/**
*/
unsigned int SinglePositionBase::numNoRef() const
{
    return num_no_ref_;
}

/**
*/
const vector<double>& SinglePositionBase::values() const
{
    return values_;
}

/****************************************************************************
 * JoinedPositionBase
 ****************************************************************************/

/**
*/
JoinedPositionBase::JoinedPositionBase(const std::string& result_type,
                                       const std::string& result_id, 
                                       std::shared_ptr<EvaluationRequirement::Base> requirement,
                                       const SectorLayer& sector_layer, 
                                       EvaluationManager& eval_man)
: Joined(result_type, result_id, requirement, sector_layer, eval_man)
{
}

}
