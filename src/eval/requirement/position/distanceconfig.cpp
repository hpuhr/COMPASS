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

#include "eval/requirement/position/distanceconfig.h"
#include "eval/requirement/position/distanceconfigwidget.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base/base.h"

using namespace std;


namespace EvaluationRequirement
{
    PositionDistanceConfig::PositionDistanceConfig(
            const std::string& class_id, const std::string& instance_id,
            Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
    : BaseConfig(class_id, instance_id, group, standard, eval_man)
    {
        registerParameter("max_abs_value", &dist_abs_value_, 50.0);
        registerParameter("dist_abs_value_check_type", (unsigned int*)&dist_abs_value_check_type_,
                          (unsigned int) COMPARISON_TYPE::LESS_THAN_OR_EQUAL);
    }

    PositionDistanceConfig::~PositionDistanceConfig()
    {

    }

    std::shared_ptr<Base> PositionDistanceConfig::createRequirement()
    {
        shared_ptr<PositionDistance> req = make_shared<PositionDistance>(
                    name_, short_name_, group_.name(), prob_, prob_check_type_, eval_man_,
                    dist_abs_value_, dist_abs_value_check_type_);

        return req;
    }

    float PositionDistanceConfig::distAbsValuse() const
    {
        return dist_abs_value_;
    }

    void PositionDistanceConfig::distAbsValuse(float value)
    {
        dist_abs_value_ = value;
    }
    
    COMPARISON_TYPE PositionDistanceConfig::distAbsValueCheckType() const
    {
        return dist_abs_value_check_type_;
    }
    
    void PositionDistanceConfig::distAbsValueCheckType(const COMPARISON_TYPE &type)
    {
        dist_abs_value_check_type_ = type;
    }
    
    void PositionDistanceConfig::createWidget()
    {
        assert (!widget_);
        widget_.reset(new PositionDistanceConfigWidget(*this));
        assert (widget_);
    }
}
