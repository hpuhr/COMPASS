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

#include "eval/requirement/extra/trackconfig.h"
#include "eval/requirement/extra/trackconfigwidget.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base/base.h"

using namespace std;

namespace EvaluationRequirement
{

    ExtraTrackConfig::ExtraTrackConfig(
            const std::string& class_id, const std::string& instance_id,
            Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
        : BaseConfig(class_id, instance_id, group, standard, eval_man)
    {
        registerParameter("min_duration", &min_duration_, 60.0);
        registerParameter("min_num_updates", &min_num_updates_, 10);
        registerParameter("ignore_primary_only", &ignore_primary_only_, true);
        registerParameter("maximum_probability", &maximum_probability_, 0.0);
    }

    ExtraTrackConfig::~ExtraTrackConfig()
    {

    }

    void ExtraTrackConfig::addGUIElements(QFormLayout* layout)
    {
        assert (layout);

        BaseConfig::addGUIElements(layout);
    }

    ExtraTrackConfigWidget* ExtraTrackConfig::widget()
    {
        if (!widget_)
            widget_.reset(new ExtraTrackConfigWidget(*this));

        return widget_.get();
    }

    std::shared_ptr<Base> ExtraTrackConfig::createRequirement()
    {
        shared_ptr<ExtraTrack> req = make_shared<ExtraTrack>(
                    name_, short_name_, group_.name(), eval_man_, min_duration_,
                    min_num_updates_, ignore_primary_only_, maximum_probability_);

        return req;
    }

    float ExtraTrackConfig::minDuration() const
    {
        return min_duration_;
    }

    void ExtraTrackConfig::minDuration(float value)
    {
        min_duration_ = value;
    }

    unsigned int ExtraTrackConfig::minNumUpdates() const
    {
        return min_num_updates_;
    }

    void ExtraTrackConfig::minNumUpdates(unsigned int value)
    {
        min_num_updates_ = value;
    }

    bool ExtraTrackConfig::ignorePrimaryOnly() const
    {
        return ignore_primary_only_;
    }

    void ExtraTrackConfig::ignorePrimaryOnly(bool value)
    {
        ignore_primary_only_ = value;
    }

    float ExtraTrackConfig::maximumProbability() const
    {
        return maximum_probability_;
    }

    void ExtraTrackConfig::maximumProbability(float value)
    {
        maximum_probability_ = value;
    }
}
