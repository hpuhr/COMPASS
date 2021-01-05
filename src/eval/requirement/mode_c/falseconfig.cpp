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
#include "eval/requirement/mode_c/modecfalseconfigwidget.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base/base.h"

using namespace std;

namespace EvaluationRequirement
{

    ModeCFalseConfig::ModeCFalseConfig(const std::string& class_id, const std::string& instance_id,
                             Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
                         : BaseConfig(class_id, instance_id, group, standard, eval_man)
    {
        registerParameter("maximum_probability_false", &maximum_probability_false_, 0.01);

        registerParameter("max_difference", &max_difference_, 100);
    }

    std::shared_ptr<Base> ModeCFalseConfig::createRequirement()
    {
        shared_ptr<ModeCFalse> req = make_shared<ModeCFalse>(
                    name_, short_name_, group_.name(), eval_man_, maximum_probability_false_, max_difference_);

        return req;
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

    void ModeCFalseConfig::createWidget()
    {
        assert (!widget_);
        widget_.reset(new ModeCFalseConfigWidget(*this));
        assert (widget_);
    }
    
}
