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

#include "eval/results/base/joined.h"
#include "eval/results/base/single.h"
#include "eval/results/base/result_t.h"

#include "eval/results/report/sectioncontenttable.h"
#include "eval/results/report/section.h"
#include "eval/results/report/section_id.h"

#include "eval/requirement/base/base.h"

#include "evaluationmanager.h"
#include "sectorlayer.h"

#include "view/points/viewpointgenerator.h"
#include "view/points/viewpoint.h"

#include "view/gridview/grid2d.h"
#include "view/gridview/grid2dlayer.h"
#include "view/gridview/grid2d_defs.h"

#include "compass.h"

#include <QFileDialog>
#include <QMessageBox>

namespace EvaluationRequirementResult
{

const std::string Joined::SectorOverviewID              = "sector_overview";
const int         Joined::SectorOverviewRenderDelayMSec = 2000;

/**
*/
Joined::Joined(const std::string& type, 
                const std::string& result_id,
                std::shared_ptr<EvaluationRequirement::Base> requirement, 
                const SectorLayer& sector_layer,
                EvaluationManager& eval_man)
:   Base(type, result_id, requirement, sector_layer, eval_man)
{
}

/**
*/
Joined::~Joined() = default;

/**
*/
void Joined::clearResults()
{
    num_targets_        = 0;
    num_failed_targets_ = 0;

    //invoke derived
    clearResults_impl();
}

/**
*/
void Joined::updateResult()
{
    Base::updateResult(computeResult());
}

/**
*/
boost::optional<double> Joined::computeResult() const
{
    return computeResult_impl();
}

/**
*/
unsigned int Joined::numSingleResults() const
{
    return results_.size();
}

/**
*/
unsigned int Joined::numUsableSingleResults() const
{
    unsigned int cnt {0};

    for (auto& result_it : results_)
        if (result_it->use())
            ++cnt;

    return cnt;
}

/**
*/
unsigned int Joined::numUnusableSingleResults() const
{
    unsigned int cnt {0};

    for (auto& result_it : results_)
        if (!result_it->use())
            ++cnt;

    return cnt;
}

/**
*/
bool Joined::hasReference(const EvaluationResultsReport::SectionContentTable& table, 
                          const QVariant& annotation) const
{
    if (table.name() == req_overview_table_name_)
        return true;
    else
        return false;;
}

/**
*/
std::string Joined::reference(const EvaluationResultsReport::SectionContentTable& table, 
                              const QVariant& annotation) const
{
    assert (hasReference(table, annotation));
    return EvaluationResultsReport::SectionID::createForRequirementResult(*this);
}

/**
*/
std::string Joined::getRequirementAnnotationID_impl() const
{
    return (requirement_->name() + ":" + sector_layer_.name());
}

/**
*/
void Joined::iterateDetails(const DetailFunc& func,
                            const DetailSkipFunc& skip_func) const
{
    auto funcSingleResults = [ & ] (const std::shared_ptr<Single>& result)
    {
        result->iterateDetails(func, skip_func);
    };

    iterateSingleResults({}, funcSingleResults, {});
}

/**
*/
void Joined::addToReport(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "Joined: addToReport: " <<  requirement_->name();

    if (!results_.size()) // some data must exist
    {
        logerr << "Joined: addToReport: " <<  requirement_->name() <<": no data";
        return;
    }

    logdbg << "Joined: addToReport: " <<  requirement_->name() << ": adding joined result";

    addSectorToOverviewTable(root_item);
    addSectorDetailsToReport(root_item);
}

/**
*/
void Joined::addSectorToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::SectionContentTable& ov_table = getReqOverviewTable(root_item);

    auto must_hold_for_any_target = requirement_->mustHoldForAnyTarget();

