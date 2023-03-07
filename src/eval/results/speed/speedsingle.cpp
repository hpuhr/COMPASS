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

#include "eval/results/speed/speedsingle.h"
#include "eval/results/speed/speedjoined.h"
#include "eval/requirement/base/base.h"
#include "eval/requirement/speed/speed.h"
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

namespace EvaluationRequirementResult
{

SingleSpeed::SingleSpeed(
        const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
        const SectorLayer& sector_layer,
        unsigned int utn, const EvaluationTargetData* target, EvaluationManager& eval_man,
        unsigned int num_pos, unsigned int num_no_ref,
        unsigned int num_pos_outside, unsigned int num_pos_inside, unsigned int num_no_tst_value,
        unsigned int num_comp_failed, unsigned int num_comp_passed,
        vector<double> values,
        std::vector<EvaluationRequirement::SpeedDetail> details)
    : Single("SingleSpeed", result_id, requirement, sector_layer, utn, target, eval_man),
      num_pos_(num_pos), num_no_ref_(num_no_ref), num_pos_outside_(num_pos_outside),
      num_pos_inside_(num_pos_inside), num_no_tst_value_(num_no_tst_value),
      num_comp_failed_(num_comp_failed), num_comp_passed_(num_comp_passed),
      values_(values), details_(details)
{
    update();
}


void SingleSpeed::update()
{
    assert (num_no_ref_ <= num_pos_);
    assert (num_pos_ - num_no_ref_ == num_pos_inside_ + num_pos_outside_);

    assert (values_.size() == num_comp_failed_+num_comp_passed_);

    unsigned int num_speeds = values_.size();

    if (num_speeds)
    {
        value_min_ = *min_element(values_.begin(), values_.end());
        value_max_ = *max_element(values_.begin(), values_.end());
        value_avg_ = std::accumulate(values_.begin(), values_.end(), 0.0) / (float) num_speeds;

        value_var_ = 0;
        for(auto val : values_)
            value_var_ += pow(val - value_avg_, 2);
        value_var_ /= (float)num_speeds;

        assert (num_comp_failed_ <= num_speeds);
        p_passed_ = (float)num_comp_passed_/(float)num_speeds;
        has_p_min_ = true;

        result_usable_ = true;
    }
    else
    {
        value_min_ = 0;
        value_max_ = 0;
        value_avg_ = 0;
        value_var_ = 0;

        has_p_min_ = false;
        p_passed_ = 0;

        result_usable_ = false;
    }

    updateUseFromTarget();
}

void SingleSpeed::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "SingleSpeed " <<  requirement_->name() <<": addToReport";

    // add target to requirements->group->req
    addTargetToOverviewTable(root_item);

    // add requirement results to targets->utn->requirements->group->req
    addTargetDetailsToReport(root_item);

    // TODO add requirement description, methods
}

void SingleSpeed::addTargetToOverviewTable(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::Section& tgt_overview_section = getRequirementSection(root_item);

    if (eval_man_.reportShowAdsbInfo())
        addTargetDetailsToTableADSB(tgt_overview_section, target_table_name_);
    else
        addTargetDetailsToTable(tgt_overview_section, target_table_name_);

    if (eval_man_.reportSplitResultsByMOPS()) // add to general sum table
    {
        EvaluationResultsReport::Section& sum_section = root_item->getSection(getRequirementSumSectionID());

        if (eval_man_.reportShowAdsbInfo())
            addTargetDetailsToTableADSB(sum_section, target_table_name_);
        else
            addTargetDetailsToTable(sum_section, target_table_name_);
    }
}

