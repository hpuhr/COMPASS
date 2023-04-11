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

#include "eval/results/position/distancesingle.h"
#include "eval/results/position/distancejoined.h"
#include "eval/requirement/base/base.h"
#include "eval/requirement/position/distance.h"
#include "evaluationtargetdata.h"
#include "evaluationmanager.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "logger.h"
#include "util/stringconv.h"
#include "util/timeconv.h"
#include "util/number.h"

#include <cassert>
#include <algorithm>

using namespace std;
using namespace Utils;
using namespace nlohmann;

namespace EvaluationRequirementResult
{

SinglePositionDistance::SinglePositionDistance(const std::string& result_id, 
                                               std::shared_ptr<EvaluationRequirement::Base> requirement,
                                               const SectorLayer& sector_layer,
                                               unsigned int utn,
                                               const EvaluationTargetData* target,
                                               EvaluationManager& eval_man,
                                               const EvaluationDetails& details,
                                               unsigned int num_pos,
                                               unsigned int num_no_ref,
                                               unsigned int num_pos_outside,
                                               unsigned int num_pos_inside,
                                               unsigned int num_comp_passed,
                                               unsigned int num_comp_failed,
                                               vector<double> values)
    :   SinglePositionBase("SinglePositionDistance", result_id, requirement, sector_layer, utn, target, eval_man, details,
                           num_pos, num_no_ref,num_pos_outside, num_pos_inside, num_comp_passed, num_comp_failed, values)
{
    update();
}

void SinglePositionDistance::update()
{
    assert (num_no_ref_ <= num_pos_);
    assert (num_pos_ - num_no_ref_ == num_pos_inside_ + num_pos_outside_);

    assert (values_.size() == num_passed_ + num_failed_);

    prob_.reset();

    unsigned int num_distances = values_.size();

    if (num_distances)
    {
        value_min_ = *min_element(values_.begin(), values_.end());
        value_max_ = *max_element(values_.begin(), values_.end());
        value_avg_ = std::accumulate(values_.begin(), values_.end(), 0.0) / (float) num_distances;

        value_var_ = 0;
        for(auto val : values_)
            value_var_ += pow(val - value_avg_, 2);
        value_var_ /= (float)num_distances;

        assert (num_passed_ <= num_distances);
        prob_ = (float)num_passed_/(float)num_distances;
    }
    else
    {
        value_min_ = 0;
        value_max_ = 0;
        value_avg_ = 0;
        value_var_ = 0;
    }

    result_usable_ = prob_.has_value();

    updateUseFromTarget();
}

void SinglePositionDistance::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "SinglePositionDistance " <<  requirement_->name() <<": addToReport";

    // add target to requirements->group->req
    addTargetToOverviewTable(root_item);

    // add requirement results to targets->utn->requirements->group->req
    addTargetDetailsToReport(root_item);

    // TODO add requirement description, methods
}

void SinglePositionDistance::addTargetToOverviewTable(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::Section& tgt_overview_section = getRequirementSection(root_item);

    if (eval_man_.reportShowAdsbInfo())
        addTargetDetailsToTableADSB(tgt_overview_section, target_table_name_);
    else
        addTargetDetailsToTable(tgt_overview_section, target_table_name_);

    if (eval_man_.reportSplitResultsByMOPS()) // add to general sum table
    {
        EvaluationResultsReport::Section& sum_section = root_item->getSection(getRequirementSumSectionID());

        if (eval_man_.reportShowAdsbInfo())
            addTargetDetailsToTableADSB(sum_section, target_table_name_);
        else
            addTargetDetailsToTable(sum_section, target_table_name_);
    }
}

