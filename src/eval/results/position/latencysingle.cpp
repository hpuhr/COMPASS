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

#include "eval/results/position/latencysingle.h"
#include "eval/results/position/latencyjoined.h"
#include "eval/requirement/base/base.h"
#include "eval/requirement/position/latency.h"
#include "evaluationtargetdata.h"
#include "evaluationmanager.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "logger.h"
#include "stringconv.h"
#include "number.h"

#include <cassert>
#include <algorithm>

using namespace std;
using namespace Utils;

namespace EvaluationRequirementResult
{

    SinglePositionLatency::SinglePositionLatency(
            const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
            const SectorLayer& sector_layer,
            unsigned int utn, const EvaluationTargetData* target, EvaluationManager& eval_man,
            unsigned int num_pos, unsigned int num_no_ref,
            unsigned int num_pos_outside, unsigned int num_pos_inside,
            unsigned int num_value_ok, unsigned int num_value_nok,
            vector<double> values,
            std::vector<EvaluationRequirement::PositionDetail> details)
        : Single("SinglePositionLatency", result_id, requirement, sector_layer, utn, target, eval_man),
          num_pos_(num_pos), num_no_ref_(num_no_ref), num_pos_outside_(num_pos_outside),
          num_pos_inside_(num_pos_inside), num_value_ok_(num_value_ok), num_value_nok_(num_value_nok),
          values_(values), details_(details)
    {
        update();
    }


    void SinglePositionLatency::update()
    {
        assert (num_no_ref_ <= num_pos_);
        assert (num_pos_ - num_no_ref_ == num_pos_inside_ + num_pos_outside_);

        assert (values_.size() == num_value_ok_+num_value_nok_);

        unsigned int num_distances = values_.size();

        if (num_distances)
        {
            value_min_ = *min_element(values_.begin(), values_.end());
            value_max_ = *max_element(values_.begin(), values_.end());
            value_avg_ = std::accumulate(values_.begin(), values_.end(), 0.0) / (float) num_distances;

            value_var_ = 0;
            for(auto val : values_)
                value_var_ += pow(val - value_avg_, 2);
            value_var_ /= (float)num_distances;

            assert (num_value_ok_ <= num_distances);
            p_min_ = (float)num_value_ok_/(float)num_distances;
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
            p_min_ = 0;

            result_usable_ = false;
        }

        updateUseFromTarget();
    }

    void SinglePositionLatency::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        logdbg << "SinglePositionLatency " <<  requirement_->name() <<": addToReport";

        // add target to requirements->group->req
        addTargetToOverviewTable(root_item);

        // add requirement results to targets->utn->requirements->group->req
        addTargetDetailsToReport(root_item);

