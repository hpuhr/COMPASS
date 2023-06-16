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
#include "stringconv.h"

#include <QFileDialog>

#include <algorithm>
#include <cassert>
#include <fstream>

using namespace std;
using namespace Utils;

namespace EvaluationRequirementResult
{

JoinedSpeed::JoinedSpeed(const std::string& result_id, 
                         std::shared_ptr<EvaluationRequirement::Base> requirement,
                         const SectorLayer& sector_layer, 
                         EvaluationManager& eval_man)
:   Joined("JoinedSpeed", result_id, requirement, sector_layer, eval_man)
{
}

void JoinedSpeed::join_impl(std::shared_ptr<Single> other)
{
    std::shared_ptr<SingleSpeed> other_sub =
            std::static_pointer_cast<SingleSpeed>(other);
    assert (other_sub);

    addToValues(other_sub);
}

void JoinedSpeed::addToValues (std::shared_ptr<SingleSpeed> single_result)
{
    assert (single_result);

    if (!single_result->use())
        return;

    num_pos_          += single_result->numPos();
    num_no_ref_       += single_result->numNoRef();
    num_pos_outside_  += single_result->numPosOutside();
    num_pos_inside_   += single_result->numPosInside();
    num_no_tst_value_ += single_result->numNoTstValues();
    num_comp_failed_  += single_result->numCompFailed();
    num_comp_passed_  += single_result->numCompPassed();

    const vector<double>& other_values = single_result->values();

    values_.insert(values_.end(), other_values.begin(), other_values.end());

    update();
}

void JoinedSpeed::update()
{
    assert (num_no_ref_ <= num_pos_);
    assert (num_pos_ - num_no_ref_ == num_pos_inside_ + num_pos_outside_);

    assert (values_.size() == num_comp_failed_+num_comp_passed_);

    p_passed_.reset();

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
    }
    else
    {
        value_min_ = 0;
        value_max_ = 0;
        value_avg_ = 0;
        value_var_ = 0;
    }
}

void JoinedSpeed::addToReport (
        std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "JoinedSpeed " <<  requirement_->name() <<": addToReport";

    if (!results_.size()) // some data must exist
    {
        logerr << "JoinedSpeed " <<  requirement_->name() <<": addToReport: no data";
        return;
    }

    logdbg << "JoinedSpeed " <<  requirement_->name() << ": addToReport: adding joined result";

    addToOverviewTable(root_item);
    addDetails(root_item);
}

void JoinedSpeed::addToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::SectionContentTable& ov_table = getReqOverviewTable(root_item);

    // condition
    std::shared_ptr<EvaluationRequirement::Speed> req =
            std::static_pointer_cast<EvaluationRequirement::Speed>(requirement_);
    assert (req);

    QVariant p_passed_var;

    string result {"Unknown"};

    if (p_passed_.has_value())
    {
        p_passed_var = String::percentToString(p_passed_.value() * 100.0, req->getNumProbDecimals()).c_str();

        result = req->getConditionResultStr(p_passed_.value());
    }

    // "Sector Layer", "Group", "Req.", "Id", "#Updates", "Result", "Condition", "Result"
    ov_table.addRow({sector_layer_.name().c_str(), requirement_->groupName().c_str(),
                     +(requirement_->shortname()).c_str(),
                     result_id_.c_str(), {num_comp_failed_+num_comp_passed_},
                     p_passed_var, req->getConditionStr().c_str(), result.c_str()}, this, {});
}