    if (must_hold_for_any_target.has_value() && must_hold_for_any_target.value())
    {
        //just check if any target failed
        std::string result = num_failed_targets_ == 0 ? "Passed" : "Failed";

        // "Sector Layer", "Group", "Req.", "Id", "#Updates", "Result Value", "Condition", "Condition Result"
        ov_table.addRow({ sector_layer_.name().c_str(), 
                          requirement_->groupName().c_str(),
                          requirement_->shortname().c_str(),
                          result_id_.c_str(), 
                          num_targets_,
                          num_failed_targets_, "= 0", 
                          result.c_str() }, this, {});
    }
    else
    {
        //probabilisitc condition
        QVariant result_val = resultValue();

        std::string result {"Unknown"};

        if (result_val.isValid())
            result = conditionResultString();

        // "Sector Layer", "Group", "Req.", "Id", "#Updates", "Result Value", "Condition", "Condition Result"
        ov_table.addRow({ sector_layer_.name().c_str(), 
                          requirement_->groupName().c_str(),
                          requirement_->shortname().c_str(),
                          result_id_.c_str(), 
                          numUpdates(),
                          result_val, 
                          requirement_->getConditionStr().c_str(), 
                          result.c_str() }, this, {});
    }
}

/**
*/
std::vector<Joined::SectorInfo> Joined::sectorInfosCommon() const
{
    return { { "Sector Layer"        , "Name of the sector layer"     , sector_layer_.name().c_str()     },
             { "Reqirement Group"    , "Name of the requirement group", requirement_->groupName().c_str()},
             { "Reqirement"          , "Name of the requirement"      , requirement_->name().c_str()     },
             { "Num Results"         , "Total number of results"      , numSingleResults()               },
             { "Num Usable Results"  , "Number of usable results"     , numUsableSingleResults()         },
             { "Num Unusable Results", "Number of unusable results"   , numUnusableSingleResults()       },
             { "Use"                 , "To be used in results"        , use_                             } };
}

/**
*/
std::vector<Joined::SectorInfo> Joined::sectorConditionInfos() const
{
    std::vector<SectorInfo> infos;

    auto must_hold_for_any_target = requirement_->mustHoldForAnyTarget();
    bool show_target_condition = must_hold_for_any_target.has_value() &&
                                 must_hold_for_any_target.value();

    QVariant result_val = resultValue();

    if (!show_target_condition)
    {
        infos.push_back({ requirement_->getConditionResultNameShort(true).c_str(), 
                          requirement_->getConditionResultName().c_str(), 
                          result_val });
    }

    QString result_name   = QString::fromStdString(requirement_->getConditionResultNameShort(false));
    QString condition_str = QString::fromStdString(requirement_->getConditionStr());

    QString condition_name   = show_target_condition ? "Single Target Condition" : "Condition";
    QString confition_string = show_target_condition ? result_name + " " + condition_str : condition_str;

    infos.push_back({condition_name, "", confition_string });

    if (!show_target_condition)
    {
        std::string result {"Unknown"};

        if (result_val.isValid())
            result = conditionResultString();

        infos.push_back({condition_name + " Fulfilled", "", result.c_str()});
    }

    if (must_hold_for_any_target.has_value())
    {
        infos.emplace_back("Must hold for any target ", "", must_hold_for_any_target.value());

        if (must_hold_for_any_target.value())
        {
            infos.emplace_back("#Single Targets"       , "", num_targets_       );
            infos.emplace_back("#Failed Single Targets", "", num_failed_targets_);

            infos.push_back({"Sum Condition", "", "= 0" });

            std::string result = num_failed_targets_ == 0 ? "Passed" : "Failed";

            infos.push_back({"Sum Condition Fulfilled", "", result.c_str()});
        }
    }

    return infos;
}

