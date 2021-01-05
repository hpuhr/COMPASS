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

#include "eval/requirement/detection/detectionconfig.h"
#include "eval/requirement/detection/detectionconfigwidget.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base/base.h"

using namespace std;

namespace EvaluationRequirement
{

    DetectionConfig::DetectionConfig(
            const std::string& class_id, const std::string& instance_id,
            Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
        : BaseConfig(class_id, instance_id, group, standard, eval_man)
    {
        registerParameter("update_interval", &update_interval_s_, 1);

        registerParameter("max_ref_time_diff", &max_ref_time_diff_, 5.0);

        registerParameter("minimum_probability", &minimum_probability_, 0.99);

//        registerParameter("use_max_gap_interval", &use_max_gap_interval_, true);
//        registerParameter("max_gap_interval", &max_gap_interval_s_, 30);

        registerParameter("use_miss_tolerance", &use_miss_tolerance_, false);
        registerParameter("miss_tolerance", &miss_tolerance_s_, 0.01);

    }

    DetectionConfig::~DetectionConfig()
    {

    }

    std::shared_ptr<Base> DetectionConfig::createRequirement()
    {
        shared_ptr<Detection> req = make_shared<Detection>(
                    name_, short_name_, group_.name(), eval_man_, update_interval_s_,
                    max_ref_time_diff_, minimum_probability_,
                    //use_max_gap_interval_, max_gap_interval_s_,
                    use_miss_tolerance_, miss_tolerance_s_);

        return req;
    }

    float DetectionConfig::updateInterval() const
    {
        return update_interval_s_;
    }

    void DetectionConfig::updateInterval(float value)
    {
        update_interval_s_ = value;
    }

    float DetectionConfig::maxRefTimeDiff() const
    {
        return max_ref_time_diff_;
    }

    void DetectionConfig::maxRefTimeDiff(float value)
    {
        max_ref_time_diff_ = value;
    }

    float DetectionConfig::minimumProbability() const
    {
        return minimum_probability_;
    }

    void DetectionConfig::minimumProbability(float value)
    {
        minimum_probability_ = value;
    }

//    bool DetectionConfig::useMaxGapInterval() const
//    {
//        return use_max_gap_interval_;
//    }

//    void DetectionConfig::useMaxGapInterval(bool value)
//    {
//        use_max_gap_interval_ = value;
//    }

//    float DetectionConfig::maxGapInterval() const
//    {
//        return max_gap_interval_s_;
//    }

//    void DetectionConfig::maxGapInterval(float value)
//    {
//        max_gap_interval_s_ = value;
//    }

    bool DetectionConfig::useMissTolerance() const
    {
        return use_miss_tolerance_;
    }

    void DetectionConfig::useMissTolerance(bool value)
    {
        use_miss_tolerance_ = value;
    }

    float DetectionConfig::missTolerance() const
    {
        return miss_tolerance_s_;
    }

    void DetectionConfig::missTolerance(float value)
    {
        miss_tolerance_s_ = value;
    }

    void DetectionConfig::createWidget()
    {
        assert (!widget_);
        widget_.reset(new DetectionConfigWidget(*this));
        assert (widget_);
    }

}
