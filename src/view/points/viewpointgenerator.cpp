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

#include "viewpointgenerator.h"
#include "viewpoint.h"

#include "util/stringconv.h"

/********************************************************************************
 * ViewPointGenFilters
 ********************************************************************************/

/**
*/
void ViewPointGenFilters::addFilter(std::unique_ptr<ViewPointGenFilter>&& filter)
{
    filters_[filter->name()] = std::move(filter);
}

/**
*/
void ViewPointGenFilters::toJSON(nlohmann::json& j) const
{
    for (const auto& f : filters_)
    {
        nlohmann::json j_filter;
        f.second->toJSON(j_filter);

        j[ f.first ] = j_filter;
    }
}

/********************************************************************************
 * ViewPointGenFilterUTN
 ********************************************************************************/

const std::string ViewPointGenFilterUTN::UTNFilterNodeName  = "UTNs";
const std::string ViewPointGenFilterUTN::UTNFilterFieldName = "utns";

/**
*/
ViewPointGenFilterUTN::ViewPointGenFilterUTN(const std::vector<uint32_t>& utns)
:   utns_(utns)
{
}

/**
*/
ViewPointGenFilterUTN::ViewPointGenFilterUTN(uint32_t utn)
{
    utns_.push_back(utn);
}

/**
*/
void ViewPointGenFilterUTN::toJSON(nlohmann::json& j) const
{
    if (utns_.empty())
        return;

    j[UTNFilterFieldName] = Utils::String::compress(utns_, ',');
}

/********************************************************************************
 * ViewPointGenFeature
 ********************************************************************************/

const std::string ViewPointGenFeature::FeatureTypeFieldName      = "type";
const std::string ViewPointGenFeature::FeatureTypeNameFeat       = "feature";
const std::string ViewPointGenFeature::FeatureFieldNameGeom      = "geometry";
const std::string ViewPointGenFeature::FeatureFieldNameGeomType  = "type";
const std::string ViewPointGenFeature::FeatureFieldNameCoords    = "coordinates";
const std::string ViewPointGenFeature::FeatureFieldNameColors    = "colors";
const std::string ViewPointGenFeature::FeatureFieldNameProps     = "properties";
const std::string ViewPointGenFeature::FeatureFieldNamePropColor = "color";

/**
*/
ViewPointGenFeature::ViewPointGenFeature(const std::string& type)
:   type_(type)
{
}

/**
*/
void ViewPointGenFeature::toJSON(nlohmann::json& j) const
{
    j[FeatureTypeFieldName] = type_;

    toJSON_impl(j);
}

/**
*/
void ViewPointGenFeature::print(std::ostream& strm, const std::string& prefix) const
{
    strm << prefix << "[Feature]" << std::endl;
    strm << prefix << "name: " << name() << std::endl;
    strm << prefix << "type: " << type_ << std::endl;
    strm << prefix << "size: " << size() << std::endl;
}

/********************************************************************************
 * ViewPointGenFeaturePointGeometry
 ********************************************************************************/

/**
*/
ViewPointGenFeaturePointGeometry::ViewPointGenFeaturePointGeometry(const std::string& geom_type,
                                                                   const std::vector<Eigen::Vector2d>& positions,
                                                                   const std::vector<QColor>& colors)
:   ViewPointGenFeature()
,   geom_type_      (geom_type)
,   positions_      (positions)
,   colors_         (colors   )
{
}

/**
*/
void ViewPointGenFeaturePointGeometry::reserve(size_t n, bool reserve_cols)
{
    positions_.reserve(n);
    if (reserve_cols)
        colors_.reserve(n);
}

/**
*/
void ViewPointGenFeaturePointGeometry::addPoint(const Eigen::Vector2d& pos, 
                                                const boost::optional<QColor>& color)
{
    positions_.push_back(pos);
    if (color.has_value())
        colors_.push_back(color.value());
}

/**
*/
void ViewPointGenFeaturePointGeometry::addPoints(const std::vector<Eigen::Vector2d>& positions,
                                                 const boost::optional<std::vector<QColor>>& colors)
{
    positions_.insert(positions_.end(), positions.begin(), positions.end());

    if (colors.has_value())
        colors_.insert(colors_.end(), colors->begin(), colors->end());
}

