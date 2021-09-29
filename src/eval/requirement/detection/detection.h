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

#ifndef EVALUATIONREQUIREMENTDETECTION_H
#define EVALUATIONREQUIREMENTDETECTION_H

#include "eval/requirement/base/base.h"
#include "evaluationtargetposition.h"

#include <QVariant>

namespace EvaluationRequirement
{

class DetectionDetail
{
public:
    DetectionDetail(
            float tod, QVariant d_tod,
            bool miss_occurred, EvaluationTargetPosition pos_current,
            bool ref_exists, int missed_uis, const std::string& comment)
        : tod_(tod), d_tod_(d_tod), miss_occurred_(miss_occurred), pos_current_(pos_current),
          ref_exists_(ref_exists), missed_uis_(missed_uis),
          comment_(comment)
    {
    }

    float tod_ {0};
    QVariant d_tod_;
    bool miss_occurred_ {false};

    EvaluationTargetPosition pos_current_;

    bool ref_exists_ {false};

    int missed_uis_ {0};
    int max_gap_uis_ {0};
    int no_ref_uis_ {0};

    std::string comment_;

    bool has_last_position_ {false};
    EvaluationTargetPosition pos_last_;
};

class Detection : public Base
{
public:
    Detection(
            const std::string& name, const std::string& short_name, const std::string& group_name,
            float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man,
            float update_interval_s,
            bool use_min_gap_length, float min_gap_length_s,
            bool use_max_gap_length, float max_gap_length_s, bool invert_prob,
            bool use_miss_tolerance, float miss_tolerance_s);

    float updateInterval() const;

    bool useMinGapLength() const;

    float minGapLength() const;

    bool useMaxGapLength() const;

    float maxGapLength() const;

    bool invertProb() const;

    bool useMissTolerance() const;

    float missTolerance() const;

    float missThreshold() const;

    virtual std::shared_ptr<EvaluationRequirementResult::Single> evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
            const SectorLayer& sector_layer) override;

protected:
    float update_interval_s_{0};

    bool use_min_gap_length_ {false};
    float min_gap_length_s_{0};

    bool use_max_gap_length_ {false};
    float max_gap_length_s_{0};

    bool invert_prob_ {false};

    bool use_miss_tolerance_{false};
    float miss_tolerance_s_{0};

    bool isMiss (float d_tod);
    unsigned int getNumMisses(float d_tod);
};

}
#endif // EVALUATIONREQUIREMENTDETECTION_H
