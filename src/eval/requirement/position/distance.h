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

#ifndef EVALUATIONREQUIREMENPOSITIONDISTANCE_H
#define EVALUATIONREQUIREMENPOSITIONDISTANCE_H

#include "eval/requirement/base/probabilitybase.h"
//#include "eval/requirement/position/detail.h"

namespace EvaluationRequirement
{

class PositionDistance : public ProbabilityBase
{
public:
    PositionDistance(
            const std::string& name, const std::string& short_name, const std::string& group_name,
            float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man,
            float threshold_value, COMPARISON_TYPE threshold_value_check_type,
            bool failed_values_of_interest);

    float thresholdValue() const;
    COMPARISON_TYPE thresholdValueCheckType() const;
    bool failedValuesOfInterest() const;

    virtual std::shared_ptr<EvaluationRequirementResult::Single> evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
            const SectorLayer& sector_layer) override;


protected:
    float threshold_value_ {0};
    COMPARISON_TYPE threshold_value_check_type_ {COMPARISON_TYPE::LESS_THAN_OR_EQUAL};
    bool failed_values_of_interest_ {true};
};

}

#endif // EVALUATIONREQUIREMENPOSITIONDISTANCE_H
