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

#include "eval/results/identification/correctsingle.h"
#include "eval/results/identification/correctjoined.h"
#include "eval/requirement/base/base.h"
#include "eval/requirement/identification/correct.h"
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

const std::string SingleIdentificationCorrect::DetailRefExists     = "RefExists";
const std::string SingleIdentificationCorrect::DetailPosInside     = "PosInside";
const std::string SingleIdentificationCorrect::DetailIsNotCorrect  = "IsNotCorrect";
const std::string SingleIdentificationCorrect::DetailNumUpdates    = "NumUpdates";
const std::string SingleIdentificationCorrect::DetailNumNoRef      = "NumNoRef";
const std::string SingleIdentificationCorrect::DetailNumInside     = "NumInside";
const std::string SingleIdentificationCorrect::DetailNumOutside    = "NumOutside";
const std::string SingleIdentificationCorrect::DetailNumCorrect    = "NumCorrect";
const std::string SingleIdentificationCorrect::DetailNumNotCorrect = "NumNotCorrect";

SingleIdentificationCorrect::SingleIdentificationCorrect(const std::string& result_id, 
                                                         std::shared_ptr<EvaluationRequirement::Base> requirement,
                                                         const SectorLayer& sector_layer,
                                                         unsigned int utn, 
                                                         const EvaluationTargetData* target, 
                                                         EvaluationManager& eval_man,
                                                         const boost::optional<EvaluationDetails>& details,
                                                         unsigned int num_updates, 
                                                         unsigned int num_no_ref_pos, 
                                                         unsigned int num_no_ref_id,
                                                         unsigned int num_pos_outside, 
                                                         unsigned int num_pos_inside,
                                                         unsigned int num_correct, 
                                                         unsigned int num_not_correct)
:   Single("SingleIdentificationCorrect", result_id, requirement, sector_layer, utn, target, eval_man, details)
,   num_updates_    (num_updates)
,   num_no_ref_pos_ (num_no_ref_pos)
,   num_no_ref_id_  (num_no_ref_id)
,   num_pos_outside_(num_pos_outside)
,   num_pos_inside_ (num_pos_inside)
,   num_correct_    (num_correct)
,   num_not_correct_(num_not_correct)
{
    updatePID();
}

void SingleIdentificationCorrect::updatePID()
{
    assert (num_updates_ - num_no_ref_pos_ == num_pos_inside_ + num_pos_outside_);
    assert (num_pos_inside_ == num_no_ref_id_+num_correct_+num_not_correct_);

    pid_.reset();

    if (num_correct_+num_not_correct_)
    {
        pid_ = (float)num_correct_/(float)(num_correct_+num_not_correct_);
    }

    result_usable_ = pid_.has_value();

    updateUseFromTarget();
}

void SingleIdentificationCorrect::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "SingleIdentification " <<  requirement_->name() <<": addToReport";

    // add target to requirements->group->req
    addTargetToOverviewTable(root_item);

    // add requirement requirement to targets->utn->requirements->group->req
    addTargetDetailsToReport(root_item);

    // TODO add requirement description, methods
}

void SingleIdentificationCorrect::addTargetToOverviewTable(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    addTargetDetailsToTable(getRequirementSection(root_item), target_table_name_);

    if (eval_man_.reportSplitResultsByMOPS()) // add to general sum table
        addTargetDetailsToTable(root_item->getSection(getRequirementSumSectionID()), target_table_name_);
}

void SingleIdentificationCorrect::addTargetDetailsToTable (
        EvaluationResultsReport::Section& section, const std::string& table_name)
{
    if (!section.hasTable(table_name))
        section.addTable(table_name, 13,
                         {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                          "#Up", "#NoRef", "#CID", "#NCID", "PID"}, true, 12);

    EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

    QVariant pd_var;

    if (pid_.has_value())
        pd_var = roundf(pid_.value() * 10000.0) / 100.0;

    target_table.addRow(
                { utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
                 target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
                 target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(), target_->modeCMaxStr().c_str(),
                 num_updates_, num_no_ref_pos_+num_no_ref_id_, num_correct_, num_not_correct_,
                 pd_var}, this, {utn_});
}

void SingleIdentificationCorrect::addTargetDetailsToReport(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    QVariant pd_var;

    if (pid_.has_value())
        pd_var = roundf(pid_.value() * 10000.0) / 100.0;

    root_item->getSection(getTargetSectionID()).perTargetSection(true); // mark utn section per target
    EvaluationResultsReport::Section& utn_req_section = root_item->getSection(getTargetRequirementSectionID());

    if (!utn_req_section.hasTable("details_overview_table"))
        utn_req_section.addTable("details_overview_table", 3, {"Name", "comment", "Value"}, false);

    EvaluationResultsReport::SectionContentTable& utn_req_table =
            utn_req_section.getTable("details_overview_table");

    addCommonDetails(root_item);

    utn_req_table.addRow({"Use", "To be used in results", use_}, this);
    utn_req_table.addRow({"#Up [1]", "Number of updates", num_updates_}, this);
    utn_req_table.addRow({"#NoRef [1]", "Number of updates w/o reference position or identification",
                          num_no_ref_pos_+num_no_ref_id_}, this);
    utn_req_table.addRow({"#NoRefPos [1]", "Number of updates w/o reference position ", num_no_ref_pos_}, this);
    utn_req_table.addRow({"#NoRef [1]", "Number of updates w/o reference identification", num_no_ref_id_}, this);
    utn_req_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
    utn_req_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_pos_outside_}, this);
    utn_req_table.addRow({"#CID [1]", "Number of updates with correct identification", num_correct_}, this);
    utn_req_table.addRow({"#NCID [1]", "Number of updates with no correct identification", num_not_correct_}, this);
    utn_req_table.addRow({"POK [%]", "Probability of correct identification", pd_var}, this);

    // condition
    std::shared_ptr<EvaluationRequirement::IdentificationCorrect> req =
            std::static_pointer_cast<EvaluationRequirement::IdentificationCorrect>(requirement_);
    assert (req);

    utn_req_table.addRow({"Condition", "", req->getConditionStr().c_str()}, this);

    string result {"Unknown"};

    if (pid_.has_value())
        result = req-> getResultConditionStr(pid_.value());

    utn_req_table.addRow({"Condition Fulfilled", "", result.c_str()}, this);

    if (result == "Failed")
    {
        root_item->getSection(getTargetSectionID()).perTargetWithIssues(true); // mark utn section as with issue
        utn_req_section.perTargetWithIssues(true);
    }

    if (pid_.has_value() && pid_.value() != 1.0)
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

