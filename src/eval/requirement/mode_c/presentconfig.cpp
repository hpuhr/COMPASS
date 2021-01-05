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

#include "eval/requirement/mode_c/presentconfig.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base/base.h"

using namespace std;

namespace EvaluationRequirement
{

    ModeCPresentConfig::ModeCPresentConfig(const std::string& class_id, const std::string& instance_id,
                             Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
                         : BaseConfig(class_id, instance_id, group, standard, eval_man)
    {
        registerParameter("minimum_probability_present", &minimum_probability_present_, 0.98);
    }

    std::shared_ptr<Base> ModeCPresentConfig::createRequirement()
    {
        shared_ptr<ModeCPresent> req = make_shared<ModeCPresent>(
                    name_, short_name_, group_.name(), eval_man_, minimum_probability_present_);

        return req;
    }


    float ModeCPresentConfig::minimumProbabilityPresent() const
    {
        return minimum_probability_present_;
    }

    void ModeCPresentConfig::minimumProbabilityPresent(float value)
    {
        minimum_probability_present_ = value;
    }

    void ModeCPresentConfig::createWidget()
    {
        assert (!widget_);
        widget_.reset(new ModeCPresentConfigWidget(*this));
        assert (widget_);
    }
}
