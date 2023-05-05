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
#include "eval/requirement/base/base.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttable.h"
#include "evaluationtargetdata.h"
#include "evaluationmanager.h"
#include "evaluationresultsgenerator.h"
#include "sectorlayer.h"

using namespace std;
using namespace nlohmann;

namespace EvaluationRequirementResult
{
const std::string Single::tr_details_table_name_ {"Target Reports Details"};
const std::string Single::target_table_name_     {"Targets"};

Single::Single(const std::string& type, 
               const std::string& result_id,
               std::shared_ptr<EvaluationRequirement::Base> requirement,
               const SectorLayer& sector_layer,
               unsigned int utn,
               const EvaluationTargetData* target,
               EvaluationManager& eval_man,
               const EvaluationDetails& details)
    : Base (type, result_id, requirement, sector_layer, eval_man),
      utn_ (utn), target_(target)
{
    setDetails(details);
}

Single::~Single() = default;

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

std::string Single::getRequirementSectionID () // TODO hack
{
    if (eval_man_.reportSplitResultsByMOPS())
    {
        string tmp = target()->mopsVersionStr();

        if (!tmp.size())
            tmp = "Unknown";

        tmp = "MOPS "+tmp+" Sum";

        return "Sectors:"+requirement_->groupName()+" "+sector_layer_.name()+":"+tmp+":"+requirement_->name();
    }
    else if (eval_man_.reportSplitResultsByACOnlyMS())
    {
        string tmp = "Primary";

        if (target()->isModeS())
            tmp = "Mode S";
        else if (target()->isModeACOnly())
            tmp = "Mode A/C";
        else
            assert (target()->isPrimaryOnly());

        return "Sectors:"+requirement_->groupName()+" "+sector_layer_.name()+":"+tmp+" Sum"+":"+requirement_->name();
    }
    else
        return "Sectors:"+requirement_->groupName()+" "+sector_layer_.name()+":Sum:"+requirement_->name();
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
        utn_table.addRow({"Callsign", "Mode S target identification(s)", target_->acidsStr().c_str()}, this);
        utn_table.addRow({"Target Addr.", "Mode S target address(es)", target_->acadsStr().c_str()}, this);
        utn_table.addRow({"Mode 3/A", "Mode 3/A code(s)", target_->modeACodesStr().c_str()}, this);
        utn_table.addRow({"Mode C Min", "Minimum Mode C code [ft]", target_->modeCMinStr().c_str()}, this);
        utn_table.addRow({"Mode C Max", "Maximum Mode C code [ft]", target_->modeCMaxStr().c_str()}, this);
    }
}

void Single::addAnnotationFeatures(nlohmann::json::object_t& viewable)
{
    if (!viewable.count("annotations"))
        viewable["annotations"] = json::array();

    if (!viewable.at("annotations").size()) // not yet initialized
    {
        // errors
        viewable.at("annotations").push_back(json::object()); // errors

        {
            viewable.at("annotations").at(0)["name"] = result_id_+" Errors";
            viewable.at("annotations").at(0)["features"] = json::array();

            // lines
            viewable.at("annotations").at(0).at("features").push_back(json::object());

            json& feature_lines = viewable.at("annotations").at(0).at("features").at(0);

            feature_lines["type"] = "feature";
            feature_lines["geometry"] = json::object();
            feature_lines.at("geometry")["type"] = "lines";
            feature_lines.at("geometry")["coordinates"] = json::array();

            feature_lines["properties"] = json::object();
            feature_lines.at("properties")["color"] = "#FF0000";
            feature_lines.at("properties")["line_width"] = 2;

            // symbols

            viewable.at("annotations").at(0).at("features").push_back(json::object());

            json& feature_points = viewable.at("annotations").at(0).at("features").at(1);

            feature_points["type"] = "feature";
            feature_points["geometry"] = json::object();
            feature_points.at("geometry")["type"] = "points";
            feature_points.at("geometry")["coordinates"] = json::array();

            feature_points["properties"] = json::object();
            feature_points.at("properties")["color"] = "#FF0000";
            feature_points.at("properties")["symbol"] = "circle";
            feature_points.at("properties")["symbol_size"] = 8;

        }

        // ok
        {
            viewable.at("annotations").push_back(json::object()); // ok
            viewable.at("annotations").at(1)["name"] = result_id_+" OK";
            viewable.at("annotations").at(1)["features"] = json::array();

            // lines
            viewable.at("annotations").at(1).at("features").push_back(json::object());

            json& feature_lines = viewable.at("annotations").at(1).at("features").at(0);

            feature_lines["type"] = "feature";
            feature_lines["geometry"] = json::object();
            feature_lines.at("geometry")["type"] = "lines";
            feature_lines.at("geometry")["coordinates"] = json::array();

            feature_lines["properties"] = json::object();
            feature_lines.at("properties")["color"] = "#00FF00";
            feature_lines.at("properties")["line_width"] = 2;

            // symbols

            viewable.at("annotations").at(1).at("features").push_back(json::object());

            json& feature_points = viewable.at("annotations").at(1).at("features").at(1);

            feature_points["type"] = "feature";
            feature_points["geometry"] = json::object();
            feature_points.at("geometry")["type"] = "points";
            feature_points.at("geometry")["coordinates"] = json::array();

            feature_points["properties"] = json::object();
            feature_points.at("properties")["color"] = "#00FF00";
            feature_points.at("properties")["symbol"] = "circle";
            feature_points.at("properties")["symbol_size"] = 8;
        }
    }
}

std::unique_ptr<Single::EvaluationDetails> Single::generateDetails() const
{
    if (!requirement_)
        return {};

    if (!eval_man_.getData().hasTargetData(utn_))
        return {};

    const auto& data = eval_man_.getData().targetData(utn_);

    auto result = requirement_->evaluate(data, requirement_, sector_layer_);
    if (!result)
        return {};

    auto eval_details = new EvaluationDetails;
    *eval_details = result->getDetails();

    return std::unique_ptr<EvaluationDetails>(eval_details);
}

}
