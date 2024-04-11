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
#include "eval/requirement/identification/correct_period.h"

namespace EvaluationRequirementResult
{

/********************************************************************************
 * SingleModeACorrectPeriod
 ********************************************************************************/

/**
*/
SingleIdentificationCorrectPeriod::SingleIdentificationCorrectPeriod(const std::string& result_type, 
                                                                     const std::string& result_id, 
                                                                     std::shared_ptr<EvaluationRequirement::Base> requirement,
                                                                     const SectorLayer& sector_layer, 
                                                                     unsigned int utn, 
                                                                     const EvaluationTargetData* target,
                                                                     EvaluationManager& eval_man,
                                                                     const EvaluationDetails& details,
                                                                     int sum_uis, 
                                                                     int missed_uis, 
                                                                     TimePeriodCollection ref_periods,
                                                                     const std::vector<dbContent::TargetPosition>& ref_updates)
:   SingleIntervalBase(result_type, result_id, requirement, sector_layer, utn, target, eval_man, details, sum_uis, missed_uis, ref_periods, ref_updates)
{
}

/**
*/
std::shared_ptr<Joined> SingleIdentificationCorrectPeriod::createEmptyJoined(const std::string& result_id)
{
    return make_shared<JoinedIdentificationCorrectPeriod> ("JoinedIdentificationCorrectPeriod", result_id, requirement_, sector_layer_, eval_man_);
}

/**
*/
std::string SingleIdentificationCorrectPeriod::probabilityName() const
{
    std::shared_ptr<EvaluationRequirement::IdentificationCorrectPeriod> req = std::static_pointer_cast<EvaluationRequirement::IdentificationCorrectPeriod>(requirement_);
    assert (req);

    return EvaluationRequirement::IdentificationCorrectPeriod::probabilityName(req->identificationType());
}

/**
*/
std::string SingleIdentificationCorrectPeriod::probabilityDescription() const
{
    std::shared_ptr<EvaluationRequirement::IdentificationCorrectPeriod> req = std::static_pointer_cast<EvaluationRequirement::IdentificationCorrectPeriod>(requirement_);
    assert (req);

    return EvaluationRequirement::IdentificationCorrectPeriod::probabilityDescription(req->identificationType());
}

/********************************************************************************
 * JoinedModeACorrectPeriod
 ********************************************************************************/

/**
*/
JoinedIdentificationCorrectPeriod::JoinedIdentificationCorrectPeriod(const std::string& result_type, 
                                                                     const std::string& result_id, 
                                                                     std::shared_ptr<EvaluationRequirement::Base> requirement,
                                                                     const SectorLayer& sector_layer, 
                                                                     EvaluationManager& eval_man)
:   JoinedIntervalBase(result_type, result_id, requirement, sector_layer, eval_man)
{
}

/**
*/
std::string JoinedIdentificationCorrectPeriod::probabilityName() const
{
    std::shared_ptr<EvaluationRequirement::IdentificationCorrectPeriod> req = std::static_pointer_cast<EvaluationRequirement::IdentificationCorrectPeriod>(requirement_);
    assert (req);

    return EvaluationRequirement::IdentificationCorrectPeriod::probabilityName(req->identificationType());
}

/**
*/
std::string JoinedIdentificationCorrectPeriod::probabilityDescription() const
{
    std::shared_ptr<EvaluationRequirement::IdentificationCorrectPeriod> req = std::static_pointer_cast<EvaluationRequirement::IdentificationCorrectPeriod>(requirement_);
    assert (req);

    return EvaluationRequirement::IdentificationCorrectPeriod::probabilityDescription(req->identificationType());
}

} // namespace EvaluationRequirementResult
