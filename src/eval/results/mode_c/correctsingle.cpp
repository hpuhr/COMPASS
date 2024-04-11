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

#include "eval/results/mode_c/correctsingle.h"
#include "eval/results/mode_c/correctjoined.h"
#include "eval/requirement/base/base.h"
#include "eval/requirement/mode_c/correct.h"
#include "evaluationtargetdata.h"
#include "evaluationmanager.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "logger.h"
#include "util/stringconv.h"
#include "util/timeconv.h"
#include "viewpoint.h"

#include <cassert>

using namespace std;
using namespace Utils;
using namespace nlohmann;

namespace EvaluationRequirementResult
{

SingleModeCCorrect::SingleModeCCorrect(const std::string& result_id,
                                                         std::shared_ptr<EvaluationRequirement::Base> requirement,
                                                         const SectorLayer& sector_layer,
                                                         unsigned int utn,
                                                         const EvaluationTargetData* target,
                                                         EvaluationManager& eval_man,
                                                         const EvaluationDetails& details,
                                                         unsigned int num_updates,
                                                         unsigned int num_no_ref_pos,
                                                         unsigned int num_no_ref_id,
                                                         unsigned int num_pos_outside,
                                                         unsigned int num_pos_inside,
                                                         unsigned int num_correct,
                                                         unsigned int num_not_correct)
    :   Single("SingleModeCCorrect", result_id, requirement, sector_layer, utn, target, eval_man, details)
    ,   num_updates_    (num_updates)
    ,   num_no_ref_pos_ (num_no_ref_pos)
    ,   num_no_ref_id_  (num_no_ref_id)
    ,   num_pos_outside_(num_pos_outside)
    ,   num_pos_inside_ (num_pos_inside)
    ,   num_correct_    (num_correct)
    ,   num_not_correct_(num_not_correct)
{
    updatePCor();
}

void SingleModeCCorrect::updatePCor()
{
    assert (num_updates_ - num_no_ref_pos_ == num_pos_inside_ + num_pos_outside_);
    assert (num_pos_inside_ == num_no_ref_id_+num_correct_+num_not_correct_);

    pcor_.reset();

    if (num_correct_+num_not_correct_)
    {
        pcor_ = (float)num_correct_/(float)(num_correct_+num_not_correct_);
    }

    result_usable_ = pcor_.has_value();

    updateUseFromTarget();
}

void SingleModeCCorrect::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "SingleModeC " <<  requirement_->name() <<": addToReport";

    // add target to requirements->group->req
    addTargetToOverviewTable(root_item);

    // add requirement requirement to targets->utn->requirements->group->req
    addTargetDetailsToReport(root_item);

    // TODO add requirement description, methods
}

void SingleModeCCorrect::addTargetToOverviewTable(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    addTargetDetailsToTable(getRequirementSection(root_item), target_table_name_);

    if (eval_man_.settings().report_split_results_by_mops_
            || eval_man_.settings().report_split_results_by_aconly_ms_) // add to general sum table
        addTargetDetailsToTable(root_item->getSection(getRequirementSumSectionID()), target_table_name_);
}

void SingleModeCCorrect::addTargetDetailsToTable (
        EvaluationResultsReport::Section& section, const std::string& table_name)
{
    if (!section.hasTable(table_name))
        section.addTable(table_name, 13,
                         {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                          "#Up", "#NoRef", "#CMC", "#NCMC", "PC"}, true, 12);

    EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

    QVariant pd_var;

    if (pcor_.has_value())
        pd_var = roundf(pcor_.value() * 10000.0) / 100.0;

    target_table.addRow(
                { utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
                  target_->acidsStr().c_str(), target_->acadsStr().c_str(),
                  target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(), target_->modeCMaxStr().c_str(),
                  num_updates_, num_no_ref_pos_+num_no_ref_id_, num_correct_, num_not_correct_,
                  pd_var}, this, {utn_});
}

