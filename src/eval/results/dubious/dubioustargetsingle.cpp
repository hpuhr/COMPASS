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

#include "eval/results/dubious/dubioustargetsingle.h"
#include "eval/results/dubious/dubioustargetjoined.h"
#include "eval/requirement/base/base.h"
#include "eval/requirement/dubious/dubioustarget.h"
#include "evaluationtargetdata.h"
#include "evaluationmanager.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "logger.h"
#include "util/stringconv.h"
#include "util/timeconv.h"
#include "util/number.h"

#include <cassert>
#include <algorithm>

using namespace std;
using namespace Utils;
using namespace nlohmann;

namespace EvaluationRequirementResult
{

SingleDubiousTarget::SingleDubiousTarget(const std::string& result_id,
                                         std::shared_ptr<EvaluationRequirement::Base> requirement,
                                         const SectorLayer& sector_layer,
                                         unsigned int utn,
                                         const EvaluationTargetData* target,
                                         EvaluationManager& eval_man,
                                         const EvaluationDetails& details,
                                         unsigned int num_updates,
                                         unsigned int num_pos_outside,
                                         unsigned int num_pos_inside,
                                         unsigned int num_pos_inside_dubious)
    :   SingleDubiousBase("SingleDubiousTarget", result_id, requirement, sector_layer, utn, target, eval_man, details, num_updates, num_pos_outside, num_pos_inside, num_pos_inside_dubious)
{
    update();
}

void SingleDubiousTarget::update()
{
    p_dubious_target_.reset();
    p_dubious_update_.reset();

    assert (num_updates_ == num_pos_inside_ + num_pos_outside_);

    //assert (values_.size() == num_comp_failed_+num_comp_passed_);

    //unsigned int num_speeds = values_.size();

    assert (numDetails() == 1);

    const auto& detail = getDetail(0);

    p_dubious_target_ = (float)detail.getValue(DetailKey::IsDubious).toBool();

    result_usable_ = true;

    if (num_pos_inside_)
    {
        p_dubious_update_ = (float)num_pos_inside_dubious_/(float)num_pos_inside_;
    }

    logdbg << "SingleDubiousTarget "      << requirement_->name() << " " << target_->utn_
           << " has_p_dubious_update_ "   << p_dubious_update_.has_value()
           << " num_pos_inside_dubious_ " << num_pos_inside_dubious_
           << " num_pos_inside_ "         << num_pos_inside_
           << " p_dubious_update_ "       << (p_dubious_update_.has_value() ? p_dubious_update_.value() : 0);

    updateUseFromTarget();
}

void SingleDubiousTarget::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "SingleDubiousTarget " <<  requirement_->name() <<": addToReport";

    // add target to requirements->group->req
    addTargetToOverviewTable(root_item);

    // add requirement results to targets->utn->requirements->group->req
    addTargetDetailsToReport(root_item);

    // TODO add requirement description, methods
}

void SingleDubiousTarget::addTargetToOverviewTable(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::Section& tgt_overview_section = getRequirementSection(root_item);

    if (eval_man_.reportShowAdsbInfo())
        addTargetDetailsToTableADSB(tgt_overview_section, target_table_name_);
    else
        addTargetDetailsToTable(tgt_overview_section, target_table_name_);

    if (eval_man_.reportSplitResultsByMOPS()
            || eval_man_.reportSplitResultsByACOnlyMS()) // add to general sum table
    {
        EvaluationResultsReport::Section& sum_section = root_item->getSection(getRequirementSumSectionID());

        if (eval_man_.reportShowAdsbInfo())
            addTargetDetailsToTableADSB(sum_section, target_table_name_);
        else
            addTargetDetailsToTable(sum_section, target_table_name_);
    }
}

void SingleDubiousTarget::addTargetDetailsToTable (EvaluationResultsReport::Section& section,
                                                   const std::string& table_name)
{
    if (!section.hasTable(table_name))
    {
        Qt::SortOrder order = Qt::AscendingOrder;

        if(req()->probCheckType() == EvaluationRequirement::COMPARISON_TYPE::LESS_THAN
                || req()->probCheckType() == EvaluationRequirement::COMPARISON_TYPE::LESS_THAN_OR_EQUAL)
            order = Qt::DescendingOrder;


        section.addTable(table_name, 13,
                         {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                          "#PosInside", "#DU", "PDU", "Reasons", "PDT"}, true, 12, order);
    }

    EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

    QVariant p_dubious_up_var;

    if (p_dubious_update_.has_value())
        p_dubious_up_var = roundf(p_dubious_update_.value() * 10000.0) / 100.0;

    QVariant p_dubious_target_var;

    if (p_dubious_target_.has_value())
        p_dubious_target_var = roundf(p_dubious_target_.value() * 10000.0) / 100.0;

    assert(numDetails() > 0);
    std::string dub_string = dubiousReasonsString(getDetail(0).comments());

    target_table.addRow(
                {utn_,
                 target_->timeBeginStr().c_str(),
                 target_->timeEndStr().c_str(),
                 target_->acidsStr().c_str(),
                 target_->acadsStr().c_str(),
                 target_->modeACodesStr().c_str(),
                 target_->modeCMinStr().c_str(),
                 target_->modeCMaxStr().c_str(),
                 num_pos_inside_, // "#PosInside"
                 num_pos_inside_dubious_, // "#DU"
                 p_dubious_up_var, // "PDU"
                 dub_string.c_str(),  // "Reasons"
                 p_dubious_target_var}, // "PDT"
                this, {utn_});
}

