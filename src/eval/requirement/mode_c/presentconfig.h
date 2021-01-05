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

#ifndef EVALUATIONREQUIREMENTMODECRESENTCONFIG_H
#define EVALUATIONREQUIREMENTMODECRESENTCONFIG_H

#include "configurable.h"
#include "eval/requirement/base/baseconfig.h"
#include "eval/requirement/mode_c/present.h"
#include "eval/requirement/mode_c/presentconfigwidget.h"

#include <memory>

class Group;
class EvaluationStandard;

namespace EvaluationRequirement
{
    class ModeCPresentConfig : public BaseConfig
    {
    public:
        ModeCPresentConfig(const std::string& class_id, const std::string& instance_id,
                    Group& group, EvaluationStandard& standard, EvaluationManager& eval_man);

        virtual void addGUIElements(QFormLayout* layout) override;
        ModeCPresentConfigWidget* widget() override;
        std::shared_ptr<Base> createRequirement() override;

        float minimumProbabilityPresent() const;
        void minimumProbabilityPresent(float value);

    protected:
        float minimum_probability_present_{0};

        std::unique_ptr<ModeCPresentConfigWidget> widget_;
    };

}

#endif // EVALUATIONREQUIREMENTMODECRESENTCONFIG_H
