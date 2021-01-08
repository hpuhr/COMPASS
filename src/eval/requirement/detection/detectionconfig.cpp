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

    registerParameter("use_min_gap_length", &use_min_gap_length_, false);
    registerParameter("min_gap_length_s", &min_gap_length_s_, 3.0);

    registerParameter("use_max_gap_length", &use_max_gap_length_, false);
    registerParameter("max_gap_length_s", &max_gap_length_s_, 5.0);

    registerParameter("use_miss_tolerance", &use_miss_tolerance_, false);
    registerParameter("miss_tolerance", &miss_tolerance_s_, 0.01);
}

DetectionConfig::~DetectionConfig()
{

}

std::shared_ptr<Base> DetectionConfig::createRequirement()
{
    shared_ptr<Detection> req = make_shared<Detection>(
                name_, short_name_, group_.name(), prob_, prob_check_type_, eval_man_, update_interval_s_,
                use_min_gap_length_, min_gap_length_s_, use_max_gap_length_, max_gap_length_s_,
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

bool DetectionConfig::useMinGapLength() const
{
    return use_min_gap_length_;
}

void DetectionConfig::useMinGapLength(bool value)
{
    use_min_gap_length_ = value;
}

float DetectionConfig::minGapLength() const
{
    return min_gap_length_s_;
}

void DetectionConfig::minGapLength(float value)
{
    min_gap_length_s_ = value;
}

bool DetectionConfig::useMaxGapLength() const
{
    return use_max_gap_length_;
}

void DetectionConfig::useMaxGapLength(bool value)
{
    use_max_gap_length_ = value;
}

float DetectionConfig::maxGapLength() const
{
    return max_gap_length_s_;
}

void DetectionConfig::maxGapLength(float value)
{
    max_gap_length_s_ = value;
}

void DetectionConfig::createWidget()
{
    assert (!widget_);
    widget_.reset(new DetectionConfigWidget(*this));
    assert (widget_);
}

}
