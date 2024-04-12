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
#include "eval/results/evaluationdetail.h"
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
#include "viewpoint.h"

#include <cassert>

using namespace std;
using namespace Utils;
using namespace nlohmann;

namespace EvaluationRequirementResult
{

SingleDetection::SingleDetection(const std::string& result_id, 
                                 std::shared_ptr<EvaluationRequirement::Base> requirement,
                                 const SectorLayer& sector_layer,
                                 unsigned int utn,
                                 const EvaluationTargetData* target,
                                 EvaluationManager& eval_man,
                                 const EvaluationDetails& details,
                                 int sum_uis,
                                 int missed_uis,
                                 TimePeriodCollection ref_periods,
                                 const std::vector<dbContent::TargetPosition>& ref_updates)
    :   Single      ("SingleDetection", result_id, requirement, sector_layer, utn, target, eval_man, details)
    ,   sum_uis_    (sum_uis)
    ,   missed_uis_ (missed_uis)
    ,   ref_periods_(ref_periods)
    ,   ref_updates_(ref_updates)
{
    updatePD();
}

void SingleDetection::updatePD()
{
    pd_.reset();

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
    }

    result_usable_ = pd_.has_value();

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

    if (eval_man_.settings().report_split_results_by_mops_
            || eval_man_.settings().report_split_results_by_aconly_ms_) // add to general sum table
        addTargetDetailsToTable(root_item->getSection(getRequirementSumSectionID()), target_table_name_);
}

void SingleDetection::addTargetDetailsToTable (
        EvaluationResultsReport::Section& section, const std::string& table_name)
{




    if (!section.hasTable(table_name))
    {
        std::shared_ptr<EvaluationRequirement::Detection> req =
                std::static_pointer_cast<EvaluationRequirement::Detection>(requirement_);
        assert (req);

        Qt::SortOrder order;

        if (req->invertProb())
            order=Qt::DescendingOrder;
        else
            order=Qt::AscendingOrder;

        section.addTable(table_name, 11,
                         {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                          "#EUIs", "#MUIs", "PD"}, true, 10, order);
    }

    EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

    QVariant pd_var;

    if (pd_.has_value())
        pd_var = roundf(pd_.value() * 10000.0) / 100.0;

    target_table.addRow(
                {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
                 target_->acidsStr().c_str(), target_->acadsStr().c_str(),
                 target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(),
                 target_->modeCMaxStr().c_str(), sum_uis_, missed_uis_, pd_var}, this, {utn_});
}

void SingleDetection::addTargetDetailsToReport(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    QVariant pd_var;

    if (pd_.has_value())
        pd_var = roundf(pd_.value() * 10000.0) / 100.0;

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

    if (pd_.has_value())
        result = req->getConditionResultStr(pd_.value());

    utn_req_table.addRow({"Condition Fulfilled", "", result.c_str()}, this);

    utn_req_table.addRow({"Must hold for any target ", "", req->holdForAnyTarget()}, this);

    // add figure
    if (pd_.has_value() && pd_.value() != 1.0)
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

void SingleDetection::reportDetails(EvaluationResultsReport::Section& utn_req_section)
{
    if (!utn_req_section.hasTable(tr_details_table_name_))
        utn_req_section.addTable(tr_details_table_name_, 5,
                                 {"ToD", "DToD", "Ref.", "MUI", "Comment"});

    EvaluationResultsReport::SectionContentTable& utn_req_details_table =
            utn_req_section.getTable(tr_details_table_name_);

    utn_req_details_table.setCreateOnDemand(
                [this, &utn_req_details_table](void)
    {
        unsigned int detail_cnt = 0;

        for (auto& rq_det_it : getDetails())
        {
            auto d_tod = rq_det_it.getValue(DetailKey::DiffTOD);

            if (d_tod.isValid())
            {
                utn_req_details_table.addRow(
                            { Time::toString(rq_det_it.timestamp()).c_str(),
                              String::timeStringFromDouble(d_tod.toFloat()).c_str(),
                              rq_det_it.getValue(DetailKey::RefExists),
                              rq_det_it.getValue(DetailKey::MissedUIs),
                              rq_det_it.comments().generalComment().c_str() },
                            this, detail_cnt);
            }
            else
            {
                utn_req_details_table.addRow(
                            { Time::toString(rq_det_it.timestamp()).c_str(),
                              QVariant(),
                              rq_det_it.getValue(DetailKey::RefExists),
                              rq_det_it.getValue(DetailKey::MissedUIs),
                              rq_det_it.comments().generalComment().c_str() },
                            this, detail_cnt);
            }

            ++detail_cnt;
        }
    });
}

bool SingleDetection::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else if (table.name() == tr_details_table_name_ && annotation.isValid() && annotation.toUInt() < getDetails().size())
        return true;
    
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

        std::unique_ptr<nlohmann::json::object_t> viewable_ptr = getTargetErrorsViewable(true);
        assert (viewable_ptr);

        const auto& detail = getDetail(detail_cnt);

        assert (detail.numPositions() >= 1);

        (*viewable_ptr)[ViewPoint::VP_POS_LAT_KEY    ] = detail.position(0).latitude_;
        (*viewable_ptr)[ViewPoint::VP_POS_LON_KEY    ] = detail.position(0).longitude_;
        (*viewable_ptr)[ViewPoint::VP_POS_WIN_LAT_KEY] = eval_man_.settings().result_detail_zoom_;
        (*viewable_ptr)[ViewPoint::VP_POS_WIN_LON_KEY] = eval_man_.settings().result_detail_zoom_;
        (*viewable_ptr)[ViewPoint::VP_TIMESTAMP_KEY  ] = Time::toString(detail.timestamp());

        auto miss_occurred = detail.getValueAs<bool>(MissOccurred);
        assert (miss_occurred.has_value());

        addAnnotationPos(*viewable_ptr, detail.position(0), TypeHighlight);

        if (detail.numPositions() >= 2)
            addAnnotationLine(*viewable_ptr, detail.position(0), detail.position(1), TypeHighlight);

        return viewable_ptr;
    }
    else
        return nullptr;
}

