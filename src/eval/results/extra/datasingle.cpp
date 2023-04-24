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

#include "eval/results/extra/datasingle.h"
#include "eval/results/extra/datajoined.h"
#include "eval/requirement/base/base.h"
#include "eval/requirement/extra/data.h"
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

SingleExtraData::SingleExtraData(const std::string& result_id, 
                                 std::shared_ptr<EvaluationRequirement::Base> requirement,
                                 const SectorLayer& sector_layer,
                                 unsigned int utn,
                                 const EvaluationTargetData* target,
                                 EvaluationManager& eval_man,
                                 const EvaluationDetails& details,
                                 bool ignore,
                                 unsigned int num_extra,
                                 unsigned int num_ok,
                                 bool has_extra_test_data)
    :   Single("SingleExtraData", result_id, requirement, sector_layer, utn, target, eval_man, details)
    ,   ignore_             (ignore)
    ,   num_extra_          (num_extra)
    ,   num_ok_             (num_ok)
    ,   has_extra_test_data_(has_extra_test_data)
{
    updateProb();
}

void SingleExtraData::updateProb()
{
    prob_.reset();

    if (num_extra_ + num_ok_)
    {
        logdbg << "SingleExtraData: updateProb: result_id " << result_id_ << " num_extra " << num_extra_
               << " num_ok " << num_ok_;

        prob_          = (float)num_extra_/(float)(num_extra_ + num_ok_);
        result_usable_ = !ignore_;
    }

    if (!prob_.has_value())
        result_usable_ = false;

    updateUseFromTarget();
}

void SingleExtraData::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "ExtraData " <<  requirement_->name() <<": addToReport";

    // add target to requirements->group->req
    addTargetToOverviewTable(root_item);

    // add requirement results to targets->utn->requirements->group->req
    addTargetDetailsToReport(root_item);

    // TODO add requirement description, methods
}

void SingleExtraData::addTargetToOverviewTable(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    addTargetDetailsToTable(getRequirementSection(root_item), target_table_name_);

    if (eval_man_.reportSplitResultsByMOPS()
            || eval_man_.reportSplitResultsByACOnlyMS()) // add to general sum table
        addTargetDetailsToTable(root_item->getSection(getRequirementSumSectionID()), target_table_name_);
}

void SingleExtraData::addTargetDetailsToTable (
        EvaluationResultsReport::Section& section, const std::string& table_name)
{
    QVariant prob_var;

    if (prob_.has_value())
        prob_var = roundf(prob_.value() * 10000.0) / 100.0;

    if (!section.hasTable(table_name))
        section.addTable(table_name, 14,
                         {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                          "#Tst", "Ign.", "#Check", "#OK", "#Extra", "PEx"}, true, 13,
                         Qt::DescendingOrder);

    EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

    target_table.addRow(
                {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
                 target_->acidsStr().c_str(), target_->acadsStr().c_str(),
                 target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(),
                 target_->modeCMaxStr().c_str(), target_->numTstUpdates(),
                 ignore_, num_extra_+num_ok_, num_ok_, num_extra_, prob_var}, this, {utn_});
}

void SingleExtraData::addTargetDetailsToReport(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    root_item->getSection(getTargetSectionID()).perTargetSection(true); // mark utn section per target
    EvaluationResultsReport::Section& utn_req_section = root_item->getSection(getTargetRequirementSectionID());

    if (!utn_req_section.hasTable("details_overview_table"))
        utn_req_section.addTable("details_overview_table", 3, {"Name", "comment", "Value"}, false);

    std::shared_ptr<EvaluationRequirement::ExtraData> req =
            std::static_pointer_cast<EvaluationRequirement::ExtraData>(requirement_);
    assert (req);

    EvaluationResultsReport::SectionContentTable& utn_req_table =
            utn_req_section.getTable("details_overview_table");

    addCommonDetails(root_item);

    utn_req_table.addRow({"Use", "To be used in results", use_}, this);
    utn_req_table.addRow({"#Ref [1]", "Number of reference updates", target_->numRefUpdates()}, this);
    utn_req_table.addRow({"#Tst [1]", "Number of test updates",  target_->numTstUpdates()}, this);
    utn_req_table.addRow({"Ign.", "Ignore target", ignore_}, this);
    utn_req_table.addRow({"#Check.", "Number of checked test updates", num_extra_+num_ok_}, this);
    utn_req_table.addRow({"#OK.", "Number of OK test updates", num_ok_}, this);
    utn_req_table.addRow({"#Extra", "Number of extra test updates", num_extra_}, this);

    // condition
    {
        QVariant prob_var;

        if (prob_.has_value())
            prob_var = roundf(prob_.value() * 10000.0) / 100.0;

        utn_req_table.addRow({"PEx [%]", "Probability of update with extra data", prob_var}, this);

        utn_req_table.addRow({"Condition", {}, req->getConditionStr().c_str()}, this);

        string result {"Unknown"};

        if (prob_.has_value())
            result = req->getConditionResultStr(prob_.value());

        utn_req_table.addRow({"Condition Fulfilled", "", result.c_str()}, this);

        if (result == "Failed")
        {
            root_item->getSection(getTargetSectionID()).perTargetWithIssues(true); // mark utn section as with issue
            utn_req_section.perTargetWithIssues(true);
        }
    }

    // add figure
    if (!ignore_ && num_extra_)
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

void SingleExtraData::reportDetails(EvaluationResultsReport::Section& utn_req_section)
{
    if (!utn_req_section.hasTable(tr_details_table_name_))
        utn_req_section.addTable(tr_details_table_name_, 5,
                                 {"ToD", "Inside", "Extra", "Ref.", "Comment"});

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
                          rq_det_it.getValue(DetailKey::Inside),
                          rq_det_it.getValue(DetailKey::Extra),
                          rq_det_it.getValue(DetailKey::RefExists),
                          rq_det_it.comments().generalComment().c_str() },
                        this, detail_cnt);

            ++detail_cnt;
        }});
}

