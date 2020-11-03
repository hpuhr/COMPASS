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

#ifndef EVALUATIONREQUIREMENTPOSITIONMAXDISTANCECONFIG_H
#define EVALUATIONREQUIREMENTPOSITIONMAXDISTANCECONFIG_H

#include "configurable.h"
#include "eval/requirement/config.h"
#include "eval/requirement/position/positionmaxdistanceconfigwidget.h"
#include "eval/requirement/position/positionmaxdistance.h"

class Group;
class EvaluationStandard;

namespace EvaluationRequirement
{

class PositionMaxDistanceConfig : public Config
{
public:
    PositionMaxDistanceConfig(const std::string& class_id, const std::string& instance_id,
                              Group& group, EvaluationStandard& standard,
                              EvaluationManager& eval_ma);
    virtual ~PositionMaxDistanceConfig();

    virtual void addGUIElements(QFormLayout* layout) override;
    PositionMaxDistanceConfigWidget* widget() override;
    std::shared_ptr<Base> createRequirement() override;

    float maxDistance() const;
    void maxDistance(float value);

    float minimumProbability() const;
    void minimumProbability(float value);

    float maxRefTimeDiff() const;
    void maxRefTimeDiff(float value);

protected:
    float max_ref_time_diff_ {0};
    float max_distance_ {0};
    float minimum_probability_{0};

    std::unique_ptr<PositionMaxDistanceConfigWidget> widget_;
};

}
#endif // EVALUATIONREQUIREMENTPOSITIONMAXDISTANCECONFIG_H