/**
*/
void ViewPointGenFeaturePointGeometry::toJSON_impl(nlohmann::json& j) const
{
    nlohmann::json geom;
    geom[FeatureFieldNameGeomType] = geom_type_;

    nlohmann::json props;

    auto coords = nlohmann::json::array();

    for (const auto& pos : positions_)
    {
        auto coord = nlohmann::json::array();
        coord.push_back(pos.x());
        coord.push_back(pos.y());

        coords.push_back(coord);
    }

    geom[FeatureFieldNameCoords] = coords; 
    
    if (positions_.size() == colors_.size())
    {
        auto colors = nlohmann::json::array();

        for (const auto& c : colors_)
        {
            nlohmann::json color = c.name().toStdString();
            colors.push_back(color);
        }

        geom[FeatureFieldNameColors] = colors;
    }
    else
    {
        props[FeatureFieldNamePropColor] = color_.name().toStdString();
    }

    writeGeometry(geom);
    writeProperties(props);

    j[FeatureFieldNameGeom ] = geom;
    j[FeatureFieldNameProps] = props; 
}

/********************************************************************************
 * ViewPointGenFeaturePoints
 ********************************************************************************/

const std::string ViewPointGenFeaturePoints::FeaturePointsTypeName            = "points";
const std::string ViewPointGenFeaturePoints::FeaturePointsFieldNameSymbol     = "symbol";
const std::string ViewPointGenFeaturePoints::FeaturePointsFieldNameSymbolSize = "symbol_size";

const std::string ViewPointGenFeaturePoints::SymbolNameCircle   = "circle";
const std::string ViewPointGenFeaturePoints::SymbolNameTriangle = "triangle";
const std::string ViewPointGenFeaturePoints::SymbolNameSquare   = "square";
const std::string ViewPointGenFeaturePoints::SymbolNameCross    = "cross";

/**
*/
ViewPointGenFeaturePoints::ViewPointGenFeaturePoints(Symbol symbol,
                                                     float symbol_size,
                                                     const std::vector<Eigen::Vector2d>& positions,
                                                     const std::vector<QColor>& colors)
:   ViewPointGenFeaturePointGeometry(FeaturePointsTypeName, positions, colors)
,   symbol_     (symbol     )
,   symbol_size_(symbol_size)
{
}

/**
*/
std::string ViewPointGenFeaturePoints::symbolString() const
{
    if (symbol_ == Symbol::Circle)
        return SymbolNameCircle;
    if (symbol_ == Symbol::Triangle)
        return SymbolNameTriangle;
    if (symbol_ == Symbol::Square)
        return SymbolNameSquare;
    if (symbol_ == Symbol::Cross)
        return SymbolNameCross;
    return "";
}

/**
*/
void ViewPointGenFeaturePoints::writeProperties(nlohmann::json& j) const
{
    j[FeaturePointsFieldNameSymbol    ] = symbolString();
    j[FeaturePointsFieldNameSymbolSize] = symbol_size_;
}

/********************************************************************************
 * ViewPointGenFeatureLineString
 ********************************************************************************/

const std::string ViewPointGenFeatureLineString::FeatureLineStringTypeName           = "line_string";
const std::string ViewPointGenFeatureLineString::FeatureLineStringInterpTypeName     = "line_string_interpolated";
const std::string ViewPointGenFeatureLineString::FeatureLineStringFieldNameLineWidth = "line_width";

/**
*/
ViewPointGenFeatureLineString::ViewPointGenFeatureLineString(bool interpolated,
                                                             float line_width,
                                                             const std::vector<Eigen::Vector2d>& positions,
                                                             const std::vector<QColor>& colors)
:   ViewPointGenFeaturePointGeometry(interpolated ? FeatureLineStringInterpTypeName : FeatureLineStringTypeName, positions, colors)
,   line_width_(line_width)
{
}

/**
*/
void ViewPointGenFeatureLineString::writeProperties(nlohmann::json& j) const
{
    j[FeatureLineStringFieldNameLineWidth] = line_width_;
}

/********************************************************************************
 * ViewPointGenFeatureLines
 ********************************************************************************/

