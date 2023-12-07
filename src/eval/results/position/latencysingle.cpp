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

#include "eval/results/position/latencysingle.h"
#include "eval/results/position/latencyjoined.h"
#include "eval/requirement/base/base.h"
#include "eval/requirement/position/latency.h"
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
#include "viewpoint.h"

#include <cassert>
#include <algorithm>

using namespace std;
using namespace Utils;
using namespace nlohmann;

namespace EvaluationRequirementResult
{

SinglePositionLatency::SinglePositionLatency(const std::string& result_id, 
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
                                             unsigned int num_value_ok,
                                             unsigned int num_value_nok,
                                             vector<double> values)
    :   SinglePositionBase("SinglePositionLatency", result_id, requirement, sector_layer, utn, target, eval_man, details,
                           num_pos, num_no_ref,num_pos_outside, num_pos_inside, num_value_ok, num_value_nok, values)
{
    update();
}

void SinglePositionLatency::update()
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

void SinglePositionLatency::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "SinglePositionLatency " <<  requirement_->name() <<": addToReport";

    // add target to requirements->group->req
    addTargetToOverviewTable(root_item);

    // add requirement results to targets->utn->requirements->group->req
    addTargetDetailsToReport(root_item);

    // TODO add requirement description, methods
}

void SinglePositionLatency::addTargetToOverviewTable(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::Section& tgt_overview_section = getRequirementSection(root_item);

    if (eval_man_.settings().report_show_adsb_info_)
        addTargetDetailsToTableADSB(tgt_overview_section, target_table_name_);
    else
        addTargetDetailsToTable(tgt_overview_section, target_table_name_);

    if (eval_man_.settings().report_split_results_by_mops_
            || eval_man_.settings().report_split_results_by_aconly_ms_) // add to general sum table
    {
        EvaluationResultsReport::Section& sum_section = root_item->getSection(getRequirementSumSectionID());

        if (eval_man_.settings().report_show_adsb_info_)
            addTargetDetailsToTableADSB(sum_section, target_table_name_);
        else
            addTargetDetailsToTable(sum_section, target_table_name_);
    }
}

void SinglePositionLatency::addTargetDetailsToTable (
        EvaluationResultsReport::Section& section, const std::string& table_name)
{
    if (!section.hasTable(table_name))
        section.addTable(table_name, 15,
                         {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                          "LTMin", "LTMax", "LTAvg", "LTSDev", "#LTOK", "#LTNOK", "PLTOK"}, true, 14);

    EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

    QVariant p_min_var;

    if (prob_.has_value())
        p_min_var = roundf(prob_.value() * 10000.0) / 100.0;

    target_table.addRow(
                {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
                 target_->acidsStr().c_str(), target_->acadsStr().c_str(),
                 target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(), target_->modeCMaxStr().c_str(),
                 Number::round(value_min_,2), // "LTMin"
                 Number::round(value_max_,2), // "LTMax"
                 Number::round(value_avg_,2), // "LTAvg"
                 Number::round(sqrt(value_var_),2), // "LTSDev"
                 num_passed_, // "#LTOK"
                 num_failed_, // "#LTNOK"
                 p_min_var},  // "PLTOK"
                this, {utn_});
}

void SinglePositionLatency::addTargetDetailsToTableADSB (
        EvaluationResultsReport::Section& section, const std::string& table_name)
{
    if (!section.hasTable(table_name))
        section.addTable(table_name, 16,
                         {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                          "LTMin", "LTMax", "LTAvg", "LTSDev", "#LTOK", "#LTNOK", "PLTOK",
                          "MOPS"}, true, 14);

    EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

    QVariant p_min_var;

    if (prob_.has_value())
        p_min_var = roundf(prob_.value() * 10000.0) / 100.0;

    // "UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
    // "#ACOK", "#ACNOK", "PACOK", "#LTOK", "#LTNOK", "PLTOK", "MOPS", "NUCp/NIC", "NACp"

    target_table.addRow(
                {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
                 target_->acidsStr().c_str(), target_->acadsStr().c_str(),
                 target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(),
                 target_->modeCMaxStr().c_str(),
                 Number::round(value_min_,2), // "LTMin"
                 Number::round(value_max_,2), // "LTMax"
                 Number::round(value_avg_,2), // "LTAvg"
                 Number::round(sqrt(value_var_),2), // "LTSDev"
                 num_passed_, // "#LTOK"
                 num_failed_, // "#LTNOK"
                 p_min_var, // "PLTOK"
                 target_->mopsVersionStr().c_str()}, // "MOPS"
                this, {utn_});

}

