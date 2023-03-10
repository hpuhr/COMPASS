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

#ifndef EVALUATIONREQUIREMENTRACKANGLE_H
#define EVALUATIONREQUIREMENTRACKANGLE_H

#include "eval/requirement/base/base.h"
#include "eval/requirement/trackangle/detail.h"
#include "transformation.h"

namespace EvaluationRequirement
{

class TrackAngle : public Base
{
public:
    TrackAngle(
            const std::string& name, const std::string& short_name, const std::string& group_name,
            float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man,
            float threshold, bool use_minimum_speed, float minimum_speed,
            COMPARISON_TYPE threshold_value_check_type,
            bool failed_values_of_interest);

    float threshold() const;
    bool useMinimumSpeed() const;
    float minimumSpeed() const;

    COMPARISON_TYPE thresholdValueCheckType() const;
    bool failedValuesOfInterest() const;

    virtual std::shared_ptr<EvaluationRequirementResult::Single> evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
            const SectorLayer& sector_layer) override;

protected:
    float threshold_ {15.0};

    bool use_minimum_speed_ {true};
    float minimum_speed_ {3.0}; // m/s

    COMPARISON_TYPE threshold_value_check_type_ {COMPARISON_TYPE::LESS_THAN_OR_EQUAL};

    bool failed_values_of_interest_ {true};

    Transformation trafo_;

    // deg, m/s
    EvaluationTargetPosition getPositionAtAngle(const EvaluationTargetPosition& org, double track_angle, double speed);
};

}

#endif // EVALUATIONREQUIREMENTRACKANGLE_H
