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

#include "eval/results/joined.h"
#include "eval/results/report/sectioncontenttable.h"
#include "eval/requirement/base/base.h"
#include "sectorlayer.h"

namespace EvaluationRequirementResult
{


    Joined::Joined(const std::string& type, const std::string& result_id,
                   std::shared_ptr<EvaluationRequirement::Base> requirement, const SectorLayer& sector_layer,
                   EvaluationManager& eval_man)
        : Base(type, result_id, requirement, sector_layer, eval_man)
    {
    }

    void Joined::join(std::shared_ptr<Base> other)
    {
        results_.push_back(other);
    }

    unsigned int Joined::numResults()
    {
        return results_.size();
    }

    unsigned int Joined::numUsableResults()
    {
        unsigned int cnt {0};

        for (auto& result_it : results_)
            if (result_it->use())
                ++cnt;

        return cnt;
    }

    unsigned int Joined::numUnusableResults()
    {
        unsigned int cnt {0};

        for (auto& result_it : results_)
            if (!result_it->use())
                ++cnt;

        return cnt;
    }

    void Joined::addCommonDetails (EvaluationResultsReport::SectionContentTable& sector_details_table)
    {
        sector_details_table.addRow({"Sector Layer", "Name of the sector layer", sector_layer_.name().c_str()}, this);
        sector_details_table.addRow({"Reqirement Group", "Name of the requirement group",
                                     requirement_->groupName().c_str()}, this);
        sector_details_table.addRow({"Reqirement", "Name of the requirement", requirement_->name().c_str()}, this);
        sector_details_table.addRow({"Num Results", "Total number of results", numResults()}, this);
        sector_details_table.addRow({"Num Usable Results", "Number of usable results", numUsableResults()}, this);
        sector_details_table.addRow({"Num Unusable Results", "Number of unusable results", numUnusableResults()}, this);
    }

}
