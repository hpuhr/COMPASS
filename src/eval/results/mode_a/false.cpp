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

#include "eval/results/mode_a/false.h"

namespace EvaluationRequirementResult
{

/********************************************************************************
 * SingleModeAFalse
 ********************************************************************************/

/**
*/
SingleModeAFalse::SingleModeAFalse(const std::string& result_id, 
                                   std::shared_ptr<EvaluationRequirement::Base> requirement,
                                   const SectorLayer& sector_layer,
                                   unsigned int utn,
                                   const EvaluationTargetData* target,
                                   EvaluationManager& eval_man,
                                   const EvaluationDetails& details,
                                   int num_updates,
                                   int num_no_ref_pos,
                                   int num_no_ref_val,
                                   int num_pos_outside,
                                   int num_pos_inside,
                                   int num_unknown,
                                   int num_correct,
                                   int num_false)
:   SingleFalseBase("SingleModeAFalse", result_id, requirement, sector_layer, utn, target, eval_man, details,
                    num_updates, num_no_ref_pos, num_no_ref_val, num_pos_outside, num_pos_inside, num_unknown, num_correct, num_false, "code")
{
    updateResult();
}

/**
*/
std::shared_ptr<Joined> SingleModeAFalse::createEmptyJoined(const std::string& result_id)
{
    return std::make_shared<JoinedModeAFalse> (result_id, requirement_, sector_layer_, eval_man_);
}

/********************************************************************************
 * JoinedModeAFalse
 ********************************************************************************/

/**
*/
JoinedModeAFalse::JoinedModeAFalse(const std::string& result_id, 
                                   std::shared_ptr<EvaluationRequirement::Base> requirement,
                                   const SectorLayer& sector_layer, 
                                   EvaluationManager& eval_man)
:   JoinedFalseBase("JoinedModeAFalse", result_id, requirement, sector_layer, eval_man, "code")
{
}

}
