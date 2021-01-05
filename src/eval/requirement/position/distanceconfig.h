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

#ifndef EVALUATIONREQUIREMENTPOSITIONDISTANCECONFIG_H
#define EVALUATIONREQUIREMENTPOSITIONDISTANCECONFIG_H

#include "configurable.h"
#include "eval/requirement/config.h"
#include "eval/requirement/position/distanceconfigwidget.h"
#include "eval/requirement/position/distance.h"

class Group;
class EvaluationStandard;

namespace EvaluationRequirement
{

class PositionDistanceConfig : public Config
{
public:
    PositionDistanceConfig(const std::string& class_id, const std::string& instance_id,
                        Group& group, EvaluationStandard& standard,
                        EvaluationManager& eval_ma);
    virtual ~PositionDistanceConfig();

    virtual void addGUIElements(QFormLayout* layout) override;
    PositionDistanceConfigWidget* widget() override;
    std::shared_ptr<Base> createRequirement() override;

    float maxAbsValue() const;
    void maxAbsValue(float value);

    float minimumProbability() const;
    void minimumProbability(float value);

protected:
    float max_abs_value_ {0};
    float minimum_probability_{0};

    std::unique_ptr<PositionDistanceConfigWidget> widget_;
};

}

#endif // EVALUATIONREQUIREMENTPOSITIONDISTANCECONFIG_H