void SingleDubiousTarget::addTargetDetailsToTableADSB (
        EvaluationResultsReport::Section& section, const std::string& table_name)
{
    if (!section.hasTable(table_name))
    {
        Qt::SortOrder order = Qt::AscendingOrder;

        if(req()->probCheckType() == EvaluationRequirement::COMPARISON_TYPE::LESS_THAN
                || req()->probCheckType() == EvaluationRequirement::COMPARISON_TYPE::LESS_THAN_OR_EQUAL)
            order = Qt::DescendingOrder;

        section.addTable(table_name, 14,
                         {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                          "#PosInside", "#DU", "PDU", "Reasons", "PDT",
                          "MOPS"}, true, 12, order);
    }

    EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

    QVariant p_dubious_up_var;

    if (p_dubious_update_.has_value())
        p_dubious_up_var = roundf(p_dubious_update_.value() * 10000.0) / 100.0;

    QVariant p_dubious_var;

    if (p_dubious_target_.has_value())
        p_dubious_var = roundf(p_dubious_target_.value() * 10000.0) / 100.0;

    assert(numDetails() > 0);
    std::string dub_string = dubiousReasonsString(getDetail(0).comments());

    target_table.addRow(
                {utn_,
                 target_->timeBeginStr().c_str(),
                 target_->timeEndStr().c_str(),
                 target_->acidsStr().c_str(),
                 target_->acadsStr().c_str(),
                 target_->modeACodesStr().c_str(),
                 target_->modeCMinStr().c_str(),
                 target_->modeCMaxStr().c_str(),
                 num_pos_inside_, // "#PosInside"
                 num_pos_inside_dubious_, // "#DU"
                 p_dubious_up_var, // "PDU"
                 dub_string.c_str(),  // "Reasons"
                 p_dubious_var, // "PDT"
                 target_->mopsVersionStr().c_str()}, // "MOPS"
                this, {utn_});

}

void SingleDubiousTarget::addTargetDetailsToReport(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    root_item->getSection(getTargetSectionID()).perTargetSection(true); // mark utn section per target
    EvaluationResultsReport::Section& utn_req_section = root_item->getSection(getTargetRequirementSectionID());

    if (!utn_req_section.hasTable("details_overview_table"))
        utn_req_section.addTable("details_overview_table", 3, {"Name", "comment", "Value"}, false);

    std::shared_ptr<EvaluationRequirement::DubiousTarget> req =
            std::static_pointer_cast<EvaluationRequirement::DubiousTarget>(requirement_);
    assert (req);

    EvaluationResultsReport::SectionContentTable& utn_req_table =
            utn_req_section.getTable("details_overview_table");

    addCommonDetails(root_item);

    assert(numDetails() > 0);
    const auto& detail = getDetail(0);

    auto duration = detail.getValueAs<boost::posix_time::time_duration>(DetailKey::Duration);
    assert(duration.has_value());

    // "UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
    // "#ACOK", "#ACNOK", "PACOK", "#DOK", "#DNOK", "PDOK"

    utn_req_table.addRow({"Use", "To be used in results", use_}, this);
    utn_req_table.addRow({"#Up [1]", "Number of updates", num_updates_}, this);
    utn_req_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
    utn_req_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_pos_outside_}, this);
    utn_req_table.addRow({"#DU [1]", "Number of dubious updates inside sector", num_pos_inside_dubious_}, this);

    QVariant p_dubious_up_var;

    if (p_dubious_update_.has_value())
        p_dubious_up_var = roundf(p_dubious_update_.value() * 10000.0) / 100.0;

    utn_req_table.addRow({"PDU [%]", "Probability of dubious update", p_dubious_up_var}, this);

    utn_req_table.addRow({"Duration [s]", "Duration",
                          Time::toString(duration.value(),2).c_str()}, this);

    // condition
    {
        QVariant p_dubious_var;

        if (p_dubious_target_.has_value())
            p_dubious_target_ = roundf(p_dubious_target_.value() * 10000.0) / 100.0;

        utn_req_table.addRow({"PDT [%]", "Probability of dubious target", p_dubious_var}, this);

        utn_req_table.addRow({"Condition", {}, req->getConditionStr().c_str()}, this);

        string result {"Unknown"};

        if (p_dubious_target_.has_value())
            result = req->getConditionResultStr(p_dubious_target_.value());

        utn_req_table.addRow({"Condition Fulfilled", "", result.c_str()}, this);

        if (result == "Failed")
        {
            root_item->getSection(getTargetSectionID()).perTargetWithIssues(true); // mark utn section as with issue
            utn_req_section.perTargetWithIssues(true);
        }

    }

    if (p_dubious_target_.has_value() && p_dubious_target_.value() != 0.0) // TODO
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