const std::string ViewPointGenFeatureLines::FeatureLinesTypeName           = "lines";
const std::string ViewPointGenFeatureLines::FeatureLinesFieldNameLineWidth = "line_width";

/**
*/
ViewPointGenFeatureLines::ViewPointGenFeatureLines(float line_width,
                                                   const std::vector<Eigen::Vector2d>& positions,
                                                   const std::vector<QColor>& colors)
:   ViewPointGenFeaturePointGeometry(FeatureLinesTypeName, positions, colors)
,   line_width_(line_width)
{
}

/**
*/
void ViewPointGenFeatureLines::writeProperties(nlohmann::json& j) const
{
    j[FeatureLinesFieldNameLineWidth] = line_width_;
}

/********************************************************************************
 * ViewPointGenFeatureErrorEllipses
 ********************************************************************************/

const std::string ViewPointGenFeatureErrEllipses::FeatureErrEllipsesTypeName           = "ellipses";
const std::string ViewPointGenFeatureErrEllipses::FeatureErrEllipsesFieldNameLineWidth = "line_width";
const std::string ViewPointGenFeatureErrEllipses::FeatureErrEllipsesFieldNameNumPoints = "num_points";
const std::string ViewPointGenFeatureErrEllipses::FeatureErrEllipsesFieldNameSizes     = "sizes";

/**
*/
ViewPointGenFeatureErrEllipses::ViewPointGenFeatureErrEllipses(float line_width,
                                                               size_t num_points,
                                                               const std::vector<Eigen::Vector2d>& positions,
                                                               const std::vector<QColor>& colors,
                                                               const std::vector<Eigen::Vector3d>& sizes)
:   ViewPointGenFeaturePointGeometry(FeatureErrEllipsesTypeName, positions, colors)
,   line_width_(line_width)
,   num_points_(num_points)
,   sizes_     (sizes     )
{
}

/**
*/
void ViewPointGenFeatureErrEllipses::reserve(size_t n, bool reserve_cols)
{
    ViewPointGenFeaturePointGeometry::reserve(n, reserve_cols);

    sizes_.reserve(n);
}

/**
*/
void ViewPointGenFeatureErrEllipses::addSize(const Eigen::Vector3d& size)
{
    sizes_.push_back(size);
}

/**
*/
void ViewPointGenFeatureErrEllipses::addSizes(const std::vector<Eigen::Vector3d>& sizes)
{
    sizes_.insert(sizes_.end(), sizes.begin(), sizes.end());
}

/**
*/
void ViewPointGenFeatureErrEllipses::writeGeometry(nlohmann::json& j) const
{
    nlohmann::json sizes = nlohmann::json::array();
    for (const auto& s : sizes_)
    {
        nlohmann::json size = nlohmann::json::array();
        size.push_back(s.x());
        size.push_back(s.y());
        size.push_back(s.z());

        sizes.push_back(size);
    }

    j[FeatureErrEllipsesFieldNameSizes] = sizes;
}

/**
*/
void ViewPointGenFeatureErrEllipses::writeProperties(nlohmann::json& j) const
{
    j[FeatureErrEllipsesFieldNameLineWidth] = line_width_;
    j[FeatureErrEllipsesFieldNameNumPoints] = num_points_;
}

/********************************************************************************
 * ViewPointGenFeatureText
 ********************************************************************************/

const std::string ViewPointGenFeatureText::FeatureTypeNameText          = "text";
const std::string ViewPointGenFeatureText::FeatureTextFieldNameText     = "text";
const std::string ViewPointGenFeatureText::FeatureTextFieldNamePos      = "position";
const std::string ViewPointGenFeatureText::FeatureTextFieldNameDir      = "direction";
const std::string ViewPointGenFeatureText::FeatureTextFieldNameFontSize = "font_size";

const std::string ViewPointGenFeatureText::TextDirNameRightUp   = "right-up";
const std::string ViewPointGenFeatureText::TextDirNameRightDown = "right-down";
const std::string ViewPointGenFeatureText::TextDirNameLeftUp    = "left-up";
const std::string ViewPointGenFeatureText::TextDirNameLeftDown  = "left-down";

