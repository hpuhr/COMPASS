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
//#include "evaluationresultsgenerator.h"
#include "sectorlayer.h"
#include "viewpoint.h"

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

    annotation_type_names_[AnnotationType::TypeHighlight] = "Selected";
    annotation_type_names_[AnnotationType::TypeError] = "Errors";
    annotation_type_names_[AnnotationType::TypeOk] =  "OK";
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
    if (eval_man_.settings().report_split_results_by_mops_)
    {
        string tmp = target()->mopsVersionStr();

        if (!tmp.size())
            tmp = "Unknown";

        tmp = "MOPS "+tmp+" Sum";

        return "Sectors:"+requirement_->groupName()+" "+sector_layer_.name()+":"+tmp+":"+requirement_->name();
    }
    else if (eval_man_.settings().report_split_results_by_aconly_ms_)
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

//void Single::addAnnotationFeatures(nlohmann::json::object_t& viewable,
//                                   bool overview,
//                                   bool add_highlight)  const
//{
//    if (!viewable.count("annotations"))
//        viewable["annotations"] = json::array();

//    auto addAnnotation = [ & ] (const std::string& name,
//            const std::string& symbol_color,
//            const std::string& symbol,
//            const std::string& point_color,
//            int point_size,
//            const std::string& line_color,
//            int line_width)
//    {
//        viewable.at("annotations").push_back(json::object()); // errors

//        nlohmann::json& annotation = viewable.at("annotations").back();

//        annotation["name"        ] = name;
//        annotation["symbol_color"] = symbol_color;
//        annotation["features"    ] = json::array();

//        // lines
//        annotation.at("features").push_back(json::object());

//        json& feature_lines = annotation.at("features").back();

//        feature_lines["type"] = "feature";
//        feature_lines["geometry"] = json::object();
//        feature_lines.at("geometry")["type"] = "lines";
//        feature_lines.at("geometry")["coordinates"] = json::array();

//        feature_lines["properties"] = json::object();
//        feature_lines.at("properties")["color"] = line_color;
//        feature_lines.at("properties")["line_width"] = line_width;

//        // symbols

//        annotation.at("features").push_back(json::object());

//        json& feature_points = annotation.at("features").back();

//        feature_points["type"] = "feature";
//        feature_points["geometry"] = json::object();
//        feature_points.at("geometry")["type"] = "points";
//        feature_points.at("geometry")["coordinates"] = json::array();

//        feature_points["properties"] = json::object();
//        feature_points.at("properties")["color"] = point_color;

//        feature_points.at("properties")["symbol"] = symbol;
//        feature_points.at("properties")["symbol_size"] = point_size;
//    };

//    if (!viewable.at("annotations").size()) // not yet initialized
//    {
//        //ATTENTION: !ORDER IMPORTANT!

//        // highlight
//        if (add_highlight)
//        {
//            addAnnotation(result_id_+" Selected",
//                          "#FFFF00",
//                          overview ? "circle" : "border",
//                          "#FFFF00",
//                          overview ? 8 : 10,
//                          "#FFFF00",
//                          4);
//        }

//        // errors
//        addAnnotation(result_id_+" Errors",
//                      "#FF6666",
//                      overview ? "circle" : "border",
//                      "#FF6666",
//                      overview ? 8 : 10,
//                      "#FF6666",
//                      2);
//        // ok
//        addAnnotation(result_id_+" OK",
//                      "#66FF66",
//                      overview ? "circle" : "border",
//                      "#66FF66",
//                      overview ? 8 : 10,
//                      "#66FF66",
//                      2);

        
//    }
//}

nlohmann::json& Single::annotationPointCoords(nlohmann::json::object_t& viewable, AnnotationType type, bool overview) const
{
    nlohmann::json& annotation = getOrCreateAnnotation(viewable, type, overview);

    assert (annotation.contains("features")
            && annotation.at("features").size() >= 2
            && annotation.at("features").at( 1 ).count("geometry"));

    return annotation.at("features").at( 1 ).at("geometry")["coordinates"];
}

nlohmann::json& Single::annotationLineCoords(nlohmann::json::object_t& viewable, AnnotationType type, bool overview) const
{
    nlohmann::json& annotation = getOrCreateAnnotation(viewable, type, overview);

    assert (annotation.contains("features")
            && annotation.at("features").size() >= 1
            && annotation.at("features").at( 0 ).count("geometry"));

    return annotation.at("features").at( 0 ).at("geometry")["coordinates"];
}