void SingleSpeed::addTargetDetailsToTable (
        EvaluationResultsReport::Section& section, const std::string& table_name)
{
    if (!section.hasTable(table_name))
    {
        Qt::SortOrder order = Qt::AscendingOrder;

        if(req()->probCheckType() == EvaluationRequirement::COMPARISON_TYPE::LESS_THAN
                || req()->probCheckType() == EvaluationRequirement::COMPARISON_TYPE::LESS_THAN_OR_EQUAL)
            order = Qt::DescendingOrder;


        section.addTable(table_name, 15,
                         {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                          "OMin", "OMax", "OAvg", "OSDev", "#CF", "#CP", "PCP"}, true, 14, order);
    }

    EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

    QVariant p_min_var;

    if (has_p_min_)
        p_min_var = roundf(p_passed_ * 10000.0) / 100.0;

    target_table.addRow(
                {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
                 target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
                 target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(), target_->modeCMaxStr().c_str(),
                 Number::round(value_min_,2), // "DMin"
                 Number::round(value_max_,2), // "DMax"
                 Number::round(value_avg_,2), // "DAvg"
                 Number::round(sqrt(value_var_),2), // "DSDev"
                 num_comp_failed_, // "#DOK"
                 num_comp_passed_, // "#DNOK"
                 p_min_var}, // "PDOK"
                this, {utn_});
}

void SingleSpeed::addTargetDetailsToTableADSB (
        EvaluationResultsReport::Section& section, const std::string& table_name)
{
    if (!section.hasTable(table_name))
    {
        Qt::SortOrder order = Qt::AscendingOrder;

        if(req()->probCheckType() == EvaluationRequirement::COMPARISON_TYPE::LESS_THAN
                || req()->probCheckType() == EvaluationRequirement::COMPARISON_TYPE::LESS_THAN_OR_EQUAL)
            order = Qt::DescendingOrder;

        section.addTable(table_name, 18,
                         {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                          "OMin", "OMax", "OAvg", "OSDev", "#CF", "#CP", "PCP",
                          "MOPS", "NUCp/NIC", "NACp"}, true, 14, order);
    }

    EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

    QVariant prob_var;

    if (has_p_min_)
        prob_var = roundf(p_passed_ * 10000.0) / 100.0;

    // "UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
    // "#ACOK", "#ACNOK", "PACOK", "#DOK", "#DNOK", "PDOK", "MOPS", "NUCp/NIC", "NACp"

    target_table.addRow(
                {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
                 target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
                 target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(),
                 target_->modeCMaxStr().c_str(),
                 Number::round(value_min_,2), // "DMin"
                 Number::round(value_max_,2), // "DMax"
                 Number::round(value_avg_,2), // "DAvg"
                 Number::round(sqrt(value_var_),2), // "DSDev"
                 num_comp_failed_, // "#DOK"
                 num_comp_passed_, // "#DNOK"
                 prob_var, // "PDOK"
                 target_->mopsVersionStr().c_str(), // "MOPS"
                 target_->nucpNicStr().c_str(), // "NUCp/NIC"
                 target_->nacpStr().c_str()}, // "NACp"
                this, {utn_});

}

