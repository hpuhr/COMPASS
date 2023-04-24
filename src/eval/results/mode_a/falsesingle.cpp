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

#include "eval/results/mode_a/falsesingle.h"
#include "eval/results/mode_a/falsejoined.h"
#include "eval/requirement/base/base.h"
#include "eval/requirement/mode_a/false.h"
#include "evaluationtargetdata.h"
#include "evaluationmanager.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "logger.h"
#include "util/stringconv.h"
#include "util/timeconv.h"

#include <cassert>

using namespace std;
using namespace Utils;
using namespace nlohmann;

namespace EvaluationRequirementResult
{

SingleModeAFalse::SingleModeAFalse(const std::string& result_id, 
                                   std::shared_ptr<EvaluationRequirement::Base> requirement,
                                   const SectorLayer& sector_layer,
                                   unsigned int utn,
                                   const EvaluationTargetData* target,
                                   EvaluationManager& eval_man,
                                   const EvaluationDetails& details,
                                   int num_updates,
                                   int num_no_ref_pos,
                                   int num_no_ref_val,
                                   int num_pos_outside,
                                   int num_pos_inside,
                                   int num_unknown,
                                   int num_correct,
                                   int num_false)
    :   SingleFalseBase("SingleModeAFalse", result_id, requirement, sector_layer, utn, target, eval_man, details,
                        num_updates, num_no_ref_pos, num_no_ref_val, num_pos_outside, num_pos_inside, num_unknown, num_correct, num_false)
{
    updateProbabilities();
}

void SingleModeAFalse::updateProbabilities()
{
    assert (num_updates_ - num_no_ref_pos_ == num_pos_inside_ + num_pos_outside_);
    assert (num_pos_inside_ == num_no_ref_val_+num_unknown_+num_correct_+num_false_);

    p_false_.reset();

    if (num_correct_ + num_false_)
    {
        p_false_ = (float)(num_false_)/(float)(num_correct_+num_false_);
    }
    
    result_usable_ = p_false_.has_value();

    updateUseFromTarget();
}

void SingleModeAFalse::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "SingleModeA " <<  requirement_->name() <<": addToReport";

    // add target to requirements->group->req
    addTargetToOverviewTable(root_item);

    // add requirement requirement to targets->utn->requirements->group->req
    addTargetDetailsToReport(root_item);

    // TODO add requirement description, methods
}

void SingleModeAFalse::addTargetToOverviewTable(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    addTargetDetailsToTable(getRequirementSection(root_item), target_table_name_);

    if (eval_man_.reportSplitResultsByMOPS()
            || eval_man_.reportSplitResultsByACOnlyMS()) // add to general sum table
        addTargetDetailsToTable(root_item->getSection(getRequirementSumSectionID()), target_table_name_);
}

void SingleModeAFalse::addTargetDetailsToTable (
        EvaluationResultsReport::Section& section, const std::string& table_name)
{
    if (!section.hasTable(table_name))
        section.addTable(table_name, 14,
                         {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                          "#Up", "#NoRef", "#Unknown", "#Correct", "#False", "PF"}, true, 13, Qt::DescendingOrder);

    EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

    QVariant pf_var;

    if (p_false_.has_value())
        pf_var = roundf(p_false_.value() * 10000.0) / 100.0;

    target_table.addRow(
                {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
                 target_->acidsStr().c_str(), target_->acadsStr().c_str(),
                 target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(), target_->modeCMaxStr().c_str(),
                 num_updates_, num_no_ref_pos_+num_no_ref_val_, num_unknown_, num_correct_, num_false_, pf_var},
                this, {utn_});
}

void SingleModeAFalse::addTargetDetailsToReport(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    root_item->getSection(getTargetSectionID()).perTargetSection(true); // mark utn section per target
    EvaluationResultsReport::Section& utn_req_section = root_item->getSection(getTargetRequirementSectionID());

    if (!utn_req_section.hasTable("details_overview_table"))
        utn_req_section.addTable("details_overview_table", 3, {"Name", "comment", "Value"}, false);

    EvaluationResultsReport::SectionContentTable& utn_req_table =
            utn_req_section.getTable("details_overview_table");

    addCommonDetails(root_item);

    utn_req_table.addRow({"Use", "To be used in results", use_}, this);
    utn_req_table.addRow({"#Up [1]", "Number of updates", num_updates_}, this);
    utn_req_table.addRow({"#NoRef [1]", "Number of updates w/o reference position or code",
                          num_no_ref_pos_+num_no_ref_val_}, this);
    utn_req_table.addRow({"#NoRefPos [1]", "Number of updates w/o reference position ", num_no_ref_pos_}, this);
    utn_req_table.addRow({"#NoRef [1]", "Number of updates w/o reference code", num_no_ref_val_}, this);
    utn_req_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
    utn_req_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_pos_outside_}, this);
    utn_req_table.addRow({"#Unknown [1]", "Number of updates unknown code", num_unknown_}, this);
    utn_req_table.addRow({"#Correct [1]", "Number of updates with correct code", num_correct_}, this);
    utn_req_table.addRow({"#False [1]", "Number of updates with false code", num_false_}, this);

    // condition
    {
        std::shared_ptr<EvaluationRequirement::ModeAFalse> req =
                std::static_pointer_cast<EvaluationRequirement::ModeAFalse>(requirement_);
        assert (req);

        QVariant pf_var;

        if (p_false_.has_value())
            pf_var = roundf(p_false_.value() * 10000.0) / 100.0;

        utn_req_table.addRow({"PF [%]", "Probability of Mode 3/A false", pf_var}, this);

        utn_req_table.addRow({"Condition", "", req->getConditionStr().c_str()}, this);

        string result {"Unknown"};

        if (p_false_.has_value())
            result = req->getResultConditionStr(p_false_.value());

        utn_req_table.addRow({"Condition Fulfilled", "", result.c_str()}, this);

        if (result == "Failed")
        {
            root_item->getSection(getTargetSectionID()).perTargetWithIssues(true); // mark utn section as with issue
            utn_req_section.perTargetWithIssues(true);
        }

    }

    if (p_false_.has_value() && p_false_.value() != 0.0)
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

