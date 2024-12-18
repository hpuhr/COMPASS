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

#ifndef EVALUATIONREQUIREMENTMODECCORRECTCONFIG_H
#define EVALUATIONREQUIREMENTMODECCORRECTCONFIG_H

//#include "configurable.h"
#include "eval/requirement/base/probabilitybaseconfig.h"
//#include "eval/requirement/mode_c/correct.h"
//#include "eval/requirement/mode_c/modeccorrectconfigwidget.h"

#include <memory>

class Group;
class EvaluationStandard;

namespace EvaluationRequirement
{

class ModeCCorrectConfig : public ProbabilityBaseConfig
{
public:
    ModeCCorrectConfig(const std::string& class_id, const std::string& instance_id,
                                Group& group, EvaluationStandard& standard,
                                EvaluationManager& eval_man);

    std::shared_ptr<Base> createRequirement() override;

    float maxDistanceFt() const;
    void maxDistanceFt(float value);

    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

protected:
    float max_distance_ft_ {300};

    virtual void createWidget() override;
};

}

#endif // EVALUATIONREQUIREMENTMODECCONFIG_H
