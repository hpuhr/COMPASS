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

#include "eval/results/generic/genericsingle.h"
#include "eval/results/generic/genericjoined.h"
#include "eval/requirement/base/base.h"
#include "eval/requirement/generic/generic.h"
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

JoinedGeneric::JoinedGeneric(const std::string& result_type, const std::string& result_id,
                                   std::shared_ptr<EvaluationRequirement::Base> requirement,
                                   const SectorLayer& sector_layer, 
                                   EvaluationManager& eval_man)
:   Joined(result_type, result_id, requirement, sector_layer, eval_man)
{
}

void JoinedGeneric::addToReport (
        std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "JoinedGeneric " <<  requirement_->name() <<": addToReport";

    if (!results_.size()) // some data must exist
    {
        logerr << "JoinedGeneric " <<  requirement_->name() <<": addToReport: no data";
        return;
    }

    logdbg << "JoinedGeneric " <<  requirement_->name() << ": addToReport: adding joined result";

    addToOverviewTable(root_item);
    addDetails(root_item);
}

void JoinedGeneric::addToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::SectionContentTable& ov_table = getReqOverviewTable(root_item);

    // condition
    std::shared_ptr<EvaluationRequirement::GenericInteger> req =
            std::static_pointer_cast<EvaluationRequirement::GenericInteger>(requirement_);
    assert (req);

    // p false
    {
        QVariant pf_var;

        string result {"Unknown"};

        if (prob_.has_value())
        {
            result = req->getConditionResultStr(prob_.value());
            pf_var = roundf(prob_.value() * 10000.0) / 100.0;
        }

        // "Sector Layer", "Group", "Req.", "Id", "#Updates", "Result", "Condition", "Result"
        ov_table.addRow({sector_layer_.name().c_str(), requirement_->groupName().c_str(),
                         requirement_->shortname().c_str(),
                         result_id_.c_str(), {num_correct_+num_false_},
                         pf_var, req->getConditionStr().c_str(), result.c_str()}, this, {});
    }
}

void JoinedGeneric::addDetails(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::Section& sector_section = getRequirementSection(root_item);

    if (!sector_section.hasTable("sector_details_table"))
        sector_section.addTable("sector_details_table", 3, {"Name", "comment", "Value"}, false);

    EvaluationResultsReport::SectionContentTable& sec_det_table =
            sector_section.getTable("sector_details_table");

    addCommonDetails(sec_det_table);

    EvaluationRequirement::GenericBase& req = genericRequirement();

    sec_det_table.addRow({"Use", "To be used in results", use_}, this);
    sec_det_table.addRow({"#Up [1]", "Number of updates", num_updates_}, this);
    sec_det_table.addRow({"#NoRef [1]", ("Number of updates w/o reference position or "+req.valueName()).c_str(),
                          num_no_ref_pos_+num_no_ref_val_}, this);
    sec_det_table.addRow({"#NoRefPos [1]", "Number of updates w/o reference position ", num_no_ref_pos_}, this);
    sec_det_table.addRow({"#NoRef [1]", ("Number of updates w/o reference "+req.valueName()).c_str(), num_no_ref_val_}, this);
    sec_det_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
    sec_det_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_pos_outside_}, this);
    sec_det_table.addRow({"#Unknown [1]", ("Number of updates unknown "+req.valueName()).c_str(), num_unknown_}, this);
    sec_det_table.addRow({"#Correct [1]", ("Number of updates with correct "+req.valueName()).c_str(), num_correct_}, this);
    sec_det_table.addRow({"#False [1]", ("Number of updates with incorrect "+req.valueName()).c_str(), num_false_}, this);

    // condition
    {
        QVariant pf_var;

        if (prob_.has_value())
            pf_var = roundf(prob_.value() * 10000.0) / 100.0;

        sec_det_table.addRow({(req.probabilityNameShort()+" [%]").c_str(), req.probabilityName().c_str(), pf_var}, this);

        sec_det_table.addRow({"Condition", "", req.getConditionStr().c_str()}, this);

        string result {"Unknown"};

        if (prob_.has_value())
            result = req.getConditionResultStr(prob_.value());

        sec_det_table.addRow({"Condition Fulfilled", "", result.c_str()}, this);
    }

    // figure
    sector_section.addFigure("sector_overview", "Sector Overview",
                             [this](void) { return this->getErrorsViewable(); });
}

bool JoinedGeneric::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == req_overview_table_name_)
        return true;
    else
        return false;
}

std::unique_ptr<nlohmann::json::object_t> JoinedGeneric::viewableDataImpl(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasViewableData(table, annotation));

    return getErrorsViewable();
}

std::unique_ptr<nlohmann::json::object_t> JoinedGeneric::getErrorsViewable ()
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

bool JoinedGeneric::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == req_overview_table_name_)
        return true;
    else
        return false;;
}

std::string JoinedGeneric::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasReference(table, annotation));
    return "Report:Results:"+getRequirementSectionID();

    return nullptr;
}

void JoinedGeneric::updateToChanges_impl()
{
    loginf << "JoinedGeneric: updateToChanges_impl: prev num_updates " << num_updates_
           << " num_no_ref_pos " << num_no_ref_pos_ << " num_no_ref_id " << num_no_ref_val_
           << " num_unknown_id " << num_unknown_
           << " num_correct_id " << num_correct_ << " num_false_id " << num_false_;

    num_updates_     = 0;
    num_no_ref_pos_  = 0;
    num_no_ref_val_  = 0;
    num_pos_outside_ = 0;
    num_pos_inside_  = 0;
    num_unknown_     = 0;
    num_correct_     = 0;
    num_false_       = 0;

    // process
    for (auto& result_it : results_)
    {
        std::shared_ptr<SingleGeneric> single_result =
                std::static_pointer_cast<SingleGeneric>(result_it);
        assert (single_result);

        if (!single_result->use())
            continue;

        num_updates_     += single_result->numUpdates();
        num_no_ref_pos_  += single_result->numNoRefPos();
        num_no_ref_val_  += single_result->numNoRefValue();
        num_pos_outside_ += single_result->numPosOutside();
        num_pos_inside_  += single_result->numPosInside();
        num_unknown_     += single_result->numUnknown();
        num_correct_     += single_result->numCorrect();
        num_false_       += single_result->numFalse();

    }

    loginf << "JoinedGeneric: updateToChanges_impl: updt num_updates " << num_updates_
           << " num_no_ref_pos " << num_no_ref_pos_ << " num_no_ref_id " << num_no_ref_val_
           << " num_unknown_id " << num_unknown_
           << " num_correct_id " << num_correct_ << " num_false_id " << num_false_;

    assert (num_updates_ - num_no_ref_pos_ == num_pos_inside_ + num_pos_outside_);
    assert (num_pos_inside_ == num_no_ref_val_+num_unknown_+num_correct_+num_false_);

    prob_.reset();

    if (num_correct_+num_false_)
    {
        prob_ = (float)(num_correct_)/(float)(num_correct_+num_false_);
    }
}

EvaluationRequirement::GenericBase& JoinedGeneric::genericRequirement() const
{
    assert (requirement_);
    EvaluationRequirement::GenericBase* req_ptr = dynamic_cast<EvaluationRequirement::GenericBase*>(requirement_.get());
    assert (req_ptr);

    return *req_ptr;
}

}