void SinglePositionLatency::addTargetDetailsToReport(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    root_item->getSection(getTargetSectionID()).perTargetSection(true); // mark utn section per target
    EvaluationResultsReport::Section& utn_req_section = root_item->getSection(getTargetRequirementSectionID());

    if (!utn_req_section.hasTable("details_overview_table"))
        utn_req_section.addTable("details_overview_table", 3, {"Name", "comment", "Value"}, false);

    std::shared_ptr<EvaluationRequirement::PositionLatency> req =
            std::static_pointer_cast<EvaluationRequirement::PositionLatency>(requirement_);
    assert (req);

    EvaluationResultsReport::SectionContentTable& utn_req_table =
            utn_req_section.getTable("details_overview_table");

    addCommonDetails(root_item);

    // "UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
    // "#ACOK", "#ACNOK", "PACOK", "#LTOK", "#LTNOK", "PLTOK"

    utn_req_table.addRow({"Use", "To be used in results", use_}, this);
    utn_req_table.addRow({"#Pos [1]", "Number of updates", num_pos_}, this);
    utn_req_table.addRow({"#NoRef [1]", "Number of updates w/o reference positions", num_no_ref_}, this);
    utn_req_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
    utn_req_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_pos_outside_}, this);

    // along
    utn_req_table.addRow({"LTMin [s]", "Minimum of latency",
                          String::timeStringFromDouble(value_min_,2).c_str()}, this);
    utn_req_table.addRow({"LTMax [s]", "Maximum of latency",
                          String::timeStringFromDouble(value_max_,2).c_str()}, this);
    utn_req_table.addRow({"LTAvg [s]", "Average of latency",
                          String::timeStringFromDouble(value_avg_,2).c_str()}, this);
    utn_req_table.addRow({"LTSDev [s]", "Standard Deviation of latency",
                          String::timeStringFromDouble(sqrt(value_var_),2).c_str()}, this);
    utn_req_table.addRow({"LTVar [s^2]", "Variance of latency",
                          String::timeStringFromDouble(value_var_,2).c_str()}, this);
    utn_req_table.addRow({"#LTOK [1]", "Number of updates with latency", num_passed_}, this);
    utn_req_table.addRow({"#LTNOK [1]", "Number of updates with unacceptable latency ", num_failed_},
                         this);

    // condition
    {
        QVariant p_min_var;

        if (prob_.has_value())
            p_min_var = roundf(prob_.value() * 10000.0) / 100.0;

        utn_req_table.addRow({"PLTOK [%]", "Probability of acceptable latency", p_min_var}, this);

        utn_req_table.addRow({"Condition Latency", {}, req->getConditionStr().c_str()}, this);

        string result {"Unknown"};

        if (prob_.has_value())
            result = req->getConditionResultStr(prob_.value());

        utn_req_table.addRow({"Condition Latency Fulfilled", "", result.c_str()}, this);

        if (result == "Failed")
        {
            root_item->getSection(getTargetSectionID()).perTargetWithIssues(true); // mark utn section as with issue
            utn_req_section.perTargetWithIssues(true);
        }

    }

    if (prob_.has_value() && prob_.value() != 1.0)
    {
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

void SinglePositionLatency::reportDetails(EvaluationResultsReport::Section& utn_req_section)
{
    if (!utn_req_section.hasTable(tr_details_table_name_))
        utn_req_section.addTable(tr_details_table_name_, 8,
                                 {"ToD", "NoRef", "PosInside",
                                  "DLatency", "DLatencyOK", "#LTOK", "#LTNOK",
                                  "Comment"});

    EvaluationResultsReport::SectionContentTable& utn_req_details_table =
            utn_req_section.getTable(tr_details_table_name_);

    utn_req_details_table.setCreateOnDemand(
                [this, &utn_req_details_table](void)
    {

        unsigned int detail_cnt = 0;

        for (auto& rq_det_it : getDetails())
        {
            bool has_ref_pos = rq_det_it.numPositions() >= 2;

            utn_req_details_table.addRow(
                        { Time::toString(rq_det_it.timestamp()).c_str(),
                          !has_ref_pos,
                          rq_det_it.getValue(DetailKey::PosInside),
                          rq_det_it.getValue(DetailKey::Value),                // "DLatency"
                          rq_det_it.getValue(DetailKey::CheckPassed),          // DLatencyOK"
                          rq_det_it.getValue(DetailKey::NumCheckPassed),       // "#LTOK",
                          rq_det_it.getValue(DetailKey::NumCheckFailed),       // "#LTNOK"
                          rq_det_it.comments().generalComment().c_str()}, // "Comment"
                        this, detail_cnt);

            ++detail_cnt;
        }});
}

bool SinglePositionLatency::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else if (table.name() == tr_details_table_name_ && annotation.isValid() && annotation.toUInt() < numDetails())
        return true;
    else
        return false;
}