        // TODO add requirement description, methods
    }

    void SinglePositionLatency::addTargetToOverviewTable(shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        EvaluationResultsReport::Section& tgt_overview_section = getRequirementSection(root_item);

        if (eval_man_.resultsGenerator().showAdsbInfo())
            addTargetDetailsToTableADSB(tgt_overview_section, target_table_name_);
        else
            addTargetDetailsToTable(tgt_overview_section, target_table_name_);

        if (eval_man_.resultsGenerator().splitResultsByMOPS()) // add to general sum table
        {
            EvaluationResultsReport::Section& sum_section = root_item->getSection(getRequirementSumSectionID());

            if (eval_man_.resultsGenerator().showAdsbInfo())
                addTargetDetailsToTableADSB(sum_section, target_table_name_);
            else
                addTargetDetailsToTable(sum_section, target_table_name_);
        }
    }

    void SinglePositionLatency::addTargetDetailsToTable (
            EvaluationResultsReport::Section& section, const std::string& table_name)
    {
        if (!section.hasTable(table_name))
            section.addTable(table_name, 15,
            {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
             "LTMin", "LTMax", "LTAvg", "LTSDev", "#LTOK", "#LTNOK", "PLTOK"}, true, 14);

        EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

        QVariant p_min_var;

        if (has_p_min_)
            p_min_var = roundf(p_min_ * 10000.0) / 100.0;

        target_table.addRow(
        {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
         target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
         target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(), target_->modeCMaxStr().c_str(),
         Number::round(value_min_,2), // "LTMin"
         Number::round(value_max_,2), // "LTMax"
         Number::round(value_avg_,2), // "LTAvg"
         Number::round(sqrt(value_var_),2), // "LTSDev"
         num_value_ok_, // "#LTOK"
         num_value_nok_, // "#LTNOK"
         p_min_var}, // "PLTOK"
                    this, {utn_});
    }

    void SinglePositionLatency::addTargetDetailsToTableADSB (
            EvaluationResultsReport::Section& section, const std::string& table_name)
    {
        if (!section.hasTable(table_name))
            section.addTable(table_name, 18,
            {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
             "LTMin", "LTMax", "LTAvg", "LTSDev", "#LTOK", "#LTNOK", "PLTOK",
             "MOPS", "NUCp/NIC", "NACp"}, true, 14);

        EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

        QVariant p_min_var;

        if (has_p_min_)
            p_min_var = roundf(p_min_ * 10000.0) / 100.0;

        // "UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
        // "#ACOK", "#ACNOK", "PACOK", "#LTOK", "#LTNOK", "PLTOK", "MOPS", "NUCp/NIC", "NACp"

        target_table.addRow(
        {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
         target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
         target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(),
         target_->modeCMaxStr().c_str(),
         Number::round(value_min_,2), // "LTMin"
         Number::round(value_max_,2), // "LTMax"
         Number::round(value_avg_,2), // "LTAvg"
         Number::round(sqrt(value_var_),2), // "LTSDev"
         num_value_ok_, // "#LTOK"
         num_value_nok_, // "#LTNOK"
         p_min_var, // "PLTOK"
         target_->mopsVersionsStr().c_str(), // "MOPS"
         target_->nucpNicStr().c_str(), // "NUCp/NIC"
         target_->nacpStr().c_str()}, // "NACp"
                    this, {utn_});

    }

    void SinglePositionLatency::addTargetDetailsToReport(shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        EvaluationResultsReport::Section& utn_req_section = root_item->getSection(getTargetRequirementSectionID());

        if (!utn_req_section.hasTable("details_overview_table"))
            utn_req_section.addTable("details_overview_table", 3, {"Name", "comment", "Value"}, false);

        std::shared_ptr<EvaluationRequirement::PositionLatency> req =
                std::static_pointer_cast<EvaluationRequirement::PositionLatency>(requirement_);
        assert (req);

        EvaluationResultsReport::SectionContentTable& utn_req_table =
                utn_req_section.getTable("details_overview_table");

        addCommonDetails(root_item);

        // "UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
        // "#ACOK", "#ACNOK", "PACOK", "#LTOK", "#LTNOK", "PLTOK"

        utn_req_table.addRow({"Use", "To be used in results", use_}, this);
        utn_req_table.addRow({"#Pos [1]", "Number of updates", num_pos_}, this);
        utn_req_table.addRow({"#NoRef [1]", "Number of updates w/o reference positions", num_no_ref_}, this);
        utn_req_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
        utn_req_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_pos_outside_}, this);

        // along
        utn_req_table.addRow({"LTMin [s]", "Minimum of latency",
                              String::timeStringFromDouble(value_min_,2).c_str()}, this);
        utn_req_table.addRow({"LTMax [s]", "Maximum of latency",
                              String::timeStringFromDouble(value_max_,2).c_str()}, this);
        utn_req_table.addRow({"LTAvg [s]", "Average of latency",
                              String::timeStringFromDouble(value_avg_,2).c_str()}, this);
        utn_req_table.addRow({"LTSDev [s]", "Standard Deviation of latency",
                              String::timeStringFromDouble(sqrt(value_var_),2).c_str()}, this);
        utn_req_table.addRow({"LTVar [s]", "Variance of latency",
                              String::timeStringFromDouble(value_var_,2).c_str()}, this);
        utn_req_table.addRow({"#LTOK [1]", "Number of updates with latency", num_value_ok_}, this);
        utn_req_table.addRow({"#LTNOK [1]", "Number of updates with unacceptable latency ", num_value_nok_},
                             this);

        // condition
        {
            QVariant p_min_var;

            if (has_p_min_)
                p_min_var = roundf(p_min_ * 10000.0) / 100.0;

            utn_req_table.addRow({"PLTOK [%]", "Probability of acceptable latency", p_min_var}, this);

            utn_req_table.addRow({"Condition Latency", {}, req->getConditionStr().c_str()}, this);

            string result {"Unknown"};

            if (has_p_min_)
                result = req->getResultConditionStr(p_min_);

            utn_req_table.addRow({"Condition Latency Fulfilled", "", result.c_str()}, this);
        }

        if (has_p_min_ && p_min_ != 1.0)
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

    void SinglePositionLatency::reportDetails(EvaluationResultsReport::Section& utn_req_section)
    {
        if (!utn_req_section.hasTable(tr_details_table_name_))
            utn_req_section.addTable(tr_details_table_name_, 8,
            {"ToD", "NoRef", "PosInside",
             "DLatency", "DLatencyOK", "#LTOK", "#LTNOK",
             "Comment"});

        EvaluationResultsReport::SectionContentTable& utn_req_details_table =
                utn_req_section.getTable(tr_details_table_name_);

        unsigned int detail_cnt = 0;

        for (auto& rq_det_it : details_)
        {
            utn_req_details_table.addRow(
            {String::timeStringFromDouble(rq_det_it.tod_).c_str(),
             !rq_det_it.has_ref_pos_, rq_det_it.pos_inside_,
             rq_det_it.value_,  // "DLatency"
             rq_det_it.check_passed_, // DLatencyOK"
             rq_det_it.num_check_failed_, // "#LTOK",
             rq_det_it.num_check_passed_, // "#LTNOK"
             rq_det_it.comment_.c_str()}, // "Comment"
                        this, detail_cnt);

            ++detail_cnt;
        }
    }

    bool SinglePositionLatency::hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
            return true;
        else if (table.name() == tr_details_table_name_ && annotation.isValid() && annotation.toUInt() < details_.size())
            return true;
        else
            return false;
    }

    std::unique_ptr<nlohmann::json::object_t> SinglePositionLatency::viewableData(
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

            loginf << "SinglePositionLatency: viewableData: detail_cnt " << detail_cnt;

            std::unique_ptr<nlohmann::json::object_t> viewable_ptr
                    = eval_man_.getViewableForEvaluation(utn_, req_grp_id_, result_id_);
            assert (viewable_ptr);

            const EvaluationRequirement::PositionDetail& detail = details_.at(detail_cnt);

            (*viewable_ptr)["position_latitude"] = detail.tst_pos_.latitude_;
            (*viewable_ptr)["position_longitude"] = detail.tst_pos_.longitude_;
            (*viewable_ptr)["position_window_latitude"] = 0.02;
            (*viewable_ptr)["position_window_longitude"] = 0.02;
            (*viewable_ptr)["time"] = detail.tod_;

            if (!detail.check_passed_)
                (*viewable_ptr)["evaluation_results"]["highlight_details"] = vector<unsigned int>{detail_cnt};

            return viewable_ptr;
        }
        else
            return nullptr;
    }

    std::unique_ptr<nlohmann::json::object_t> SinglePositionLatency::getTargetErrorsViewable ()
    {
        std::unique_ptr<nlohmann::json::object_t> viewable_ptr = eval_man_.getViewableForEvaluation(
                    utn_, req_grp_id_, result_id_);

        bool has_pos = false;
        double lat_min, lat_max, lon_min, lon_max;

        for (auto& detail_it : details_)
        {
            if (detail_it.check_passed_)
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
            (*viewable_ptr)["position_latitude"] = (lat_max+lat_min)/2.0;
            (*viewable_ptr)["position_longitude"] = (lon_max+lon_min)/2.0;;

            double lat_w = 1.1*(lat_max-lat_min)/2.0;
            double lon_w = 1.1*(lon_max-lon_min)/2.0;

            if (lat_w < 0.02)
                lat_w = 0.02;

            if (lon_w < 0.02)
                lon_w = 0.02;

            (*viewable_ptr)["position_window_latitude"] = lat_w;
            (*viewable_ptr)["position_window_longitude"] = lon_w;
        }

        return viewable_ptr;
    }

    bool SinglePositionLatency::hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
            return true;
        else
            return false;;
    }

    std::string SinglePositionLatency::reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        assert (hasReference(table, annotation));

        return "Report:Results:"+getTargetRequirementSectionID();
    }

    unsigned int SinglePositionLatency::numValueOk() const
    {
        return num_value_ok_;
    }

    unsigned int SinglePositionLatency::numValueNOk() const
    {
        return num_value_nok_;
    }


    const vector<double>& SinglePositionLatency::values() const
    {
        return values_;
    }

    unsigned int SinglePositionLatency::numPosOutside() const
    {
        return num_pos_outside_;
    }

    unsigned int SinglePositionLatency::numPosInside() const
    {
        return num_pos_inside_;
    }

    std::shared_ptr<Joined> SinglePositionLatency::createEmptyJoined(const std::string& result_id)
    {
        return make_shared<JoinedPositionLatency> (result_id, requirement_, sector_layer_, eval_man_);
    }

    unsigned int SinglePositionLatency::numPos() const
    {
        return num_pos_;
    }

    unsigned int SinglePositionLatency::numNoRef() const
    {
        return num_no_ref_;
    }

    std::vector<EvaluationRequirement::PositionDetail>& SinglePositionLatency::details()
    {
        return details_;
    }
}