void SinglePositionDistance::addTargetDetailsToTable (
        EvaluationResultsReport::Section& section, const std::string& table_name)
{
    if (!section.hasTable(table_name))
    {
        Qt::SortOrder order = Qt::AscendingOrder;

        if(req()->probCheckType() == EvaluationRequirement::COMPARISON_TYPE::LESS_THAN
                || req()->probCheckType() == EvaluationRequirement::COMPARISON_TYPE::LESS_THAN_OR_EQUAL)
            order = Qt::DescendingOrder;


        section.addTable(table_name, 15,
                         {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                          "DMin", "DMax", "DAvg", "DSDev", "#CF", "#CP", "PCP"}, true, 14, order);
    }

    EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

    QVariant p_min_var;

    if (prob_.has_value())
        p_min_var = roundf(prob_.value() * 10000.0) / 100.0;

    target_table.addRow(
                {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
                 target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
                 target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(), target_->modeCMaxStr().c_str(),
                 Number::round(value_min_,2), // "DMin"
                 Number::round(value_max_,2), // "DMax"
                 Number::round(value_avg_,2), // "DAvg"
                 Number::round(sqrt(value_var_),2), // "DSDev"
                 num_failed_, // "#DOK"
                 num_passed_, // "#DNOK"
                 p_min_var},  // "PDOK"
                this, {utn_});
}

void SinglePositionDistance::addTargetDetailsToTableADSB (
        EvaluationResultsReport::Section& section, const std::string& table_name)
{
    if (!section.hasTable(table_name))
    {
        Qt::SortOrder order = Qt::AscendingOrder;

        if(req()->probCheckType() == EvaluationRequirement::COMPARISON_TYPE::LESS_THAN
                || req()->probCheckType() == EvaluationRequirement::COMPARISON_TYPE::LESS_THAN_OR_EQUAL)
            order = Qt::DescendingOrder;

        section.addTable(table_name, 18,
                         {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                          "DMin", "DMax", "DAvg", "DSDev", "#CF", "#CP", "PCP",
                          "MOPS", "NUCp/NIC", "NACp"}, true, 14, order);
    }

    EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

    QVariant prob_var;

    if (prob_.has_value())
        prob_var = roundf(prob_.value() * 10000.0) / 100.0;

    // "UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
    // "#ACOK", "#ACNOK", "PACOK", "#DOK", "#DNOK", "PDOK", "MOPS", "NUCp/NIC", "NACp"

    target_table.addRow(
                {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
                 target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
                 target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(),
                 target_->modeCMaxStr().c_str(),
                 Number::round(value_min_,2), // "DMin"
                 Number::round(value_max_,2), // "DMax"
                 Number::round(value_avg_,2), // "DAvg"
                 Number::round(sqrt(value_var_),2), // "DSDev"
                 num_failed_, // "#CF"
                 num_passed_, // "#CP"
                 prob_var, // "PDOK"
                 target_->mopsVersionStr().c_str(), // "MOPS"
                 target_->nucpNicStr().c_str(), // "NUCp/NIC"
                 target_->nacpStr().c_str()}, // "NACp"
                this, {utn_});
}