std::unique_ptr<nlohmann::json::object_t> SinglePositionLatency::viewableData(
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

        loginf << "SinglePositionLatency: viewableData: detail_cnt " << detail_cnt;

        std::unique_ptr<nlohmann::json::object_t> viewable_ptr = getTargetErrorsViewable(true);
        assert (viewable_ptr);

        const auto& detail = getDetail(detail_cnt);

        assert (detail.numPositions() >= 1);

        (*viewable_ptr)[VP_POS_LAT_KEY    ] = detail.position(0).latitude_;
        (*viewable_ptr)[VP_POS_LON_KEY    ] = detail.position(0).longitude_;
        (*viewable_ptr)[VP_POS_WIN_LAT_KEY] = eval_man_.settings().result_detail_zoom_;
        (*viewable_ptr)[VP_POS_WIN_LON_KEY] = eval_man_.settings().result_detail_zoom_;
        (*viewable_ptr)[VP_TIMESTAMP_KEY  ] = Time::toString(detail.timestamp());

        auto check_passed = detail.getValueAs<bool>(CheckPassed);
        assert(check_passed.has_value());

        addAnnotationPos(*viewable_ptr, detail.position(0), TypeHighlight);
        addAnnotationLine(*viewable_ptr, detail.position(0), detail.position(1), TypeHighlight);

        return viewable_ptr;
    }
    else
        return nullptr;
}

std::unique_ptr<nlohmann::json::object_t> SinglePositionLatency::getTargetErrorsViewable (bool add_highlight)
{
    std::unique_ptr<nlohmann::json::object_t> viewable_ptr = eval_man_.getViewableForEvaluation(
                utn_, req_grp_id_, result_id_);

    bool has_pos = false;
    double lat_min, lat_max, lon_min, lon_max;

    for (auto& detail_it : getDetails())
    {
        auto check_passed = detail_it.getValueAs<bool>(DetailKey::CheckPassed);
        assert(check_passed.has_value());

        if (check_passed.value())
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

        double lat_w = OSGVIEW_POS_WINDOW_SCALE*(lat_max-lat_min)/2.0;
        double lon_w = OSGVIEW_POS_WINDOW_SCALE*(lon_max-lon_min)/2.0;

        if (lat_w < eval_man_.settings().result_detail_zoom_)
            lat_w = eval_man_.settings().result_detail_zoom_;

        if (lon_w < eval_man_.settings().result_detail_zoom_)
            lon_w = eval_man_.settings().result_detail_zoom_;

        (*viewable_ptr)[VP_POS_WIN_LAT_KEY] = lat_w;
        (*viewable_ptr)[VP_POS_WIN_LON_KEY] = lon_w;
    }

    //addAnnotationFeatures(*viewable_ptr, false, add_highlight);
    addAnnotations(*viewable_ptr, false, true);

    return viewable_ptr;
}

bool SinglePositionLatency::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else
        return false;;
}

std::string SinglePositionLatency::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasReference(table, annotation));

    return "Report:Results:"+getTargetRequirementSectionID();
}

void SinglePositionLatency::addAnnotations(nlohmann::json::object_t& viewable, bool overview, bool add_ok)
{
    //addAnnotationFeatures(viewable, overview);

    json& error_line_coordinates  = annotationLineCoords(viewable, TypeError, overview);
    json& error_point_coordinates = annotationPointCoords(viewable, TypeError, overview);
    json& ok_line_coordinates     = annotationLineCoords(viewable, TypeOk, overview);
    json& ok_point_coordinates    = annotationPointCoords(viewable, TypeOk, overview);

    for (auto& detail_it : getDetails())
    {
        auto check_passed = detail_it.getValueAsOrAssert<bool>(
                    EvaluationRequirementResult::SinglePositionLatency::DetailKey::CheckPassed);

        if (detail_it.numPositions() == 1) // no ref pos
            continue;

        assert (detail_it.numPositions() == 2);

        if (!check_passed)
        {
            error_point_coordinates.push_back(detail_it.position(0).asVector());

            if (!overview)
            {
                error_line_coordinates.push_back(detail_it.position(0).asVector());
                error_line_coordinates.push_back(detail_it.position(1).asVector());
            }
        }
        else if (add_ok)
        {
            ok_point_coordinates.push_back(detail_it.position(0).asVector());

            if (!overview)
            {
                ok_line_coordinates.push_back(detail_it.position(0).asVector());
                ok_line_coordinates.push_back(detail_it.position(1).asVector());
            }
        }
    }
}

std::shared_ptr<Joined> SinglePositionLatency::createEmptyJoined(const std::string& result_id)
{
    return make_shared<JoinedPositionLatency> (result_id, requirement_, sector_layer_, eval_man_);
}

}
