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

#include "eval/results/extra/trackjoined.h"
#include "eval/results/extra/tracksingle.h"
#include "eval/requirement/base/base.h"
#include "eval/requirement/extra/track.h"
//#include "evaluationtargetdata.h"
#include "evaluationmanager.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
//#include "eval/results/report/sectioncontenttext.h"
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

JoinedExtraTrack::JoinedExtraTrack(const std::string& result_id, 
                                    std::shared_ptr<EvaluationRequirement::Base> requirement,
                                    const SectorLayer& sector_layer, 
                                    EvaluationManager& eval_man)
:   Joined("JoinedExtraTrack", result_id, requirement, sector_layer, eval_man)
{
}

void JoinedExtraTrack::join_impl(std::shared_ptr<Single> other)
{
    std::shared_ptr<SingleExtraTrack> other_sub =
            std::static_pointer_cast<SingleExtraTrack>(other);
    assert (other_sub);

    addToValues(other_sub);
}

void JoinedExtraTrack::addToValues (std::shared_ptr<SingleExtraTrack> single_result)
{
    assert (single_result);

    if (!single_result->use())
        return;

    num_inside_ += single_result->numInside();
    num_extra_ += single_result->numExtra();
    num_ok_ += single_result->numOK();

    updateProb();
}

void JoinedExtraTrack::updateProb()
{
    assert (num_inside_ >= num_extra_ + num_ok_);

    prob_.reset();

    if (num_extra_ + num_ok_)
    {
        logdbg << "JoinedTrack: updateProb: result_id " << result_id_ << " num_extra " << num_extra_
                << " num_ok " << num_ok_;

        prob_ = (float)num_extra_/(float)(num_extra_ + num_ok_);
    }
}

void JoinedExtraTrack::addToReport (
        std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "JoinedTrack " <<  requirement_->name() <<": addToReport";

    if (!results_.size()) // some data must exist
    {
        logerr << "JoinedTrack " <<  requirement_->name() <<": addToReport: no data";
        return;
    }

    logdbg << "JoinedTrack " <<  requirement_->name() << ": addToReport: adding joined result";

    addToOverviewTable(root_item);
    addDetails(root_item);
}

void JoinedExtraTrack::addToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::SectionContentTable& ov_table = getReqOverviewTable(root_item);

    // condition
    std::shared_ptr<EvaluationRequirement::ExtraTrack> req =
            std::static_pointer_cast<EvaluationRequirement::ExtraTrack>(requirement_);
    assert (req);

    // pd
    QVariant prob_var;

    string result {"Unknown"};

    if (prob_.has_value())
    {
        prob_var = String::percentToString(prob_.value() * 100.0, req->getNumProbDecimals()).c_str();

        result = req->getConditionResultStr(prob_.value());
    }

    // "Sector Layer", "Group", "Req.", "Id", "#Updates", "Result", "Condition", "Result"
    ov_table.addRow({sector_layer_.name().c_str(), requirement_->groupName().c_str(),
                        requirement_->shortname().c_str(),
                        result_id_.c_str(), {num_extra_+num_ok_},
                        prob_var, req->getConditionStr().c_str(), result.c_str()}, this, {});
    // "Report:Results:Overview"
}

void JoinedExtraTrack::addDetails(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::Section& sector_section = getRequirementSection(root_item);

    if (!sector_section.hasTable("sector_details_table"))
        sector_section.addTable("sector_details_table", 3, {"Name", "comment", "Value"}, false);

    EvaluationResultsReport::SectionContentTable& sec_det_table =
            sector_section.getTable("sector_details_table");

    addCommonDetails(sec_det_table);

    sec_det_table.addRow({"#Check.", "Number of checked test track updates", num_extra_+num_ok_}, this);
    sec_det_table.addRow({"#OK.", "Number of OK test track updates", num_ok_}, this);
    sec_det_table.addRow({"#Extra", "Number of extra test track updates", num_extra_}, this);

    // condition
    std::shared_ptr<EvaluationRequirement::ExtraTrack> req =
            std::static_pointer_cast<EvaluationRequirement::ExtraTrack>(requirement_);
    assert (req);

    // pd
    QVariant prob_var;

    string result {"Unknown"};

    if (prob_.has_value())
    {
        prob_var = String::percentToString(prob_.value() * 100.0, req->getNumProbDecimals()).c_str();

        result = req->getConditionResultStr(prob_.value());
    }

    sec_det_table.addRow({"PEx [%]", "Probability of update with extra track", prob_var}, this);
    sec_det_table.addRow({"Condition", {}, req->getConditionStr().c_str()}, this);
    sec_det_table.addRow({"Condition Fulfilled", {}, result.c_str()}, this);

    // figure
    sector_section.addFigure("sector_overview", "Sector Overview",
                             [this](void) { return this->getErrorsViewable(); });
}

bool JoinedExtraTrack::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == req_overview_table_name_)
        return true;
    else
        return false;
}

std::unique_ptr<nlohmann::json::object_t> JoinedExtraTrack::viewableData(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasViewableData(table, annotation));
    return getErrorsViewable();
}

std::unique_ptr<nlohmann::json::object_t> JoinedExtraTrack::getErrorsViewable ()
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

bool JoinedExtraTrack::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    //loginf << "UGA3 '"  << table.name() << "'" << " other '" << req_overview_table_name_ << "'";

    if (table.name() == req_overview_table_name_)
        return true;
    else
        return false;;
}

std::string JoinedExtraTrack::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasReference(table, annotation));
    return "Report:Results:"+getRequirementSectionID();
}

void JoinedExtraTrack::updatesToUseChanges_impl()
{
    num_inside_ = 0;
    num_extra_  = 0;
    num_ok_     = 0;

    for (auto result_it : results_)
    {
        std::shared_ptr<SingleExtraTrack> result =
                std::static_pointer_cast<SingleExtraTrack>(result_it);
        assert (result);

        addToValues(result);
    }
}

}