/**
*/
void Joined::addSectorDetailsToReport(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::Section& sector_section = getRequirementSection(root_item);

    if (!sector_section.hasTable("sector_details_table"))
        sector_section.addTable("sector_details_table", 3, {"Name", "Comment", "Value"}, false);

    EvaluationResultsReport::SectionContentTable& sec_det_table = sector_section.getTable("sector_details_table");

    // callbacks
    if (canExportCSV())
    {
        auto exportAsCSV_lambda = [this]() { this->exportAsCSV(); };

        sec_det_table.registerCallBack("Save Data As CSV", exportAsCSV_lambda);
    }

    // details
    auto infos_common = sectorInfosCommon();

    for (const auto& info : infos_common)
        sec_det_table.addRow({ info.info_name, info.info_comment, info.info_value }, this);

    auto infos = sectorInfos();

    for (const auto& info : infos)
        sec_det_table.addRow({ info.info_name, info.info_comment, info.info_value }, this);

    auto infos_condition = sectorConditionInfos();

    for (const auto& info : infos_condition)
        sec_det_table.addRow({ info.info_name, info.info_comment, info.info_value }, this);

    // figure
    addOverview(sector_section);
}

/**
*/
bool Joined::exportAsCSV() const
{
    if (!canExportCSV())
        return false;

    loginf << "Joined: exportAsCSV: " << type();

    QFileDialog dialog(nullptr);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDirectory(COMPASS::instance().lastUsedPath().c_str());
    dialog.setNameFilter("CSV Files (*.csv)");
    dialog.setDefaultSuffix("csv");
    dialog.setAcceptMode(QFileDialog::AcceptMode::AcceptSave);

    if (dialog.exec() != QDialog::Accepted)
        return true;

    QStringList file_names = dialog.selectedFiles();
    assert (file_names.size() == 1);

    std::string filename = file_names.at(0).toStdString();

    std::ofstream output_file;
    output_file.open(filename, std::ios_base::out);

    if (!output_file || !output_file.is_open() || !exportAsCSV(output_file))
    {
        QMessageBox::critical(nullptr, "Error", "Exporting CSV file failed.");
        return false;
    }

    return true;
}

/**
*/
void Joined::addOverview (EvaluationResultsReport::Section& section,
                          const std::string& name)
{
    section.addFigure(SectorOverviewID, 
                      name, 
                      [this](void) { return this->viewableOverviewData(); }, 
                      SectorOverviewRenderDelayMSec);
}

/**
*/
bool Joined::resultUsed(const std::shared_ptr<Single>& result) const
{
    return (result->use() && result->resultUsable());
}

/**
*/
void Joined::iterateSingleResults(const SingleResultFunc& func,
                                  const SingleResultFunc& func_used,
                                  const SingleResultFunc& func_unused) const
{
    assert(func_used);

    for (const auto& result_it : results_)
    {
        if (func) 
            func(result_it);

        if (!resultUsed(result_it))
        {
            if (func_unused) 
                func_unused(result_it);
            continue;
        }

        if (func_used) 
            func_used(result_it);
    }
}

/**
*/
void Joined::addSingleResult(std::shared_ptr<Single> other)
{
    results_.push_back(other);
}

/**
*/
std::vector<std::shared_ptr<Single>>& Joined::singleResults() 
{ 
    return results_; 
}

/**
*/
std::vector<std::shared_ptr<Single>> Joined::usedSingleResults() const
{
    std::vector<std::shared_ptr<Single>> used_results;

    auto funcSingleResults = [ & ] (const std::shared_ptr<Single>& result)
    {
        used_results.push_back(result);
    };

    iterateSingleResults({}, funcSingleResults, {});

    return used_results;
}

/**
*/
bool Joined::hasStoredDetails() const
{
    bool has_details = true;

    for (const auto& single : results_)
    {
        if (!resultUsed(single))
            continue;

        if (!single->hasStoredDetails())
        {
            has_details = false;
            break;
        }
    }

    return has_details;
}

