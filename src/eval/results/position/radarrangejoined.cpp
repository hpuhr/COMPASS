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

#include "eval/results/position/radarrangesingle.h"
#include "eval/results/position/radarrangejoined.h"
#include "eval/requirement/base/base.h"
#include "eval/requirement/position/radarrange.h"
//#include "evaluationtargetdata.h"
#include "evaluationmanager.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
//#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "logger.h"
#include "stringconv.h"
#include "compass.h"
#include "viewpoint.h"
#include "sectorlayer.h"

#include <QFileDialog>

#include <Eigen/Dense>

#include <algorithm>
#include <cassert>
#include <fstream>

using namespace std;
using namespace Utils;

namespace EvaluationRequirementResult
{

JoinedPositionRadarRange::JoinedPositionRadarRange(const std::string& result_id,
                                                   std::shared_ptr<EvaluationRequirement::Base> requirement,
                                                   const SectorLayer& sector_layer,
                                                   EvaluationManager& eval_man)
    :   JoinedPositionBase("JoinedPositionRadarRange", result_id, requirement, sector_layer, eval_man)
{
}

void JoinedPositionRadarRange::updateToChanges_impl()
{
    JoinedPositionBase::updateToChanges_impl();

    assert (num_no_ref_ <= num_pos_);
    assert (num_pos_ - num_no_ref_ == num_pos_inside_ + num_pos_outside_);

    vector<double> all_values = values();

    if (all_values.size() != num_failed_ + num_passed_)
        logerr << "JoinedPositionRadarRange: update: wrong size all_values.size() " << all_values.size()
               << " num_failed_ " << num_failed_ << " num_passed_ " << num_passed_;

    assert (all_values.size() == num_failed_ + num_passed_);
    unsigned int num_distances = all_values.size();

    if (num_distances)
    {
        value_min_ = *min_element(all_values.begin(), all_values.end());
        value_max_ = *max_element(all_values.begin(), all_values.end());
        value_avg_ = std::accumulate(all_values.begin(), all_values.end(), 0.0) / (float) num_distances;

        value_var_ = 0;

        for(auto val : all_values)
        {
            value_var_ += pow(val - value_avg_, 2);
        }

        value_var_ /= (float)num_distances;

        value_rms_ = 0; // not used

        assert (num_passed_ <= num_distances);

        // linear regression

        vector<double> ref_range_values = refRangeValues();
        vector<double> tst_range_values = tstRangeValues();

        assert (all_values.size() == ref_range_values.size() && ref_range_values.size() == tst_range_values.size());

        Eigen::MatrixXd x_mat = Eigen::MatrixXd::Ones(num_distances, 2);
        Eigen::MatrixXd y_mat = Eigen::MatrixXd::Ones(num_distances, 1);

        for (unsigned int cnt=0; cnt < num_distances; ++cnt)
        {
            x_mat(cnt, 0) = tst_range_values.at(cnt);
            y_mat(cnt, 0) = ref_range_values.at(cnt);
        }

        Eigen::JacobiSVD<Eigen::MatrixXd> svd;

        svd.compute(x_mat, Eigen::ComputeThinV | Eigen::ComputeThinU);
        Eigen::MatrixXd x = svd.solve(y_mat);

        //loginf << "x " << x;

        range_gain_ = x(0, 0);
        range_bias_ = x(1, 0);

                // add importance
        if (num_failed_)
        {
            for (auto& result_it : results_)
            {
                std::shared_ptr<SinglePositionBase> single_result =
                    std::static_pointer_cast<SinglePositionBase>(result_it);
                assert (single_result);

                if (!single_result->use())
                    continue;

                assert (num_failed_ >= single_result->numFailed());

                single_result->setInterestFactor(
                    (float) single_result->numFailed() / (float)num_failed_);
            }
        }
    }
    else
    {
        value_min_ = 0;
        value_max_ = 0;
        value_avg_ = 0;
        value_var_ = 0;
        value_rms_ = 0;
    }
}

void JoinedPositionRadarRange::addToReport (
        std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "JoinedPositionRadarRange " <<  requirement_->name() <<": addToReport";

    if (!results_.size()) // some data must exist
    {
        logerr << "JoinedPositionRadarRange " <<  requirement_->name() <<": addToReport: no data";
        return;
    }

    logdbg << "JoinedPositionRadarRange " <<  requirement_->name() << ": addToReport: adding joined result";

    addToOverviewTable(root_item);
    addDetails(root_item);
}

void JoinedPositionRadarRange::addToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::SectionContentTable& ov_table = getReqOverviewTable(root_item);

    // condition
    std::shared_ptr<EvaluationRequirement::PositionRadarRange> req =
            std::static_pointer_cast<EvaluationRequirement::PositionRadarRange>(requirement_);
    assert (req);

    QVariant calc_val;
    string result {"Unknown"};

    if (num_passed_ + num_failed_)
    {
        calc_val = String::doubleToStringPrecision(value_avg_,2).c_str();
        result = req->getConditionResultStr(value_avg_);
    }

    // "Sector Layer", "Group", "Req.", "Id", "#Updates", "Result", "Condition", "Result"
    ov_table.addRow({sector_layer_.name().c_str(), requirement_->groupName().c_str(),
                     +(requirement_->shortname()).c_str(),
                     result_id_.c_str(), {num_passed_ + num_failed_},
                     calc_val, req->getConditionStr().c_str(), result.c_str()}, this, {});
}