void SingleModeCCorrect::addTargetDetailsToReport(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    std::shared_ptr<EvaluationRequirement::ModeCCorrect> req =
            std::static_pointer_cast<EvaluationRequirement::ModeCCorrect>(requirement_);
    assert (req);

    QVariant pd_var;

    if (pcor_.has_value())
        pd_var = roundf(pcor_.value() * 10000.0) / 100.0;

    root_item->getSection(getTargetSectionID()).perTargetSection(true); // mark utn section per target
    EvaluationResultsReport::Section& utn_req_section = root_item->getSection(getTargetRequirementSectionID());

    if (!utn_req_section.hasTable("details_overview_table"))
        utn_req_section.addTable("details_overview_table", 3, {"Name", "comment", "Value"}, false);

    EvaluationResultsReport::SectionContentTable& utn_req_table =
            utn_req_section.getTable("details_overview_table");

    addCommonDetails(root_item);

    utn_req_table.addRow({"Use", "To be used in results", use_}, this);
    utn_req_table.addRow({"#Up [1]", "Number of updates", num_updates_}, this);
    utn_req_table.addRow({"#NoRef [1]", "Number of updates w/o reference position or Mode C",
                          num_no_ref_pos_+num_no_ref_id_}, this);
    utn_req_table.addRow({"#NoRefPos [1]", "Number of updates w/o reference position ", num_no_ref_pos_}, this);
    utn_req_table.addRow({"#NoRef [1]", "Number of updates w/o reference Mode C", num_no_ref_id_}, this);
    utn_req_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
    utn_req_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_pos_outside_}, this);
    utn_req_table.addRow({"Max Dist. [ft]", "Maximum offset",
                          String::doubleToStringPrecision(req->maxDistanceFt(),2).c_str()}, this);
    utn_req_table.addRow({"#CMC [1]", "Number of updates with correct Mode C", num_correct_}, this);
    utn_req_table.addRow({"#NCMC [1]", "Number of updates with no correct Mode C", num_not_correct_}, this);
    utn_req_table.addRow({"PC [%]", "Probability of correct Mode C", pd_var}, this);


    utn_req_table.addRow({"Condition", "", req->getConditionStr().c_str()}, this);

    string result {"Unknown"};

    if (pcor_.has_value())
        result = req->getConditionResultStr(pcor_.value());

    utn_req_table.addRow({"Condition Fulfilled", "", result.c_str()}, this);

    if (result == "Failed")
    {
        root_item->getSection(getTargetSectionID()).perTargetWithIssues(true); // mark utn section as with issue
        utn_req_section.perTargetWithIssues(true);
    }

    if (pcor_.has_value() && pcor_.value() != 1.0)
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

void SingleModeCCorrect::reportDetails(EvaluationResultsReport::Section& utn_req_section)
{
    if (!utn_req_section.hasTable(tr_details_table_name_))
        utn_req_section.addTable(tr_details_table_name_, 10,
                                 {"ToD", "Ref", "Ok", "#Up", "#NoRef", "#PosInside", "#PosOutside", "#CID", "#NCID", "Comment"});

    EvaluationResultsReport::SectionContentTable& utn_req_details_table =
            utn_req_section.getTable(tr_details_table_name_);

    utn_req_details_table.setCreateOnDemand(
                [this, &utn_req_details_table](void)
    {

        unsigned int detail_cnt = 0;

        for (auto& rq_det_it : getDetails())
        {
            utn_req_details_table.addRow(
                        { Time::toString(rq_det_it.timestamp()).c_str(),
                          rq_det_it.getValue(DetailKey::RefExists),
                          !rq_det_it.getValue(DetailKey::IsNotCorrect).toBool(),
                          rq_det_it.getValue(DetailKey::NumUpdates),
                          rq_det_it.getValue(DetailKey::NumNoRef),
                          rq_det_it.getValue(DetailKey::NumInside),
                          rq_det_it.getValue(DetailKey::NumOutside),
                          rq_det_it.getValue(DetailKey::NumCorrect),
                          rq_det_it.getValue(DetailKey::NumNotCorrect),
                          rq_det_it.comments().generalComment().c_str() },
                        this, detail_cnt);

            ++detail_cnt;
        }});
}

bool SingleModeCCorrect::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else if (table.name() == tr_details_table_name_ && annotation.isValid() && annotation.toUInt() < numDetails())
        return true;
    
    return false;
}

