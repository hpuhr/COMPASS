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

#pragma once

#include "eval/requirement/base/probabilitybase.h"

namespace EvaluationRequirement
{

/**
*/
class Speed : public ProbabilityBase
{
public:
    Speed(const std::string& name, const std::string& short_name, const std::string& group_name,
          double prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man,
          float threshold_value, bool use_percent_if_higher, float threshold_percent,
          COMPARISON_TYPE threshold_value_check_type,
          bool failed_values_of_interest);

    float thresholdValue() const;
    bool usePercentIfHigher() const;
    float thresholdPercent() const;
    COMPARISON_TYPE thresholdValueCheckType() const;
    bool failedValuesOfInterest() const;

    virtual std::shared_ptr<EvaluationRequirementResult::Single> evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
            const SectorLayer& sector_layer) override;

    std::string probabilityNameShort() const override final { return "PCP"; }
    std::string probabilityName() const override final { return "Probability of passed comparison"; }

protected:
    float threshold_value_ {0}; // offset

    bool use_percent_if_higher_ {false};
    float threshold_percent_ {10.0};

    COMPARISON_TYPE threshold_value_check_type_ {COMPARISON_TYPE::LESS_THAN_OR_EQUAL};

    bool failed_values_of_interest_ {true};
};

}