/**
*/
ViewPointGenFeatureText::ViewPointGenFeatureText(const std::string& text,
                                                 double x,
                                                 double y,
                                                 float font_size,
                                                 TextDirection text_dir)
:   ViewPointGenFeature(FeatureTypeNameText)
,   text_           (text     )
,   x_              (x        )
,   y_              (y        )
,   font_size_      (font_size)
,   text_dir_       (text_dir )
{
}

/**
*/
std::string ViewPointGenFeatureText::textDirString() const
{
    if (text_dir_ == TextDirection::RightUp)
        return TextDirNameRightUp;
    if (text_dir_ == TextDirection::RightDown)
        return TextDirNameRightDown;
    if (text_dir_ == TextDirection::LeftUp)
        return TextDirNameLeftUp;
    if (text_dir_ == TextDirection::LeftDown)
        return TextDirNameLeftDown;
    return "";
}

/**
*/
void ViewPointGenFeatureText::toJSON_impl(nlohmann::json& j) const
{
    j[FeatureTextFieldNameText] = text_;

    nlohmann::json pos = nlohmann::json::array();
    pos.push_back(x_);
    pos.push_back(y_);
    j[FeatureTextFieldNamePos] = pos;

    nlohmann::json props;
    props[FeatureTextFieldNameFontSize] = font_size_;
    props[FeatureFieldNamePropColor   ] = color_.name().toStdString();
    props[FeatureTextFieldNameDir     ] = textDirString();

    j[FeatureFieldNameProps] = props;
}

/********************************************************************************
 * ViewPointGenAnnotation
 ********************************************************************************/

const std::string ViewPointGenAnnotation::AnnotationFieldName        = "name";
const std::string ViewPointGenAnnotation::AnnotationFieldHidden      = "hidden";
const std::string ViewPointGenAnnotation::AnnotationFieldSymbolColor = "symbol_color";
const std::string ViewPointGenAnnotation::AnnotationFieldFeatures    = "features";
const std::string ViewPointGenAnnotation::AnnotationFieldAnnotations = "annotations";

/**
*/
ViewPointGenAnnotation::ViewPointGenAnnotation(const std::string& name,
                                               bool hidden)
:   name_  (name  )
,   hidden_(hidden)
{
}

/**
*/
void ViewPointGenAnnotation::setSymbolColor(const QColor& color)
{
    symbol_color_ = color;
}

/**
*/
void ViewPointGenAnnotation::addFeature(std::unique_ptr<ViewPointGenFeature>&& feat)
{
    features_.push_back(std::move(feat));
}

/**
*/
ViewPointGenAnnotation* ViewPointGenAnnotation::addAnnotation(const std::string& name, bool hidden)
{
    annotations_.emplace_back(new ViewPointGenAnnotation(name, hidden));
    return annotations_.back().get();
}

/**
*/
void ViewPointGenAnnotation::addAnnotation(std::unique_ptr<ViewPointGenAnnotation>&& a)
{
    annotations_.push_back(std::move(a));
}

/**
*/
void ViewPointGenAnnotation::toJSON(nlohmann::json& j) const
{
    j[AnnotationFieldName  ] = name_;
    j[AnnotationFieldHidden] = hidden_;

    if (symbol_color_.has_value())
        j[AnnotationFieldSymbolColor] = symbol_color_.value().name().toStdString();

    nlohmann::json feats = nlohmann::json::array();

    for (const auto& f : features_)
    {
        nlohmann::json feat;
        f->toJSON(feat);

        feats.push_back(feat);
    }

    j[AnnotationFieldFeatures] = feats;

    nlohmann::json annos = nlohmann::json::array();

    for (const auto& a : annotations_)
    {
        nlohmann::json anno;
        a->toJSON(anno);

        annos.push_back(anno);
    }

    j[AnnotationFieldAnnotations] = annos;
}

/**
*/
void ViewPointGenAnnotation::print(std::ostream& strm, const std::string& prefix) const
{
    strm << prefix << "[Annotation]" << std::endl;
    strm << prefix << "name:   " << name_ << std::endl;
    strm << prefix << "hidden: " << hidden_ << std::endl;

    std::string p2 = prefix + "   ";

    for (const auto& f : features_)
        f->print(strm, p2);

    for (const auto& a : annotations_)
        a->print(strm, p2);
}

