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
#include "eval/requirement/detection/detection.h"
#include "evaluationtargetdata.h"
#include "evaluationmanager.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "logger.h"
#include "stringconv.h"

#include <algorithm>
#include <cassert>

using namespace std;
using namespace Utils;

namespace EvaluationRequirementResult
{

    JoinedPositionAlongAcross::JoinedPositionAlongAcross(
            const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
            const SectorLayer& sector_layer, EvaluationManager& eval_man)
        : Joined("JoinedPositionAlongAcross", result_id, requirement, sector_layer, eval_man)
    {
    }


    void JoinedPositionAlongAcross::join(std::shared_ptr<Base> other)
    {
        Joined::join(other);

        std::shared_ptr<SinglePositionAlongAcross> other_sub =
                std::static_pointer_cast<SinglePositionAlongAcross>(other);
        assert (other_sub);

        addToValues(other_sub);
    }

    void JoinedPositionAlongAcross::addToValues (std::shared_ptr<SinglePositionAlongAcross> single_result)
    {
        assert (single_result);

        if (!single_result->use())
            return;

        num_pos_ += single_result->numPos();
        num_no_ref_ += single_result->numNoRef();
        num_pos_outside_ += single_result->numPosOutside();
        num_pos_inside_ += single_result->numPosInside();
        num_along_ok_ += single_result->numAlongOk();
        num_along_nok_ += single_result->numAlongNOk();
        num_across_ok_ += single_result->numAcrossOk();
        num_across_nok_ += single_result->numAcrossNOk();

        const tuple<vector<double>, vector<double>, vector<double>, vector<double>, vector<double>>&
                other_distance_values = single_result->distanceValues();

        get<0>(distance_values_).insert(get<0>(distance_values_).end(),
                                        get<0>(other_distance_values).begin(), get<0>(other_distance_values).end());
        get<1>(distance_values_).insert(get<1>(distance_values_).end(),
                                        get<1>(other_distance_values).begin(), get<1>(other_distance_values).end());
        get<2>(distance_values_).insert(get<2>(distance_values_).end(),
                                        get<2>(other_distance_values).begin(), get<2>(other_distance_values).end());
        get<3>(distance_values_).insert(get<3>(distance_values_).end(),
                                        get<3>(other_distance_values).begin(), get<3>(other_distance_values).end());
        get<4>(distance_values_).insert(get<4>(distance_values_).end(),
                                        get<4>(other_distance_values).begin(), get<4>(other_distance_values).end());
        update();
    }

    void JoinedPositionAlongAcross::update()
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
            along_min_ = *min_element(along_vals.begin(), along_vals.end());;
            along_max_ = *max_element(along_vals.begin(), along_vals.end());;
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

            assert (num_along_ok_ <= num_distances);
            p_min_along_ = (float)num_along_ok_/(float)num_distances;
            has_p_min_along_ = true;

            assert (num_across_ok_ <= num_distances);
            p_min_across_ = (float)num_across_ok_/(float)num_distances;
            has_p_min_across_ = true;
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