void SingleSpeed::addTargetDetailsToReport(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    root_item->getSection(getTargetSectionID()).perTargetSection(true); // mark utn section per target
    EvaluationResultsReport::Section& utn_req_section = root_item->getSection(getTargetRequirementSectionID());

    if (!utn_req_section.hasTable("details_overview_table"))
        utn_req_section.addTable("details_overview_table", 3, {"Name", "comment", "Value"}, false);

    std::shared_ptr<EvaluationRequirement::Speed> req =
            std::static_pointer_cast<EvaluationRequirement::Speed>(requirement_);
    assert (req);

    EvaluationResultsReport::SectionContentTable& utn_req_table =
            utn_req_section.getTable("details_overview_table");

    addCommonDetails(root_item);

    // "UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
    // "#ACOK", "#ACNOK", "PACOK", "#DOK", "#DNOK", "PDOK"

    utn_req_table.addRow({"Use", "To be used in results", use_}, this);
    utn_req_table.addRow({"#Pos [1]", "Number of updates", num_pos_}, this);
    utn_req_table.addRow({"#NoRef [1]", "Number of updates w/o reference speeds", num_no_ref_}, this);
    utn_req_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
    utn_req_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_pos_outside_}, this);
    utn_req_table.addRow({"#NoTstData [1]", "Number of updates without tst speed data", num_no_tst_value_}, this);

    // along
    utn_req_table.addRow({"OMin [m/s]", "Minimum of speed offset",
                          String::doubleToStringPrecision(value_min_,2).c_str()}, this);
    utn_req_table.addRow({"OMax [m/s]", "Maximum of speed offset",
                          String::doubleToStringPrecision(value_max_,2).c_str()}, this);
    utn_req_table.addRow({"OAvg [m/s]", "Average of speed offset",
                          String::doubleToStringPrecision(value_avg_,2).c_str()}, this);
    utn_req_table.addRow({"OSDev [m/s]", "Standard Deviation of speed offset",
                          String::doubleToStringPrecision(sqrt(value_var_),2).c_str()}, this);
    utn_req_table.addRow({"OVar [m^2/s^2]", "Variance of speed offset",
                          String::doubleToStringPrecision(value_var_,2).c_str()}, this);
    utn_req_table.addRow({"#CF [1]", "Number of updates with failed comparison", num_comp_failed_}, this);
    utn_req_table.addRow({"#CP [1]", "Number of updates with  passed comparison", num_comp_passed_},
                         this);

    // condition
    {
        QVariant p_passed_var;

        if (has_p_min_)
            p_passed_var = roundf(p_passed_ * 10000.0) / 100.0;

        utn_req_table.addRow({"PCP [%]", "Probability of passed comparison", p_passed_var}, this);

        utn_req_table.addRow({"Condition", {}, req->getConditionStr().c_str()}, this);

        string result {"Unknown"};

        if (has_p_min_)
            result = req->getResultConditionStr(p_passed_);

        utn_req_table.addRow({"Condition Fulfilled", "", result.c_str()}, this);

        if (result == "Failed")
        {
            root_item->getSection(getTargetSectionID()).perTargetWithIssues(true); // mark utn section as with issue
            utn_req_section.perTargetWithIssues(true);
        }

    }

    if (has_p_min_ && p_passed_ != 1.0) // TODO
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

void SingleSpeed::reportDetails(EvaluationResultsReport::Section& utn_req_section)
{
    if (!utn_req_section.hasTable(tr_details_table_name_))
        utn_req_section.addTable(tr_details_table_name_, 8,
                                 {"ToD", "NoRef", "PosInside", "Distance", "CP", "#CF", "#CP", "Comment"});

    EvaluationResultsReport::SectionContentTable& utn_req_details_table =
            utn_req_section.getTable(tr_details_table_name_);

    unsigned int detail_cnt = 0;

    for (auto& rq_det_it : details_)
    {
        utn_req_details_table.addRow(
                    {Time::toString(rq_det_it.timestamp_).c_str(),
                     !rq_det_it.has_ref_pos_, rq_det_it.pos_inside_,
                     rq_det_it.offset_,  // "Distance"
                     rq_det_it.check_passed_, // CP"
                     rq_det_it.num_check_failed_, // "#CF",
                     rq_det_it.num_check_passed_, // "#CP"
                     rq_det_it.comment_.c_str()}, // "Comment"
                    this, detail_cnt);

        ++detail_cnt;
    }
}

bool SingleSpeed::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else if (table.name() == tr_details_table_name_ && annotation.isValid() && annotation.toUInt() < details_.size())
        return true;
    else
        return false;
}

std::unique_ptr<nlohmann::json::object_t> SingleSpeed::viewableData(
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

        loginf << "SingleSpeed: viewableData: detail_cnt " << detail_cnt;

        std::unique_ptr<nlohmann::json::object_t> viewable_ptr
                = eval_man_.getViewableForEvaluation(utn_, req_grp_id_, result_id_);
        assert (viewable_ptr);

        const EvaluationRequirement::SpeedDetail& detail = details_.at(detail_cnt);

        (*viewable_ptr)[VP_POS_LAT_KEY] = detail.tst_pos_.latitude_;
        (*viewable_ptr)[VP_POS_LON_KEY] = detail.tst_pos_.longitude_;
        (*viewable_ptr)[VP_POS_WIN_LAT_KEY] = eval_man_.resultDetailZoom();
        (*viewable_ptr)[VP_POS_WIN_LON_KEY] = eval_man_.resultDetailZoom();
        (*viewable_ptr)[VP_TIMESTAMP_KEY] = Time::toString(detail.timestamp_);

        if (!detail.check_passed_)
            (*viewable_ptr)[VP_EVAL_KEY][VP_EVAL_HIGHDET_KEY] = vector<unsigned int>{detail_cnt};

        return viewable_ptr;
    }
    else
        return nullptr;
}

