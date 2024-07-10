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

#include "eval/results/base/base.h"

#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttable.h"
#include "eval/results/report/section_id.h"

#include "eval/requirement/base/base.h"

#include "sectorlayer.h"
//#include "logger.h"
#include "evaluationmanager.h"

#include "viewpoint.h"
#include "viewpointgenerator.h"
#include "histograminitializer.h"
#include "grid2d_defs.h"
#include "grid2d.h"
#include "grid2dlayer.h"

#include <sstream>
#include <cassert>

using namespace std;

namespace EvaluationRequirementResult
{

const std::string Base::req_overview_table_name_ {"Results Overview"};

const QColor Base::HistogramColorDefault = QColor(0, 0, 255);

/**
*/
Base::Base(const std::string& type, 
           const std::string& result_id,
           std::shared_ptr<EvaluationRequirement::Base> requirement, 
           const SectorLayer& sector_layer,
           EvaluationManager& eval_man)
:   type_        (type)
,   result_id_   (result_id)
,   requirement_ (requirement)
,   sector_layer_(sector_layer)
,   eval_man_    (eval_man)
{
    assert (requirement_);

    req_grp_id_ = EvaluationResultsReport::SectionID::requirementGroupResultID(*this);
}

/**
*/
Base::~Base() = default;

/**
*/
bool Base::isSingle() const
{
    return (baseType() == BaseType::Single);
}

/**
*/
bool Base::isJoined() const
{
    return (baseType() == BaseType::Joined);
}

/**
*/
std::shared_ptr<EvaluationRequirement::Base> Base::requirement() const
{
    return requirement_;
}

/**
*/
std::string Base::type() const
{
    return type_;
}

/**
*/
std::string Base::resultId() const
{
    return result_id_;
}

/**
*/
std::string Base::reqGrpId() const
{
    return req_grp_id_;
}

/**
*/
bool Base::use() const
{
    return use_;
}

/**
*/
void Base::use(bool use)
{
    use_ = use;
}

/**
*/
const boost::optional<double>& Base::result() const
{
    return result_;
}

/**
*/
void Base::updateResult()
{
    result_.reset();

    auto result = computeResult();
    if (result.has_value())
        result_ = result.value();
}

/**
*/
bool Base::resultUsable() const
{
    return (result_.has_value() && !ignore_);
}

/**
*/
bool Base::hasFailed() const
{
    return (resultUsable() && !requirement_->conditionPassed(result_.value()));
}

/**
*/
bool Base::hasIssues() const
{
    return (resultUsable() && numIssues() > 0);
}

/**
*/
QVariant Base::resultValue() const
{
    return resultValueOptional(result_);
}

/**
*/
QVariant Base::resultValueOptional(const boost::optional<double>& value) const
{
    if (!value.has_value())
        return QVariant();

    return resultValue(value.value());
}

/**
*/
QVariant Base::resultValue(double value) const
{
    return requirement_->getResultValueString(value).c_str();
}

/**
*/
std::string Base::conditionResultString() const
{
    if (!resultUsable())
        return "";

    return requirement_->getConditionResultStr(result_.value());
}

/**
*/
void Base::setIgnored()
{
    ignore_ = true;
}

/**
*/
bool Base::isIgnored() const
{
    return ignore_;
}

/**
*/
boost::optional<double> Base::computeResult() const
{
    return computeResult_impl();
}

/**
*/
EvaluationResultsReport::SectionContentTable& Base::getReqOverviewTable (
        std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::Section& ov_sec = root_item->getSection("Overview:Results");

    if (!ov_sec.hasTable(req_overview_table_name_))
        ov_sec.addTable(req_overview_table_name_, 8,
        {"Sector Layer", "Group", "Req.", "Id", "#Updates", "Value", "Condition", "Result"});

    return ov_sec.getTable(req_overview_table_name_);
}

/**
*/
std::string Base::getRequirementSectionID() const
{
    return EvaluationResultsReport::SectionID::requirementResultID(*this);
}

/**
*/
std::string Base::getRequirementSumSectionID() const
{
    return EvaluationResultsReport::SectionID::requirementResultSumID(*this);
}

/**
*/
EvaluationResultsReport::Section& Base::getRequirementSection (
        std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    return root_item->getSection(getRequirementSectionID());
}

/**
*/
size_t Base::numDetails() const
{
    return details_.size();
}

/**
*/
const Base::EvaluationDetails& Base::getDetails() const
{
    return details_;
}

/**
*/
const EvaluationDetail& Base::getDetail(int idx) const
{
    return getDetails().at(idx);
}

/**
*/
const EvaluationDetail& Base::getDetail(const DetailIndex& index) const
{
    const auto& d = getDetail(index[ 0 ]);

    if (index[ 1 ] < 0)
        return d;

    return d.details().at(index[ 1 ]);
}

/**
*/
bool Base::detailIndexValid(const DetailIndex& index) const
{
    if (index[ 0 ] < 0 || index[ 0 ] >= (int)details_.size())
        return false;
    if (index[ 1 ] >= 0 && index[ 1 ] >= (int)details_[ index[ 0 ] ].numDetails())
        return false;

    return true;
}

/**
*/
void Base::clearDetails()
{
    details_ = {};
}

/**
*/
void Base::setDetails(const EvaluationDetails& details)
{
    details_ = details;
}

/**
*/
void Base::addDetails(const EvaluationDetails& details)
{
    details_.insert(details_.end(), details.begin(), details.end());
}

/**
*/
std::unique_ptr<nlohmann::json::object_t> Base::createViewable(const AnnotationOptions& options) const
{
    auto viewable_ptr = createBaseViewable();        // create basic viewable
    auto info         = createViewableInfo(options); // create viewable info (data bounds etc.)

    //configure viewable depending on type
    if (info.viewable_type == ViewableType::Overview)
    {
        //overview => set region of interest
        if (!info.bounds.isEmpty())
        {
            (*viewable_ptr)[ViewPoint::VP_POS_LAT_KEY] = info.bounds.center().x();
            (*viewable_ptr)[ViewPoint::VP_POS_LON_KEY] = info.bounds.center().y();

            double lat_w = info.bounds.width();
            double lon_w = info.bounds.height();

            if (lat_w < eval_man_.settings().result_detail_zoom_)
                lat_w = eval_man_.settings().result_detail_zoom_;

            if (lon_w < eval_man_.settings().result_detail_zoom_)
                lon_w = eval_man_.settings().result_detail_zoom_;

            (*viewable_ptr)[ViewPoint::VP_POS_WIN_LAT_KEY] = lat_w;
            (*viewable_ptr)[ViewPoint::VP_POS_WIN_LON_KEY] = lon_w;
        }
    }
    else
    {
        //detail => set position of interest
        (*viewable_ptr)[ViewPoint::VP_POS_LAT_KEY    ] = info.bounds.x();
        (*viewable_ptr)[ViewPoint::VP_POS_LON_KEY    ] = info.bounds.y();
        (*viewable_ptr)[ViewPoint::VP_POS_WIN_LAT_KEY] = eval_man_.settings().result_detail_zoom_;
        (*viewable_ptr)[ViewPoint::VP_POS_WIN_LON_KEY] = eval_man_.settings().result_detail_zoom_;
        (*viewable_ptr)[ViewPoint::VP_TIMESTAMP_KEY  ] = Utils::Time::toString(info.timestamp);
    }

    //init annotation array
    (*viewable_ptr)[ViewPoint::VP_ANNOTATION_KEY] = nlohmann::json::array();

    //add annotations
    createAnnotations((*viewable_ptr)[ViewPoint::VP_ANNOTATION_KEY], options);

    return viewable_ptr;
}

/**
*/
QString Base::formatValue(double v, int precision) const
{
    return QString::fromStdString(Utils::String::doubleToStringPrecision(v, precision));
}

/**
*/
AnnotationDefinitions Base::getCustomAnnotationDefinitions() const
{
    //by default return empty definitions => no custom annotations will be generated
    return AnnotationDefinitions();
}

/**
*/
void Base::addCustomAnnotations(nlohmann::json& annotations_json) const
{
    auto defs = getCustomAnnotationDefinitions();

    //add all grids
    for (const auto& grids : defs.grids())
        addGrids(annotations_json, grids.first, grids.second);

    //add all histograms
    for (const auto& histograms : defs.histograms())
        addHistograms(annotations_json, histograms.first, histograms.second);
}

namespace
{
    QRectF gridBounds(const SectorLayer& sector_layer, double border_factor)
    {
        auto lat_range = sector_layer.getMinMaxLatitude();
        auto lon_range = sector_layer.getMinMaxLongitude();

        QRectF roi(lon_range.first, lat_range.first, lon_range.second - lon_range.first, lat_range.second - lat_range.first);

        return grid2d::GridResolution::addBorder(roi, border_factor, -180.0, 180.0, -90.0, 90.0);
    };
}

/**
*/
void Base::addGrids(nlohmann::json& annotations_json, 
                    const std::string& annotation_name, 
                    const std::vector<AnnotationDefinitions::GridDefinition>& defs) const
{
    loginf << "Base: addGrids: creating grid...";

    //create suitably sized grid
    QRectF roi = gridBounds(sector_layer_, 0.01);

    auto resolution = grid2d::GridResolution().setCellCount(eval_man_.settings().grid_num_cells_x,
                                                            eval_man_.settings().grid_num_cells_y);
    Grid2D grid;
    bool grid_ok = grid.create(roi, resolution, "wgs84", true);

    //!shall not fail! (otherwise sector bounds might be strange)
    assert(grid_ok);

    loginf << "Base: addGrids: filling grid...";

    //generate grid layers
    Grid2DLayers layers;
    std::map<std::string, Grid2DRenderSettings> render_settings_map;

    for (const auto& def : defs)
    {
        if (!def.isValid())
            continue;

        //reset grid for new data layer
        grid.reset();

        bool add_as_polys = def.add_detail_mode == AnnotationDefinitions::GridDefinition::AddDetailMode::AddPositionsAsPolygon;

        //obtain positions + values
        std::vector<std::pair<size_t, size_t>> detail_ranges;
        auto values = getValuesPlusPos(def.value_source, 
                                       def.pos_mode,
                                       add_as_polys ? &detail_ranges : nullptr);
        if (values.empty())
            continue;

        if (add_as_polys)
        {
            //add as per detail polygons
            for (const auto& r : detail_ranges)
            {
                //skip single positions
                if (r.second < 2)
                    continue;

                auto pos_getter = [ & ] (double& x, double& y, size_t idx) 
                { 
                    x =  values[ r.first + idx ].x();
                    y =  values[ r.first + idx ].y();
                };

                grid.addPoly(pos_getter, r.second, values[ r.first ].z());
            }
        }
        else
        {
            //just add as single values
            for (const auto& pos : values)
                grid.addValue(pos.x(), pos.y(), pos.z());
        }

        assert(grid.numOutOfRange() == 0);

        //get render settings and override some values
        Grid2DRenderSettings render_settings = def.render_settings;
        render_settings.pixels_per_cell = eval_man_.settings().grid_pixels_per_cell;

        //obtain all layer values
        for (auto value_type : def.value_types)
        {
            //create layer name which incorporates the desired value type
            std::string lname = def.name + (def.value_types.size() > 1 ? "_" + grid2d::valueTypeToString(value_type) : "");

            //store layer
            grid.addToLayers(layers, lname, value_type);

            //store render settings
            render_settings_map[ lname ] = render_settings;
        }
    }

    loginf << "Base: addGrids: creating features...";

    ViewPointGenAnnotations annotations;
    
    //render layers and add them as annotation features
    for(const auto& l : layers.layers())
    {
        //get render settings
        assert(render_settings_map.count(l.first));
        const auto& rs = render_settings_map.at(l.first);

        //render layer
        auto render_result = Grid2DLayerRenderer::render(*l.second, rs);

        //loginf << "rendered image size: " << render_result.first.width() << "x" << render_result.first.height();
        //render_result.first.save(QString::fromStdString("/home/mcphatty/layer_" + l.first + ".png"));

        //create geo image annotation
        std::unique_ptr<ViewPointGenFeatureGeoImage> geo_image(new ViewPointGenFeatureGeoImage(render_result.first,
                                                                                               render_result.second));
        geo_image->setName(l.first);

        std::string anno_name = annotation_name;
        anno_name += (annotation_name.empty() ? "" : "_") + l.first;

        std::unique_ptr<ViewPointGenAnnotation> a(new ViewPointGenAnnotation(anno_name, false));
        a->addFeature(std::move(geo_image));

        annotations.addAnnotation(std::move(a));
    }

    loginf << "Base: addGrids: created " << annotations.size() << " feature(s)";

    //add created annotations as JSON 
    for (size_t i = 0; i < annotations.size(); ++i)
    {
        nlohmann::json a_info;
        annotations.annotation(i).toJSON(a_info);

        annotations_json.push_back(a_info);
    }
}

/**
*/
void Base::addHistograms(nlohmann::json& annotations_json, 
                         const std::string& annotation_name, 
                         const std::vector<AnnotationDefinitions::HistogramDefinition>& defs) const
{
    if (!annotations_json.is_array())
        return;

    RawHistogramCollection hcollection;

    for (const auto& def : defs)
    {
        if (!def.isValid())
            continue;

        auto values = getValues(def.value_source);
        if (values.empty())
            return;

        //@TODO: integrate other value types (e.g. bool and int)
        HistogramInitializer<double> init(eval_man_.settings().histogram_num_bins);
        init.scan(values);

        auto config = init.currentConfiguration();

        //override initializer auto config?
        if (def.force_range_histogram)
        {
            config          = HistogramConfig();
            config.type     = HistogramConfig::Type::Range;
            config.num_bins = eval_man_.settings().histogram_num_bins;
        }

        //init histogram using config
        HistogramT<double> histogram;
        if (!init.initHistogram(histogram, config))
            continue;

        //add values
        histogram.add(values);

        hcollection.addLayer(histogram.toRaw(), def.name, HistogramColorDefault);
    }

    if (hcollection.numLayers() == 0)
        return;

    ViewPointGenAnnotation annotation(annotation_name, true);

    std::unique_ptr<ViewPointGenFeatureHistogram> feat_h;
    feat_h.reset(new ViewPointGenFeatureHistogram(hcollection));
    feat_h->setName(annotation_name);

    annotation.addFeature(std::move(feat_h));

    nlohmann::json feat_json;
    annotation.toJSON(feat_json);

    annotations_json.push_back(feat_json);
}

}