bool SingleExtraData::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else if (table.name() == tr_details_table_name_ && annotation.isValid() && annotation.toUInt() < numDetails())
        return true;
    else
        return false;
}

std::unique_ptr<nlohmann::json::object_t> SingleExtraData::viewableData(
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

        std::unique_ptr<nlohmann::json::object_t> viewable_ptr
                = eval_man_.getViewableForEvaluation(utn_, req_grp_id_, result_id_);
        assert (viewable_ptr);

        const auto& detail = getDetail(detail_cnt);

        assert(detail.numPositions() > 0);

        (*viewable_ptr)[VP_POS_LAT_KEY    ] = detail.position(0).latitude_;
        (*viewable_ptr)[VP_POS_LON_KEY    ] = detail.position(0).longitude_;
        (*viewable_ptr)[VP_POS_WIN_LAT_KEY] = eval_man_.resultDetailZoom();
        (*viewable_ptr)[VP_POS_WIN_LON_KEY] = eval_man_.resultDetailZoom();
        (*viewable_ptr)[VP_TIMESTAMP_KEY  ] = Time::toString(detail.timestamp());

        return viewable_ptr;
    }
    else
        return nullptr;
}

std::unique_ptr<nlohmann::json::object_t> SingleExtraData::getTargetErrorsViewable ()
{
    std::unique_ptr<nlohmann::json::object_t> viewable_ptr = eval_man_.getViewableForEvaluation(
                utn_, req_grp_id_, result_id_);

    //    bool has_pos = false;
    //    double lat_min, lat_max, lon_min, lon_max;

    //    for (auto& detail_it : details_)
    //    {
    //        if (!detail_it.miss_occurred_)
    //            continue;

    //        if (has_pos)
    //        {
    //            lat_min = min(lat_min, detail_it.pos_current_.latitude_);
    //            lat_max = max(lat_max, detail_it.pos_current_.latitude_);

    //            lon_min = min(lon_min, detail_it.pos_current_.longitude_);
    //            lon_max = max(lon_max, detail_it.pos_current_.longitude_);
    //        }
    //        else // tst pos always set
    //        {
    //            lat_min = detail_it.pos_current_.latitude_;
    //            lat_max = detail_it.pos_current_.latitude_;

    //            lon_min = detail_it.pos_current_.longitude_;
    //            lon_max = detail_it.pos_current_.longitude_;

    //            has_pos = true;
    //        }

    //        if (detail_it.has_last_position_)
    //        {
    //            lat_min = min(lat_min, detail_it.pos_last.latitude_);
    //            lat_max = max(lat_max, detail_it.pos_last.latitude_);

    //            lon_min = min(lon_min, detail_it.pos_last.longitude_);
    //            lon_max = max(lon_max, detail_it.pos_last.longitude_);
    //        }
    //    }

    //    if (has_pos)
    //    {
    //        (*viewable_ptr)[VP_POS_LAT_KEY] = (lat_max+lat_min)/2.0;
    //        (*viewable_ptr)[VP_POS_LON_KEY] = (lon_max+lon_min)/2.0;;

    //        double lat_w = 1.1*(lat_max-lat_min)/2.0;
    //        double lon_w = 1.1*(lon_max-lon_min)/2.0;

    //        if (lat_w < eval_man_.resultDetailZoom())
    //            lat_w = eval_man_.resultDetailZoom();

    //        if (lon_w < eval_man_.resultDetailZoom())
    //            lon_w = eval_man_.resultDetailZoom();

    //        (*viewable_ptr)[VP_POS_WIN_LAT_KEY] = lat_w;
    //        (*viewable_ptr)[VP_POS_WIN_LON_KEY] = lon_w;
    //    }

    addAnnotations(*viewable_ptr);

    return viewable_ptr;
}

bool SingleExtraData::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else
        return false;;
}

std::string SingleExtraData::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasReference(table, annotation));

    return "Report:Results:" + getTargetRequirementSectionID();
}

std::shared_ptr<Joined> SingleExtraData::createEmptyJoined(const std::string& result_id)
{
    return make_shared<JoinedExtraData> (result_id, requirement_, sector_layer_, eval_man_);
}

bool SingleExtraData::ignore() const
{
    return ignore_;
}

bool SingleExtraData::hasExtraTestData() const
{
    return has_extra_test_data_;
}

unsigned int SingleExtraData::numExtra() const
{
    return num_extra_;
}

unsigned int SingleExtraData::numOK() const
{
    return num_ok_;
}

void SingleExtraData::addAnnotations(nlohmann::json::object_t& viewable)
{
    addAnnotationFeatures(viewable);

    json& error_line_coordinates =
            viewable.at("annotations").at(0).at("features").at(0).at("geometry").at("coordinates");
    json& error_point_coordinates =
            viewable.at("annotations").at(0).at("features").at(1).at("geometry").at("coordinates");
    json& ok_line_coordinates =
            viewable.at("annotations").at(1).at("features").at(0).at("geometry").at("coordinates");
    json& ok_point_coordinates =
            viewable.at("annotations").at(1).at("features").at(1).at("geometry").at("coordinates");
}

}
