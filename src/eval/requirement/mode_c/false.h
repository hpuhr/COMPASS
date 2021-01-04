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

#ifndef EVALUATIONREQUIREMENTMODECFALSE_H
#define EVALUATIONREQUIREMENTMODECFALSE_H

#include "eval/requirement/base.h"

namespace EvaluationRequirement
{

    class ModeCFalse : public Base
    {
    public:
        ModeCFalse(const std::string& name, const std::string& short_name, const std::string& group_name,
              EvaluationManager& eval_man, float max_ref_time_diff,
              bool use_minimum_probability_present, float minimum_probability_present,
              bool use_maximum_probability_false, float maximum_probability_false,
              float maximum_difference);

        virtual std::shared_ptr<EvaluationRequirementResult::Single> evaluate (
                const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
                const SectorLayer& sector_layer) override;

        float maxRefTimeDiff() const;

        bool useMinimumProbabilityPresent() const;
        float minimumProbabilityPresent() const;

        bool useMaximumProbabilityFalse() const;
        float maximumProbabilityFalse() const;

    protected:
        float max_ref_time_diff_ {0};

        bool use_minimum_probability_present_ {true};
        float minimum_probability_present_{0};

        bool use_maximum_probability_false_ {true};
        float maximum_probability_false_{0};

        float maximum_difference_ {0};
    };

}
#endif // EVALUATIONREQUIREMENTMODECFALSE_H
