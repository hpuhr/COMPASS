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

#include "correct_period.h"

namespace EvaluationRequirementResult
{

/********************************************************************************
 * SingleModeACorrectPeriod
 ********************************************************************************/

/**
*/
SingleModeACorrectPeriod::SingleModeACorrectPeriod(const std::string& result_type, 
                                                   const std::string& result_id, 
                                                   std::shared_ptr<EvaluationRequirement::Base> requirement,
                                                   const SectorLayer& sector_layer, 
                                                   unsigned int utn, 
                                                   const EvaluationTargetData* target,
                                                   EvaluationManager& eval_man,
                                                   const EvaluationDetails& details,
                                                   int sum_uis, 
                                                   int missed_uis, 
                                                   TimePeriodCollection ref_periods)
:   SingleIntervalBase(result_type, result_id, requirement, sector_layer, utn, target, eval_man, details, sum_uis, missed_uis, ref_periods)
{
}

/**
*/
std::shared_ptr<Joined> SingleModeACorrectPeriod::createEmptyJoined(const std::string& result_id)
{
    return make_shared<JoinedModeACorrectPeriod> ("JoinedModeACorrectPeriod", result_id, requirement_, sector_layer_, eval_man_);
}

/**
*/
std::string SingleModeACorrectPeriod::probabilityName() const
{
    return "PCMAD [%]";
}

/**
*/
std::string SingleModeACorrectPeriod::probabilityDescription() const
{
    return "Probability of Correct Mode A Detection";
}

/********************************************************************************
 * JoinedModeACorrectPeriod
 ********************************************************************************/

/**
*/
JoinedModeACorrectPeriod::JoinedModeACorrectPeriod(const std::string& result_type, 
                                                   const std::string& result_id, 
                                                   std::shared_ptr<EvaluationRequirement::Base> requirement,
                                                   const SectorLayer& sector_layer, 
                                                   EvaluationManager& eval_man)
:   JoinedIntervalBase(result_type, result_id, requirement, sector_layer, eval_man)
{
}

/**
*/
std::string JoinedModeACorrectPeriod::probabilityName() const
{
    return "PCMAD [%]";
}

/**
*/
std::string JoinedModeACorrectPeriod::probabilityDescription() const
{
    return "Probability of Correct Mode A Detection";
}

} // namespace EvaluationRequirementResult
