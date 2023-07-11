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

#include "eval/requirement/base/probabilitybase.h"
#include "dbcontent/target/targetposition.h"

#include <QVariant>

#include "boost/date_time/posix_time/ptime.hpp"

namespace EvaluationRequirement
{

class ExtraTrack : public ProbabilityBase
{
public:
    ExtraTrack(
            const std::string& name, const std::string& short_name, const std::string& group_name,
            float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man,
            float min_duration, unsigned int min_num_updates, bool ignore_primary_only);

    float minDuration() const;

    unsigned int minNumUpdates() const;

    bool ignorePrimaryOnly() const;

    virtual std::shared_ptr<EvaluationRequirementResult::Single> evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
            const SectorLayer& sector_layer) override;

protected:
    boost::posix_time::time_duration min_duration_;
    unsigned int min_num_updates_ {0};
    bool ignore_primary_only_ {true};
};

}
#endif // EVALUATIONREQUIREMENTEXTRATRACK_H
