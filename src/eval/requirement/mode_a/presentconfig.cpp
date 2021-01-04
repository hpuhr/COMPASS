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

#include "eval/requirement/mode_a/presentconfig.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base.h"

using namespace std;

namespace EvaluationRequirement
{

    ModeAPresentConfig::ModeAPresentConfig(const std::string& class_id, const std::string& instance_id,
                             Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
                         : Config(class_id, instance_id, group, standard, eval_man)
    {
        registerParameter("max_ref_time_diff", &max_ref_time_diff_, 4.0);

        registerParameter("minimum_probability_present", &minimum_probability_present_, 0.98);
    }

    void ModeAPresentConfig::addGUIElements(QFormLayout* layout)
    {
        assert (layout);

        Config::addGUIElements(layout);
    }

    ModeAPresentConfigWidget* ModeAPresentConfig::widget()
    {
        if (!widget_)
            widget_.reset(new ModeAPresentConfigWidget(*this));

        return widget_.get();
    }

    std::shared_ptr<Base> ModeAPresentConfig::createRequirement()
    {
        shared_ptr<ModeAPresent> req = make_shared<ModeAPresent>(
                    name_, short_name_, group_.name(), eval_man_, max_ref_time_diff_,
                    minimum_probability_present_);

        return req;
    }

    float ModeAPresentConfig::maxRefTimeDiff() const
    {
        return max_ref_time_diff_;
    }

    void ModeAPresentConfig::maxRefTimeDiff(float value)
    {
        max_ref_time_diff_ = value;
    }

    float ModeAPresentConfig::minimumProbabilityPresent() const
    {
        return minimum_probability_present_;
    }

    void ModeAPresentConfig::minimumProbabilityPresent(float value)
    {
        minimum_probability_present_ = value;
    }
}