void SinglePositionDistance::addTargetDetailsToReport(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    root_item->getSection(getTargetSectionID()).perTargetSection(true); // mark utn section per target
    EvaluationResultsReport::Section& utn_req_section = root_item->getSection(getTargetRequirementSectionID());

    if (!utn_req_section.hasTable("details_overview_table"))
        utn_req_section.addTable("details_overview_table", 3, {"Name", "comment", "Value"}, false);

    std::shared_ptr<EvaluationRequirement::PositionDistance> req =
            std::static_pointer_cast<EvaluationRequirement::PositionDistance>(requirement_);
    assert (req);

    EvaluationResultsReport::SectionContentTable& utn_req_table =
            utn_req_section.getTable("details_overview_table");

    addCommonDetails(root_item);

    // "UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
    // "#ACOK", "#ACNOK", "PACOK", "#DOK", "#DNOK", "PDOK"

    utn_req_table.addRow({"Use", "To be used in results", use_}, this);
    utn_req_table.addRow({"#Pos [1]", "Number of updates", num_pos_}, this);
    utn_req_table.addRow({"#NoRef [1]", "Number of updates w/o reference positions", num_no_ref_}, this);
    utn_req_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
    utn_req_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_pos_outside_}, this);

    // along
    utn_req_table.addRow({"DMin [m]", "Minimum of distance",
                          String::doubleToStringPrecision(value_min_,2).c_str()}, this);
    utn_req_table.addRow({"DMax [m]", "Maximum of distance",
                          String::doubleToStringPrecision(value_max_,2).c_str()}, this);
    utn_req_table.addRow({"DAvg [m]", "Average of distance",
                          String::doubleToStringPrecision(value_avg_,2).c_str()}, this);
    utn_req_table.addRow({"DSDev [m]", "Standard Deviation of distance",
                          String::doubleToStringPrecision(sqrt(value_var_),2).c_str()}, this);
    utn_req_table.addRow({"DVar [m^2]", "Variance of distance",
                          String::doubleToStringPrecision(value_var_,2).c_str()}, this);
    utn_req_table.addRow({"#CF [1]", "Number of updates with failed comparison", num_failed_}, this);
    utn_req_table.addRow({"#CP [1]", "Number of updates with passed comparison", num_passed_},
                         this);
    // condition
    {
        QVariant p_passed_var;

        if (prob_.has_value())
            p_passed_var = roundf(prob_.value() * 10000.0) / 100.0;

        utn_req_table.addRow({"PCP [%]", "Probability of passed comparison", p_passed_var}, this);

        utn_req_table.addRow({"Condition", {}, req->getConditionStr().c_str()}, this);

        string result {"Unknown"};

        if (prob_.has_value())
            result = req->getResultConditionStr(prob_.value());

        utn_req_table.addRow({"Condition Fulfilled", "", result.c_str()}, this);

        if (result == "Failed")
        {
            root_item->getSection(getTargetSectionID()).perTargetWithIssues(true); // mark utn section as with issue
            utn_req_section.perTargetWithIssues(true);
        }

    }

    if (prob_.has_value() && prob_.value() != 1.0) // TODO
    {
        //        utn_req_section.addFigure("target_errors_overview", "Target Errors Overview",
        //                                  [this](void) { return this->getTargetErrorsViewable(); });

        utn_req_section.addFigure("target_errors_overview", "Target Errors Overview",
                                  [this](void) { return this->getTargetErrorsViewable(); });
    }
    else
    {
        utn_req_section.addText("target_errors_overview_no_figure");
        utn_req_section.getText("target_errors_overview_no_figure").addText(
                    "No target errors found, therefore no figure was generated.");
    }

    // add further details
    reportDetails(utn_req_section);
}

void SinglePositionDistance::reportDetails(EvaluationResultsReport::Section& utn_req_section)
{
    if (!utn_req_section.hasTable(tr_details_table_name_))
        utn_req_section.addTable(tr_details_table_name_, 8,
                                 {"ToD", "NoRef", "PosInside", "Distance", "CP", "#CF", "#CP", "Comment"});

    EvaluationResultsReport::SectionContentTable& utn_req_details_table =
            utn_req_section.getTable(tr_details_table_name_);

    unsigned int detail_cnt = 0;

    for (auto& rq_det_it : getDetails())
    {
        bool has_ref_pos = rq_det_it.numPositions() >= 2;

        utn_req_details_table.addRow(
                    { Time::toString(rq_det_it.timestamp()).c_str(),
                      !has_ref_pos,
                      rq_det_it.getValue(DetailPosInside),
                      rq_det_it.getValue(DetailValue),                 // "Distance"
                      rq_det_it.getValue(DetailCheckPassed),           // CP"
                      rq_det_it.getValue(DetailNumCheckFailed),        // "#CF",
                      rq_det_it.getValue(DetailNumCheckPassed),        // "#CP"
                      rq_det_it.comments().generalComment().c_str() }, // "Comment"
                    this, detail_cnt);

        ++detail_cnt;
    }
}

bool SinglePositionDistance::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else if (table.name() == tr_details_table_name_ && annotation.isValid() && annotation.toUInt() < numDetails())
        return true;
    else
        return false;
}

