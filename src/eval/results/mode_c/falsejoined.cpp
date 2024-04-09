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

#include "eval/results/mode_c/falsesingle.h"
#include "eval/results/mode_c/falsejoined.h"
#include "eval/requirement/base/base.h"
#include "eval/requirement/mode_c/false.h"
//#include "evaluationtargetdata.h"
#include "evaluationmanager.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
//#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "logger.h"
//#include "stringconv.h"
#include "viewpoint.h"
#include "sectorlayer.h"

#include <cassert>

using namespace std;
using namespace Utils;

namespace EvaluationRequirementResult
{

JoinedModeCFalse::JoinedModeCFalse(const std::string& result_id, 
                                   std::shared_ptr<EvaluationRequirement::Base> requirement,
                                   const SectorLayer& sector_layer, 
                                   EvaluationManager& eval_man)
:   JoinedFalseBase("JoinedModeCFalse", result_id, requirement, sector_layer, eval_man)
{
}

void JoinedModeCFalse::join_impl(std::shared_ptr<Single> other)
{
    std::shared_ptr<SingleModeCFalse> other_sub =
            std::static_pointer_cast<SingleModeCFalse>(other);
    assert (other_sub);

    addToValues(other_sub);
}

void JoinedModeCFalse::addToValues (std::shared_ptr<SingleModeCFalse> single_result)
{
    assert (single_result);

    if (!single_result->use())
        return;

    num_updates_     += single_result->numUpdates();
    num_no_ref_pos_  += single_result->numNoRefPos();
    num_no_ref_val_  += single_result->numNoRefValue();
    num_pos_outside_ += single_result->numPosOutside();
    num_pos_inside_  += single_result->numPosInside();
    num_unknown_     += single_result->numUnknown();
    num_correct_     += single_result->numCorrect();
    num_false_       += single_result->numFalse();

    updateProbabilities();
}

void JoinedModeCFalse::updateProbabilities()
{
    assert (num_updates_ - num_no_ref_pos_ == num_pos_inside_ + num_pos_outside_);
    assert (num_pos_inside_ == num_no_ref_val_+num_unknown_+num_correct_+num_false_);

    p_false_.reset();

    if (num_correct_+num_false_)
    {
        p_false_ = (float)(num_false_)/(float)(num_correct_+num_false_);
    }
}

void JoinedModeCFalse::addToReport (
        std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "JoinedModeC " <<  requirement_->name() <<": addToReport";

    if (!results_.size()) // some data must exist
    {
        logerr << "JoinedModeC " <<  requirement_->name() <<": addToReport: no data";
        return;
    }

    logdbg << "JoinedModeC " <<  requirement_->name() << ": addToReport: adding joined result";

    addToOverviewTable(root_item);
    addDetails(root_item);
}

void JoinedModeCFalse::addToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::SectionContentTable& ov_table = getReqOverviewTable(root_item);

    // condition
    std::shared_ptr<EvaluationRequirement::ModeCFalse> req =
            std::static_pointer_cast<EvaluationRequirement::ModeCFalse>(requirement_);
    assert (req);

    // p false
    {
        std::shared_ptr<EvaluationRequirement::ModeCFalse> req =
                std::static_pointer_cast<EvaluationRequirement::ModeCFalse>(requirement_);
        assert (req);

        QVariant pf_var;

        string result {"Unknown"};

        if (p_false_.has_value())
        {
            result = req->getConditionResultStr(p_false_.value());
            pf_var = roundf(p_false_.value() * 10000.0) / 100.0;
        }

        // "Sector Layer", "Group", "Req.", "Id", "#Updates", "Result", "Condition", "Result"
        ov_table.addRow({sector_layer_.name().c_str(), requirement_->groupName().c_str(),
                         requirement_->shortname().c_str(),
                         result_id_.c_str(), {num_correct_+num_false_},
                         pf_var, req->getConditionStr().c_str(), result.c_str()}, this, {});
    }
}

