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

#include "eval/results/mode_c/correct.h"

#include "eval/requirement/mode_c/correct.h"

namespace EvaluationRequirementResult
{

/********************************************************************************
 * SingleModeCCorrect
 ********************************************************************************/

/**
*/
SingleModeCCorrect::SingleModeCCorrect(const std::string& result_id,
                                       std::shared_ptr<EvaluationRequirement::Base> requirement,
                                       const SectorLayer& sector_layer,
                                       unsigned int utn,
                                       const EvaluationTargetData* target,
                                       EvaluationManager& eval_man,
                                       const EvaluationDetails& details,
                                       unsigned int num_updates,
                                       unsigned int num_no_ref_pos,
                                       unsigned int num_no_ref_id,
                                       unsigned int num_pos_outside,
                                       unsigned int num_pos_inside,
                                       unsigned int num_correct,
                                       unsigned int num_not_correct)
:   SingleCorrectBase("SingleModeCCorrect", 
                      result_id, 
                      requirement, 
                      sector_layer, 
                      utn, target, 
                      eval_man, 
                      details, 
                      num_updates,
                      num_no_ref_pos,
                      num_no_ref_id,
                      num_pos_outside,
                      num_pos_inside,
                      num_correct,
                      num_not_correct,
                      "Mode C",
                      "#CMC",
                      "#NCMC")
{
    updateResult(details);
}

/**
*/
std::shared_ptr<Joined> SingleModeCCorrect::createEmptyJoined(const std::string& result_id)
{
    return std::make_shared<JoinedModeCCorrect> (result_id, requirement_, sector_layer_, eval_man_);
}

/**
*/
std::vector<Single::TargetInfo> SingleModeCCorrect::additionalTargetInfos() const
{
    std::shared_ptr<EvaluationRequirement::ModeCCorrect> req = std::static_pointer_cast<EvaluationRequirement::ModeCCorrect>(requirement_);
    assert (req);

    return { {"Max Dist. [ft]", "Maximum offset", formatValue(req->maxDistanceFt()) } };
}

/********************************************************************************
 * JoinedModeCCorrect
 ********************************************************************************/

JoinedModeCCorrect::JoinedModeCCorrect(const std::string& result_id,
                                       std::shared_ptr<EvaluationRequirement::Base> requirement,
                                       const SectorLayer& sector_layer,
                                       EvaluationManager& eval_man)
:   JoinedCorrectBase("JoinedModeCCorrect", 
                      result_id, 
                      requirement, 
                      sector_layer, 
                      eval_man, 
                      "Mode C",
                      "#CMC",
                      "#NCMC")
{
}

}
