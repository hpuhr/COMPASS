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

SingleModeCFalse::SingleModeCFalse(
        const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
        const SectorLayer& sector_layer,
        unsigned int utn, const EvaluationTargetData* target, EvaluationManager& eval_man,
        int num_updates, int num_no_ref_pos, int num_no_ref_val, int num_pos_outside, int num_pos_inside,
        int num_unknown, int num_correct, int num_false,
        std::vector<EvaluationRequirement::CheckDetail> details)
    : Single("SingleModeCFalse", result_id, requirement, sector_layer, utn, target, eval_man),
      num_updates_(num_updates), num_no_ref_pos_(num_no_ref_pos), num_no_ref_val_(num_no_ref_val),
      num_pos_outside_(num_pos_outside), num_pos_inside_(num_pos_inside),
      num_unknown_(num_unknown),
      num_correct_(num_correct), num_false_(num_false), details_(details)
{
    updateProbabilities();
}

void SingleModeCFalse::updateProbabilities()
{
    assert (num_updates_ - num_no_ref_pos_ == num_pos_inside_ + num_pos_outside_);
    assert (num_pos_inside_ == num_no_ref_val_+num_unknown_+num_correct_+num_false_);

    if (num_correct_+num_false_)
    {
        p_false_ = (float)(num_false_)/(float)(num_correct_+num_false_);
        has_p_false_ = true;

        result_usable_ = true;
    }
    else
    {
        has_p_false_ = false;
        p_false_ = 0;

        result_usable_ = false;
    }

    updateUseFromTarget();
}

void SingleModeCFalse::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "SingleModeC " <<  requirement_->name() <<": addToReport";

    // add target to requirements->group->req
    addTargetToOverviewTable(root_item);

    // add requirement requirement to targets->utn->requirements->group->req
    addTargetDetailsToReport(root_item);

    // TODO add requirement description, methods
}

void SingleModeCFalse::addTargetToOverviewTable(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    addTargetDetailsToTable(getRequirementSection(root_item), target_table_name_);

    if (eval_man_.reportSplitResultsByMOPS()) // add to general sum table
        addTargetDetailsToTable(root_item->getSection(getRequirementSumSectionID()), target_table_name_);

}

void SingleModeCFalse::addTargetDetailsToTable (
        EvaluationResultsReport::Section& section, const std::string& table_name)
{
    if (!section.hasTable(table_name))
        section.addTable(table_name, 14,
                         {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                          "#Up", "#NoRef", "#Unknown", "#Correct", "#False", "PF"}, true, 13, Qt::DescendingOrder);

    EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

    QVariant pf_var;

    if (has_p_false_)
        pf_var = roundf(p_false_ * 10000.0) / 100.0;

    target_table.addRow(
                {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
                 target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
                 target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(), target_->modeCMaxStr().c_str(),
                 num_updates_, num_no_ref_pos_+num_no_ref_val_, num_unknown_, num_correct_, num_false_,
                 pf_var}, this, {utn_});
}

//    void SingleModeC::addTargetDetailsToTableADSB (EvaluationResultsReport::SectionContentTable& target_table)
//    {
//        QVariant pd_var;

//        if (has_pid_)
//            pd_var = roundf(pid_ * 10000.0) / 100.0;

//        string utn_req_section_heading = getTargetRequirementSectionID();

//        target_table.addRow(
//        {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
//         target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
//         target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(), target_->modeCMaxStr().c_str(),
//         num_updates_, num_no_ref_pos_+num_no_ref_id_, num_unknown_id_, num_correct_id_, num_false_id_,
//         pd_var}, this, {utn_});
//    }

void SingleModeCFalse::addTargetDetailsToReport(shared_ptr<EvaluationResultsReport::RootItem> root_item)
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
        std::shared_ptr<EvaluationRequirement::ModeCFalse> req =
                std::static_pointer_cast<EvaluationRequirement::ModeCFalse>(requirement_);
        assert (req);

        QVariant pf_var;

        if (has_p_false_)
            pf_var = roundf(p_false_ * 10000.0) / 100.0;

        utn_req_table.addRow({"PF [%]", "Probability of Mode C false", pf_var}, this);

        utn_req_table.addRow(
                    {"Condition", "", req->getConditionStr().c_str()}, this);

        string result {"Unknown"};

        if (has_p_false_)
            result = req-> getResultConditionStr(p_false_);

        utn_req_table.addRow({"Condition Fulfilled", "", result.c_str()}, this);

        if (result == "Failed")
        {
            root_item->getSection(getTargetSectionID()).perTargetWithIssues(true); // mark utn section as with issue
            utn_req_section.perTargetWithIssues(true);
        }

    }

    if (has_p_false_ && p_false_ != 0.0)
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

