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

//#include "configurable.h"
#include "eval/requirement/base/probabilitybaseconfig.h"
//#include "eval/requirement/detection/detectionconfigwidget.h"
//#include "eval/requirement/detection/detection.h"

#include <memory>


class Group;
class EvaluationStandard;

namespace EvaluationRequirement
{

class DetectionConfig : public ProbabilityBaseConfig
{
public:
    DetectionConfig(const std::string& class_id, const std::string& instance_id,
                    Group& group, EvaluationStandard& standard, EvaluationManager& eval_man);
    virtual ~DetectionConfig();

    std::shared_ptr<Base> createRequirement() override;

    bool useMinGapLength() const;
    void useMinGapLength(bool value);

    float minGapLength() const;
    void minGapLength(float value);

    bool useMaxGapLength() const;
    void useMaxGapLength(bool value);

    float maxGapLength() const;
    void maxGapLength(float value);

    float updateInterval() const;
    void updateInterval(float value);

    bool invertProb() const;
    void invertProb(bool value);

    bool useMissTolerance() const;
    void useMissTolerance(bool value);

    float missTolerance() const;
    void missTolerance(float value);

    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

    bool holdForAnyTarget() const;
    void holdForAnyTarget(bool value);

protected:
    float update_interval_s_{0};

    bool use_min_gap_length_ {false};
    float min_gap_length_s_{0};

    bool use_max_gap_length_ {false};
    float max_gap_length_s_{0};

    bool invert_prob_ {false};

    bool use_miss_tolerance_{false};
    float miss_tolerance_s_{0};

    bool hold_for_any_target_ {false}; // if requirement must hold for any target (all single targets)

    virtual void createWidget() override;
};

}

#endif // EVALUATIONREQUIREMENTDETECTIONCONFIG_H