void JoinedSpeed::addDetails(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::Section& sector_section = getRequirementSection(root_item);

    if (!sector_section.hasTable("sector_details_table"))
        sector_section.addTable("sector_details_table", 3, {"Name", "comment", "Value"}, false);

    std::shared_ptr<EvaluationRequirement::Speed> req =
            std::static_pointer_cast<EvaluationRequirement::Speed>(requirement_);
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
    sec_det_table.addRow({"#NoRef [1]", "Number of updates w/o reference speeds", num_no_ref_}, this);
    sec_det_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
    sec_det_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_pos_outside_}, this);
    sec_det_table.addRow({"#NoTstData [1]", "Number of updates without tst speed data", num_no_tst_value_}, this);

    // along
    sec_det_table.addRow({"OMin [m/s]", "Minimum of speed offset",
                          String::doubleToStringPrecision(value_min_,2).c_str()}, this);
    sec_det_table.addRow({"OMax [m/s]", "Maximum of speed offset",
                          String::doubleToStringPrecision(value_max_,2).c_str()}, this);
    sec_det_table.addRow({"OAvg [m/s]", "Average of speed offset",
                          String::doubleToStringPrecision(value_avg_,2).c_str()}, this);
    sec_det_table.addRow({"OSDev [m/s]", "Standard Deviation of speed offset",
                          String::doubleToStringPrecision(sqrt(value_var_),2).c_str()}, this);
    sec_det_table.addRow({"OVar [m^2/s^2]", "Variance of speed offset",
                          String::doubleToStringPrecision(value_var_,2).c_str()}, this);
    sec_det_table.addRow({"#CF [1]", "Number of updates with failed comparison", num_comp_failed_}, this);
    sec_det_table.addRow({"#CP [1]", "Number of updates with passed comparison ", num_comp_passed_},
                         this);
    // condition
    {
        QVariant p_passed_var;

        if (p_passed_.has_value())
            p_passed_var = roundf(p_passed_.value() * 10000.0) / 100.0;

        sec_det_table.addRow({"PCP [%]", "Probability of passed comparison", p_passed_var}, this);

        sec_det_table.addRow({"Condition", {}, req->getConditionStr().c_str()}, this);

        string result {"Unknown"};

        if (p_passed_.has_value())
            result = req->getConditionResultStr(p_passed_.value());

        sec_det_table.addRow({"Condition Fulfilled", "", result.c_str()}, this);
    }

    // figure
    sector_section.addFigure("sector_overview", "Sector Overview",
                             [this](void) { return this->getErrorsViewable(); });
}

bool JoinedSpeed::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == req_overview_table_name_)
        return true;
    else
        return false;
}

std::unique_ptr<nlohmann::json::object_t> JoinedSpeed::viewableData(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasViewableData(table, annotation));

    return getErrorsViewable();
}

std::unique_ptr<nlohmann::json::object_t> JoinedSpeed::getErrorsViewable ()
{
    std::unique_ptr<nlohmann::json::object_t> viewable_ptr =
            eval_man_.getViewableForEvaluation(req_grp_id_, result_id_);

    double lat_min, lat_max, lon_min, lon_max;

    tie(lat_min, lat_max) = sector_layer_.getMinMaxLatitude();
    tie(lon_min, lon_max) = sector_layer_.getMinMaxLongitude();

    (*viewable_ptr)[VP_POS_LAT_KEY] = (lat_max+lat_min)/2.0;
    (*viewable_ptr)[VP_POS_LON_KEY] = (lon_max+lon_min)/2.0;;

    double lat_w = 1.1*(lat_max-lat_min)/2.0;
    double lon_w = 1.1*(lon_max-lon_min)/2.0;

    if (lat_w < eval_man_.settings().result_detail_zoom_)
        lat_w = eval_man_.settings().result_detail_zoom_;

    if (lon_w < eval_man_.settings().result_detail_zoom_)
        lon_w = eval_man_.settings().result_detail_zoom_;

    (*viewable_ptr)["speed_window_latitude"] = lat_w;
    (*viewable_ptr)["speed_window_longitude"] = lon_w;

    addAnnotationsFromSingles(*viewable_ptr);

    return viewable_ptr;
}

bool JoinedSpeed::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == req_overview_table_name_)
        return true;
    else
        return false;;
}

std::string JoinedSpeed::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasReference(table, annotation));
    return "Report:Results:"+getRequirementSectionID();
}

void JoinedSpeed::updatesToUseChanges_impl()
{
    loginf << "JoinedSpeed: updatesToUseChanges";

    num_pos_          = 0;
    num_no_ref_       = 0;
    num_pos_outside_  = 0;
    num_pos_inside_   = 0;
    num_no_tst_value_ = 0;
    num_comp_failed_  = 0;
    num_comp_passed_  = 0;

    values_.clear();

    for (auto result_it : results_)
    {
        std::shared_ptr<SingleSpeed> result =
                std::static_pointer_cast<SingleSpeed>(result_it);
        assert (result);

        addToValues(result);
    }
}

void JoinedSpeed::exportAsCSV()
{
    loginf << "JoinedSpeed: exportAsCSV";

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
            output_file << "speed_offset\n";
            unsigned int size = values_.size();

            for (unsigned int cnt=0; cnt < size; ++cnt)
                output_file << values_.at(cnt) << "\n";
        }
    }
}

}
