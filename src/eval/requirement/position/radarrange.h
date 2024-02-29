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

#ifndef EVALUATIONREQUIREMENPOSITIONRADARRANGE_H
#define EVALUATIONREQUIREMENPOSITIONRADARRANGE_H

#include "eval/requirement/base/base.h"
//#include "eval/requirement/position/detail.h"

namespace EvaluationRequirement
{

class PositionRadarRange : public Base
{
public:
    PositionRadarRange(
            const std::string& name, const std::string& short_name, const std::string& group_name,
            EvaluationManager& eval_man, float threshold_value);

    float thresholdValue() const;

    virtual std::shared_ptr<EvaluationRequirementResult::Single> evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
            const SectorLayer& sector_layer) override;

    std::string getConditionStr () const override;
    std::string getConditionResultStr (float rms_value) const;

protected:
    float threshold_value_ {0};
};

}

#endif // EVALUATIONREQUIREMENPOSITIONRADARRANGE_H
