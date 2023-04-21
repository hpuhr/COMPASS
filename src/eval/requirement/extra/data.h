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

#ifndef EVALUATIONREQUIREMENTEXTRADATA_H
#define EVALUATIONREQUIREMENTEXTRADATA_H

#include "eval/requirement/base/probabilitybase.h"
#include "dbcontent/target/targetposition.h"

#include <QVariant>

#include "boost/date_time/posix_time/ptime.hpp"

namespace EvaluationRequirement
{

class ExtraDataDetail
{
public:
    ExtraDataDetail(
            boost::posix_time::ptime timestamp, dbContent::TargetPosition pos_current,
            bool inside, bool extra, bool ref_exists, const std::string& comment)
        : timestamp_(timestamp), pos_current_(pos_current), inside_(inside), extra_(extra), ref_exists_(ref_exists),
          comment_(comment)
    {
    }

    boost::posix_time::ptime timestamp_;
    dbContent::TargetPosition pos_current_;
    bool inside_ {false};
    bool extra_ {false};

    bool ref_exists_ {false};

    std::string comment_;
};

class ExtraData : public ProbabilityBase
{
public:
    ExtraData(
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
#endif // EVALUATIONREQUIREMENTEXTRADATA_H
