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

#include "eval/results/position/alongsingle.h"
#include "eval/results/position/alongjoined.h"
#include "eval/requirement/base/base.h"
#include "eval/requirement/position/along.h"
#include "evaluationtargetdata.h"
#include "evaluationmanager.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "logger.h"
#include "stringconv.h"
#include "number.h"

#include <cassert>
#include <algorithm>

using namespace std;
using namespace Utils;

namespace EvaluationRequirementResult
{

SinglePositionAlong::SinglePositionAlong(const std::string& result_id, 
                                         std::shared_ptr<EvaluationRequirement::Base> requirement,
                                         const SectorLayer& sector_layer,
                                         unsigned int utn, 
                                         const EvaluationTargetData* target, 
                                         EvaluationManager& eval_man,
                                         const boost::optional<EvaluationDetails>& details,
                                         unsigned int num_pos, 
                                         unsigned int num_no_ref,
                                         unsigned int num_pos_outside, 
                                         unsigned int num_pos_inside,
                                         unsigned int num_value_ok, 
                                         unsigned int num_value_nok,
                                         vector<double> values)
:   SinglePositionBase("SinglePositionAlong", result_id, requirement, sector_layer, utn, target, eval_man, details,
                       num_pos, num_no_ref,num_pos_outside, num_pos_inside, num_value_ok, num_value_nok, values)
{
    update();
}

void SinglePositionAlong::update()
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
        prob_ = (float)num_passed_ / (float)num_distances;
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

void SinglePositionAlong::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "SinglePositionAlong " <<  requirement_->name() <<": addToReport";

    // add target to requirements->group->req
    addTargetToOverviewTable(root_item);

    // add requirement results to targets->utn->requirements->group->req
    addTargetDetailsToReport(root_item);

    // TODO add requirement description, methods
}

void SinglePositionAlong::addTargetToOverviewTable(shared_ptr<EvaluationResultsReport::RootItem> root_item)
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

void SinglePositionAlong::addTargetDetailsToTable (
        EvaluationResultsReport::Section& section, const std::string& table_name)
{
    if (!section.hasTable(table_name))
        section.addTable(table_name, 15,
                         {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                          "ALMin", "ALMax", "ALAvg", "ALSDev", "#ALOK", "#ALNOK", "PALOK"}, true, 14);

    EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

    QVariant p_min_var;

    if (prob_.has_value())
        p_min_var = roundf(prob_.value() * 10000.0) / 100.0;

    target_table.addRow(
                {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
                 target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
                 target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(), target_->modeCMaxStr().c_str(),
                 Number::round(value_min_,2), // "ALMin"
                 Number::round(value_max_,2), // "ALMax"
                 Number::round(value_avg_,2), // "ALAvg"
                 Number::round(sqrt(value_var_),2), // "ALSDev"
                 num_passed_, // "#ALOK"
                 num_failed_, // "#ALNOK"
                 p_min_var}, // "PALOK"
                this, {utn_});
}

void SinglePositionAlong::addTargetDetailsToTableADSB (
        EvaluationResultsReport::Section& section, const std::string& table_name)
{
    if (!section.hasTable(table_name))
        section.addTable(table_name, 18,
                         {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                          "ALMin", "ALMax", "ALAvg", "ALSDev", "#ALOK", "#ALNOK", "PALOK",
                          "MOPS", "NUCp/NIC", "NACp"}, true, 14);

    EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

    QVariant p_min_var;

    if (prob_.has_value())
        p_min_var = roundf(prob_.value() * 10000.0) / 100.0;

    // "UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
    // "#ACOK", "#ACNOK", "PACOK", "#ALOK", "#ALNOK", "PALOK", "MOPS", "NUCp/NIC", "NACp"

    target_table.addRow(
                {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
                 target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
                 target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(),
                 target_->modeCMaxStr().c_str(),
                 Number::round(value_min_,2), // "ALMin"
                 Number::round(value_max_,2), // "ALMax"
                 Number::round(value_avg_,2), // "ALAvg"
                 Number::round(sqrt(value_var_),2), // "ALSDev"
                 num_passed_, // "#ALOK"
                 num_failed_, // "#ALNOK"
                 p_min_var, // "PALOK"
                 target_->mopsVersionStr().c_str(), // "MOPS"
                 target_->nucpNicStr().c_str(), // "NUCp/NIC"
                 target_->nacpStr().c_str()}, // "NACp"
                this, {utn_});

}

