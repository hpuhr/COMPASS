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

#ifndef EVALUATIONREQUIREMENTPOSITIONDISTANCERMSCONFIG_H
#define EVALUATIONREQUIREMENTPOSITIONDISTANCERMSCONFIG_H

#include "eval/requirement/base/baseconfig.h"

class Group;
class EvaluationStandard;

namespace ResultReport
{
    class Report;
}

namespace EvaluationRequirement
{

class PositionDistanceRMSConfig : public BaseConfig
{
public:
    PositionDistanceRMSConfig(const std::string& class_id, const std::string& instance_id,
                        Group& group, EvaluationStandard& standard, EvaluationCalculator& calculator);
    virtual ~PositionDistanceRMSConfig();

    std::shared_ptr<Base> createRequirement() override;

    double thresholdValue() const;
    void thresholdValue(double value);

    virtual void addToReport (std::shared_ptr<ResultReport::Report> report) override;

protected:
    double threshold_value_ {0};

    virtual BaseConfigWidget* createWidget() override;
};

}

#endif // EVALUATIONREQUIREMENTPOSITIONDISTANCERMSCONFIG_H
