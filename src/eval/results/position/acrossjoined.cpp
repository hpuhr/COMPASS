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

#include "eval/results/position/acrosssingle.h"
#include "eval/results/position/acrossjoined.h"
#include "eval/requirement/base/base.h"
#include "evaluationtargetdata.h"
#include "evaluationmanager.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "logger.h"
#include "stringconv.h"

#include <QFileDialog>

#include <algorithm>
#include <cassert>
#include <fstream>

using namespace std;
using namespace Utils;

namespace EvaluationRequirementResult
{

JoinedPositionAcross::JoinedPositionAcross(
        const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
        const SectorLayer& sector_layer, EvaluationManager& eval_man)
    : Joined("JoinedPositionAcross", result_id, requirement, sector_layer, eval_man)
{
}

void JoinedPositionAcross::join(std::shared_ptr<Base> other)
{
    Joined::join(other);

    std::shared_ptr<SinglePositionAcross> other_sub =
            std::static_pointer_cast<SinglePositionAcross>(other);
    assert (other_sub);

    addToValues(other_sub);
}

void JoinedPositionAcross::addToValues (std::shared_ptr<SinglePositionAcross> single_result)
{
    assert (single_result);

    if (!single_result->use())
        return;

    num_pos_ += single_result->numPos();
    num_no_ref_ += single_result->numNoRef();
    num_pos_outside_ += single_result->numPosOutside();
    num_pos_inside_ += single_result->numPosInside();
    num_value_ok_ += single_result->numValueOk();
    num_value_nok_ += single_result->numValueNOk();

    const vector<double>& other_values = single_result->values();

    values_.insert(values_.end(), other_values.begin(), other_values.end());

    update();
}

void JoinedPositionAcross::update()
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
    }
    else
    {
        value_min_ = 0;
        value_max_ = 0;
        value_avg_ = 0;
        value_var_ = 0;

        has_p_min_ = false;
        p_min_ = 0;
    }
}

void JoinedPositionAcross::addToReport (
        std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "JoinedPositionAcross " <<  requirement_->name() <<": addToReport";

    if (!results_.size()) // some data must exist
    {
        logerr << "JoinedPositionAcross " <<  requirement_->name() <<": addToReport: no data";
        return;
    }

    logdbg << "JoinedPositionAcross " <<  requirement_->name() << ": addToReport: adding joined result";

    addToOverviewTable(root_item);
    addDetails(root_item);
}

void JoinedPositionAcross::addToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::SectionContentTable& ov_table = getReqOverviewTable(root_item);

    // condition
    std::shared_ptr<EvaluationRequirement::PositionAcross> req =
            std::static_pointer_cast<EvaluationRequirement::PositionAcross>(requirement_);
    assert (req);

    QVariant p_min_var;

    string result {"Unknown"};

    if (has_p_min_)
    {
        p_min_var = String::percentToString(p_min_ * 100.0).c_str();
        result = req->getResultConditionStr(p_min_);
    }

    // "Sector Layer", "Group", "Req.", "Id", "#Updates", "Result", "Condition", "Result"
    ov_table.addRow({sector_layer_.name().c_str(), requirement_->groupName().c_str(),
                     +(requirement_->shortname()+" Across").c_str(),
                     result_id_.c_str(), {num_value_ok_+num_value_nok_},
                     p_min_var, req->getConditionStr().c_str(), result.c_str()}, this, {});
}

