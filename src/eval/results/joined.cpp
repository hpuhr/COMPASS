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

#include "eval/results/joined.h"
#include "eval/results/single.h"
#include "eval/results/report/sectioncontenttable.h"
#include "evaluationmanager.h"

#include "eval/requirement/base/base.h"

#include "sectorlayer.h"

#include "view/points/viewpointgenerator.h"
#include "view/points/viewpoint.h"

namespace EvaluationRequirementResult
{

Joined::Joined(const std::string& type, 
                const std::string& result_id,
                std::shared_ptr<EvaluationRequirement::Base> requirement, 
                const SectorLayer& sector_layer,
                EvaluationManager& eval_man)
:   Base(type, result_id, requirement, sector_layer, eval_man)
{
}

Joined::~Joined() = default;

unsigned int Joined::numResults()
{
    return results_.size();
}

unsigned int Joined::numUsableResults()
{
    unsigned int cnt {0};

    for (auto& result_it : results_)
        if (result_it->use())
            ++cnt;

    return cnt;
}

unsigned int Joined::numUnusableResults()
{
    unsigned int cnt {0};

    for (auto& result_it : results_)
        if (!result_it->use())
            ++cnt;

    return cnt;
}

void Joined::addCommonDetails (EvaluationResultsReport::SectionContentTable& sector_details_table)
{
    sector_details_table.addRow({"Sector Layer", "Name of the sector layer", sector_layer_.name().c_str()}, this);
    sector_details_table.addRow({"Reqirement Group", "Name of the requirement group",
                                    requirement_->groupName().c_str()}, this);
    sector_details_table.addRow({"Reqirement", "Name of the requirement", requirement_->name().c_str()}, this);
    sector_details_table.addRow({"Num Results", "Total number of results", numResults()}, this);
    sector_details_table.addRow({"Num Usable Results", "Number of usable results", numUsableResults()}, this);
    sector_details_table.addRow({"Num Unusable Results", "Number of unusable results", numUnusableResults()}, this);
}

void Joined::updatesToUseChanges()
{
    clearDetails();

    //invoke derived
    updatesToUseChanges_impl();
}

void Joined::join(std::shared_ptr<Single> other)
{
    //add result
    results_.push_back(other);

    //invoke derived
    join_impl(other);
}

std::vector<std::shared_ptr<Single>>& Joined::results() 
{ 
    return results_; 
}

void Joined::addAnnotationsFromSingles(nlohmann::json::object_t& viewable_ref)
{
    for (auto& single_result : results_)
    {
        if (single_result->use())
            single_result->addAnnotations(viewable_ref, true, eval_man_.settings().show_ok_joined_target_reports_);
    }
}

namespace
{
    QRectF gridBounds(const SectorLayer& sector_layer, double border_factor)
    {
        auto lat_range = sector_layer.getMinMaxLatitude();
        auto lon_range = sector_layer.getMinMaxLongitude();

        double lat_size = lat_range.second - lat_range.first;
        double lon_size = lon_range.second - lon_range.first;

        double lat_border = lat_size * border_factor * 0.5;
        double lon_border = lon_size * border_factor * 0.5;

        //std::cout << lat_range.first << "-" << lat_range.second << " / "
        //          << lon_range.first << "-" << lon_range.second << std::endl;

        lat_range.first  = std::max( -90.0, lat_range.first  - lat_border);
        lat_range.second = std::min(  90.0, lat_range.second + lat_border);
        lon_range.first  = std::max(-180.0, lon_range.first  - lon_border);
        lon_range.second = std::min( 180.0, lon_range.second + lon_border);

        //std::cout << lat_range.first << "-" << lat_range.second << " / "
        //          << lon_range.first << "-" << lon_range.second << std::endl;

        return QRectF(lon_range.first, 
                      lat_range.first, 
                      lon_range.second - lon_range.first,
                      lat_range.second - lat_range.first);
    }
}

void Joined::createGrid(size_t num_cells_x, size_t num_cells_y)
{
    QRectF roi = gridBounds(sector_layer_, 0.05);

    grid_.reset(new Grid2D);
    grid_->create(roi, num_cells_x, num_cells_y, "wgs84", true);
}

void Joined::createGrid(double cell_size_x, double cell_size_y)
{
    QRectF roi = gridBounds(sector_layer_, 0.05);

    grid_.reset(new Grid2D);
    grid_->create(roi, cell_size_x, cell_size_y, "wgs84", true);
}

bool Joined::addToGrid(double lon, double lat, double value)
{
    assert(grid_);
    return grid_->addValue(lon, lat, value);
}

void Joined::addGridToViewData(nlohmann::json::object_t& view_data)
{
    if (results_.empty() || view_data.count(ViewPoint::VP_ANNOTATION_KEY) == 0)
        return;

    loginf << "Joined: addGridToViewData: creating grid";

    //create the grid
    createGrid((size_t)eval_man_.settings().grid_num_cells_x, 
               (size_t)eval_man_.settings().grid_num_cells_y);

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

        size_t total = 0;
        size_t added = 0;

        //add result values
        for (const auto& single : results_)
        {
            if (!single->use())
                continue;

            auto values = single->getGridValues(l.first);
            total += values.size();

            //loginf << "Adding " << values.size() << " value(s) to grid";

            for (const auto& v : values)
                if (addToGrid(v.x(), v.y(), v.z()))
                    ++added;
        }

        //loginf << "Could add " << added << " / " << total << " value(s)";

        //obtain all layer values
        for (const auto& layer_def : l.second)
        {
            std::string lname = l.first + (l.second.size() > 1 ? "_" + Grid2D::valueTypeToString(layer_def.value_type) : "");

            //loginf << "Generating render layer " << lname;

            auto ldata = grid_->getValues(layer_def.value_type);
            layers.addLayer(lname, grid_->getReference(), ldata);

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

        auto render_result = Grid2DLayerRenderer::render(l.second, rs);

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
        auto& annos_json = view_data.at(ViewPoint::VP_ANNOTATION_KEY);
        assert (annos_json.is_array());

        for (size_t i = 0; i < annotations.size(); ++i)
        {
            nlohmann::json a_info;
            annotations.annotation(i).toJSON(a_info);

            annos_json.push_back(a_info);
        }
    }
}

std::unique_ptr<nlohmann::json::object_t> Joined::viewableData(const EvaluationResultsReport::SectionContentTable& table, 
                                                               const QVariant& annotation)
{
    //no results no viewable
    if (results_.empty())
        return {};

    auto overview_mode = overviewMode();

    std::unique_ptr<nlohmann::json::object_t> vdata;

    bool has_grid_info = !results_[ 0 ]->gridLayers().empty();

    //create features?
    if (overview_mode == OverviewMode::Features ||
        overview_mode == OverviewMode::GridPlusFeatures ||
        overview_mode == OverviewMode::GridOrFeatures)
    {
        if (overview_mode != OverviewMode::GridOrFeatures || !has_grid_info)
            vdata = viewableDataImpl(table, annotation);
    }

    //create grid?
    if (overview_mode == OverviewMode::Grid ||
        overview_mode == OverviewMode::GridPlusFeatures ||
        overview_mode == OverviewMode::GridOrFeatures)
    {
        if (overview_mode != OverviewMode::GridOrFeatures || !vdata)
        {
            if (has_grid_info)
            {
                //create viewable if not created yet
                if (!vdata)
                    vdata = createViewable();

                //add grid data if grid layers are specified
                addGridToViewData(*vdata);
            }
        }
    }

    return vdata;
}

std::unique_ptr<nlohmann::json::object_t> Joined::createViewable() const
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

    //add annotation array
    (*viewable_ptr)[ViewPoint::VP_ANNOTATION_KEY] = nlohmann::json::array();

    return viewable_ptr;
}

}