nlohmann::json& Single::getOrCreateAnnotation(nlohmann::json::object_t& viewable, AnnotationType type, bool overview) const
{
    string anno_name = annotation_type_names_.at(type);

    loginf << "Single: getOrCreateAnnotation: anno_name '" << anno_name << "' overview " << overview;

    if (!viewable.count(ViewPoint::VP_ANNOTATION_KEY))
        viewable[ViewPoint::VP_ANNOTATION_KEY] = json::array();

    auto& annos = viewable.at(ViewPoint::VP_ANNOTATION_KEY);
    assert (annos.is_array());

    auto insertAnnotation = [ & ] (const std::string& name,
            unsigned int position,
            const std::string& symbol_color,
            const std::string& symbol,
            const std::string& point_color,
            int point_size,
            const std::string& line_color,
            int line_width)
    {
        loginf << "Single: getOrCreateAnnotation: size " << annos.size()
               << " creating '" << name << "' at pos " << position;

        for (unsigned int cnt=0; cnt < annos.size(); ++cnt)
            loginf << "Single: getOrCreateAnnotation: start: index " << cnt <<" '" << annos.at(cnt).at("name") << "'";

        annos.insert(annos.begin() + position, json::object()); // errors
        assert (position < annos.size());

        nlohmann::json& annotation = annos.at(position);

        annotation["name"        ] = name;
        annotation["symbol_color"] = symbol_color;
        annotation["features"    ] = json::array();

        // lines
        annotation.at("features").push_back(json::object());

        json& feature_lines = annotation.at("features").back();

        feature_lines["type"] = "feature";
        feature_lines["geometry"] = json::object();
        feature_lines.at("geometry")["type"] = "lines";
        feature_lines.at("geometry")["coordinates"] = json::array();

        feature_lines["properties"] = json::object();
        feature_lines.at("properties")["color"] = line_color;
        feature_lines.at("properties")["line_width"] = line_width;

        // symbols

        annotation.at("features").push_back(json::object());

        json& feature_points = annotation.at("features").back();

        feature_points["type"] = "feature";
        feature_points["geometry"] = json::object();
        feature_points.at("geometry")["type"] = "points";
        feature_points.at("geometry")["coordinates"] = json::array();

        feature_points["properties"] = json::object();
        feature_points.at("properties")["color"] = point_color;

        feature_points.at("properties")["symbol"] = symbol;
        feature_points.at("properties")["symbol_size"] = point_size;

        for (unsigned int cnt=0; cnt < annos.size(); ++cnt)
            loginf << "Single: getOrCreateAnnotation: end: index " << cnt <<" '" << annos.at(cnt).at("name") << "'";
    };

    //ATTENTION: !ORDER IMPORTANT!

    if (type == AnnotationType::TypeHighlight)
    {
        // should be first

        if (!annos.size() || annos.at(0).at("name") != anno_name)
        { // empty or not selected, add at first position
            insertAnnotation(anno_name, 0,
                             "#FFFF00",
                             overview ? "circle" : "border",
                             "#FFFF00",
                             overview ? 8 : 12,
                             "#FFFF00",
                             4);
        }

        assert (annos.at(0).at("name") == anno_name);
        return annos.at(0);
    }
    else if (type == AnnotationType::TypeError)
    {
        // should be first or second

        bool insert_needed = false;
        unsigned int anno_pos = 0;

        if (!annos.size()) // empty, add at first pos
        {
            insert_needed = true;
            anno_pos = 0;
        }
        else if (annos.at(0).at("name") != anno_name)
        {
            if (annos.size() > 1 && annos.at(1).at("name") == anno_name)
            {
                // found at second pos
                anno_pos = 1;
            }
            else if (annos.at(0).at("name") == annotation_type_names_.at(AnnotationType::TypeHighlight))
            {
                // insert after
                insert_needed = true;
                anno_pos = 1;
            }
            else
            {
                assert (annos.at(0).at("name") == annotation_type_names_.at(AnnotationType::TypeOk));
                // insert before
                insert_needed = true;
                anno_pos = 0;
            }
        }
        else
            anno_pos = 0; // only for you

        if (insert_needed)
        {
            insertAnnotation(anno_name, anno_pos,
                             "#FF6666",
                             overview ? "circle" : "border_thick",
                             "#FF6666",
                             overview ? 8 : 16,
                             "#FF6666",
                             4);
        }

        assert (annos.at(anno_pos).at("name") == anno_name);
        return annos.at(anno_pos);
    }
    else
    {
        assert (type == AnnotationType::TypeOk);
        // should be last

        if (!annos.size() || annos.back().at("name") != anno_name)
        {
            //unsigned int anno_pos = annos.size() ? annos.size()-1 : 0;

            // insert at last
            insertAnnotation(anno_name, annos.size(),
                             "#66FF66",
                             overview ? "circle" : "border",
                             "#66FF66",
                             overview ? 8 : 10,
                             "#66FF66",
                             2);
        }

        assert (annos.back().at("name") == anno_name);
        return annos.back();
    }
}

void Single::addAnnotationPos(nlohmann::json::object_t& viewable, 
                              const EvaluationDetail::Position& pos,
                              AnnotationType type) const
{
    auto& coords = annotationPointCoords(viewable, type);
    coords.push_back(pos.asVector());
}

void Single::addAnnotationLine(nlohmann::json::object_t& viewable,
                               const EvaluationDetail::Position& pos0,
                               const EvaluationDetail::Position& pos1,
                               AnnotationType type) const
{
    auto& coords = annotationLineCoords(viewable, type);
    coords.push_back(pos0.asVector());
    coords.push_back(pos1.asVector());
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
