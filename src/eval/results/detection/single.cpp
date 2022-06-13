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

#include "eval/results/detection/single.h"
#include "eval/results/detection/joined.h"
#include "eval/requirement/base/base.h"
#include "eval/requirement/detection/detection.h"
#include "evaluationtargetdata.h"
#include "evaluationmanager.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "logger.h"
#include "stringconv.h"

#include <cassert>

using namespace std;
using namespace Utils;

namespace EvaluationRequirementResult
{

SingleDetection::SingleDetection(
        const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
        const SectorLayer& sector_layer, unsigned int utn, const EvaluationTargetData* target,
        EvaluationManager& eval_man,
        int sum_uis, int missed_uis, TimePeriodCollection ref_periods,
        std::vector<EvaluationRequirement::DetectionDetail> details)
    : Single("SingleDetection", result_id, requirement, sector_layer, utn, target, eval_man),
      sum_uis_(sum_uis), missed_uis_(missed_uis), ref_periods_(ref_periods), details_(details)
{
    updatePD();
}


void SingleDetection::updatePD()
{
    if (sum_uis_)
    {
        logdbg << "SingleDetection: updatePD: utn " << utn_ << " missed_uis " << missed_uis_
               << " sum_uis " << sum_uis_;

        assert (missed_uis_ <= sum_uis_);

        std::shared_ptr<EvaluationRequirement::Detection> req =
                std::static_pointer_cast<EvaluationRequirement::Detection>(requirement_);
        assert (req);

        if (req->invertProb())
            pd_ = (float)missed_uis_/(float)(sum_uis_);
        else
            pd_ = 1.0 - ((float)missed_uis_/(float)(sum_uis_));

        has_pd_ = true;

        result_usable_ = true;
    }
    else
    {
        pd_ = 0;
        has_pd_ = false;

        result_usable_ = false;
    }

    updateUseFromTarget();
}

void SingleDetection::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "SingleDetection " <<  requirement_->name() <<": addToReport";

    // add target to requirements->group->req
    addTargetToOverviewTable(root_item);

    // add requirement results to targets->utn->requirements->group->req
    addTargetDetailsToReport(root_item);

    // TODO add requirement description, methods
}

void SingleDetection::addTargetToOverviewTable(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    addTargetDetailsToTable(getRequirementSection(root_item), target_table_name_);

    if (eval_man_.reportSplitResultsByMOPS()) // add to general sum table
        addTargetDetailsToTable(root_item->getSection(getRequirementSumSectionID()), target_table_name_);
}

void SingleDetection::addTargetDetailsToTable (
        EvaluationResultsReport::Section& section, const std::string& table_name)
{
    if (!section.hasTable(table_name))
        section.addTable(table_name, 11,
                         {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                          "#EUIs", "#MUIs", "PD"}, true, 10);

    EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

    QVariant pd_var;

    if (has_pd_)
        pd_var = roundf(pd_ * 10000.0) / 100.0;

    target_table.addRow(
                {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
                 target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
                 target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(),
                 target_->modeCMaxStr().c_str(), sum_uis_, missed_uis_, pd_var}, this, {utn_});
}

void SingleDetection::addTargetDetailsToReport(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    QVariant pd_var;

    if (has_pd_)
        pd_var = roundf(pd_ * 10000.0) / 100.0;

    root_item->getSection(getTargetSectionID()).perTargetSection(true); // mark utn section per target
    EvaluationResultsReport::Section& utn_req_section = root_item->getSection(getTargetRequirementSectionID());

    if (!utn_req_section.hasTable("details_overview_table"))
        utn_req_section.addTable("details_overview_table", 3, {"Name", "comment", "Value"}, false);

    EvaluationResultsReport::SectionContentTable& utn_req_table =
            utn_req_section.getTable("details_overview_table");

    addCommonDetails(root_item);

    utn_req_table.addRow({"Use", "To be used in results", use_}, this);
    utn_req_table.addRow({"#EUIs [1]", "Expected Update Intervals", sum_uis_}, this);
    utn_req_table.addRow({"#MUIs [1]", "Missed Update Intervals", missed_uis_}, this);
    utn_req_table.addRow({"PD [%]", "Probability of Detection", pd_var}, this);

    for (unsigned int cnt=0; cnt < ref_periods_.size(); ++cnt)
        utn_req_table.addRow(
                    {("Reference Period "+to_string(cnt)).c_str(), "Time inside sector",
                     ref_periods_.period(cnt).str().c_str()}, this);

    if (!ref_periods_.size())
        utn_req_table.addRow(
                    {"Reference Period", "Time inside sector", "None"}, this);

    // condition
    std::shared_ptr<EvaluationRequirement::Detection> req =
            std::static_pointer_cast<EvaluationRequirement::Detection>(requirement_);
    assert (req);

    utn_req_table.addRow({"Condition", "", req->getConditionStr().c_str()}, this);

    if (req->getConditionStr() == "Failed")
    {
        root_item->getSection(getTargetSectionID()).perTargetWithIssues(true); // mark utn section as with issue
        utn_req_section.perTargetWithIssues(true);
    }

    string result {"Unknown"};

    if (has_pd_)
        result = req-> getResultConditionStr(pd_);

    utn_req_table.addRow({"Condition Fulfilled", "", result.c_str()}, this);

    // add figure
    if (has_pd_ && pd_ != 1.0)
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

void SingleDetection::reportDetails(EvaluationResultsReport::Section& utn_req_section)
{
    if (!utn_req_section.hasTable(tr_details_table_name_))
        utn_req_section.addTable(tr_details_table_name_, 5,
                                 {"ToD", "DToD", "Ref.", "MUI", "Comment"});

    EvaluationResultsReport::SectionContentTable& utn_req_details_table =
            utn_req_section.getTable(tr_details_table_name_);

    unsigned int detail_cnt = 0;

    for (auto& rq_det_it : details_)
    {
        if (rq_det_it.d_tod_.isValid())
            utn_req_details_table.addRow(
                        {String::timeStringFromDouble(rq_det_it.tod_).c_str(),
                         String::timeStringFromDouble(rq_det_it.d_tod_.toFloat()).c_str(),
                         rq_det_it.ref_exists_, rq_det_it.missed_uis_, rq_det_it.comment_.c_str()},
                        this, detail_cnt);
        else
            utn_req_details_table.addRow(
                        {String::timeStringFromDouble(rq_det_it.tod_).c_str(),
                         rq_det_it.d_tod_,
                         rq_det_it.ref_exists_, rq_det_it.missed_uis_,
                         rq_det_it.comment_.c_str()},
                        this, detail_cnt);

        ++detail_cnt;
    }
}

bool SingleDetection::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else if (table.name() == tr_details_table_name_ && annotation.isValid() && annotation.toUInt() < details_.size())
        return true;
    else
        return false;
}