void SinglePositionAlong::addTargetDetailsToReport(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    root_item->getSection(getTargetSectionID()).perTargetSection(true); // mark utn section per target
    EvaluationResultsReport::Section& utn_req_section = root_item->getSection(getTargetRequirementSectionID());

    if (!utn_req_section.hasTable("details_overview_table"))
        utn_req_section.addTable("details_overview_table", 3, {"Name", "comment", "Value"}, false);

    std::shared_ptr<EvaluationRequirement::PositionAlong> req =
            std::static_pointer_cast<EvaluationRequirement::PositionAlong>(requirement_);
    assert (req);

    EvaluationResultsReport::SectionContentTable& utn_req_table =
            utn_req_section.getTable("details_overview_table");

    addCommonDetails(root_item);

    // "UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
    // "#ACOK", "#ACNOK", "PACOK", "#ALOK", "#ALNOK", "PALOK"

    utn_req_table.addRow({"Use", "To be used in results", use_}, this);
    utn_req_table.addRow({"#Pos [1]", "Number of updates", num_pos_}, this);
    utn_req_table.addRow({"#NoRef [1]", "Number of updates w/o reference positions", num_no_ref_}, this);
    utn_req_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
    utn_req_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_pos_outside_}, this);

    // along
    utn_req_table.addRow({"ALMin [m]", "Minimum of along-track error",
                          String::doubleToStringPrecision(value_min_,2).c_str()}, this);
    utn_req_table.addRow({"ALMax [m]", "Maximum of along-track error",
                          String::doubleToStringPrecision(value_max_,2).c_str()}, this);
    utn_req_table.addRow({"ALAvg [m]", "Average of along-track error",
                          String::doubleToStringPrecision(value_avg_,2).c_str()}, this);
    utn_req_table.addRow({"ALSDev [m]", "Standard Deviation of along-track error",
                          String::doubleToStringPrecision(sqrt(value_var_),2).c_str()}, this);
    utn_req_table.addRow({"ALVar [m^2]", "Variance of along-track error",
                          String::doubleToStringPrecision(value_var_,2).c_str()}, this);
    utn_req_table.addRow({"#ALOK [1]", "Number of updates with along-track error", num_passed_}, this);
    utn_req_table.addRow({"#ALNOK [1]", "Number of updates with unacceptable along-track error ", num_failed_},
                         this);

    // condition
    {
        QVariant p_min_var;

        if (prob_.has_value())
            p_min_var = roundf(prob_.value() * 10000.0) / 100.0;

        utn_req_table.addRow({"PALOK [%]", "Probability of acceptable along-track error", p_min_var}, this);

        utn_req_table.addRow({"Condition Along", {}, req->getConditionStr().c_str()}, this);

        string result {"Unknown"};

        if (prob_.has_value())
            result = req->getResultConditionStr(prob_.value());

        utn_req_table.addRow({"Condition Along Fulfilled", "", result.c_str()}, this);

        if (result == "Failed")
        {
            root_item->getSection(getTargetSectionID()).perTargetWithIssues(true); // mark utn section as with issue
            utn_req_section.perTargetWithIssues(true);
        }
    }

    if (prob_.has_value() && prob_.value() != 1.0)
    {
        utn_req_section.addFigure("target_errors_overview", "Target Errors Overview",
                                  getTargetErrorsViewable());
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

void SinglePositionAlong::reportDetails(EvaluationResultsReport::Section& utn_req_section)
{
    if (!utn_req_section.hasTable(tr_details_table_name_))
        utn_req_section.addTable(tr_details_table_name_, 8,
                                 {"ToD", "NoRef", "PosInside",
                                  "DAlong", "DAlongOK", "#ALOK", "#ALNOK",
                                  "Comment"});

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
                      rq_det_it.getValue(DetailValue),                 // "DAlong"
                      rq_det_it.getValue(DetailCheckPassed),           // DAlongOK"
                      rq_det_it.getValue(DetailNumCheckPassed),        // "#ALOK",
                      rq_det_it.getValue(DetailNumCheckFailed),        // "#ALNOK"
                      rq_det_it.comments().generalComment().c_str() }, // "Comment"
                    this, detail_cnt);

        ++detail_cnt;
    }
}

