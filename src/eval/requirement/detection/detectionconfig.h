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

#ifndef EVALUATIONREQUIREMENTDETECTIONCONFIG_H
#define EVALUATIONREQUIREMENTDETECTIONCONFIG_H

#include "configurable.h"
#include "eval/requirement/base/baseconfig.h"
#include "eval/requirement/detection/detectionconfigwidget.h"
#include "eval/requirement/detection/detection.h"

#include <memory>


class Group;
class EvaluationStandard;

namespace EvaluationRequirement
{

    class DetectionConfig : public BaseConfig
    {
    public:
        DetectionConfig(const std::string& class_id, const std::string& instance_id,
                        Group& group, EvaluationStandard& standard,
                        EvaluationManager& eval_man);
        virtual ~DetectionConfig();

        std::shared_ptr<Base> createRequirement() override;

        float updateInterval() const;
        void updateInterval(float value);

        float maxRefTimeDiff() const;
        void maxRefTimeDiff(float value);

        float minimumProbability() const;
        void minimumProbability(float value);

//        bool useMaxGapInterval() const;
//        void useMaxGapInterval(bool value);

//        float maxGapInterval() const;
//        void maxGapInterval(float value);

        bool useMissTolerance() const;
        void useMissTolerance(bool value);

        float missTolerance() const;
        void missTolerance(float value);

    protected:
        float update_interval_s_{0};

        float max_ref_time_diff_ {0};

        float minimum_probability_{0};

//        bool use_max_gap_interval_{true};
//        float max_gap_interval_s_{0};

        bool use_miss_tolerance_{false};
        float miss_tolerance_s_{0};

        virtual void createWidget() override;
    };

}

#endif // EVALUATIONREQUIREMENTDETECTIONCONFIG_H
