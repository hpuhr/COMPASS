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

#include "eval/results/position/alongacrosssingle.h"
#include "eval/results/position/alongacrossjoined.h"
#include "eval/requirement/base.h"
#include "eval/requirement/position/alongacross.h"
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

    SinglePositionAlongAcross::SinglePositionAlongAcross(
            const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
            const SectorLayer& sector_layer,
            unsigned int utn, const EvaluationTargetData* target, EvaluationManager& eval_man,
            unsigned int num_pos, unsigned int num_no_ref,
            unsigned int num_pos_outside, unsigned int num_pos_inside,
            unsigned int num_along_ok, unsigned int num_along_nok,
            unsigned int num_across_ok, unsigned int num_across_nok,
            tuple<vector<double>, vector<double>, vector<double>, vector<double>, vector<double>> distance_values,
            std::vector<EvaluationRequirement::PositionAlongAcrossDetail> details)
        : Single("SinglePositionAlongAcross", result_id, requirement, sector_layer, utn, target, eval_man),
          num_pos_(num_pos), num_no_ref_(num_no_ref), num_pos_outside_(num_pos_outside),
          num_pos_inside_(num_pos_inside), num_along_ok_(num_along_ok), num_along_nok_(num_along_nok),
          num_across_ok_(num_across_ok), num_across_nok_(num_across_nok), distance_values_(distance_values),
          details_(details)
    {
        update();
    }


    void SinglePositionAlongAcross::update()
    {
        assert (num_no_ref_ <= num_pos_);
        assert (num_pos_ - num_no_ref_ == num_pos_inside_ + num_pos_outside_);

        assert (get<0>(distance_values_).size() == num_along_ok_+num_along_nok_);
        assert (get<0>(distance_values_).size() == num_across_ok_+num_across_nok_);

        unsigned int num_distances = get<0>(distance_values_).size();

        // dx, dy, dalong, dacross
        if (num_distances)
        {
            vector<double>& along_vals = get<2>(distance_values_);
            vector<double>& across_vals = get<3>(distance_values_);
            vector<double>& latency_vals = get<4>(distance_values_);

            // along
            along_min_ = *min_element(along_vals.begin(), along_vals.end());
            along_max_ = *max_element(along_vals.begin(), along_vals.end());
            along_avg_ = std::accumulate(along_vals.begin(), along_vals.end(), 0.0) / (float) num_distances;

            along_var_ = 0;
            for(auto val : along_vals)
                along_var_ += pow(val - along_avg_, 2);
            along_var_ /= (float)num_distances;

            // across
            across_min_ = *min_element(across_vals.begin(), across_vals.end());;
            across_max_ = *max_element(across_vals.begin(), across_vals.end());;
            across_avg_ = std::accumulate(across_vals.begin(), across_vals.end(), 0.0) / (float) num_distances;

            across_var_ = 0;
            for(auto val : across_vals)
                across_var_ += pow(val - across_avg_, 2);
            across_var_ /= (float)num_distances;

            // latency
            latency_min_ = *min_element(latency_vals.begin(), latency_vals.end());;
            latency_max_ = *max_element(latency_vals.begin(), latency_vals.end());;
            latency_avg_ = std::accumulate(latency_vals.begin(), latency_vals.end(), 0.0) / (float) num_distances;

            latency_var_ = 0;
            for(auto val : latency_vals)
                latency_var_ += pow(val - latency_avg_, 2);
            latency_var_ /= (float)num_distances;

            //            loginf << "UGA utn " << utn_ << " along_avg " << along_avg_ << " along_var " << along_var_
            //                   << " across_avg " << across_avg_ << " across_var " << across_var_
            //                   << " num_distances " << num_distances;

            assert (num_along_ok_ <= num_distances);
            p_min_along_ = (float)num_along_ok_/(float)num_distances;
            has_p_min_along_ = true;

            assert (num_across_ok_ <= num_distances);
            p_min_across_ = (float)num_across_ok_/(float)num_distances;
            has_p_min_across_ = true;

            result_usable_ = true;
        }
        else
        {
            along_min_ = 0;
            along_max_ = 0;
            along_avg_ = 0;
            along_var_ = 0;

            across_min_ = 0;
            across_max_ = 0;
            across_avg_ = 0;
            across_var_ = 0;

            latency_min_ = 0;
            latency_max_ = 0;
            latency_avg_ = 0;
            latency_var_ = 0;

            has_p_min_along_ = false;
            p_min_along_ = 0;

            has_p_min_across_ =false;
            p_min_across_ = 0;

            result_usable_ = false;
        }

        updateUseFromTarget();
    }

    void SinglePositionAlongAcross::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        logdbg << "SinglePositionAlongAcross " <<  requirement_->name() <<": addToReport";

        // add target to requirements->group->req
        addTargetToOverviewTable(root_item);

        // add requirement results to targets->utn->requirements->group->req
        addTargetDetailsToReport(root_item);

        // TODO add requirement description, methods
    }

    void SinglePositionAlongAcross::addTargetToOverviewTable(shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        EvaluationResultsReport::Section& tgt_overview_section = getRequirementSection(root_item);

        if (eval_man_.showAdsbInfo())
        {
            if (!tgt_overview_section.hasTable(target_table_name_))
                tgt_overview_section.addTable(target_table_name_, 22,
                {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                 "ALAvg", "ALSDev", "#ALOK", "#ALNOK", "PALOK", "ALAvg",
                 "ACAvg", "ACSDev", "#ACOK", "#ACNOK", "PACOK",
                 "MOPS", "NUCp/NIC", "NACp"}, true, 12);

            addTargetDetailsToTableADSB(tgt_overview_section.getTable(target_table_name_));
        }
        else
        {
            if (!tgt_overview_section.hasTable(target_table_name_))
                tgt_overview_section.addTable(target_table_name_, 19,
                {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                 "ALAvg", "ALSDev", "#ALOK", "#ALNOK", "PALOK", "ALAvg",
                 "ACAvg", "ACSDev", "#ACOK", "#ACNOK", "PACOK"}, true, 12);

            addTargetDetailsToTable(tgt_overview_section.getTable(target_table_name_));
        }

        if (eval_man_.splitResultsByMOPS()) // add to general sum table
        {
            EvaluationResultsReport::Section& sum_section = root_item->getSection(getRequirementSumSectionID());

            if (eval_man_.showAdsbInfo())
            {
                if (!sum_section.hasTable(target_table_name_))
                    sum_section.addTable(target_table_name_, 22,
                    {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                     "ALAvg", "ALSDev", "#ALOK", "#ALNOK", "PALOK", "ALAvg",
                     "ACAvg", "ACSDev", "#ACOK", "#ACNOK", "PACOK",
                     "MOPS", "NUCp/NIC", "NACp"}, true, 12);

                addTargetDetailsToTableADSB(sum_section.getTable(target_table_name_));
            }
            else
            {
                if (!sum_section.hasTable(target_table_name_))
                    sum_section.addTable(target_table_name_, 19,
                    {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                     "ALAvg", "ALSDev", "#ALOK", "#ALNOK", "PALOK", "ALAvg",
                     "ACAvg", "ACSDev", "#ACOK", "#ACNOK", "PACOK"}, true, 12);

                addTargetDetailsToTable(sum_section.getTable(target_table_name_));
            }
        }
    }

    void SinglePositionAlongAcross::addTargetDetailsToTable (EvaluationResultsReport::SectionContentTable& target_table)
    {
        QVariant p_along_var;

        if (has_p_min_along_)
            p_along_var = roundf(has_p_min_along_ * 10000.0) / 100.0;

        QVariant p_across_var;

        if (has_p_min_across_)
            p_across_var = roundf(has_p_min_across_ * 10000.0) / 100.0;

        // "UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
        // "#ACOK", "#ACNOK", "PACOK", "#ALOK", "#ALNOK", "PALOK"

        target_table.addRow(
        {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
         target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
         target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(), target_->modeCMaxStr().c_str(),
         Number::round(along_avg_,2), // "ALAvg"
         Number::round(sqrt(along_var_),2), // "ALSDev"
         num_along_ok_, // "#ALOK"
         num_along_nok_, // "#ALNOK"
         p_along_var, // "PALOK"
         Number::round(latency_avg_,2), // "ALAvg"
         Number::round(across_avg_,2), // "ACAvg"
         Number::round(sqrt(across_var_),2), // "ACSDev"
         num_across_ok_, // "#ACOK"
         num_across_nok_, // "#ACNOK"
         p_across_var}, // "PACOK"
                    this, {utn_});
    }

    void SinglePositionAlongAcross::addTargetDetailsToTableADSB (
            EvaluationResultsReport::SectionContentTable& target_table)
    {
        QVariant p_along_var;

        if (has_p_min_along_)
            p_along_var = roundf(has_p_min_along_ * 10000.0) / 100.0;

        QVariant p_across_var;

        if (has_p_min_across_)
            p_across_var = roundf(has_p_min_across_ * 10000.0) / 100.0;

        // "UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
        // "#ACOK", "#ACNOK", "PACOK", "#ALOK", "#ALNOK", "PALOK", "MOPS", "NUCp/NIC", "NACp"

        target_table.addRow(
        {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
         target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
         target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(),
         target_->modeCMaxStr().c_str(),
         Number::round(along_avg_,2), // "ALAvg"
         Number::round(sqrt(along_var_),2), // "ALSDev"
         num_along_ok_, // "#ALOK"
         num_along_nok_, // "#ALNOK"
         p_along_var, // "PALOK"
         Number::round(latency_avg_,2), // "ALAvg"
         Number::round(across_avg_,2), // "ACAvg"
         Number::round(sqrt(across_var_),2), // "ACSDev"
         num_across_ok_, // "#ACOK"
         num_across_nok_, // "#ACNOK"
         p_across_var, // "PACOK"
         target_->mopsVersionsStr().c_str(), // "MOPS"
         target_->nucpNicStr().c_str(), // "NUCp/NIC"
         target_->nacpStr().c_str()}, // "NACp"
                    this, {utn_});

    }

    void SinglePositionAlongAcross::addTargetDetailsToReport(shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        EvaluationResultsReport::Section& utn_req_section = root_item->getSection(getTargetRequirementSectionID());

        if (!utn_req_section.hasTable("details_overview_table"))
            utn_req_section.addTable("details_overview_table", 3, {"Name", "comment", "Value"}, false);

        std::shared_ptr<EvaluationRequirement::PositionAlongAcross> req =
                std::static_pointer_cast<EvaluationRequirement::PositionAlongAcross>(requirement_);
        assert (req);

        EvaluationResultsReport::SectionContentTable& utn_req_table =
                utn_req_section.getTable("details_overview_table");

        addCommonDetails(root_item);

        // "UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
        // "#ACOK", "#ACNOK", "PACOK", "#ALOK", "#ALNOK", "PALOK"

        utn_req_table.addRow({"Use", "To be used in results", use_}, this);
        utn_req_table.addRow({"#Pos [1]", "Number of updates", num_pos_}, this);
        utn_req_table.addRow({"#NoRef [1]", "Number of updates w/o reference positions", num_no_ref_}, this);
        utn_req_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
        utn_req_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_pos_outside_}, this);

        // along
        utn_req_table.addRow({"Min Along [m]", "Minimum of along-track error",
                              String::doubleToStringPrecision(along_min_,2).c_str()}, this);
        utn_req_table.addRow({"Max Along [m]", "Maximum of along-track error",
                              String::doubleToStringPrecision(along_max_,2).c_str()}, this);
        utn_req_table.addRow({"ALAvg [m]", "Average of along-track error",
                              String::doubleToStringPrecision(along_avg_,2).c_str()}, this);
        utn_req_table.addRow({"ALSDev Along [m]", "Standard Deviation of along-track error",
                              String::doubleToStringPrecision(sqrt(along_var_),2).c_str()}, this);
        utn_req_table.addRow({"Variance Along [m]", "Variance of along-track error",
                              String::doubleToStringPrecision(along_var_,2).c_str()}, this);
        utn_req_table.addRow({"#ALOK [1]", "Number of updates with along-track error", num_along_ok_}, this);
        utn_req_table.addRow({"#ALNOK [1]", "Number of updates with unacceptable along-track error ", num_along_nok_},
                             this);

        // condition
        {
            QVariant pd_var;

            if (has_p_min_along_)
                pd_var = String::percentToString(p_min_along_ * 100.0).c_str();

            utn_req_table.addRow({"PALOK [%]", "Probability of acceptable along-track error", pd_var}, this);

            string condition = ">= "+String::percentToString(req->minimumProbability() * 100.0);

            utn_req_table.addRow({"Condition Along", {}, condition.c_str()}, this);

            string result {"Unknown"};

            if (has_p_min_along_)
                result = p_min_along_ >= req->minimumProbability() ? "Passed" : "Failed";

            utn_req_table.addRow({"Condition Along Fulfilled", "", result.c_str()}, this);
        }

        // latency
        utn_req_table.addRow({"Min Latency [s]", "Minimum of latency",
                              String::timeStringFromDouble(latency_min_).c_str()}, this);
        utn_req_table.addRow({"Max Latency [s]", "Maximum of latency",
                              String::timeStringFromDouble(latency_max_).c_str()}, this);
        utn_req_table.addRow({"ALAvg [s]", "Average of latency",
                              String::timeStringFromDouble(latency_avg_).c_str()}, this);
        utn_req_table.addRow({"ALSDev Latency [s]", "Standard Deviation of latency",
                              String::timeStringFromDouble(sqrt(latency_var_)).c_str()}, this);
        utn_req_table.addRow({"Variance Latency [s]", "Variance of latency",
                              String::timeStringFromDouble(latency_var_).c_str()}, this);

        // across
        utn_req_table.addRow({"Min Across [m]", "Minimum of across-track error",
                              String::doubleToStringPrecision(across_min_,2).c_str()}, this);
        utn_req_table.addRow({"Max Across [m]", "Maximum of across-track error",
                              String::doubleToStringPrecision(across_max_,2).c_str()}, this);
        utn_req_table.addRow({"ACAvg [m]", "Average of across-track error",
                              String::doubleToStringPrecision(across_avg_,2).c_str()}, this);
        utn_req_table.addRow({"ACSDev Across [m]", "Standard Deviation of across-track error",
                              String::doubleToStringPrecision(sqrt(across_var_),2).c_str()}, this);
        utn_req_table.addRow({"Variance Across [m^2]", "Variance of across-track error",
                              String::doubleToStringPrecision(across_var_,2).c_str()}, this);

        utn_req_table.addRow({"#ACOK [1]", "Number of updates with across-track error", num_across_ok_}, this);
        utn_req_table.addRow({"#ACNOK [1]", "Number of updates with unacceptable across-track error ", num_across_nok_},
                             this);

        // condition
        {
            QVariant pd_var;

            if (has_p_min_across_)
                pd_var = String::percentToString(p_min_across_ * 100.0).c_str();

            utn_req_table.addRow({"PACOK [%]", "Probability of acceptable across-track error", pd_var}, this);

            string condition = ">= "+String::percentToString(req->minimumProbability() * 100.0);

            utn_req_table.addRow({"Condition Across", {}, condition.c_str()}, this);

            string result {"Unknown"};

            if (has_p_min_across_)
                result = p_min_across_ >= req->minimumProbability() ? "Passed" : "Failed";

            utn_req_table.addRow({"Condition Across", "", result.c_str()}, this);
        }



        if ((has_p_min_along_ && p_min_along_ != 1.0)
                || (has_p_min_across_ && p_min_across_ != 1.0))
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

    void SinglePositionAlongAcross::reportDetails(EvaluationResultsReport::Section& utn_req_section)
    {
        //        if (!utn_req_section.hasTable(tr_details_table_name_))
        //            utn_req_section.addTable(tr_details_table_name_, 12,
        //            {"ToD", "NoRef", "PosInside", "Distance", "PosOK", "#Pos", "#NoRef",
        //             "#PosInside", "#PosOutside", "#PosOK", "#PosNOK", "Comment"});

        //        EvaluationResultsReport::SectionContentTable& utn_req_details_table =
        //                utn_req_section.getTable(tr_details_table_name_);

        //        unsigned int detail_cnt = 0;

        //        for (auto& rq_det_it : details_)
        //        {
        //            utn_req_details_table.addRow(
        //            {String::timeStringFromDouble(rq_det_it.tod_).c_str(),
        //             !rq_det_it.has_ref_pos_, rq_det_it.pos_inside_, rq_det_it.distance_, rq_det_it.pos_ok_,
        //             rq_det_it.num_pos_, rq_det_it.num_no_ref_,
        //             rq_det_it.num_inside_, rq_det_it.num_outside_, rq_det_it.num_pos_ok_, rq_det_it.num_pos_nok_,
        //             rq_det_it.comment_.c_str()},
        //                        this, detail_cnt);

        //            ++detail_cnt;
        //        }
    }

    bool SinglePositionAlongAcross::hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
            return true;
        else if (table.name() == tr_details_table_name_ && annotation.isValid() && annotation.toUInt() < details_.size())
            return true;
        else
            return false;
    }

    std::unique_ptr<nlohmann::json::object_t> SinglePositionAlongAcross::viewableData(
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

            loginf << "SinglePositionAlongAcross: viewableData: detail_cnt " << detail_cnt;

            std::unique_ptr<nlohmann::json::object_t> viewable_ptr
                    = eval_man_.getViewableForEvaluation(utn_, req_grp_id_, result_id_);
            assert (viewable_ptr);

            const EvaluationRequirement::PositionAlongAcrossDetail& detail = details_.at(detail_cnt);

            (*viewable_ptr)["position_latitude"] = detail.tst_pos_.latitude_;
            (*viewable_ptr)["position_longitude"] = detail.tst_pos_.longitude_;
            (*viewable_ptr)["position_window_latitude"] = 0.02;
            (*viewable_ptr)["position_window_longitude"] = 0.02;
            (*viewable_ptr)["time"] = detail.tod_;

            if (!detail.distance_along_ok_ || !detail.distance_across_ok_)
                (*viewable_ptr)["evaluation_results"]["highlight_details"] = vector<unsigned int>{detail_cnt};

            return viewable_ptr;
        }
        else
            return nullptr;
    }

    std::unique_ptr<nlohmann::json::object_t> SinglePositionAlongAcross::getTargetErrorsViewable ()
    {
        std::unique_ptr<nlohmann::json::object_t> viewable_ptr = eval_man_.getViewableForEvaluation(
                    utn_, req_grp_id_, result_id_);

        bool has_pos = false;
        double lat_min, lat_max, lon_min, lon_max;

        for (auto& detail_it : details_)
        {
            if (detail_it.distance_along_ok_ && detail_it.distance_across_ok_)
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

    bool SinglePositionAlongAcross::hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
            return true;
        else
            return false;;
    }

    std::string SinglePositionAlongAcross::reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        assert (hasReference(table, annotation));

        return "Report:Results:"+getTargetRequirementSectionID();
    }

    unsigned int SinglePositionAlongAcross::numAlongOk() const
    {
        return num_along_ok_;
    }

    unsigned int SinglePositionAlongAcross::numAlongNOk() const
    {
        return num_along_nok_;
    }

    unsigned int SinglePositionAlongAcross::numAcrossOk() const
    {
        return num_across_ok_;
    }

    unsigned int SinglePositionAlongAcross::numAcrossNOk() const
    {
        return num_across_nok_;
    }

    const tuple<vector<double>, vector<double>, vector<double>, vector<double>, vector<double>>&
    SinglePositionAlongAcross::distanceValues() const
    {
        return distance_values_;
    }

    unsigned int SinglePositionAlongAcross::numPosOutside() const
    {
        return num_pos_outside_;
    }

    unsigned int SinglePositionAlongAcross::numPosInside() const
    {
        return num_pos_inside_;
    }

    std::shared_ptr<Joined> SinglePositionAlongAcross::createEmptyJoined(const std::string& result_id)
    {
        return make_shared<JoinedPositionAlongAcross> (result_id, requirement_, sector_layer_, eval_man_);
    }

    unsigned int SinglePositionAlongAcross::numPos() const
    {
        return num_pos_;
    }

    unsigned int SinglePositionAlongAcross::numNoRef() const
    {
        return num_no_ref_;
    }

    std::vector<EvaluationRequirement::PositionAlongAcrossDetail>& SinglePositionAlongAcross::details()
    {
        return details_;
    }
}
