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

#include "eval/results/single.h"
#include "eval/requirement/base.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttable.h"
#include "evaluationtargetdata.h"
#include "sectorlayer.h"

namespace EvaluationRequirementResult
{

    Single::Single(
            const std::string& type, const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
            const SectorLayer& sector_layer, unsigned int utn, const EvaluationTargetData* target,
            EvaluationManager& eval_man)
        : Base(type, result_id, requirement, sector_layer, eval_man), utn_(utn), target_(target)
    {
    }

    unsigned int Single::utn() const
    {
        return utn_;
    }

    const EvaluationTargetData* Single::target() const
    {
        return target_;
    }

    void Single::updateUseFromTarget ()
    {
        use_ = result_usable_ && target_->use();
    }

    std::string Single::getTargetSectionID()
    {
        return "Targets:UTN "+to_string(utn_);
    }

    std::string Single::getTargetRequirementSectionID ()
    {
        return getTargetSectionID()+":"+sector_layer_.name()+":"+requirement_->groupName()+":"+requirement_->name();
    }

    void Single::addCommonDetails (shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        EvaluationResultsReport::Section& utn_section = root_item->getSection(getTargetSectionID());

        if (!utn_section.hasTable("details_overview_table")) // only set once, called from many
        {
            utn_section.addTable("details_overview_table", 3, {"Name", "comment", "Value"}, false);

            EvaluationResultsReport::SectionContentTable& utn_table =
                    utn_section.getTable("details_overview_table");

            utn_table.addRow({"UTN", "Unique Target Number", utn_}, this);
            utn_table.addRow({"Begin", "Begin time of target", target_->timeBeginStr().c_str()}, this);
            utn_table.addRow({"End", "End time of target", target_->timeEndStr().c_str()}, this);
            utn_table.addRow({"Callsign", "Mode S target identification(s)", target_->callsignsStr().c_str()}, this);
            utn_table.addRow({"Target Addr.", "Mode S target address(es)", target_->targetAddressesStr().c_str()}, this);
            utn_table.addRow({"Mode 3/A", "Mode 3/A code(s)", target_->modeACodesStr().c_str()}, this);
            utn_table.addRow({"Mode C Min", "Minimum Mode C code [ft]", target_->modeCMinStr().c_str()}, this);
            utn_table.addRow({"Mode C Max", "Maximum Mode C code [ft]", target_->modeCMaxStr().c_str()}, this);
        }
    }

}