std::unique_ptr<nlohmann::json::object_t> SingleSpeed::getTargetErrorsViewable ()
{
    std::unique_ptr<nlohmann::json::object_t> viewable_ptr = eval_man_.getViewableForEvaluation(
                utn_, req_grp_id_, result_id_);

    bool has_pos = false;
    double lat_min, lat_max, lon_min, lon_max;

    bool failed_values_of_interest = req()->failedValuesOfInterest();

    for (auto& detail_it : details_)
    {
        if ((failed_values_of_interest && detail_it.check_passed_)
                || (!failed_values_of_interest && !detail_it.check_passed_))
            continue;

        if (has_pos)
        {
            lat_min = min(lat_min, detail_it.tst_pos_.latitude_);
            lat_max = max(lat_max, detail_it.tst_pos_.latitude_);

            lon_min = min(lon_min, detail_it.tst_pos_.longitude_);
            lon_max = max(lon_max, detail_it.tst_pos_.longitude_);
        }
        else // tst pos always set
        {
            lat_min = detail_it.tst_pos_.latitude_;
            lat_max = detail_it.tst_pos_.latitude_;

            lon_min = detail_it.tst_pos_.longitude_;
            lon_max = detail_it.tst_pos_.longitude_;

            has_pos = true;
        }

        if (detail_it.has_ref_pos_)
        {
            lat_min = min(lat_min, detail_it.ref_pos_.latitude_);
            lat_max = max(lat_max, detail_it.ref_pos_.latitude_);

            lon_min = min(lon_min, detail_it.ref_pos_.longitude_);
            lon_max = max(lon_max, detail_it.ref_pos_.longitude_);
        }
    }

    if (has_pos)
    {
        (*viewable_ptr)["speed_latitude"] = (lat_max+lat_min)/2.0;
        (*viewable_ptr)["speed_longitude"] = (lon_max+lon_min)/2.0;;

        double lat_w = 1.1*(lat_max-lat_min)/2.0;
        double lon_w = 1.1*(lon_max-lon_min)/2.0;

        if (lat_w < eval_man_.resultDetailZoom())
            lat_w = eval_man_.resultDetailZoom();

        if (lon_w < eval_man_.resultDetailZoom())
            lon_w = eval_man_.resultDetailZoom();

        (*viewable_ptr)["speed_window_latitude"] = lat_w;
        (*viewable_ptr)["speed_window_longitude"] = lon_w;
    }

    return viewable_ptr;
}

bool SingleSpeed::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else
        return false;;
}

std::string SingleSpeed::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasReference(table, annotation));

    return "Report:Results:"+getTargetRequirementSectionID();
}

unsigned int SingleSpeed::numCompFailed() const
{
    return num_comp_failed_;
}

unsigned int SingleSpeed::numCompPassed() const
{
    return num_comp_passed_;
}


const vector<double>& SingleSpeed::values() const
{
    return values_;
}

unsigned int SingleSpeed::numPosOutside() const
{
    return num_pos_outside_;
}

unsigned int SingleSpeed::numPosInside() const
{
    return num_pos_inside_;
}

unsigned int SingleSpeed::numNoTstValues() const
{
    return num_no_tst_value_;
}

std::shared_ptr<Joined> SingleSpeed::createEmptyJoined(const std::string& result_id)
{
    return make_shared<JoinedSpeed> (result_id, requirement_, sector_layer_, eval_man_);
}

unsigned int SingleSpeed::numPos() const
{
    return num_pos_;
}

unsigned int SingleSpeed::numNoRef() const
{
    return num_no_ref_;
}

std::vector<EvaluationRequirement::SpeedDetail>& SingleSpeed::details()
{
    return details_;
}

EvaluationRequirement::Speed* SingleSpeed::req ()
{
    EvaluationRequirement::Speed* req =
            dynamic_cast<EvaluationRequirement::Speed*>(requirement_.get());
    assert (req);
    return req;
}

}