            has_p_min_across_ = false;
            p_min_across_ = 0;
        }
    }

    void JoinedPositionAlongAcross::addToReport (
            std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        logdbg << "JoinedPositionAlongAcross " <<  requirement_->name() <<": addToReport";

        if (!results_.size()) // some data must exist
        {
            logerr << "JoinedPositionAlongAcross " <<  requirement_->name() <<": addToReport: no data";
            return;
        }

        logdbg << "JoinedPositionAlongAcross " <<  requirement_->name() << ": addToReport: adding joined result";

        addToOverviewTable(root_item);
        addDetails(root_item);
    }

    void JoinedPositionAlongAcross::addToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        EvaluationResultsReport::SectionContentTable& ov_table = getReqOverviewTable(root_item);

        // condition
        std::shared_ptr<EvaluationRequirement::PositionAlongAcross> req =
                std::static_pointer_cast<EvaluationRequirement::PositionAlongAcross>(requirement_);
        assert (req);

        string condition = ">= "+String::percentToString(req->minimumProbability() * 100.0);

        // along
        {
            QVariant pd_var;

            string result {"Unknown"};

            if (has_p_min_along_)
            {
                pd_var = String::percentToString(p_min_along_ * 100.0).c_str();

                result = p_min_along_ >= req->minimumProbability() ? "Passed" : "Failed";
            }

            // "Sector Layer", "Group", "Req.", "Id", "#Updates", "Result", "Condition", "Result"
            ov_table.addRow({sector_layer_.name().c_str(), requirement_->groupName().c_str(),
                             +(requirement_->shortname()+" Along").c_str(),
                             result_id_.c_str(), {num_along_ok_+num_along_nok_},
                             pd_var, condition.c_str(), result.c_str()}, this, {});
        }

        // across
        {
            QVariant pd_var;

            string result {"Unknown"};

            if (has_p_min_across_)
            {
                pd_var = String::percentToString(p_min_across_ * 100.0).c_str();

                result = p_min_across_ >= req->minimumProbability() ? "Passed" : "Failed";
            }

            // "Sector Layer", "Group", "Req.", "Id", "#Updates", "Result", "Condition", "Result"
            ov_table.addRow({sector_layer_.name().c_str(), requirement_->groupName().c_str(),
                             +(requirement_->shortname()+" Across").c_str(),
                             result_id_.c_str(), {num_across_ok_+num_across_nok_},
                             pd_var, condition.c_str(), result.c_str()}, this, {});
        }
    }

    void JoinedPositionAlongAcross::addDetails(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        EvaluationResultsReport::Section& sector_section = getRequirementSection(root_item);

        if (!sector_section.hasTable("sector_details_table"))
            sector_section.addTable("sector_details_table", 3, {"Name", "comment", "Value"}, false);

        std::shared_ptr<EvaluationRequirement::PositionAlongAcross> req =
                std::static_pointer_cast<EvaluationRequirement::PositionAlongAcross>(requirement_);
        assert (req);

        EvaluationResultsReport::SectionContentTable& sec_det_table =
                sector_section.getTable("sector_details_table");

        addCommonDetails(sec_det_table);

        sec_det_table.addRow({"Use", "To be used in results", use_}, this);
        sec_det_table.addRow({"#Pos [1]", "Number of updates", num_pos_}, this);
        sec_det_table.addRow({"#NoRef [1]", "Number of updates w/o reference positions", num_no_ref_}, this);
        sec_det_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
        sec_det_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_pos_outside_}, this);

        // along
        sec_det_table.addRow({"Min Along [m]", "Minimum of along-track error",
                              String::doubleToStringPrecision(along_min_,2).c_str()}, this);
        sec_det_table.addRow({"Max Along [m]", "Maximum of along-track error",
                              String::doubleToStringPrecision(along_max_,2).c_str()}, this);
        sec_det_table.addRow({"Average Along [m]", "Average of along-track error",
                              String::doubleToStringPrecision(along_avg_,2).c_str()}, this);
        sec_det_table.addRow({"Standard Deviation Along [m]", "Standard Deviation of along-track error",
                              String::doubleToStringPrecision(sqrt(along_var_),2).c_str()}, this);
        sec_det_table.addRow({"Variance Along [m^2]", "Variance of along-track error",
                              String::doubleToStringPrecision(along_var_,2).c_str()}, this);
        sec_det_table.addRow({"#ALOK [1]", "Number of updates with along-track error", num_along_ok_}, this);
        sec_det_table.addRow({"#ALNOK [1]", "Number of updates with unacceptable along-track error ", num_along_nok_},
                             this);


        // condition
        {
            QVariant pd_var;

            if (has_p_min_along_)
                pd_var = String::percentToString(p_min_along_ * 100.0).c_str();

            sec_det_table.addRow({"PALOK [%]", "Probability of acceptable along-track error", pd_var}, this);

            string condition = ">= "+String::percentToString(req->minimumProbability() * 100.0);

            sec_det_table.addRow({"Condition Along", {}, condition.c_str()}, this);

            string result {"Unknown"};

            if (has_p_min_along_)
                result = p_min_along_ >= req->minimumProbability() ? "Passed" : "Failed";

            sec_det_table.addRow({"Condition Along Fulfilled", "", result.c_str()}, this);
        }

        sec_det_table.addRow({"#ALOK [1]", "Number of updates with across-track error", num_across_ok_}, this);
        sec_det_table.addRow({"#ALNOK [1]", "Number of updates with unacceptable across-track error ", num_across_nok_},
                             this);

        // latency
        sec_det_table.addRow({"Min Latency [s]", "Minimum of latency",
                              String::timeStringFromDouble(latency_min_).c_str()}, this);
        sec_det_table.addRow({"Max Latency [s]", "Maximum of latency",
                              String::timeStringFromDouble(latency_max_).c_str()}, this);
        sec_det_table.addRow({"LAvg [s]", "Average of latency",
                              String::timeStringFromDouble(latency_avg_).c_str()}, this);
        sec_det_table.addRow({"LSDev Latency [s]", "Standard Deviation of latency",
                              String::timeStringFromDouble(sqrt(latency_var_)).c_str()}, this);
        sec_det_table.addRow({"Variance Latency [s]", "Variance of latency",
                              String::timeStringFromDouble(latency_var_).c_str()}, this);

        // across
        sec_det_table.addRow({"Min Across [m]", "Minimum of across-track error",
                              String::doubleToStringPrecision(across_min_,2).c_str()}, this);
        sec_det_table.addRow({"Max Across [m]", "Maximum of across-track error",
                              String::doubleToStringPrecision(across_max_,2).c_str()}, this);
        sec_det_table.addRow({"Average Across [m]", "Average of across-track error",
                              String::doubleToStringPrecision(across_avg_,2).c_str()}, this);
        sec_det_table.addRow({"Standard Deviation Across [m]", "Standard Deviation of across-track error",
                              String::doubleToStringPrecision(sqrt(across_var_),2).c_str()}, this);
        sec_det_table.addRow({"Variance Across [m^2]", "Variance of across-track error",
                              String::doubleToStringPrecision(across_var_,2).c_str()}, this);

        sec_det_table.addRow({"#ACOK [1]", "Number of updates with across-track error", num_across_ok_}, this);
        sec_det_table.addRow({"#ACNOK [1]", "Number of updates with unacceptable across-track error ", num_across_nok_},
                             this);

        // condition
        {
            QVariant pd_var;

            if (has_p_min_across_)
                pd_var = String::percentToString(p_min_across_ * 100.0).c_str();

            sec_det_table.addRow({"PACOK [%]", "Probability of acceptable across-track error", pd_var}, this);

            string condition = ">= "+String::percentToString(req->minimumProbability() * 100.0);

            sec_det_table.addRow({"Condition Across", {}, condition.c_str()}, this);

            string result {"Unknown"};

            if (has_p_min_across_)
                result = p_min_across_ >= req->minimumProbability() ? "Passed" : "Failed";

            sec_det_table.addRow({"Condition Across", "", result.c_str()}, this);
        }

        // figure
        if ((has_p_min_along_ && p_min_along_ != 1.0)
                || (has_p_min_across_ && p_min_across_ != 1.0))
        {
            sector_section.addFigure("sector_errors_overview", "Sector Errors Overview",
                                     getErrorsViewable());
        }
        else
        {
            sector_section.addText("sector_errors_overview_no_figure");
            sector_section.getText("sector_errors_overview_no_figure").addText(
                        "No target errors found, therefore no figure was generated.");
        }
    }

    bool JoinedPositionAlongAcross::hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        if (table.name() == req_overview_table_name_)
            return true;
        else
            return false;
    }

    std::unique_ptr<nlohmann::json::object_t> JoinedPositionAlongAcross::viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        assert (hasViewableData(table, annotation));

        return getErrorsViewable();
    }

    std::unique_ptr<nlohmann::json::object_t> JoinedPositionAlongAcross::getErrorsViewable ()
    {
        std::unique_ptr<nlohmann::json::object_t> viewable_ptr =
                eval_man_.getViewableForEvaluation(req_grp_id_, result_id_);

        double lat_min, lat_max, lon_min, lon_max;

        tie(lat_min, lat_max) = sector_layer_.getMinMaxLatitude();
        tie(lon_min, lon_max) = sector_layer_.getMinMaxLongitude();

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

        return viewable_ptr;
    }

    bool JoinedPositionAlongAcross::hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        if (table.name() == req_overview_table_name_)
            return true;
        else
            return false;;
    }

    std::string JoinedPositionAlongAcross::reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        assert (hasReference(table, annotation));
        return "Report:Results:"+getRequirementSectionID();
    }

    void JoinedPositionAlongAcross::updatesToUseChanges()
    {
        loginf << "JoinedPositionAlongAcross: updatesToUseChanges";

        num_pos_ = 0;
        num_no_ref_ = 0;
        num_pos_outside_ = 0;
        num_pos_inside_ = 0;
        num_along_ok_ = 0;
        num_along_nok_ = 0;
        num_across_ok_ = 0;
        num_across_nok_ = 0;

        get<0>(distance_values_).clear();
        get<1>(distance_values_).clear();
        get<2>(distance_values_).clear();
        get<3>(distance_values_).clear();
        get<4>(distance_values_).clear();

        for (auto result_it : results_)
        {
            std::shared_ptr<SinglePositionAlongAcross> result =
                    std::static_pointer_cast<SinglePositionAlongAcross>(result_it);
            assert (result);

            addToValues(result);
        }
    }

}