void JoinedPositionRadarRange::addDetails(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::Section& sector_section = getRequirementSection(root_item);

    if (!sector_section.hasTable("sector_details_table"))
        sector_section.addTable("sector_details_table", 3, {"Name", "comment", "Value"}, false);

    std::shared_ptr<EvaluationRequirement::PositionRadarRange> req =
            std::static_pointer_cast<EvaluationRequirement::PositionRadarRange>(requirement_);
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
    sec_det_table.addRow({"DMin [m]", "Minimum of distance",
                          String::doubleToStringPrecision(value_min_,2).c_str()}, this);
    sec_det_table.addRow({"DMax [m]", "Maximum of distance",
                          String::doubleToStringPrecision(value_max_,2).c_str()}, this);
    sec_det_table.addRow({"DAvg [m]", "Average of distance",
                          String::doubleToStringPrecision(value_avg_,2).c_str()}, this);
    sec_det_table.addRow({"DSDev [m]", "Standard Deviation of distance",
                          String::doubleToStringPrecision(sqrt(value_var_),2).c_str()}, this);
    sec_det_table.addRow({"DVar [m^2]", "Variance of distance",
                          String::doubleToStringPrecision(value_var_,2).c_str()}, this);
    sec_det_table.addRow({"#CF [1]", "Number of updates with failed comparison", num_failed_}, this);
    sec_det_table.addRow({"#CP [1]", "Number of updates with passed comparison ", num_passed_},
                         this);


    if (range_bias_.isValid())
        sec_det_table.addRow({"Range Bias [m]", "Range bias (linear estimation)",
                              String::doubleToStringPrecision(range_bias_.toDouble(),2).c_str()}, this);

    if (range_gain_.isValid())
        sec_det_table.addRow({"Range Gain [1]", "Range gain (linear estimation)",
                              String::doubleToStringPrecision(range_gain_.toDouble(),5).c_str()}, this);

    // condition

    sec_det_table.addRow({"Condition", {}, req->getConditionStr().c_str()}, this);

    string result {"Unknown"};

    if (num_failed_ + num_passed_)
        result = req->getConditionResultStr(value_avg_);

    sec_det_table.addRow({"Condition Fulfilled", "", result.c_str()}, this);

    // figure
    sector_section.addFigure("sector_overview", "Sector Overview",
                             [this](void) { return this->getErrorsViewable(); });
}

bool JoinedPositionRadarRange::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == req_overview_table_name_)
        return true;
    else
        return false;
}

std::unique_ptr<nlohmann::json::object_t> JoinedPositionRadarRange::viewableDataImpl(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasViewableData(table, annotation));

    return getErrorsViewable();
}

std::unique_ptr<nlohmann::json::object_t> JoinedPositionRadarRange::getErrorsViewable ()
{
    std::unique_ptr<nlohmann::json::object_t> viewable_ptr =
            eval_man_.getViewableForEvaluation(req_grp_id_, result_id_);

    double lat_min, lat_max, lon_min, lon_max;

    tie(lat_min, lat_max) = sector_layer_.getMinMaxLatitude();
    tie(lon_min, lon_max) = sector_layer_.getMinMaxLongitude();

    (*viewable_ptr)[ViewPoint::VP_POS_LAT_KEY] = (lat_max+lat_min)/2.0;
    (*viewable_ptr)[ViewPoint::VP_POS_LON_KEY] = (lon_max+lon_min)/2.0;;

    double lat_w = lat_max-lat_min;
    double lon_w = lon_max-lon_min;

    if (lat_w < eval_man_.settings().result_detail_zoom_)
        lat_w = eval_man_.settings().result_detail_zoom_;

    if (lon_w < eval_man_.settings().result_detail_zoom_)
        lon_w = eval_man_.settings().result_detail_zoom_;

    (*viewable_ptr)[ViewPoint::VP_POS_WIN_LAT_KEY] = lat_w;
    (*viewable_ptr)[ViewPoint::VP_POS_WIN_LON_KEY] = lon_w;

    addAnnotationsFromSingles(*viewable_ptr);

    return viewable_ptr;
}

bool JoinedPositionRadarRange::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == req_overview_table_name_)
        return true;
    else
        return false;;
}

std::string JoinedPositionRadarRange::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasReference(table, annotation));
    return "Report:Results:"+getRequirementSectionID();
}

vector<double> JoinedPositionRadarRange::refRangeValues() const
{
    vector<double> values;

    for (auto& result_it : results_)
    {
        SinglePositionRadarRange* single_result = dynamic_cast<SinglePositionRadarRange*>(result_it.get());
        assert (single_result);

        if (!single_result->use())
            continue;

        values.insert(values.end(), single_result->refRangeValues().begin(), single_result->refRangeValues().end());
    }

    return values;
}

vector<double> JoinedPositionRadarRange::tstRangeValues() const
{
    vector<double> values;

    for (auto& result_it : results_)
    {
        SinglePositionRadarRange* single_result = dynamic_cast<SinglePositionRadarRange*>(result_it.get());
        assert (single_result);

        if (!single_result->use())
            continue;

        values.insert(values.end(), single_result->tstRangeValues().begin(), single_result->tstRangeValues().end());
    }

    return values;
}

void JoinedPositionRadarRange::exportAsCSV()
{
    loginf << "JoinedPositionRadarRange: exportAsCSV";

    QFileDialog dialog(nullptr);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDirectory(COMPASS::instance().lastUsedPath().c_str());
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
            output_file << "distance\n";

            vector<double> all_values = values();
            unsigned int size = all_values.size();

            for (unsigned int cnt=0; cnt < size; ++cnt)
                output_file << all_values.at(cnt) << "\n";
        }
    }
}

}
