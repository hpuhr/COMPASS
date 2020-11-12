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

#include "eval/requirement/mode_a/modea.h"
//#include "eval/results/identification/single.h"
#include "evaluationdata.h"
#include "logger.h"
#include "stringconv.h"
#include "sectorlayer.h"

using namespace std;
using namespace Utils;

namespace EvaluationRequirement
{

    ModeA::ModeA(const std::string& name, const std::string& short_name, const std::string& group_name,
                 EvaluationManager& eval_man, float max_ref_time_diff,
                 bool use_minimum_probability_existing, float minimum_probability_existing,
                 bool use_maximum_probability_false, float maximum_probability_false)
    : Base(name, short_name, group_name, eval_man),
      max_ref_time_diff_(max_ref_time_diff),
      use_minimum_probability_existing_(use_minimum_probability_existing),
      minimum_probability_existing_(minimum_probability_existing),
      use_maximum_probability_false_(use_maximum_probability_false),
      maximum_probability_false_(maximum_probability_false)
    {

    }

    std::shared_ptr<EvaluationRequirementResult::Single> ModeA::evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
            const SectorLayer& sector_layer)
    {
        // TODO
    }
}
