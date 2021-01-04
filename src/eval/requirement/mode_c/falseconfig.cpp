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

#include "eval/requirement/mode_c/falseconfig.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base.h"

using namespace std;

namespace EvaluationRequirement
{

    ModeCFalseConfig::ModeCFalseConfig(const std::string& class_id, const std::string& instance_id,
                             Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
                         : Config(class_id, instance_id, group, standard, eval_man)
    {
        registerParameter("max_ref_time_diff", &max_ref_time_diff_, 4.0);

        registerParameter("use_minimum_probability_present", &use_minimum_probability_present_, true);
        registerParameter("minimum_probability_present", &minimum_probability_present_, 0.97);

        registerParameter("use_maximum_probability_false", &use_maximum_probability_false_, true);
        registerParameter("maximum_probability_false", &maximum_probability_false_, 0.01);

        registerParameter("max_difference", &max_difference_, 100);
    }

    void ModeCFalseConfig::addGUIElements(QFormLayout* layout)
    {
        assert (layout);

        Config::addGUIElements(layout);
    }

    ModeCFalseConfigWidget* ModeCFalseConfig::widget()
    {
        if (!widget_)
            widget_.reset(new ModeCFalseConfigWidget(*this));

        return widget_.get();
    }

    std::shared_ptr<Base> ModeCFalseConfig::createRequirement()
    {
        shared_ptr<ModeCFalse> req = make_shared<ModeCFalse>(
                    name_, short_name_, group_.name(), eval_man_, max_ref_time_diff_,
                    use_minimum_probability_present_, minimum_probability_present_,
                    use_maximum_probability_false_, maximum_probability_false_, max_difference_);

        return req;
    }

    float ModeCFalseConfig::maxRefTimeDiff() const
    {
        return max_ref_time_diff_;
    }

    void ModeCFalseConfig::maxRefTimeDiff(float value)
    {
        max_ref_time_diff_ = value;
    }

    bool ModeCFalseConfig::useMinimumProbabilityPresent() const
    {
        return use_minimum_probability_present_;
    }

    void ModeCFalseConfig::useMinimumProbabilityPresent(bool value)
    {
        use_minimum_probability_present_ = value;
    }

    float ModeCFalseConfig::minimumProbabilityPresent() const
    {
        return minimum_probability_present_;
    }

    void ModeCFalseConfig::minimumProbabilityPresent(float value)
    {
        minimum_probability_present_ = value;
    }

    bool ModeCFalseConfig::useMaximumProbabilityFalse() const
    {
        return use_maximum_probability_false_;
    }

    void ModeCFalseConfig::useMaximumProbabilityFalse(bool value)
    {
        use_maximum_probability_false_ = value;
    }

    float ModeCFalseConfig::maximumProbabilityFalse() const
    {
        return maximum_probability_false_;
    }

    void ModeCFalseConfig::maximumProbabilityFalse(float value)
    {
        maximum_probability_false_ = value;
    }
    
    float ModeCFalseConfig::maxDifference() const
    {
        return max_difference_;
    }
    
    void ModeCFalseConfig::maxDifference(float value)
    {
        max_difference_ = value;
    }
    
}
