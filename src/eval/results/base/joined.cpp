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
    //clear details
    clearDetails();

    num_targets_        = 0;
    num_failed_targets_ = 0;

    //invoke derived
    clearResults_impl();
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

        // "Sector Layer", "Group", "Req.", "Id", "#Updates", "Result", "Condition", "Result"
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

        // "Sector Layer", "Group", "Req.", "Id", "#Updates", "Result", "Condition", "Result"
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

    QVariant result_val = resultValue();

    infos.push_back({ requirement_->getConditionResultNameShort().c_str(), 
                      requirement_->getConditionResultName().c_str(), 
                      result_val });

    //@TODO_EVAL: change condition if must_hold_for_any_target is true?

    infos.push_back({"Condition", "", requirement_->getConditionStr().c_str() });

    std::string result {"Unknown"};

    if (result_val.isValid())
        result = conditionResultString();

    infos.push_back({"Condition Fulfilled", "", result.c_str()});

    if (must_hold_for_any_target.has_value())
    {
        infos.emplace_back("Must hold for any target ", "", must_hold_for_any_target.value());

        if (must_hold_for_any_target.value())
        {
            infos.emplace_back("#Single Targets"       , "", num_targets_       );
            infos.emplace_back("#Failed Single Targets", "", num_failed_targets_);
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
        sector_section.addTable("sector_details_table", 3, {"Name", "comment", "Value"}, false);

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

    bool written = false;

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
                      [this](void) { return this->viewableData(); }, 
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
void Joined::updateToChanges()
{
    //clear first
    clearResults();

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
        for (const auto& single : results_)
        {
            if (!resultUsed(single))
                continue;

            auto issues = single->numIssues();
            assert (issues_total >= issues);

            single->setInterestFactor((double)issues / (double)issues_total);
        }
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
    auto overview_mode = overviewMode();

    bool has_grid_info = !results_[ 0 ]->gridLayers().empty();

    //create features?
    bool added_annotations = false;

    if (overview_mode == OverviewMode::Annotations ||
        overview_mode == OverviewMode::GridPlusAnnotations ||
        overview_mode == OverviewMode::GridOrAnnotations)
    {
        if (overview_mode != OverviewMode::GridOrAnnotations || !has_grid_info)
        {
            addOverviewAnnotations(annotations);
            added_annotations = true;
        }
    }

    //create grid?
    if (overview_mode == OverviewMode::Grid ||
        overview_mode == OverviewMode::GridPlusAnnotations ||
        overview_mode == OverviewMode::GridOrAnnotations)
    {
        if (overview_mode != OverviewMode::GridOrAnnotations || !added_annotations)
        {
            if (has_grid_info)
            {
                //add grid data if grid layers are specified
                addOverviewGrid(annotations);
            }
        }
    }

    //add custom annotations
    {
        addCustomAnnotations(annotations);
    }
}

/**
 */
bool Joined::hasViewableData (const EvaluationResultsReport::SectionContentTable& table, 
                              const QVariant& annotation) const
{
    if (table.name() == req_overview_table_name_)
        return true;
    else
        return false;
}

/**
 */
std::unique_ptr<nlohmann::json::object_t> Joined::viewableData(const EvaluationResultsReport::SectionContentTable& table, 
                                                               const QVariant& annotation) const
{
    assert (hasViewableData(table, annotation));

    //just call the general overview version
    return viewableData();
}

/**
*/
std::unique_ptr<nlohmann::json::object_t> Joined::viewableData() const
{
    //create overview viewable
    return createViewable(AnnotationOptions().overview());
}

/**
 * Default behaviour for creating viewable annotations.
*/
void Joined::addOverviewAnnotations(nlohmann::json& annotations_json) const
{
    //add annotations from single results
    for (auto& single_result : results_)
    {
        if (single_result->use())
        {
            //create overview annotations for single result
            single_result->createSumOverviewAnnotations(annotations_json, 
                                                        eval_man_.settings().show_ok_joined_target_reports_);
        }
    }
}

namespace
{
    QRectF gridBounds(const SectorLayer& sector_layer, double border_factor)
    {
        auto lat_range = sector_layer.getMinMaxLatitude();
        auto lon_range = sector_layer.getMinMaxLongitude();

        QRectF roi(lon_range.first, lat_range.first, lon_range.second - lon_range.first, lat_range.second - lat_range.first);

        return grid2d::GridResolution::addBorder(roi, border_factor, -180.0, 180.0, -90.0, 90.0);
    }
}

/**
*/
void Joined::createGrid(const grid2d::GridResolution& resolution) const
{
    QRectF roi = gridBounds(sector_layer_, 0.01);

    grid_.reset(new Grid2D);
    bool grid_ok = grid_->create(roi, resolution, "wgs84", true);

    assert(grid_ok);
}

/**
*/
void Joined::addOverviewGrid(nlohmann::json& annotations_json) const
{
    assert (annotations_json.is_array());

    if (results_.empty())
        return;

    loginf << "Joined: addGridToViewData: creating grid";

    //create the grid
    createGrid(grid2d::GridResolution().setCellCount(eval_man_.settings().grid_num_cells_x,
                                                     eval_man_.settings().grid_num_cells_y));

    loginf << "Joined: addGridToViewData: creating grid layers";

    Grid2DLayers layers;

    std::map<std::string, Grid2DRenderSettings> render_settings;

    //get layers to be generated
    auto layer_defs = results_[ 0 ]->gridLayers();

    //generate layers
    for (const auto& l : layer_defs)
    {
        //reset grid for new data layer
        grid_->reset();

        //loginf << "Generating value layer " << l.first;

        //add result values
        for (const auto& single : results_)
        {
            if (!single->use())
                continue;

            single->addValuesToGrid(*grid_, l.first);
        }

        assert(grid_->numOutOfRange() == 0);

        //obtain all layer values
        for (const auto& layer_def : l.second)
        {
            std::string lname = l.first + (l.second.size() > 1 ? "_" + grid2d::valueTypeToString(layer_def.value_type) : "");

            //loginf << "Generating render layer " << lname;

            grid_->addToLayers(layers, lname, layer_def.value_type);

            render_settings[ lname ] = layer_def.render_settings;
        }
    }

    ViewPointGenAnnotations annotations;

    loginf << "Joined: addGridToViewData: creating features...";
    
    //render layers and add them as annotation features
    for(const auto& l : layers.layers())
    {
        //loginf << "Rendering layer " << l.first;

        assert(render_settings.count(l.first));
        const auto& rs = render_settings.at(l.first);

        //loginf << "value map size: " << l.second.data.cols() << "x" << l.second.data.rows();

        auto render_result = Grid2DLayerRenderer::render(*l.second, rs);

        //loginf << "rendered image size: " << render_result.first.width() << "x" << render_result.first.height();
        //render_result.first.save(QString::fromStdString("/home/mcphatty/layer_" + l.first + ".png"));

        std::unique_ptr<ViewPointGenFeatureGeoImage> geo_image(new ViewPointGenFeatureGeoImage(render_result.first,
                                                                                               render_result.second));
        std::unique_ptr<ViewPointGenAnnotation> a(new ViewPointGenAnnotation(l.first, false));
        a->addFeature(std::move(geo_image));

        annotations.addAnnotation(std::move(a));
    }

    loginf << "Joined: addGridToViewData: created " << annotations.size() << " feature(s)";

    //add to view data
    if (annotations.size() > 0)
    {
        for (size_t i = 0; i < annotations.size(); ++i)
        {
            nlohmann::json a_info;
            annotations.annotation(i).toJSON(a_info);

            annotations_json.push_back(a_info);
        }
    }
}

}