void JoinedModeCFalse::addDetails(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::Section& sector_section = getRequirementSection(root_item);

    if (!sector_section.hasTable("sector_details_table"))
        sector_section.addTable("sector_details_table", 3, {"Name", "comment", "Value"}, false);

    EvaluationResultsReport::SectionContentTable& sec_det_table =
            sector_section.getTable("sector_details_table");

    addCommonDetails(sec_det_table);

    sec_det_table.addRow({"Use", "To be used in results", use_}, this);
    sec_det_table.addRow({"#Up [1]", "Number of updates", num_updates_}, this);
    sec_det_table.addRow({"#NoRef [1]", "Number of updates w/o reference position or code",
                          num_no_ref_pos_+num_no_ref_val_}, this);
    sec_det_table.addRow({"#NoRefPos [1]", "Number of updates w/o reference position ", num_no_ref_pos_}, this);
    sec_det_table.addRow({"#NoRef [1]", "Number of updates w/o reference code", num_no_ref_val_}, this);
    sec_det_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
    sec_det_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_pos_outside_}, this);
    sec_det_table.addRow({"#Unknown [1]", "Number of updates unknown code", num_unknown_}, this);
    sec_det_table.addRow({"#Correct [1]", "Number of updates with correct code", num_correct_}, this);
    sec_det_table.addRow({"#False [1]", "Number of updates with false code", num_false_}, this);

    // condition
    {
        std::shared_ptr<EvaluationRequirement::ModeCFalse> req =
                std::static_pointer_cast<EvaluationRequirement::ModeCFalse>(requirement_);
        assert (req);

        QVariant pf_var;

        if (p_false_.has_value())
            pf_var = roundf(p_false_.value() * 10000.0) / 100.0;

        sec_det_table.addRow({"PF [%]", "Probability of Mode C false", pf_var}, this);

        sec_det_table.addRow({"Condition", "", req->getConditionStr().c_str()}, this);

        string result {"Unknown"};

        if (p_false_.has_value())
            result = req->getConditionResultStr(p_false_.value());

        sec_det_table.addRow({"Condition Fulfilled", "", result.c_str()}, this);
    }

    // figure
    sector_section.addFigure("sector_overview", "Sector Overview",
                             [this](void) { return this->getErrorsViewable(); });
}

bool JoinedModeCFalse::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    //loginf << "UGA4 '"  << table.name() << "'" << " other '" << req_overview_table_name_ << "'";

    if (table.name() == req_overview_table_name_)
        return true;
    else
        return false;
}

std::unique_ptr<nlohmann::json::object_t> JoinedModeCFalse::viewableDataImpl(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasViewableData(table, annotation));

    return getErrorsViewable();
}

std::unique_ptr<nlohmann::json::object_t> JoinedModeCFalse::getErrorsViewable ()
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

bool JoinedModeCFalse::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    //loginf << "UGA5 '"  << table.name() << "'" << " other '" << req_overview_table_name_ << "'";

    if (table.name() == req_overview_table_name_)
        return true;
    else
        return false;;
}

std::string JoinedModeCFalse::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasReference(table, annotation));
    return "Report:Results:"+getRequirementSectionID();

    return nullptr;
}

void JoinedModeCFalse::updatesToUseChanges_impl()
{
    loginf << "JoinedModeC: updatesToUseChanges: prev num_updates " << num_updates_
           << " num_no_ref_pos " << num_no_ref_pos_ << " num_no_ref_id " << num_no_ref_val_
           << " num_unknown_id " << num_unknown_
           << " num_correct_id " << num_correct_ << " num_false_id " << num_false_;

    //        if (has_pid_)
    //            loginf << "JoinedModeC: updatesToUseChanges: prev result " << result_id_
    //                   << " pid " << 100.0 * pid_;
    //        else
    //            loginf << "JoinedModeC: updatesToUseChanges: prev result " << result_id_ << " has no data";

    num_updates_     = 0;
    num_no_ref_pos_  = 0;
    num_no_ref_val_  = 0;
    num_pos_outside_ = 0;
    num_pos_inside_  = 0;
    num_unknown_     = 0;
    num_correct_     = 0;
    num_false_       = 0;

    for (auto result_it : results_)
    {
        std::shared_ptr<SingleModeCFalse> result =
                std::static_pointer_cast<SingleModeCFalse>(result_it);
        assert (result);

        addToValues(result);
    }

    loginf << "JoinedModeC: updatesToUseChanges: updt num_updates " << num_updates_
           << " num_no_ref_pos " << num_no_ref_pos_ << " num_no_ref_id " << num_no_ref_val_
           << " num_unknown_id " << num_unknown_
           << " num_correct_id " << num_correct_ << " num_false_id " << num_false_;

    //        if (has_pid_)
    //            loginf << "JoinedModeC: updatesToUseChanges: updt result " << result_id_
    //                   << " pid " << 100.0 * pid_;
    //        else
    //            loginf << "JoinedModeC: updatesToUseChanges: updt result " << result_id_ << " has no data";
}

}