std::unique_ptr<nlohmann::json::object_t> SingleDetection::getTargetErrorsViewable (bool add_highlight)
{
    std::unique_ptr<nlohmann::json::object_t> viewable_ptr = eval_man_.getViewableForEvaluation(
                utn_, req_grp_id_, result_id_);

    bool has_pos = false;
    double lat_min, lat_max, lon_min, lon_max;

    for (auto& detail_it : getDetails())
    {
        if (!detail_it.getValue(DetailKey::MissOccurred).toBool())
            continue;

        assert(detail_it.numPositions() >= 1);

        const auto& pos = detail_it.position(0);

        bool has_last_pos = detail_it.numPositions() >= 2;

        if (has_pos)
        {
            lat_min = min(lat_min, pos.latitude_);
            lat_max = max(lat_max, pos.latitude_);

            lon_min = min(lon_min, pos.longitude_);
            lon_max = max(lon_max, pos.longitude_);
        }
        else // tst pos always set
        {
            lat_min = pos.latitude_;
            lat_max = pos.latitude_;

            lon_min = pos.longitude_;
            lon_max = pos.longitude_;

            has_pos = true;
        }

        if (has_last_pos)
        {
            const auto& pos_last = detail_it.position(1);

            lat_min = min(lat_min, pos_last.latitude_);
            lat_max = max(lat_max, pos_last.latitude_);

            lon_min = min(lon_min, pos_last.longitude_);
            lon_max = max(lon_max, pos_last.longitude_);
        }
    }

    if (has_pos)
    {
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
    }

    addAnnotations(*viewable_ptr, false, true);

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

bool SingleDetection::hasFailed() const
{
    std::shared_ptr<EvaluationRequirement::Detection> req =
            std::static_pointer_cast<EvaluationRequirement::Detection>(requirement_);
    assert (req);

    if (pd_.has_value())
        return !req->getConditionResult(pd_.value());
    else
        return false;
}

void SingleDetection::addAnnotations(nlohmann::json::object_t& viewable, bool overview, bool add_ok)
{
    json& error_line_coordinates  = annotationLineCoords(viewable, TypeError, overview);
    json& error_point_coordinates = annotationPointCoords(viewable, TypeError, overview);
    json& ok_line_coordinates     = annotationLineCoords(viewable, TypeOk, overview);
    json& ok_point_coordinates    = annotationPointCoords(viewable, TypeOk, overview);

    for (auto& detail_it : getDetails())
    {
        auto check_failed = detail_it.getValueAsOrAssert<bool>(
                    EvaluationRequirementResult::SingleDetection::DetailKey::MissOccurred);

        if (detail_it.numPositions() == 1)
            continue;

        assert (detail_it.numPositions() >= 2);

        if (check_failed)
        {
            error_point_coordinates.push_back(detail_it.position(0).asVector());
            error_point_coordinates.push_back(detail_it.position(1).asVector());

            error_line_coordinates.push_back(detail_it.position(0).asVector());
            error_line_coordinates.push_back(detail_it.position(1).asVector());
        }
        else if (add_ok)
        {
            ok_point_coordinates.push_back(detail_it.position(0).asVector());
            ok_point_coordinates.push_back(detail_it.position(1).asVector());

            if (!overview)
            {
                ok_line_coordinates.push_back(detail_it.position(0).asVector());
                ok_line_coordinates.push_back(detail_it.position(1).asVector());
            }
        }
    }
}

/**
*/
std::map<std::string, std::vector<Single::LayerDefinition>> SingleDetection::gridLayers() const
{
    std::map<std::string, std::vector<Single::LayerDefinition>> layer_defs;

    layer_defs[ requirement_->name() ].push_back(getGridLayerDefBinary());

    return layer_defs;
}

/**
*/
void SingleDetection::addValuesToGrid(Grid2D& grid, const std::string& layer) const
{
    if (layer == requirement_->name())
    {
        for (auto& detail_it : getDetails())
        {
            auto check_failed = detail_it.getValueAsOrAssert<bool>(
                        EvaluationRequirementResult::SingleDetection::DetailKey::MissOccurred);

            if (detail_it.numPositions() == 1)
                continue;

            assert (detail_it.numPositions() >= 2);

            auto idx0 = detail_it.getValueAs<unsigned int>(EvaluationRequirementResult::SingleDetection::DetailKey::RefUpdateStartIndex).value();
            auto idx1 = detail_it.getValueAs<unsigned int>(EvaluationRequirementResult::SingleDetection::DetailKey::RefUpdateEndIndex).value();

            size_t n = idx1 - idx0 + 1;

            auto pos_getter = [ & ] (double& x, double& y, size_t idx) 
            { 
                x =  ref_updates_[ idx0 + idx ].longitude_;
                y =  ref_updates_[ idx0 + idx ].latitude_;
            };

            grid.addPoly(pos_getter, n, check_failed ? 1.0 : 0.0);
        }
    }
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

}
