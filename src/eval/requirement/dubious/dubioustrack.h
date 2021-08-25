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

#ifndef EVALUATIONREQUIREMENTDUBIOUSTRACK_H
#define EVALUATIONREQUIREMENTDUBIOUSTRACK_H

#include "eval/requirement/base/base.h"
#include "evaluationtargetposition.h"

#include <QVariant>

#include <cassert>

namespace EvaluationRequirement
{

class DubiousTrackDetail
{
public:
    DubiousTrackDetail(
            float tod_begin, float tod_end, unsigned int num_pos_inside)
        : tod_begin_(tod_begin), tod_end_(tod_end), num_pos_inside_(num_pos_inside)
    {
        assert (tod_end_ >= tod_begin_);
        duration_ = tod_end_ - tod_begin_;
    }

    float tod_begin_ {0};
    float tod_end_ {0};
    float duration_;
    unsigned int num_pos_inside_{0};
    bool is_dubious_ {false};

    std::string dubious_reason_;

    EvaluationTargetPosition pos_begin_;
    EvaluationTargetPosition pos_last_;
};


class DubiousTrack : public Base
{
public:
    DubiousTrack(const std::string& name, const std::string& short_name, const std::string& group_name,
                 bool mark_primary_only, bool use_min_updates, unsigned int min_updates,
                 bool use_min_duration, float min_duration, float prob,
                 COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man);

    virtual std::shared_ptr<EvaluationRequirementResult::Single> evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
            const SectorLayer& sector_layer) override;

    bool markPrimaryOnly() const;

    bool useMinUpdates() const;

    unsigned int minUpdates() const;

    bool useMinDuration() const;

    float minDuration() const;

protected:
    bool mark_primary_only_ {true};

    bool use_min_updates_ {true};
    unsigned int min_updates_ {10};

    bool use_min_duration_ {true};
    float min_duration_ {30.0};
};

}

#endif // EVALUATIONREQUIREMENTDUBIOUSTRACK_H
