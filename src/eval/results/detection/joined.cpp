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

#include "eval/results/detection/joined.h"
#include "eval/results/detection/single.h"
#include "eval/requirement/base/base.h"
#include "eval/requirement/detection/detection.h"
#include "evaluationmanager.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttable.h"
#include "logger.h"
#include "stringconv.h"
#include "viewpoint.h"
#include "sectorlayer.h"

#include <cassert>

using namespace std;
using namespace Utils;

namespace EvaluationRequirementResult
{

JoinedDetection::JoinedDetection(const std::string& result_id, 
                                 std::shared_ptr<EvaluationRequirement::Base> requirement,
                                 const SectorLayer& sector_layer, 
                                 EvaluationManager& eval_man)
:   Joined("JoinedDetection", result_id, requirement, sector_layer, eval_man)
{
}

void JoinedDetection::addToReport (
        std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "JoinedDetection " <<  requirement_->name() <<": addToReport";

    if (!results_.size()) // some data must exist
    {
        logerr << "JoinedDetection " <<  requirement_->name() <<": addToReport: no data";
        return;
    }

    logdbg << "JoinedDetection " <<  requirement_->name() << ": addToReport: adding joined result";

    addToOverviewTable(root_item);
    addDetails(root_item);
}

void JoinedDetection::addToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::SectionContentTable& ov_table = getReqOverviewTable(root_item);

    // condition
    std::shared_ptr<EvaluationRequirement::Detection> req =
            std::static_pointer_cast<EvaluationRequirement::Detection>(requirement_);
    assert (req);

    if (req->holdForAnyTarget()) // for any target
    {
        string result = num_failed_single_targets_ == 0 ? "Passed" : "Failed";

        // "Sector Layer", "Group", "Req.", "Id", "#Updates", "Result", "Condition", "Result"
        ov_table.addRow({sector_layer_.name().c_str(), requirement_->groupName().c_str(),
                            requirement_->shortname().c_str(),
                            result_id_.c_str(), {num_single_targets_},
                            num_failed_single_targets_, "= 0", result.c_str()}, this, {});
    }
    else // pd
    {
        QVariant pd_var;

        string result {"Unknown"};

        if (pd_.has_value())
        {
            pd_var = String::percentToString(pd_.value() * 100.0, req->getNumProbDecimals()).c_str();

            //loginf << "UGA '" << pd_var.toString().toStdString() << "' dec " << req->getNumProbDecimals();

            result = req->getConditionResultStr(pd_.value());
        }

        // "Sector Layer", "Group", "Req.", "Id", "#Updates", "Result", "Condition", "Result"
        ov_table.addRow({sector_layer_.name().c_str(), requirement_->groupName().c_str(),
                            requirement_->shortname().c_str(),
                            result_id_.c_str(), {sum_uis_},
                            pd_var, req->getConditionStr().c_str(), result.c_str()}, this, {});
    }
}

void JoinedDetection::addDetails(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::Section& sector_section = getRequirementSection(root_item);

    if (!sector_section.hasTable("sector_details_table"))
        sector_section.addTable("sector_details_table", 3, {"Name", "comment", "Value"}, false);

    EvaluationResultsReport::SectionContentTable& sec_det_table =
            sector_section.getTable("sector_details_table");

    addCommonDetails(sec_det_table);

    sec_det_table.addRow({"#Updates/#EUIs [1]", "Total number update intervals", sum_uis_}, this);
    sec_det_table.addRow({"#MUIs [1]", "Number of missed update intervals", missed_uis_}, this);

    // condition
    std::shared_ptr<EvaluationRequirement::Detection> req =
            std::static_pointer_cast<EvaluationRequirement::Detection>(requirement_);
    assert (req);

    // pd
    QVariant pd_var;

    string result {"Unknown"};

    if (pd_.has_value())
    {
        pd_var = String::percentToString(pd_.value() * 100.0, req->getNumProbDecimals()).c_str();

        result = req->getConditionResultStr(pd_.value());
    }

    sec_det_table.addRow({"PD [%]", "Probability of Detection", pd_var}, this);
    sec_det_table.addRow({"Condition", {}, req->getConditionStr().c_str()}, this);
    sec_det_table.addRow({"Condition Fulfilled", {}, result.c_str()}, this);

    sec_det_table.addRow({"Must hold for any target ", "", req->holdForAnyTarget()}, this);
    sec_det_table.addRow({"#Single Targets", {}, num_single_targets_}, this);
    sec_det_table.addRow({"#Failed Single Targets", {}, num_failed_single_targets_}, this);

    // figure
    sector_section.addFigure("sector_overview", "Sector Overview",
                             [this](void) { return this->getErrorsViewable(); });
}