/**
*/
void Joined::updateToChanges(bool reset_viewable)
{
    //clear first
    clearResults();

    //invalidate cached viewable?
    if (reset_viewable)
        viewable_.reset();

    //accumulate single results
    size_t nr = results_.size();

    std::vector<size_t> used;

    //collect used results
    for (size_t i = 0; i < nr; ++i)
    {
        auto& single = results_[ i ];

        //reset interest
        single->setInterestFactor(0);

        if (resultUsed(single))
            used.push_back(i);
    }

    size_t nu = used.size();

    //accumulate all used results
    for (size_t i = 0; i < nu; ++i)
    {
        auto& single = results_[ used[ i ] ];

        ++num_targets_;

        if (single->hasFailed())
            ++num_failed_targets_;

        accumulateSingleResult(single, i == 0, i == nu - 1);
    }

    //update result
    updateResult();

    //update interests if result is usable
    auto issues_total = numIssues();

    if (issues_total > 0 && resultUsable())
    {
        for (size_t i = 0; i < nu; ++i)
        {
            auto& single = results_[ used[ i ] ];

            auto issues = single->numIssues();
            assert (issues_total >= issues);

            single->setInterestFactor((double)issues / (double)issues_total);
        }
    }

    //update viewable immediately if details are still stored
    if (hasStoredDetails())
        getOrCreateCachedViewable();
}

/**
*/
std::vector<double> Joined::getValues(const ValueSource<double>& source) const
{
    return EvaluationResultTemplates(this).getValues<double>(source);
}

/**
*/
std::vector<double> Joined::getValues(int value_id) const
{
    return getValues(ValueSource<double>(value_id));
}

/**
*/
std::shared_ptr<nlohmann::json::object_t> Joined::getOrCreateCachedViewable() const
{
    if (!viewable_)
    {
        //cache all needed single details
        std::vector<Single::TemporaryDetails> temp_details;

        auto funcSingleResults = [ & ] (const std::shared_ptr<Single>& result)
        {
            temp_details.push_back(result->temporaryDetails());
        };
        iterateSingleResults({}, funcSingleResults, {});

        //create new viewable
        viewable_ = createViewable(AnnotationOptions().overview());

        //wipe all single result details
        temp_details.resize(0);
    }

    return viewable_;
}

/**
*/
std::unique_ptr<nlohmann::json::object_t> Joined::createBaseViewable() const
{
    return eval_man_.getViewableForEvaluation(req_grp_id_, result_id_);
}

/**
*/
Base::ViewableInfo Joined::createViewableInfo(const AnnotationOptions& options) const
{
    ViewableInfo info;
    info.viewable_type = ViewableType::Overview;

    double lat_min, lat_max, lon_min, lon_max;

    std::tie(lat_min, lat_max) = sector_layer_.getMinMaxLatitude();
    std::tie(lon_min, lon_max) = sector_layer_.getMinMaxLongitude();

    info.bounds = QRectF(lat_min, lon_min, lat_max - lat_min, lon_max - lon_min);

    return info;
}

/**
*/
void Joined::createAnnotations(nlohmann::json& annotations, 
                               const AnnotationOptions& options) const
{
    //everything handled via custom annotations
    addCustomAnnotations(annotations);
}

/**
 */
bool Joined::hasViewableData (const EvaluationResultsReport::SectionContentTable& table, 
                              const QVariant& annotation) const
{
    if (table.name() != req_overview_table_name_)
        return false;

    return true;
}

/**
 */
std::shared_ptr<nlohmann::json::object_t> Joined::viewableData(const EvaluationResultsReport::SectionContentTable& table, 
                                                               const QVariant& annotation) const
{
    assert (hasViewableData(table, annotation));

    //return cached viewable (might recreate the viewable)
    return getOrCreateCachedViewable();
}

/**
*/
std::shared_ptr<nlohmann::json::object_t> Joined::viewableOverviewData() const
{
    //return cached viewable (might recreate the viewable)
    return getOrCreateCachedViewable();
}

/**
 * Legacy code for creating annotations for every single target report in the sector.
*/
// void Joined::addOverviewAnnotations(nlohmann::json& annotations_json) const
// {
//     //add annotations from single results
//     for (auto& single_result : results_)
//     {
//         if (single_result->use())
//         {
//             //create overview annotations for single result
//             single_result->createSumOverviewAnnotations(annotations_json, 
//                                                         eval_man_.settings().show_ok_joined_target_reports_);
//         }
//     }
// }

}
