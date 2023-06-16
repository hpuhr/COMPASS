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

#include "eval/results/mode_a/presentsingle.h"
#include "eval/results/mode_a/presentjoined.h"
#include "eval/requirement/base/base.h"
#include "eval/requirement/mode_a/present.h"
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

JoinedModeAPresent::JoinedModeAPresent(const std::string& result_id, 
                                       std::shared_ptr<EvaluationRequirement::Base> requirement,
                                       const SectorLayer& sector_layer, 
                                       EvaluationManager& eval_man)
:   JoinedPresentBase("JoinedModeAPresent", result_id, requirement, sector_layer, eval_man)
{
}

void JoinedModeAPresent::join_impl(std::shared_ptr<Single> other)
{
    std::shared_ptr<SingleModeAPresent> other_sub =
            std::static_pointer_cast<SingleModeAPresent>(other);
    assert (other_sub);

    addToValues(other_sub);
}

void JoinedModeAPresent::addToValues (std::shared_ptr<SingleModeAPresent> single_result)
{
    assert (single_result);

    if (!single_result->use())
        return;

    num_updates_     += single_result->numUpdates();
    num_no_ref_pos_  += single_result->numNoRefPos();
    num_pos_outside_ += single_result->numPosOutside();
    num_pos_inside_  += single_result->numPosInside();
    num_no_ref_val_  += single_result->numNoRefValue();
    num_present_     += single_result->numPresent();
    num_missing_     += single_result->numMissing();

    updateProbabilities();
}

void JoinedModeAPresent::updateProbabilities()
{
    assert (num_updates_ - num_no_ref_pos_ == num_pos_inside_ + num_pos_outside_);
    assert (num_pos_inside_ == num_no_ref_val_ + num_present_ + num_missing_);

    p_present_.reset();

    if (num_no_ref_val_ + num_present_ + num_missing_)
    {
        p_present_ = (float)(num_no_ref_val_ + num_present_) / (float)(num_no_ref_val_ + num_present_ + num_missing_);
    }
}

void JoinedModeAPresent::addToReport (
        std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "JoinedModeA " <<  requirement_->name() <<": addToReport";

    if (!results_.size()) // some data must exist
    {
        logerr << "JoinedModeA " <<  requirement_->name() <<": addToReport: no data";
        return;
    }

    logdbg << "JoinedModeA " <<  requirement_->name() << ": addToReport: adding joined result";

    addToOverviewTable(root_item);
    addDetails(root_item);
}

void JoinedModeAPresent::addToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::SectionContentTable& ov_table = getReqOverviewTable(root_item);

    // condition
    std::shared_ptr<EvaluationRequirement::ModeAPresent> req =
            std::static_pointer_cast<EvaluationRequirement::ModeAPresent>(requirement_);
    assert (req);

    // p present
    {
        std::shared_ptr<EvaluationRequirement::ModeAPresent> req =
                std::static_pointer_cast<EvaluationRequirement::ModeAPresent>(requirement_);
        assert (req);

        QVariant pe_var;

        string result {"Unknown"};

        if (p_present_.has_value())
        {
            result = req->getConditionResultStr(p_present_.value());
            pe_var = roundf(p_present_.value() * 10000.0) / 100.0;
        }

        // "Sector Layer", "Group", "Req.", "Id", "#Updates", "Result", "Condition", "Result"
        ov_table.addRow({sector_layer_.name().c_str(), requirement_->groupName().c_str(),
                            requirement_->shortname().c_str(),
                            result_id_.c_str(), {num_no_ref_val_+num_present_+num_missing_},
                            pe_var, req->getConditionStr().c_str(), result.c_str()}, this, {});
    }

}

void JoinedModeAPresent::addDetails(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::Section& sector_section = getRequirementSection(root_item);

    if (!sector_section.hasTable("sector_details_table"))
        sector_section.addTable("sector_details_table", 3, {"Name", "comment", "Value"}, false);

    EvaluationResultsReport::SectionContentTable& sec_det_table =
            sector_section.getTable("sector_details_table");

    addCommonDetails(sec_det_table);

    sec_det_table.addRow({"Use", "To be used in results", use_}, this);
    sec_det_table.addRow({"#Up [1]", "Number of updates", num_updates_}, this);
    sec_det_table.addRow({"#NoRef [1]", "Number of updates w/o reference position", num_no_ref_pos_}, this);
    sec_det_table.addRow({"#NoRefPos [1]", "Number of updates w/o reference position ", num_no_ref_pos_}, this);
    sec_det_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
    sec_det_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_pos_outside_}, this);
    sec_det_table.addRow({"#NoRefId [1]", "Number of updates without reference code", num_no_ref_val_}, this);
    sec_det_table.addRow({"#Present [1]", "Number of updates with present tst code", num_present_}, this);
    sec_det_table.addRow({"#Missing [1]", "Number of updates with missing tst code", num_missing_}, this);

    // condition
    {
        std::shared_ptr<EvaluationRequirement::ModeAPresent> req =
                std::static_pointer_cast<EvaluationRequirement::ModeAPresent>(requirement_);
        assert (req);

        QVariant pe_var;

        if (p_present_.has_value())
            pe_var = roundf(p_present_.value() * 10000.0) / 100.0;

        sec_det_table.addRow({"PP [%]", "Probability of Mode 3/A present", pe_var}, this);

        sec_det_table.addRow(
        {"Condition", "", req->getConditionStr().c_str()}, this);

        string result {"Unknown"};

        if (p_present_.has_value())
            result = req->getConditionResultStr(p_present_.value());

        sec_det_table.addRow({"Condition Fulfilled", "", result.c_str()}, this);
    }

    // figure
    sector_section.addFigure("sector_overview", "Sector Overview",
                             [this](void) { return this->getErrorsViewable(); });
}