void SingleIdentificationCorrect::reportDetails(EvaluationResultsReport::Section& utn_req_section)
{
    if (!utn_req_section.hasTable(tr_details_table_name_))
        utn_req_section.addTable(tr_details_table_name_, 10,
                                 {"ToD", "Ref", "Ok", "#Up", "#NoRef", "#PosInside", "#PosOutside", "#CID", "#NCID", "Comment"});

    EvaluationResultsReport::SectionContentTable& utn_req_details_table =
            utn_req_section.getTable(tr_details_table_name_);

    unsigned int detail_cnt = 0;

    for (auto& rq_det_it : getDetails())
    {
        utn_req_details_table.addRow(
                    { Time::toString(rq_det_it.timestamp()).c_str(), 
                      rq_det_it.getValue(DetailRefExists),
                     !rq_det_it.getValue(DetailIsNotCorrect).toBool(),
                      rq_det_it.getValue(DetailNumUpdates), 
                      rq_det_it.getValue(DetailNumNoRef),
                      rq_det_it.getValue(DetailNumInside), 
                      rq_det_it.getValue(DetailNumOutside),
                      rq_det_it.getValue(DetailNumCorrect), 
                      rq_det_it.getValue(DetailNumNotCorrect), 
                      rq_det_it.comments().generalComment().c_str() },
                    this, detail_cnt);

        ++detail_cnt;
    }
}

bool SingleIdentificationCorrect::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else if (table.name() == tr_details_table_name_ && annotation.isValid() && annotation.toUInt() < numDetails())
        return true;
    
    return false;
}

std::unique_ptr<nlohmann::json::object_t> SingleIdentificationCorrect::viewableData(
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

        loginf << "SingleIdentification: viewableData: detail_cnt " << detail_cnt;

        std::unique_ptr<nlohmann::json::object_t> viewable_ptr
                = eval_man_.getViewableForEvaluation(utn_, req_grp_id_, result_id_);
        assert (viewable_ptr);

        const auto& detail = getDetail(detail_cnt);

        assert (detail.numPositions() >= 1);

        (*viewable_ptr)[VP_POS_LAT_KEY    ] = detail.position(0).latitude_;
        (*viewable_ptr)[VP_POS_LON_KEY    ] = detail.position(0).longitude_;
        (*viewable_ptr)[VP_POS_WIN_LAT_KEY] = eval_man_.resultDetailZoom();
        (*viewable_ptr)[VP_POS_WIN_LON_KEY] = eval_man_.resultDetailZoom();
        (*viewable_ptr)[VP_TIMESTAMP_KEY  ] = Time::toString(detail.timestamp());

        //            if (!detail.pos_ok_)
        //                (*viewable_ptr)[VP_EVAL_KEY][VP_EVAL_HIGHDET_KEY] = vector<unsigned int>{detail_cnt};

        return viewable_ptr;
    }
    
    return nullptr;
}

std::unique_ptr<nlohmann::json::object_t> SingleIdentificationCorrect::getTargetErrorsViewable ()
{
    std::unique_ptr<nlohmann::json::object_t> viewable_ptr = eval_man_.getViewableForEvaluation(
                utn_, req_grp_id_, result_id_);

    bool has_pos = false;
    double lat_min, lat_max, lon_min, lon_max;

    for (auto& detail_it : getDetails())
    {
        auto is_not_correct = detail_it.getValueAs<bool>(DetailIsNotCorrect);
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

    return viewable_ptr;
}

bool SingleIdentificationCorrect::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else
        return false;;
}

std::string SingleIdentificationCorrect::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasReference(table, annotation));

    return "Report:Results:"+getTargetRequirementSectionID();
}

std::shared_ptr<Joined> SingleIdentificationCorrect::createEmptyJoined(const std::string& result_id)
{
    return make_shared<JoinedIdentificationCorrect> (result_id, requirement_, sector_layer_, eval_man_);
}

unsigned int SingleIdentificationCorrect::numNoRefPos() const
{
    return num_no_ref_pos_;
}

unsigned int SingleIdentificationCorrect::numNoRefId() const
{
    return num_no_ref_id_;
}

unsigned int SingleIdentificationCorrect::numPosOutside() const
{
    return num_pos_outside_;
}

unsigned int SingleIdentificationCorrect::numPosInside() const
{
    return num_pos_inside_;
}

unsigned int SingleIdentificationCorrect::numUpdates() const
{
    return num_updates_;
}

unsigned int SingleIdentificationCorrect::numCorrect() const
{
    return num_correct_;
}

unsigned int SingleIdentificationCorrect::numNotCorrect() const
{
    return num_not_correct_;
}

}