std::unique_ptr<nlohmann::json::object_t> SingleModeCCorrect::viewableData(
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

        loginf << "SingleModeC: viewableData: detail_cnt " << detail_cnt;

        std::unique_ptr<nlohmann::json::object_t> viewable_ptr
                = eval_man_.getViewableForEvaluation(utn_, req_grp_id_, result_id_);
        assert (viewable_ptr);

        const auto& detail = getDetail(detail_cnt);

        assert (detail.numPositions() >= 1);

        (*viewable_ptr)[ViewPoint::VP_POS_LAT_KEY    ] = detail.position(0).latitude_;
        (*viewable_ptr)[ViewPoint::VP_POS_LON_KEY    ] = detail.position(0).longitude_;
        (*viewable_ptr)[ViewPoint::VP_POS_WIN_LAT_KEY] = eval_man_.settings().result_detail_zoom_;
        (*viewable_ptr)[ViewPoint::VP_POS_WIN_LON_KEY] = eval_man_.settings().result_detail_zoom_;
        (*viewable_ptr)[ViewPoint::VP_TIMESTAMP_KEY  ] = Time::toString(detail.timestamp());

        //            if (!detail.pos_ok_)
        //                (*viewable_ptr)[ViewPoint::VP_EVAL_KEY][ViewPoint::VP_EVAL_HIGHDET_KEY] = vector<unsigned int>{detail_cnt};

        return viewable_ptr;
    }
    
    return nullptr;
}

std::unique_ptr<nlohmann::json::object_t> SingleModeCCorrect::getTargetErrorsViewable (bool add_highlight)
{
    std::unique_ptr<nlohmann::json::object_t> viewable_ptr = eval_man_.getViewableForEvaluation(
                utn_, req_grp_id_, result_id_);

    bool has_pos = false;
    double lat_min, lat_max, lon_min, lon_max;

    for (auto& detail_it : getDetails())
    {
        auto is_not_correct = detail_it.getValueAs<bool>(DetailKey::IsNotCorrect);
        assert(is_not_correct.has_value());

        if (!is_not_correct.value())
            continue;

        assert(detail_it.numPositions());

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

    //addAnnotationFeatures(*viewable_ptr, false, add_highlight);
    addAnnotations(*viewable_ptr, false, true);

    return viewable_ptr;
}

bool SingleModeCCorrect::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else
        return false;;
}

std::string SingleModeCCorrect::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasReference(table, annotation));

    return "Report:Results:"+getTargetRequirementSectionID();
}

void SingleModeCCorrect::addAnnotations(nlohmann::json::object_t& viewable, bool overview, bool add_ok)
{
    json& error_point_coordinates = annotationPointCoords(viewable, TypeError, overview);
    json& ok_point_coordinates    = annotationPointCoords(viewable, TypeOk, overview);

    for (auto& detail_it : getDetails())
    {
        auto is_not_correct = detail_it.getValueAsOrAssert<bool>(
                    EvaluationRequirementResult::SingleModeCCorrect::DetailKey::IsNotCorrect);

        assert (detail_it.numPositions() >= 1);

        if (is_not_correct)
            error_point_coordinates.push_back(detail_it.position(0).asVector());
        else if (add_ok)
            ok_point_coordinates.push_back(detail_it.position(0).asVector());
    }
}

std::map<std::string, std::vector<Single::LayerDefinition>> SingleModeCCorrect::gridLayers() const
{
    std::map<std::string, std::vector<Single::LayerDefinition>> layer_defs;

    layer_defs[ requirement_->name() ].push_back(getGridLayerDefBinary());

    return layer_defs;
}

std::vector<Eigen::Vector3d> SingleModeCCorrect::getGridValues(const std::string& layer) const
{
    std::vector<Eigen::Vector3d> values;

    if (layer == requirement_->name())
    {
        values = getGridValuesBinary(EvaluationRequirementResult::SingleModeCCorrect::DetailKey::IsNotCorrect, true);
    }

    return values;
}

std::shared_ptr<Joined> SingleModeCCorrect::createEmptyJoined(const std::string& result_id)
{
    return make_shared<JoinedModeCCorrect> (result_id, requirement_, sector_layer_, eval_man_);
}

unsigned int SingleModeCCorrect::numNoRefPos() const
{
    return num_no_ref_pos_;
}

unsigned int SingleModeCCorrect::numNoRefId() const
{
    return num_no_ref_id_;
}

unsigned int SingleModeCCorrect::numPosOutside() const
{
    return num_pos_outside_;
}

unsigned int SingleModeCCorrect::numPosInside() const
{
    return num_pos_inside_;
}

unsigned int SingleModeCCorrect::numUpdates() const
{
    return num_updates_;
}

unsigned int SingleModeCCorrect::numCorrect() const
{
    return num_correct_;
}

unsigned int SingleModeCCorrect::numNotCorrect() const
{
    return num_not_correct_;
}

}
