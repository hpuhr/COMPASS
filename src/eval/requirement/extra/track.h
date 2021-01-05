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

#ifndef EVALUATIONREQUIREMENTEXTRATRACK_H
#define EVALUATIONREQUIREMENTEXTRATRACK_H

#include "eval/requirement/base.h"
#include "evaluationtargetposition.h"

#include <QVariant>

namespace EvaluationRequirement
{

class ExtraTrackDetail
{
public:
    ExtraTrackDetail(
            float tod, EvaluationTargetPosition pos_current, bool inside, QVariant track_num, bool extra,
            const std::string& comment)
        : tod_(tod), pos_current_(pos_current), track_num_(track_num), inside_(inside),
          extra_(extra), comment_(comment)
    {
    }

    float tod_ {0};
    EvaluationTargetPosition pos_current_;
    QVariant track_num_;
    bool inside_ {false};
    bool extra_ {false};

    std::string comment_;
};

class ExtraTrack : public Base
{
public:
    ExtraTrack(
            const std::string& name, const std::string& short_name, const std::string& group_name,
            EvaluationManager& eval_man,
            float min_duration, unsigned int min_num_updates, bool ignore_primary_only,
            float maximum_probability);

    float minDuration() const;

    unsigned int minNumUpdates() const;

    bool ignorePrimaryOnly() const;

    float maximumProbability() const;

    virtual std::shared_ptr<EvaluationRequirementResult::Single> evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
            const SectorLayer& sector_layer) override;

protected:
    float min_duration_{0};
    unsigned int min_num_updates_ {0};
    bool ignore_primary_only_ {true};
    float maximum_probability_{0};
};

}
#endif // EVALUATIONREQUIREMENTEXTRATRACK_H
