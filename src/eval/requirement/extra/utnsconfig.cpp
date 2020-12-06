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

#include "eval/requirement/extra/utnsconfig.h"
#include "eval/requirement/extra/utnsconfigwidget.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base.h"

using namespace std;

namespace EvaluationRequirement
{

    ExtraDataConfig::ExtraDataConfig(
            const std::string& class_id, const std::string& instance_id,
            Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
        : Config(class_id, instance_id, group, standard, eval_man)
    {
        registerParameter("max_ref_time_diff", &max_ref_time_diff_, 5.0);
        registerParameter("min_duration", &min_duration_, 60.0);
        registerParameter("min_num_updates", &min_num_updates_, 10);
        registerParameter("ignore_primary_only", &ignore_primary_only_, true);
        registerParameter("maximum_probability", &maximum_probability_, 0.0);
    }

    ExtraDataConfig::~ExtraDataConfig()
    {

    }

    void ExtraDataConfig::addGUIElements(QFormLayout* layout)
    {
        assert (layout);

        Config::addGUIElements(layout);
    }

    ExtraDataConfigWidget* ExtraDataConfig::widget()
    {
        if (!widget_)
            widget_.reset(new ExtraDataConfigWidget(*this));

        return widget_.get();
    }

    std::shared_ptr<Base> ExtraDataConfig::createRequirement()
    {
        shared_ptr<ExtraData> req = make_shared<ExtraData>(
                    name_, short_name_, group_.name(), eval_man_, max_ref_time_diff_, min_duration_,
                    min_num_updates_, ignore_primary_only_, maximum_probability_);

        return req;
    }

    float ExtraDataConfig::maxRefTimeDiff() const
    {
        return max_ref_time_diff_;
    }

    void ExtraDataConfig::maxRefTimeDiff(float value)
    {
        max_ref_time_diff_ = value;
    }

    float ExtraDataConfig::minDuration() const
    {
        return min_duration_;
    }

    void ExtraDataConfig::minDuration(float value)
    {
        min_duration_ = value;
    }

    unsigned int ExtraDataConfig::minNumUpdates() const
    {
        return min_num_updates_;
    }

    void ExtraDataConfig::minNumUpdates(unsigned int value)
    {
        min_num_updates_ = value;
    }

    bool ExtraDataConfig::ignorePrimaryOnly() const
    {
        return ignore_primary_only_;
    }

    void ExtraDataConfig::ignorePrimaryOnly(bool value)
    {
        ignore_primary_only_ = value;
    }

    float ExtraDataConfig::maximumProbability() const
    {
        return maximum_probability_;
    }

    void ExtraDataConfig::maximumProbability(float value)
    {
        maximum_probability_ = value;
    }
}