void SingleModeCFalse::reportDetails(EvaluationResultsReport::Section& utn_req_section)
{
    if (!utn_req_section.hasTable(tr_details_table_name_))
        utn_req_section.addTable(tr_details_table_name_, 11,
                                 {"ToD", "Ref", "Ok", "#Up", "#NoRef", "#PosInside", "#PosOutside", "#Unknwon",
                                  "#Correct", "#False", "Comment"});

    EvaluationResultsReport::SectionContentTable& utn_req_details_table =
            utn_req_section.getTable(tr_details_table_name_);

    unsigned int detail_cnt = 0;

    for (auto& rq_det_it : details_)
    {
        utn_req_details_table.addRow(
                    {String::timeStringFromDouble(rq_det_it.tod_).c_str(), rq_det_it.ref_exists_,
                     !rq_det_it.is_not_ok_,
                     rq_det_it.num_updates_, rq_det_it.num_no_ref_,
                     rq_det_it.num_inside_, rq_det_it.num_outside_, rq_det_it.num_unknown_id_,
                     rq_det_it.num_correct_id_, rq_det_it.num_false_id_, rq_det_it.comment_.c_str()},
                    this, detail_cnt);

        ++detail_cnt;
    }
}


bool SingleModeCFalse::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else if (table.name() == tr_details_table_name_ && annotation.isValid() && annotation.toUInt() < details_.size())
        return true;
    else
        return false;
}

std::unique_ptr<nlohmann::json::object_t> SingleModeCFalse::viewableData(
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

        const EvaluationRequirement::CheckDetail& detail = details_.at(detail_cnt);

        (*viewable_ptr)["position_latitude"] = detail.pos_tst_.latitude_;
        (*viewable_ptr)["position_longitude"] = detail.pos_tst_.longitude_;
        (*viewable_ptr)["position_window_latitude"] = eval_man_.resultDetailZoom();
        (*viewable_ptr)["position_window_longitude"] = eval_man_.resultDetailZoom();
        (*viewable_ptr)["time"] = detail.tod_;

        //            if (!detail.pos_ok_)
        //                (*viewable_ptr)["evaluation_results"]["highlight_details"] = vector<unsigned int>{detail_cnt};

        return viewable_ptr;
    }
    else
        return nullptr;
}

std::unique_ptr<nlohmann::json::object_t> SingleModeCFalse::getTargetErrorsViewable ()
{
    std::unique_ptr<nlohmann::json::object_t> viewable_ptr = eval_man_.getViewableForEvaluation(
                utn_, req_grp_id_, result_id_);

    bool has_pos = false;
    double lat_min, lat_max, lon_min, lon_max;

    for (auto& detail_it : details_)
    {
        if (!detail_it.is_not_ok_)
            continue;

        if (has_pos)
        {
            lat_min = min(lat_min, detail_it.pos_tst_.latitude_);
            lat_max = max(lat_max, detail_it.pos_tst_.latitude_);

            lon_min = min(lon_min, detail_it.pos_tst_.longitude_);
            lon_max = max(lon_max, detail_it.pos_tst_.longitude_);
        }
        else // tst pos always set
        {
            lat_min = detail_it.pos_tst_.latitude_;
            lat_max = detail_it.pos_tst_.latitude_;

            lon_min = detail_it.pos_tst_.longitude_;
            lon_max = detail_it.pos_tst_.longitude_;

            has_pos = true;
        }
    }

    if (has_pos)
    {
        (*viewable_ptr)["position_latitude"] = (lat_max+lat_min)/2.0;
        (*viewable_ptr)["position_longitude"] = (lon_max+lon_min)/2.0;;

        double lat_w = 1.1*(lat_max-lat_min)/2.0;
        double lon_w = 1.1*(lon_max-lon_min)/2.0;

        if (lat_w < eval_man_.resultDetailZoom())
            lat_w = eval_man_.resultDetailZoom();

        if (lon_w < eval_man_.resultDetailZoom())
            lon_w = eval_man_.resultDetailZoom();

        (*viewable_ptr)["position_window_latitude"] = lat_w;
        (*viewable_ptr)["position_window_longitude"] = lon_w;
    }

    return viewable_ptr;
}

bool SingleModeCFalse::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else
        return false;;
}

std::string SingleModeCFalse::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasReference(table, annotation));

    return "Report:Results:"+getTargetRequirementSectionID();
}

std::shared_ptr<Joined> SingleModeCFalse::createEmptyJoined(const std::string& result_id)
{
    return make_shared<JoinedModeCFalse> (result_id, requirement_, sector_layer_, eval_man_);
}

int SingleModeCFalse::numNoRefPos() const
{
    return num_no_ref_pos_;
}

int SingleModeCFalse::numNoRefValue() const
{
    return num_no_ref_val_;
}

int SingleModeCFalse::numPosOutside() const
{
    return num_pos_outside_;
}

int SingleModeCFalse::numPosInside() const
{
    return num_pos_inside_;
}

int SingleModeCFalse::numUpdates() const
{
    return num_updates_;
}

int SingleModeCFalse::numUnknown() const
{
    return num_unknown_;
}

int SingleModeCFalse::numCorrect() const
{
    return num_correct_;
}

int SingleModeCFalse::numFalse() const
{
    return num_false_;
}

std::vector<EvaluationRequirement::CheckDetail>& SingleModeCFalse::details()
{
    return details_;
}

}