/********************************************************************************
 * ViewPointGenAnnotations
 ********************************************************************************/

/**
*/
ViewPointGenAnnotation* ViewPointGenAnnotations::addAnnotation(const std::string& name, bool hidden)
{
    annotations_.emplace_back(new ViewPointGenAnnotation(name, hidden));
    return annotations_.back().get();
}

/**
*/
void ViewPointGenAnnotations::addAnnotation(std::unique_ptr<ViewPointGenAnnotation>&& a)
{
    annotations_.push_back(std::move(a));
}

/**
*/
void ViewPointGenAnnotations::toJSON(nlohmann::json& j) const
{
    j = nlohmann::json::array();

    for (const auto& a : annotations_)
    {
        nlohmann::json anno;
        a->toJSON(anno);

        j.push_back(anno);
    }
}

/**
*/
void ViewPointGenAnnotations::print(std::ostream& strm, const std::string& prefix) const
{
    for (const auto& a : annotations_)
        a->print(strm, prefix);
}

/********************************************************************************
 * ViewPointGenVP
 ********************************************************************************/

const std::string ViewPointGenVP::ViewPointFieldName        = "name";
const std::string ViewPointGenVP::ViewPointFieldID          = "id";
const std::string ViewPointGenVP::ViewPointFieldType        = "type";
const std::string ViewPointGenVP::ViewPointFieldStatus      = "status";
const std::string ViewPointGenVP::ViewPointFieldAnnotations = "annotations";
const std::string ViewPointGenVP::ViewPointFieldFilters     = "filters";
const std::string ViewPointGenVP::ViewPointFieldPosLat      = "position_latitude";
const std::string ViewPointGenVP::ViewPointFieldPosLon      = "position_longitude";
const std::string ViewPointGenVP::ViewPointFieldWinLat      = "position_window_latitude";
const std::string ViewPointGenVP::ViewPointFieldWinLon      = "position_window_longitude";

const std::string ViewPointGenVP::StatusNameOpen   = "open";
const std::string ViewPointGenVP::StatusNameClosed = "closed";
const std::string ViewPointGenVP::StatusNameTodo   = "todo";

/**
*/
ViewPointGenVP::ViewPointGenVP(const std::string& name,
                               unsigned int id,
                               const std::string& type,
                               const QRectF& roi,
                               Status status)
:   name_  (name)
,   id_    (id)
,   type_  (type)
,   roi_   (roi)
,   status_(status)
{
}

/**
*/
void ViewPointGenVP::addCustomField(const std::string& name, const QVariant& value)
{
    custom_fields_[ name ] = value;
}

/**
*/
std::string ViewPointGenVP::statusString() const
{
    if (status_ == Status::Open)
        return StatusNameOpen;
    if (status_ == Status::Closed)
        return StatusNameClosed;
    if (status_ == Status::Todo)
        return StatusNameTodo;
    return "";
}

/**
*/
void ViewPointGenVP::toJSON(nlohmann::json& j) const
{
    j[ViewPointFieldName  ] = name_;
    j[ViewPointFieldID    ] = id_;
    j[ViewPointFieldType  ] = type_;
    j[ViewPointFieldStatus] = statusString();

    if (!roi_.isEmpty())
    {
        j[ViewPointFieldPosLat] = roi_.x();
        j[ViewPointFieldPosLon] = roi_.y();
        j[ViewPointFieldWinLat] = roi_.width();
        j[ViewPointFieldWinLon] = roi_.height();
    }

    nlohmann::json annotations;
    annotations_.toJSON(annotations);
    j[ViewPointFieldAnnotations] = annotations;

    nlohmann::json filters;
    filters_.toJSON(filters);
    j[ViewPointFieldFilters] = filters;

    for (const auto& cf : custom_fields_)
    {
        if (cf.second.type() == QVariant::Type::Int)
            j[cf.first] = cf.second.toInt();
        else if (cf.second.type() == QVariant::Type::UInt)
            j[cf.first] = cf.second.toUInt();
        else if (cf.second.type() == QVariant::Type::Double)
            j[cf.first] = cf.second.toDouble();
        else if (cf.second.type() == QVariant::Type::Bool)
            j[cf.first] = cf.second.toBool();
        else if (cf.second.type() == QVariant::Type::String)
            j[cf.first] = cf.second.toString().toStdString();
    }
}

