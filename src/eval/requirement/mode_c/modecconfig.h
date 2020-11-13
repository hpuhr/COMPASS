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

#ifndef EVALUATIONREQUIREMENTMODECCONFIG_H
#define EVALUATIONREQUIREMENTMODECCONFIG_H

#include "configurable.h"
#include "eval/requirement/config.h"
#include "eval/requirement/mode_c/modec.h"
#include "eval/requirement/mode_c/modecconfigwidget.h"

#include <memory>

class Group;
class EvaluationStandard;

namespace EvaluationRequirement
{
    class ModeCConfig : public Config
    {
    public:
        ModeCConfig(const std::string& class_id, const std::string& instance_id,
                    Group& group, EvaluationStandard& standard, EvaluationManager& eval_man);

        virtual void addGUIElements(QFormLayout* layout) override;
        ModeCConfigWidget* widget() override;
        std::shared_ptr<Base> createRequirement() override;

        float maxRefTimeDiff() const;
        void maxRefTimeDiff(float value);

        bool useMinimumProbabilityPresent() const;
        void useMinimumProbabilityPresent(bool value);

        float minimumProbabilityPresent() const;
        void minimumProbabilityPresent(float value);

        bool useMaximumProbabilityFalse() const;
        void useMaximumProbabilityFalse(bool value);

        float maximumProbabilityFalse() const;
        void maximumProbabilityFalse(float value);

    protected:
        float max_ref_time_diff_ {0};

        bool use_minimum_probability_present_ {true};
        float minimum_probability_present_{0};

        bool use_maximum_probability_false_ {true};
        float maximum_probability_false_{0};

        std::unique_ptr<ModeCConfigWidget> widget_;
    };

}

#endif // EVALUATIONREQUIREMENTMODECCONFIG_H