void JoinedPositionAcross::addDetails(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::Section& sector_section = getRequirementSection(root_item);

    if (!sector_section.hasTable("sector_details_table"))
        sector_section.addTable("sector_details_table", 3, {"Name", "comment", "Value"}, false);

    std::shared_ptr<EvaluationRequirement::PositionAcross> req =
            std::static_pointer_cast<EvaluationRequirement::PositionAcross>(requirement_);
    assert (req);

    EvaluationResultsReport::SectionContentTable& sec_det_table =
            sector_section.getTable("sector_details_table");

    // callbacks
    auto exportAsCSV_lambda = [this]() {
        this->exportAsCSV();
    };

    sec_det_table.registerCallBack("Save Data As CSV", exportAsCSV_lambda);

    // details
    addCommonDetails(sec_det_table);

    sec_det_table.addRow({"Use", "To be used in results", use_}, this);
    sec_det_table.addRow({"#Pos [1]", "Number of updates", num_pos_}, this);
    sec_det_table.addRow({"#NoRef [1]", "Number of updates w/o reference positions", num_no_ref_}, this);
    sec_det_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
    sec_det_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_pos_outside_}, this);

    // along
    sec_det_table.addRow({"ACMin [m]", "Minimum of across-track error",
                          String::doubleToStringPrecision(value_min_,2).c_str()}, this);
    sec_det_table.addRow({"ACMax [m]", "Maximum of across-track error",
                          String::doubleToStringPrecision(value_max_,2).c_str()}, this);
    sec_det_table.addRow({"ACAvg [m]", "Average of across-track error",
                          String::doubleToStringPrecision(value_avg_,2).c_str()}, this);
    sec_det_table.addRow({"ACSDev [m]", "Standard Deviation of across-track error",
                          String::doubleToStringPrecision(sqrt(value_var_),2).c_str()}, this);
    sec_det_table.addRow({"ACVar [m^2]", "Variance of across-track error",
                          String::doubleToStringPrecision(value_var_,2).c_str()}, this);
    sec_det_table.addRow({"#ACOK [1]", "Number of updates with across-track error", num_value_ok_}, this);
    sec_det_table.addRow({"#ACNOK [1]", "Number of updates with unacceptable across-track error ", num_value_nok_},
                         this);


    // condition
    {
        QVariant p_min_var;

        if (has_p_min_)
            p_min_var = roundf(p_min_ * 10000.0) / 100.0;

        sec_det_table.addRow({"PACOK [%]", "Probability of acceptable across-track error", p_min_var}, this);

        sec_det_table.addRow({"Condition Across", {}, req->getConditionStr().c_str()}, this);

        string result {"Unknown"};

        if (has_p_min_)
            result = req->getResultConditionStr(p_min_);

        sec_det_table.addRow({"Condition Across Fulfilled", "", result.c_str()}, this);
    }

    // figure
    if (has_p_min_ && p_min_ != 1.0)
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

bool JoinedPositionAcross::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == req_overview_table_name_)
        return true;
    else
        return false;
}

std::unique_ptr<nlohmann::json::object_t> JoinedPositionAcross::viewableData(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasViewableData(table, annotation));

    return getErrorsViewable();
}

std::unique_ptr<nlohmann::json::object_t> JoinedPositionAcross::getErrorsViewable ()
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

    if (lat_w < eval_man_.resultDetailZoom())
        lat_w = eval_man_.resultDetailZoom();

    if (lon_w < eval_man_.resultDetailZoom())
        lon_w = eval_man_.resultDetailZoom();

    (*viewable_ptr)["position_window_latitude"] = lat_w;
    (*viewable_ptr)["position_window_longitude"] = lon_w;

    return viewable_ptr;
}

bool JoinedPositionAcross::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == req_overview_table_name_)
        return true;
    else
        return false;;
}

std::string JoinedPositionAcross::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasReference(table, annotation));
    return "Report:Results:"+getRequirementSectionID();
}

void JoinedPositionAcross::updatesToUseChanges()
{
    loginf << "JoinedPositionAcross: updatesToUseChanges";

    num_pos_ = 0;
    num_no_ref_ = 0;
    num_pos_outside_ = 0;
    num_pos_inside_ = 0;
    num_value_ok_ = 0;
    num_value_nok_ = 0;

    values_.clear();

    for (auto result_it : results_)
    {
        std::shared_ptr<SinglePositionAcross> result =
                std::static_pointer_cast<SinglePositionAcross>(result_it);
        assert (result);

        addToValues(result);
    }
}

void JoinedPositionAcross::exportAsCSV()
{
    loginf << "JoinedPositionAcross: exportAsCSV";

    QFileDialog dialog(nullptr);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter("CSV Files (*.csv)");
    dialog.setDefaultSuffix("csv");
    dialog.setAcceptMode(QFileDialog::AcceptMode::AcceptSave);

    if (dialog.exec())
    {
        QStringList file_names = dialog.selectedFiles();
        assert (file_names.size() == 1);

        string filename = file_names.at(0).toStdString();

        std::ofstream output_file;

        output_file.open(filename, std::ios_base::out);

        if (output_file)
        {
            output_file << "d_across\n";
            unsigned int size = values_.size();

            for (unsigned int cnt=0; cnt < size; ++cnt)
                output_file << values_.at(cnt) << "\n";
        }
    }
}

}