void SingleDubiousTarget::reportDetails(EvaluationResultsReport::Section& utn_req_section)
{
    if (!utn_req_section.hasTable(tr_details_table_name_))
        utn_req_section.addTable(tr_details_table_name_, 3,
                                 {"ToD", "UTN", "Dubious Comment"});

    EvaluationResultsReport::SectionContentTable& utn_req_details_table =
            utn_req_section.getTable(tr_details_table_name_);

    utn_req_details_table.setCreateOnDemand(
                [this, &utn_req_details_table](void)
    {

        assert(numDetails() > 0);
        const auto& detail = getDetail(0);

        std::string dub_string = dubiousReasonsString(detail.comments());

        if (detail.hasDetails())
        {
            unsigned int detail_cnt = 0;
            for (auto& update : detail.details())
            {
                utn_req_details_table.addRow(
                            { Time::toString(update.timestamp()).c_str(),
                              detail.getValue(DetailKey::UTNOrTrackNum),
                              dub_string.c_str() }, // "Comment"
                            this, {detail_cnt});
                ++detail_cnt;
            }
        }});
}

bool SingleDubiousTarget::hasViewableData (const EvaluationResultsReport::SectionContentTable& table,
                                           const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else if (table.name() == tr_details_table_name_ && annotation.isValid()) //  && annotation.toUInt() < details_.size()
        return true;
    else
        return false;
}

std::unique_ptr<nlohmann::json::object_t> SingleDubiousTarget::viewableData(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{

    assert (hasViewableData(table, annotation));

    if (table.name() == target_table_name_)
    {
        return getTargetErrorsViewable();
    }
    else if (table.name() == tr_details_table_name_ && annotation.isValid())
    {
        unsigned int detail_update_cnt = annotation.toUInt();

        logdbg << "SingleDubiousTarget: viewableData: detail_update_cnt " << detail_update_cnt;

        std::unique_ptr<nlohmann::json::object_t> viewable_ptr
                = eval_man_.getViewableForEvaluation(utn_, req_grp_id_, result_id_);
        assert (viewable_ptr);

        //        assert (numDetails() > 0);
        //        const auto& detail = getDetail(0);

        //        unsigned int per_detail_update_cnt = detail_update_cnt;

        //        assert (per_detail_update_cnt < detail.numDetails());

        //        const auto& update_detail = detail.details().at(per_detail_update_cnt);

        //        assert (update_detail.numPositions() >= 1);

        //        (*viewable_ptr)[VP_POS_LAT_KEY    ] = update_detail.position(0).latitude_;
        //        (*viewable_ptr)[VP_POS_LON_KEY    ] = update_detail.position(0).longitude_;
        //        (*viewable_ptr)[VP_POS_WIN_LAT_KEY] = eval_man_.resultDetailZoom();
        //        (*viewable_ptr)[VP_POS_WIN_LON_KEY] = eval_man_.resultDetailZoom();
        //        (*viewable_ptr)[VP_TIMESTAMP_KEY  ] = Time::toString(update_detail.timestamp());

        //        if (update_detail.comments().hasComments(DetailCommentGroupDubious))
        //            (*viewable_ptr)[VP_EVAL_KEY][VP_EVAL_HIGHDET_KEY] = vector<unsigned int>{detail_update_cnt};

        return viewable_ptr;
    }
    else
        return nullptr;
}

bool SingleDubiousTarget::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else
        return false;;
}

std::string SingleDubiousTarget::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasReference(table, annotation));

    return "Report:Results:"+getTargetRequirementSectionID();
}

std::shared_ptr<Joined> SingleDubiousTarget::createEmptyJoined(const std::string& result_id)
{
    return make_shared<JoinedDubiousTarget> (result_id, requirement_, sector_layer_, eval_man_);
}

EvaluationRequirement::DubiousTarget* SingleDubiousTarget::req ()
{
    EvaluationRequirement::DubiousTarget* req =
            dynamic_cast<EvaluationRequirement::DubiousTarget*>(requirement_.get());
    assert (req);
    return req;
}

void SingleDubiousTarget::addAnnotations(nlohmann::json::object_t& viewable, bool add_ok)
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