std::unique_ptr<nlohmann::json::object_t> SingleDetection::viewableData(
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

        loginf << "SinglePositionMaxDistance: viewableData: detail_cnt " << detail_cnt;

        std::unique_ptr<nlohmann::json::object_t> viewable_ptr
                = eval_man_.getViewableForEvaluation(utn_, req_grp_id_, result_id_);
        assert (viewable_ptr);

        const EvaluationRequirement::DetectionDetail& detail = details_.at(detail_cnt);

        (*viewable_ptr)["position_latitude"] = detail.pos_current_.latitude_;
        (*viewable_ptr)["position_longitude"] = detail.pos_current_.longitude_;
        (*viewable_ptr)["position_window_latitude"] = eval_man_.resultDetailZoom();
        (*viewable_ptr)["position_window_longitude"] = eval_man_.resultDetailZoom();
        (*viewable_ptr)["time"] = detail.tod_;

        if (detail.miss_occurred_)
            (*viewable_ptr)["evaluation_results"]["highlight_details"] = vector<unsigned int>{detail_cnt};

        return viewable_ptr;
    }
    else
        return nullptr;
}

std::unique_ptr<nlohmann::json::object_t> SingleDetection::getTargetErrorsViewable ()
{
    std::unique_ptr<nlohmann::json::object_t> viewable_ptr = eval_man_.getViewableForEvaluation(
                utn_, req_grp_id_, result_id_);

    bool has_pos = false;
    double lat_min, lat_max, lon_min, lon_max;

    for (auto& detail_it : details_)
    {
        if (!detail_it.miss_occurred_)
            continue;

        if (has_pos)
        {
            lat_min = min(lat_min, detail_it.pos_current_.latitude_);
            lat_max = max(lat_max, detail_it.pos_current_.latitude_);

            lon_min = min(lon_min, detail_it.pos_current_.longitude_);
            lon_max = max(lon_max, detail_it.pos_current_.longitude_);
        }
        else // tst pos always set
        {
            lat_min = detail_it.pos_current_.latitude_;
            lat_max = detail_it.pos_current_.latitude_;

            lon_min = detail_it.pos_current_.longitude_;
            lon_max = detail_it.pos_current_.longitude_;

            has_pos = true;
        }

        if (detail_it.has_last_position_)
        {
            lat_min = min(lat_min, detail_it.pos_last_.latitude_);
            lat_max = max(lat_max, detail_it.pos_last_.latitude_);

            lon_min = min(lon_min, detail_it.pos_last_.longitude_);
            lon_max = max(lon_max, detail_it.pos_last_.longitude_);
        }
    }

    if (has_pos)
    {
        (*viewable_ptr)["position_latitude"] = (lat_max+lat_min)/2.0;
        (*viewable_ptr)["position_longitude"] = (lon_max+lon_min)/2.0;;

        double lat_w = 1.1*(lat_max-lat_min)/2.0;
        double lon_w = 1.1*(lon_max-lon_min)/2.0;

        if (lat_w < eval_man_.resultDetailZoom())
            lat_w = eval_man_.resultDetailZoom();

        if (lon_w < eval_man_.resultDetailZoom())
            lon_w = eval_man_.resultDetailZoom();

        (*viewable_ptr)["position_window_latitude"] = lat_w;
        (*viewable_ptr)["position_window_longitude"] = lon_w;
    }

    return viewable_ptr;
}

bool SingleDetection::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else
        return false;;
}

std::string SingleDetection::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasReference(table, annotation));

    return "Report:Results:"+getTargetRequirementSectionID();
}

std::shared_ptr<Joined> SingleDetection::createEmptyJoined(const std::string& result_id)
{
    return make_shared<JoinedDetection> (result_id, requirement_, sector_layer_, eval_man_);
}

int SingleDetection::sumUIs() const
{
    return sum_uis_;
}

int SingleDetection::missedUIs() const
{
    return missed_uis_;
}

std::vector<EvaluationRequirement::DetectionDetail>& SingleDetection::details()
{
    return details_;
}

}