bool JoinedDetection::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    //loginf << "UGA2 '"  << table.name() << "'" << " other '" << req_overview_table_name_ << "'";

    if (table.name() == req_overview_table_name_)
        return true;
    else
        return false;;
}

std::unique_ptr<nlohmann::json::object_t> JoinedDetection::viewableDataImpl(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasViewableData(table, annotation));
    return getErrorsViewable();
}

std::unique_ptr<nlohmann::json::object_t> JoinedDetection::getErrorsViewable ()
{
    std::unique_ptr<nlohmann::json::object_t> viewable_ptr =
            eval_man_.getViewableForEvaluation(req_grp_id_, result_id_);

    double lat_min, lat_max, lon_min, lon_max;

    tie(lat_min, lat_max) = sector_layer_.getMinMaxLatitude();
    tie(lon_min, lon_max) = sector_layer_.getMinMaxLongitude();

    (*viewable_ptr)[ViewPoint::VP_POS_LAT_KEY] = (lat_max+lat_min)/2.0;
    (*viewable_ptr)[ViewPoint::VP_POS_LON_KEY] = (lon_max+lon_min)/2.0;;

    double lat_w = lat_max-lat_min;
    double lon_w = lon_max-lon_min;

    if (lat_w < eval_man_.settings().result_detail_zoom_)
        lat_w = eval_man_.settings().result_detail_zoom_;

    if (lon_w < eval_man_.settings().result_detail_zoom_)
        lon_w = eval_man_.settings().result_detail_zoom_;

    (*viewable_ptr)[ViewPoint::VP_POS_WIN_LAT_KEY] = lat_w;
    (*viewable_ptr)[ViewPoint::VP_POS_WIN_LON_KEY] = lon_w;

    addAnnotationsFromSingles(*viewable_ptr);

    return viewable_ptr;
}

bool JoinedDetection::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    //loginf << "UGA3 '"  << table.name() << "'" << " other '" << req_overview_table_name_ << "'";

    if (table.name() == req_overview_table_name_)
        return true;
    else
        return false;
}

std::string JoinedDetection::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasReference(table, annotation));
    return "Report:Results:"+getRequirementSectionID();
}

void JoinedDetection::updateToChanges_impl()
{
    loginf << "JoinedDetection: updateToChanges_impl: prev sum_uis " << sum_uis_
            << " missed_uis " << missed_uis_;

    sum_uis_    = 0;
    missed_uis_ = 0;

    for (auto& result_it : results_)
    {
        std::shared_ptr<SingleDetection> single_result =
                std::static_pointer_cast<SingleDetection>(result_it);
        assert (single_result);

        single_result->setInterestFactor(0);

        if (!single_result->use())
            continue;

        sum_uis_    += single_result->sumUIs();
        missed_uis_ += single_result->missedUIs();

        ++num_single_targets_;

        if (single_result->hasFailed())
            ++num_failed_single_targets_;

    }

    loginf << "JoinedDetection: updateToChanges_impl: updt sum_uis " << sum_uis_
            << " missed_uis " << missed_uis_;

    // calculate pd

    pd_.reset();

    if (sum_uis_)
    {
        logdbg << "JoinedDetection: updateToChanges_impl: result_id " << result_id_ << " missed_uis " << missed_uis_
               << " sum_uis " << sum_uis_;

        assert (missed_uis_ <= sum_uis_);

        std::shared_ptr<EvaluationRequirement::Detection> req =
            std::static_pointer_cast<EvaluationRequirement::Detection>(requirement_);
        assert (req);

        if (req->invertProb())
            pd_ = (float)missed_uis_/(float)(sum_uis_);
        else
            pd_ = 1.0 - ((float)missed_uis_/(float)(sum_uis_));


        // attribute interest
        for (auto& result_it : results_)
        {
            std::shared_ptr<SingleDetection> single_result =
                std::static_pointer_cast<SingleDetection>(result_it);
            assert (single_result);

            if (!single_result->use())
                continue;

            assert (missed_uis_ >= single_result->missedUIs());

            single_result->setInterestFactor((float)single_result->missedUIs() / (float)missed_uis_);
        }
    }


}

}