bool JoinedModeAPresent::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    //loginf << "UGA4 '"  << table.name() << "'" << " other '" << req_overview_table_name_ << "'";

    if (table.name() == req_overview_table_name_)
        return true;
    else
        return false;
}

std::unique_ptr<nlohmann::json::object_t> JoinedModeAPresent::viewableData(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasViewableData(table, annotation));

    return getErrorsViewable();
}

std::unique_ptr<nlohmann::json::object_t> JoinedModeAPresent::getErrorsViewable ()
{
    std::unique_ptr<nlohmann::json::object_t> viewable_ptr =
            eval_man_.getViewableForEvaluation(req_grp_id_, result_id_);

    double lat_min, lat_max, lon_min, lon_max;

    tie(lat_min, lat_max) = sector_layer_.getMinMaxLatitude();
    tie(lon_min, lon_max) = sector_layer_.getMinMaxLongitude();

    (*viewable_ptr)[VP_POS_LAT_KEY] = (lat_max+lat_min)/2.0;
    (*viewable_ptr)[VP_POS_LON_KEY] = (lon_max+lon_min)/2.0;;

    double lat_w = 1.1*(lat_max-lat_min)/2.0;
    double lon_w = 1.1*(lon_max-lon_min)/2.0;

    if (lat_w < eval_man_.settings().result_detail_zoom_)
        lat_w = eval_man_.settings().result_detail_zoom_;

    if (lon_w < eval_man_.settings().result_detail_zoom_)
        lon_w = eval_man_.settings().result_detail_zoom_;

    (*viewable_ptr)[VP_POS_WIN_LAT_KEY] = lat_w;
    (*viewable_ptr)[VP_POS_WIN_LON_KEY] = lon_w;

    addAnnotationsFromSingles(*viewable_ptr);

    return viewable_ptr;
}

bool JoinedModeAPresent::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    //loginf << "UGA5 '"  << table.name() << "'" << " other '" << req_overview_table_name_ << "'";

    if (table.name() == req_overview_table_name_)
        return true;
    else
        return false;;
}

std::string JoinedModeAPresent::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasReference(table, annotation));
    return "Report:Results:"+getRequirementSectionID();

    return nullptr;
}

void JoinedModeAPresent::updatesToUseChanges_impl()
{
    loginf << "JoinedModeA: updatesToUseChanges: prev num_updates " << num_updates_
            << " num_no_ref_pos " << num_no_ref_pos_
            << " num_no_ref_id " << num_no_ref_val_
            << " num_present_id " << num_present_ << " num_missing_id " << num_missing_;

//        if (has_pid_)
//            loginf << "JoinedModeA: updatesToUseChanges: prev result " << result_id_
//                   << " pid " << 100.0 * pid_;
//        else
//            loginf << "JoinedModeA: updatesToUseChanges: prev result " << result_id_ << " has no data";

    num_updates_     = 0;
    num_no_ref_pos_  = 0;
    num_pos_outside_ = 0;
    num_pos_inside_  = 0;
    num_no_ref_val_  = 0;
    num_present_     = 0;
    num_missing_     = 0;

    for (auto result_it : results_)
    {
        std::shared_ptr<SingleModeAPresent> result =
                std::static_pointer_cast<SingleModeAPresent>(result_it);
        assert (result);

        addToValues(result);
    }

    loginf << "JoinedModeA: updatesToUseChanges: updt num_updates " << num_updates_
            << " num_no_ref_pos " << num_no_ref_pos_
            << " num_no_ref_id " << num_no_ref_val_
            << " num_present_id " << num_present_ << " num_missing_id " << num_missing_;

//        if (has_pid_)
//            loginf << "JoinedModeA: updatesToUseChanges: updt result " << result_id_
//                   << " pid " << 100.0 * pid_;
//        else
//            loginf << "JoinedModeA: updatesToUseChanges: updt result " << result_id_ << " has no data";
}

}