/**
*/
void ViewPointGenVP::print(std::ostream& strm, const std::string& prefix) const
{
    strm << prefix << "[Viewpoint]" << std::endl;
    strm << prefix << "id:     " << id_ << std::endl;
    strm << prefix << "name:   " << name_ << std::endl;
    strm << prefix << "type:   " << type_ << std::endl;
    strm << prefix << "roi:    " << roi_.x() << "," << roi_.y() << " " << roi_.width() << "x" << roi_.height() << std::endl;
    strm << prefix << "status: " << statusString() << std::endl;

    std::string p2 = prefix + "   ";

    annotations_.print(strm, p2);
}

/**
*/
bool ViewPointGenVP::hasAnnotations(const nlohmann::json& vp_json)
{
    if (!vp_json.is_object())
        return false;

    if (vp_json.count(ViewPointFieldAnnotations) == 0)
        return false;

    auto node = vp_json[ViewPointFieldAnnotations];
    if (!node.is_array() || node.size() < 1)
        return false;

    return true;
}

/********************************************************************************
 * ViewPointGenerator
 ********************************************************************************/

const std::string ViewPointGenerator::ViewPointsFieldVersion     = "content_version";
const std::string ViewPointGenerator::ViewPointsFieldContentType = "content_type";
const std::string ViewPointGenerator::ViewPointsFieldViewPoints  = "view_points";

/**
*/
ViewPointGenVP* ViewPointGenerator::addViewPoint(const std::string& name,
                                                 unsigned int id,
                                                 const std::string& type,
                                                 const QRectF& roi,
                                                 ViewPointGenVP::Status status)
{
    view_points_.emplace_back(new ViewPointGenVP(name, id, type, roi, status));
    return view_points_.back().get();
}

/**
*/
void ViewPointGenerator::addViewPoint(std::unique_ptr<ViewPointGenVP>&& vp)
{
    view_points_.push_back(std::move(vp));
}

/**
*/
void ViewPointGenerator::toJSON(nlohmann::json& j, 
                                bool with_viewpoints_only,
                                bool with_annotations_only) const
{
    nlohmann::json vp_content;

    vp_content[ViewPointsFieldContentType] = "view_points";
    vp_content[ViewPointsFieldVersion    ] = VP_COLLECTION_CONTENT_VERSION;

    nlohmann::json viewpoints = nlohmann::json::array();

    size_t added = 0;

    for (const auto& vp : view_points_)
    {
        if (with_annotations_only && !vp->hasAnnotations())
            continue;

        nlohmann::json viewpoint;
        vp->toJSON(viewpoint);

        viewpoints.push_back(viewpoint);

        ++added;
    }

    vp_content[ViewPointsFieldViewPoints] = viewpoints;

    if (with_viewpoints_only && added == 0)
        return;

    j = vp_content;
}

/**
*/
nlohmann::json ViewPointGenerator::toJSON(bool with_viewpoints_only,
                                          bool with_annotations_only) const
{
    nlohmann::json j;
    toJSON(j, with_viewpoints_only, with_annotations_only);

    return j;
}

/**
*/
boost::optional<nlohmann::json> ViewPointGenerator::viewPointJSON(const nlohmann::json& vps_json, 
                                                                  size_t idx, 
                                                                  bool with_annotations_only)
{
    if (!vps_json.is_object())
        return {};

    if (vps_json.count(ViewPointsFieldViewPoints) == 0)
        return {};

    auto vps_node = vps_json[ViewPointsFieldViewPoints];
    if (!vps_node.is_array() || vps_node.size() <= idx)
        return {};

    auto vp_node = vps_node.at(idx);
    if (!vp_node.is_object())
        return {};

    if (with_annotations_only && !ViewPointGenVP::hasAnnotations(vp_node))
        return {};

    return vp_node;
}
