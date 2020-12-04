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

#include "eval/requirement/extra/utns.h"
#include "eval/results/extra/utnssingle.h"
#include "evaluationdata.h"
#include "evaluationmanager.h"
#include "logger.h"
#include "stringconv.h"
#include "sectorlayer.h"
#include "timeperiod.h"

using namespace std;
using namespace Utils;

namespace EvaluationRequirement
{

    ExtraUTNs::ExtraUTNs(
            const std::string& name, const std::string& short_name, const std::string& group_name,
            EvaluationManager& eval_man,
            float min_duration, unsigned int min_num_updates, bool ignore_primary_only,
            float maximum_probability)
        : Base(name, short_name, group_name, eval_man), min_duration_(min_duration),
          min_num_updates_(min_num_updates), ignore_primary_only_(ignore_primary_only),
          maximum_probability_(maximum_probability)
    {

    }

    float ExtraUTNs::minDuration() const
    {
        return min_duration_;
    }

    unsigned int ExtraUTNs::minNumUpdates() const
    {
        return min_num_updates_;
    }

    bool ExtraUTNs::ignorePrimaryOnly() const
    {
        return ignore_primary_only_;
    }

    float ExtraUTNs::maximumProbability() const
    {
        return maximum_probability_;
    }

    std::shared_ptr<EvaluationRequirementResult::Single> ExtraUTNs::evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
            const SectorLayer& sector_layer)
    {
        logdbg << "EvaluationRequirementExtraUTNs '" << name_ << "': evaluate: utn " << target_data.utn_
               << " min_duration " << min_duration_ << " min_num_updates " << min_num_updates_
               << " ignore_primary_only " << ignore_primary_only_ << " maximum_probability " << maximum_probability_;

        bool ignore = false;

        if (target_data.numUpdates() < min_num_updates_)
            ignore = true;

        if (!ignore && target_data.timeDuration() < min_duration_)
            ignore = true;

        if (!ignore && ignore_primary_only_ && target_data.isPrimaryOnly())
            ignore = true;

        bool test_data_only = !target_data.numRefUpdates() && target_data.numTstUpdates();

        return make_shared<EvaluationRequirementResult::SingleExtraUTNs>(
                    "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                    eval_man_, ignore, test_data_only);
    }
}
