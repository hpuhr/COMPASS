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

#include "eval/requirement/base/base.h"

namespace EvaluationRequirement
{

/**
*/
class PositionRadarAzimuth : public Base
{
public:
    PositionRadarAzimuth(
            const std::string& name, const std::string& short_name, const std::string& group_name,
            EvaluationManager& eval_man, float threshold_value);

    virtual std::shared_ptr<EvaluationRequirementResult::Single> evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
            const SectorLayer& sector_layer) override;

    //@TODO_EVAL: what is the name of the final value?
    std::string getConditionResultNameShort() const override final { return "TODO"; }
    std::string getConditionResultName() const override final { return "TODO"; }
    std::string getConditionUnits() const override final { return ""; }
};

}