bool SinglePositionAlong::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else if (table.name() == tr_details_table_name_ && annotation.isValid() && annotation.toUInt() < numDetails())
        return true;
    
    return false;
}

std::unique_ptr<nlohmann::json::object_t> SinglePositionAlong::viewableData(
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

        loginf << "SinglePositionAlong: viewableData: detail_cnt " << detail_cnt;

        std::unique_ptr<nlohmann::json::object_t> viewable_ptr
                = eval_man_.getViewableForEvaluation(utn_, req_grp_id_, result_id_);
        assert (viewable_ptr);

        const auto& detail = getDetail(detail_cnt);

        assert(detail.numPositions() >= 1);

        (*viewable_ptr)[VP_POS_LAT_KEY    ] = detail.position(0).latitude_;
        (*viewable_ptr)[VP_POS_LON_KEY    ] = detail.position(0).longitude_;
        (*viewable_ptr)[VP_POS_WIN_LAT_KEY] = eval_man_.resultDetailZoom();
        (*viewable_ptr)[VP_POS_WIN_LON_KEY] = eval_man_.resultDetailZoom();
        (*viewable_ptr)[VP_TIMESTAMP_KEY  ] = Time::toString(detail.timestamp());

        auto check_passed = detail.getValueAs<bool>(DetailCheckPassed);
        assert(check_passed.has_value());

        if (!check_passed.value())
            (*viewable_ptr)[VP_EVAL_KEY][VP_EVAL_HIGHDET_KEY] = vector<unsigned int>{detail_cnt};

        return viewable_ptr;
    }
    
    return nullptr;
}

std::unique_ptr<nlohmann::json::object_t> SinglePositionAlong::getTargetErrorsViewable ()
{
    std::unique_ptr<nlohmann::json::object_t> viewable_ptr = eval_man_.getViewableForEvaluation(
                utn_, req_grp_id_, result_id_);

    bool has_pos = false;
    double lat_min, lat_max, lon_min, lon_max;

    for (auto& detail_it : getDetails())
    {
        auto check_passed = detail_it.getValueAs<bool>(DetailCheckPassed);
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

        double lat_w = 1.1*(lat_max-lat_min)/2.0;
        double lon_w = 1.1*(lon_max-lon_min)/2.0;

        if (lat_w < eval_man_.resultDetailZoom())
            lat_w = eval_man_.resultDetailZoom();

        if (lon_w < eval_man_.resultDetailZoom())
            lon_w = eval_man_.resultDetailZoom();

        (*viewable_ptr)[VP_POS_WIN_LAT_KEY] = lat_w;
        (*viewable_ptr)[VP_POS_WIN_LON_KEY] = lon_w;
    }

    return viewable_ptr;
}

bool SinglePositionAlong::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else
        return false;;
}

std::string SinglePositionAlong::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasReference(table, annotation));

    return "Report:Results:"+getTargetRequirementSectionID();
}

std::shared_ptr<Joined> SinglePositionAlong::createEmptyJoined(const std::string& result_id)
{
    return make_shared<JoinedPositionAlong> (result_id, requirement_, sector_layer_, eval_man_);
}

}