void SingleModeAFalse::reportDetails(EvaluationResultsReport::Section& utn_req_section)
{
    if (!utn_req_section.hasTable(tr_details_table_name_))
        utn_req_section.addTable(tr_details_table_name_, 11,
                                 {"ToD", "Ref", "Ok", "#Up", "#NoRef", "#PosInside", "#PosOutside", "#Unknown",
                                  "#Correct", "#False", "Comment"});

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
                          !rq_det_it.getValue(DetailKey::IsNotOk).toBool(),
                          rq_det_it.getValue(DetailKey::NumUpdates),
                          rq_det_it.getValue(DetailKey::NumNoRef),
                          rq_det_it.getValue(DetailKey::NumInside),
                          rq_det_it.getValue(DetailKey::NumOutside),
                          rq_det_it.getValue(DetailKey::NumUnknownID),
                          rq_det_it.getValue(DetailKey::NumCorrectID),
                          rq_det_it.getValue(DetailKey::NumFalseID),
                          rq_det_it.comments().generalComment().c_str()},
                        this, detail_cnt);

            ++detail_cnt;
        }});
}


bool SingleModeAFalse::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else if (table.name() == tr_details_table_name_ && annotation.isValid() && annotation.toUInt() < numDetails())
        return true;
    else
        return false;
}

std::unique_ptr<nlohmann::json::object_t> SingleModeAFalse::viewableData(
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

        loginf << "SingleModeA: viewableData: detail_cnt " << detail_cnt;

        std::unique_ptr<nlohmann::json::object_t> viewable_ptr
                = eval_man_.getViewableForEvaluation(utn_, req_grp_id_, result_id_);
        assert (viewable_ptr);

        const auto& detail = getDetail(detail_cnt);

        assert(detail.numPositions() >= 1);

        (*viewable_ptr)[VP_POS_LAT_KEY] = detail.position(0).latitude_;
        (*viewable_ptr)[VP_POS_LON_KEY] = detail.position(0).longitude_;
        (*viewable_ptr)[VP_POS_WIN_LAT_KEY] = eval_man_.resultDetailZoom();
        (*viewable_ptr)[VP_POS_WIN_LON_KEY] = eval_man_.resultDetailZoom();
        (*viewable_ptr)[VP_TIMESTAMP_KEY] = Time::toString(detail.timestamp());

        //            if (!detail.pos_ok_)
        //                (*viewable_ptr)[VP_EVAL_KEY][VP_EVAL_HIGHDET_KEY] = vector<unsigned int>{detail_cnt};

        return viewable_ptr;
    }
    else
        return nullptr;
}

std::unique_ptr<nlohmann::json::object_t> SingleModeAFalse::getTargetErrorsViewable ()
{
    std::unique_ptr<nlohmann::json::object_t> viewable_ptr = eval_man_.getViewableForEvaluation(
                utn_, req_grp_id_, result_id_);

    bool has_pos = false;
    double lat_min, lat_max, lon_min, lon_max;

    for (auto& detail_it : getDetails())
    {
        assert(detail_it.numPositions() >= 1);

        auto is_not_ok = detail_it.getValueAs<bool>(DetailKey::IsNotOk);
        assert(is_not_ok.has_value());

        if (!is_not_ok.value())
            continue;

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

    addAnnotations(*viewable_ptr);

    return viewable_ptr;
}

bool SingleModeAFalse::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else
        return false;;
}

std::string SingleModeAFalse::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasReference(table, annotation));

    return "Report:Results:"+getTargetRequirementSectionID();
}

void SingleModeAFalse::addAnnotations(nlohmann::json::object_t& viewable)
{
    addAnnotationFeatures(viewable);

    json& error_point_coordinates =
            viewable.at("annotations").at(0).at("features").at(1).at("geometry").at("coordinates");
    json& ok_point_coordinates =
            viewable.at("annotations").at(1).at("features").at(1).at("geometry").at("coordinates");

    for (auto& detail_it : getDetails())
    {
        auto is_not_ok = detail_it.getValueAsOrAssert<bool>(
                    EvaluationRequirementResult::SingleModeAFalse::DetailKey::IsNotOk);

        assert (detail_it.numPositions() >= 1);

        if (is_not_ok)
            error_point_coordinates.push_back(detail_it.position(0).asVector());
        else
            ok_point_coordinates.push_back(detail_it.position(0).asVector());
    }
}


std::shared_ptr<Joined> SingleModeAFalse::createEmptyJoined(const std::string& result_id)
{
    return make_shared<JoinedModeAFalse> (result_id, requirement_, sector_layer_, eval_man_);
}

}