std::unique_ptr<nlohmann::json::object_t> SinglePositionDistance::viewableData(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasViewableData(table, annotation));

    if (table.name() == target_table_name_)
    {
        return getTargetErrorsViewable();
    }
    else if (table.name() == tr_details_table_name_ && annotation.isValid())
    {
        unsigned int detail_cnt = annotation.toUInt();

        loginf << "SinglePositionDistance: viewableData: detail_cnt " << detail_cnt;

        std::unique_ptr<nlohmann::json::object_t> viewable_ptr
                = eval_man_.getViewableForEvaluation(utn_, req_grp_id_, result_id_);
        assert (viewable_ptr);

        //        const auto& detail = getDetail(detail_cnt);

        //        assert(detail.numPositions() >= 1);

        //        (*viewable_ptr)[VP_POS_LAT_KEY    ] = detail.position(0).latitude_;
        //        (*viewable_ptr)[VP_POS_LON_KEY    ] = detail.position(0).longitude_;
        //        (*viewable_ptr)[VP_POS_WIN_LAT_KEY] = eval_man_.resultDetailZoom();
        //        (*viewable_ptr)[VP_POS_WIN_LON_KEY] = eval_man_.resultDetailZoom();
        //        (*viewable_ptr)[VP_TIMESTAMP_KEY  ] = Time::toString(detail.timestamp());

        //        auto check_passed = detail.getValueAs<bool>(DetailCheckPassed);
        //        assert(check_passed.has_value());

        //        if (!check_passed.value())
        //            (*viewable_ptr)[VP_EVAL_KEY][VP_EVAL_HIGHDET_KEY] = vector<unsigned int>{detail_cnt};

        return viewable_ptr;
    }
    else
        return nullptr;
}

std::unique_ptr<nlohmann::json::object_t> SinglePositionDistance::getTargetErrorsViewable ()
{
    std::unique_ptr<nlohmann::json::object_t> viewable_ptr = eval_man_.getViewableForEvaluation(
                utn_, req_grp_id_, result_id_);

    bool has_pos = false;
    double lat_min, lat_max, lon_min, lon_max;

    bool failed_values_of_interest = req()->failedValuesOfInterest();

    for (auto& detail_it : getDetails())
    {
        auto check_passed = detail_it.getValueAs<bool>(DetailCheckPassed);
        assert(check_passed.has_value());

        if ((failed_values_of_interest && check_passed.value()) ||
                (!failed_values_of_interest && !check_passed.value()))
            continue;

        assert(detail_it.numPositions() >= 1);

        bool has_ref_pos = detail_it.numPositions() >= 2;

        if (has_pos)
        {
            lat_min = min(lat_min, detail_it.position(0).latitude_);
            lat_max = max(lat_max, detail_it.position(0).latitude_);

            lon_min = min(lon_min, detail_it.position(0).longitude_);
            lon_max = max(lon_max, detail_it.position(0).longitude_);
        }
        else // tst pos always set
        {
            lat_min = detail_it.position(0).latitude_;
            lat_max = detail_it.position(0).latitude_;

            lon_min = detail_it.position(0).longitude_;
            lon_max = detail_it.position(0).longitude_;

            has_pos = true;
        }

        if (has_ref_pos)
        {
            lat_min = min(lat_min, detail_it.position(1).latitude_);
            lat_max = max(lat_max, detail_it.position(1).latitude_);

            lon_min = min(lon_min, detail_it.position(1).longitude_);
            lon_max = max(lon_max, detail_it.position(1).longitude_);
        }
    }

    if (has_pos)
    {
        (*viewable_ptr)[VP_POS_LAT_KEY] = (lat_max+lat_min)/2.0;
        (*viewable_ptr)[VP_POS_LON_KEY] = (lon_max+lon_min)/2.0;;

        double lat_w = 1.1*(lat_max-lat_min)/2.0;
        double lon_w = 1.1*(lon_max-lon_min)/2.0;

        if (lat_w < eval_man_.resultDetailZoom())
            lat_w = eval_man_.resultDetailZoom();

        if (lon_w < eval_man_.resultDetailZoom())
            lon_w = eval_man_.resultDetailZoom();

        (*viewable_ptr)[VP_POS_WIN_LAT_KEY] = lat_w;
        (*viewable_ptr)[VP_POS_WIN_LON_KEY] = lon_w;
    }

    addAnnotations(*viewable_ptr);

    return viewable_ptr;
}

void SinglePositionDistance::addAnnotations(nlohmann::json::object_t& viewable)
{
    loginf << "SinglePositionDistance: addAnnotations";

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

            json& error_feature_lines = viewable.at("annotations").at(0).at("features").at(0);

            error_feature_lines["type"] = "feature";
            error_feature_lines["geometry"] = json::object();
            error_feature_lines.at("geometry")["type"] = "lines";
            error_feature_lines.at("geometry")["coordinates"] = json::array();

            error_feature_lines["properties"] = json::object();
            error_feature_lines.at("properties")["color"] = "#FF0000";
            error_feature_lines.at("properties")["line_width"] = 2;

            // symbols

            viewable.at("annotations").at(0).at("features").push_back(json::object());

            json& error_feature_points = viewable.at("annotations").at(0).at("features").at(1);

            error_feature_points["type"] = "feature";
            error_feature_points["geometry"] = json::object();
            error_feature_points.at("geometry")["type"] = "points";
            error_feature_points.at("geometry")["coordinates"] = json::array();

            error_feature_points["properties"] = json::object();
            error_feature_points.at("properties")["color"] = "#FF0000";
            error_feature_points.at("properties")["symbol"] = "circle";
            error_feature_points.at("properties")["symbol_size"] = 8;

        }

        // ok

        {
            viewable.at("annotations").push_back(json::object()); // ok
            viewable.at("annotations").at(1)["name"] = result_id_+" OK";
            viewable.at("annotations").at(1)["features"] = json::array();

            // lines
            viewable.at("annotations").at(1).at("features").push_back(json::object());

            json& ok_feature_lines = viewable.at("annotations").at(1).at("features").at(0);

            ok_feature_lines["type"] = "feature";
            ok_feature_lines["geometry"] = json::object();
            ok_feature_lines.at("geometry")["type"] = "lines";
            ok_feature_lines.at("geometry")["coordinates"] = json::array();

            ok_feature_lines["properties"] = json::object();
            ok_feature_lines.at("properties")["color"] = "#00FF00";
            ok_feature_lines.at("properties")["line_width"] = 2;

            // symbols

            viewable.at("annotations").at(1).at("features").push_back(json::object());

            json& error_feature_points = viewable.at("annotations").at(1).at("features").at(1);

            error_feature_points["type"] = "feature";
            error_feature_points["geometry"] = json::object();
            error_feature_points.at("geometry")["type"] = "points";
            error_feature_points.at("geometry")["coordinates"] = json::array();

            error_feature_points["properties"] = json::object();
            error_feature_points.at("properties")["color"] = "#00FF00";
            error_feature_points.at("properties")["symbol"] = "circle";
            error_feature_points.at("properties")["symbol_size"] = 8;

        }
    }

    json& error_line_coordinates = viewable.at("annotations").at(0).at("features").at(0).at("geometry").at("coordinates");
    json& error_point_coordinates = viewable.at("annotations").at(0).at("features").at(1).at("geometry").at("coordinates");
    json& ok_line_coordinates = viewable.at("annotations").at(1).at("features").at(0).at("geometry").at("coordinates");
    json& ok_point_coordinates = viewable.at("annotations").at(1).at("features").at(1).at("geometry").at("coordinates");

    bool failed_values_of_interest = req()->failedValuesOfInterest();
    bool ok;

    for (auto& detail_it : getDetails())
    {
        auto check_passed = detail_it.getValueAs<bool>(DetailCheckPassed);
        assert(check_passed.has_value());

        ok = ((failed_values_of_interest && check_passed.value()) ||
              (!failed_values_of_interest && !check_passed.value()));

        if (detail_it.numPositions() == 1) // no ref pos
            continue;

        assert (detail_it.numPositions() == 2);

        if (ok)
        {
            ok_point_coordinates.push_back(detail_it.position(0).asVector());

            ok_line_coordinates.push_back(detail_it.position(0).asVector());
            ok_line_coordinates.push_back(detail_it.position(1).asVector());
        }
        else
        {
            error_point_coordinates.push_back(detail_it.position(0).asVector());

            error_line_coordinates.push_back(detail_it.position(0).asVector());
            error_line_coordinates.push_back(detail_it.position(1).asVector());
        }
    }
}

bool SinglePositionDistance::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else
        return false;;
}

std::string SinglePositionDistance::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasReference(table, annotation));

    return "Report:Results:"+getTargetRequirementSectionID();
}

std::shared_ptr<Joined> SinglePositionDistance::createEmptyJoined(const std::string& result_id)
{
    return make_shared<JoinedPositionDistance> (result_id, requirement_, sector_layer_, eval_man_);
}

EvaluationRequirement::PositionDistance* SinglePositionDistance::req ()
{
    EvaluationRequirement::PositionDistance* req =
            dynamic_cast<EvaluationRequirement::PositionDistance*>(requirement_.get());
    assert (req);
    return req;
}

}
