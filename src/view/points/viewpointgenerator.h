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

#pragma once

#include <memory>
#include <vector>
#include <map>

#include <Eigen/Core>

#include <QColor>
#include <QRectF>
#include <QVariant>

#include <boost/optional.hpp>

#include "json.h"

/**
*/
class ViewPointGenFilter
{
public:
    ViewPointGenFilter() = default;
    virtual ~ViewPointGenFilter() = default;

    virtual std::string name() const = 0;
    virtual void toJSON(nlohmann::json& j) const = 0;
};

/**
*/
class ViewPointGenFilters
{
public:
    ViewPointGenFilters() = default;
    virtual ~ViewPointGenFilters() = default;

    void addFilter(std::unique_ptr<ViewPointGenFilter>&& filter);

    void toJSON(nlohmann::json& j) const;

private:
    std::map<std::string, std::unique_ptr<ViewPointGenFilter>> filters_;
};

/**
*/
class ViewPointGenFilterUTN : public ViewPointGenFilter
{
public:
    ViewPointGenFilterUTN(const std::vector<uint32_t>& utns);
    ViewPointGenFilterUTN(uint32_t utn);
    virtual ~ViewPointGenFilterUTN() = default;

    virtual std::string name() const override { return UTNFilterNodeName; }
    virtual void toJSON(nlohmann::json& j) const override;

    static const std::string UTNFilterNodeName;
    static const std::string UTNFilterFieldName;

private:
    std::vector<uint32_t> utns_;
};

/**
*/
class ViewPointGenFeature
{
public:
    ViewPointGenFeature(const std::string& type = FeatureTypeNameFeat);
    virtual ~ViewPointGenFeature() = default;

    void setColor(const QColor& c) { color_ = c; }

    void toJSON(nlohmann::json& j) const;
    void print(std::ostream& strm, const std::string& prefix = "") const;

    virtual size_t size() const = 0;
    virtual std::string name() const = 0;

    static const std::string FeatureTypeFieldName;
    static const std::string FeatureTypeNameFeat;
    static const std::string FeatureFieldNameGeom;
    static const std::string FeatureFieldNameGeomType;
    static const std::string FeatureFieldNameCoords;
    static const std::string FeatureFieldNameColors;
    static const std::string FeatureFieldNameProps;
    static const std::string FeatureFieldNamePropColor;

protected:
    virtual void toJSON_impl(nlohmann::json& j) const = 0;

    QColor color_;

private:
    std::string type_;
};

/**
*/
class ViewPointGenFeaturePointGeometry : public ViewPointGenFeature
{
public:
    ViewPointGenFeaturePointGeometry(const std::string& geom_type,
                                     const std::vector<Eigen::Vector2d>& positions = std::vector<Eigen::Vector2d>(),
                                     const std::vector<QColor>& colors = std::vector<QColor>());
    virtual ~ViewPointGenFeaturePointGeometry() = default;

    virtual void reserve(size_t n, bool reserve_cols);
    virtual size_t size() const override { return positions_.size(); }
    virtual std::string name() const override { return geom_type_; }

    void addPoint(const Eigen::Vector2d& pos, 
                  const boost::optional<QColor>& color = boost::optional<QColor>());
    void addPoints(const std::vector<Eigen::Vector2d>& positions,
                   const boost::optional<std::vector<QColor>>& colors = boost::optional<std::vector<QColor>>());

protected:
    virtual void toJSON_impl(nlohmann::json& j) const override;

    virtual void writeGeometry(nlohmann::json& j) const {};
    virtual void writeProperties(nlohmann::json& j) const {};

private:
    std::string                  geom_type_;
    std::vector<Eigen::Vector2d> positions_;
    std::vector<QColor>          colors_;
};

/**
*/
class ViewPointGenFeaturePoints : public ViewPointGenFeaturePointGeometry
{
public:
    enum class Symbol
    {
        Circle = 0,
        Triangle,
        Square,
        Cross
    };

    ViewPointGenFeaturePoints(Symbol symbol = Symbol::Square,
                              float symbol_size = 6.0f,
                              const std::vector<Eigen::Vector2d>& positions = std::vector<Eigen::Vector2d>(),
                              const std::vector<QColor>& colors = std::vector<QColor>());
    virtual ~ViewPointGenFeaturePoints() = default;

    static const std::string FeaturePointsTypeName;
    static const std::string FeaturePointsFieldNameSymbol;
    static const std::string FeaturePointsFieldNameSymbolSize;

    static const std::string SymbolNameCircle;
    static const std::string SymbolNameTriangle;
    static const std::string SymbolNameSquare;
    static const std::string SymbolNameCross;

protected:
    virtual void writeProperties(nlohmann::json& j) const override;

private:
    std::string symbolString() const;

    Symbol symbol_;
    float  symbol_size_;
};

/**
*/
class ViewPointGenFeatureLineString : public ViewPointGenFeaturePointGeometry
{
public:
    ViewPointGenFeatureLineString(bool interpolated,
                                  float line_width = 1.0f,
                                  const std::vector<Eigen::Vector2d>& positions = std::vector<Eigen::Vector2d>(),
                                  const std::vector<QColor>& colors = std::vector<QColor>());
    virtual ~ViewPointGenFeatureLineString() = default;

    static const std::string FeatureLineStringTypeName;
    static const std::string FeatureLineStringInterpTypeName;
    static const std::string FeatureLineStringFieldNameLineWidth;

protected:
    virtual void writeProperties(nlohmann::json& j) const override;

private:
    float line_width_;
};

/**
*/
class ViewPointGenFeatureLines : public ViewPointGenFeaturePointGeometry
{
public:
    ViewPointGenFeatureLines(float line_width = 1.0f,
                             const std::vector<Eigen::Vector2d>& positions = std::vector<Eigen::Vector2d>(),
                             const std::vector<QColor>& colors = std::vector<QColor>());
    virtual ~ViewPointGenFeatureLines() = default;

    static const std::string FeatureLinesTypeName;
    static const std::string FeatureLinesFieldNameLineWidth;

protected:
    virtual void writeProperties(nlohmann::json& j) const override;

private:
    float line_width_;
};

/**
*/
class ViewPointGenFeatureErrEllipses : public ViewPointGenFeaturePointGeometry
{
public:
    ViewPointGenFeatureErrEllipses(float line_width = 1.0f,
                                   size_t num_points = 32,
                                   const std::vector<Eigen::Vector2d>& positions = std::vector<Eigen::Vector2d>(),
                                   const std::vector<QColor>& colors = std::vector<QColor>(),
                                   const std::vector<Eigen::Vector3d>& sizes = std::vector<Eigen::Vector3d>());
    virtual ~ViewPointGenFeatureErrEllipses() = default;

    virtual void reserve(size_t n, bool reserve_cols) override;

    void addSize(const Eigen::Vector3d& size);
    void addSizes(const std::vector<Eigen::Vector3d>& sizes);

    static const std::string FeatureErrEllipsesTypeName;
    static const std::string FeatureErrEllipsesFieldNameLineWidth;
    static const std::string FeatureErrEllipsesFieldNameNumPoints;
    static const std::string FeatureErrEllipsesFieldNameSizes;

protected:
    virtual void writeGeometry(nlohmann::json& j) const override;
    virtual void writeProperties(nlohmann::json& j) const override;

private:
    float  line_width_;
    size_t num_points_;

    std::vector<Eigen::Vector3d> sizes_;
};

/**
*/
class ViewPointGenFeatureText : public ViewPointGenFeature
{
public:
    enum class TextDirection
    {
        RightUp = 0,
        RightDown,
        LeftUp,
        LeftDown
    };

    ViewPointGenFeatureText(const std::string& text,
                            double x,
                            double y,
                            float font_size = 10.0,
                            TextDirection text_dir = TextDirection::RightUp);
    virtual ~ViewPointGenFeatureText() = default;

    virtual size_t size() const { return 1; }
    virtual std::string name() const override { return "text"; }

    static const std::string FeatureTypeNameText;
    static const std::string FeatureTextFieldNameText;
    static const std::string FeatureTextFieldNamePos;
    static const std::string FeatureTextFieldNameDir;
    static const std::string FeatureTextFieldNameFontSize;

    static const std::string TextDirNameRightUp;
    static const std::string TextDirNameRightDown;
    static const std::string TextDirNameLeftUp;
    static const std::string TextDirNameLeftDown;

protected:
    virtual void toJSON_impl(nlohmann::json& j) const override;

private:
    std::string textDirString() const;

    std::string   text_;
    double        x_;
    double        y_;
    float         font_size_;
    TextDirection text_dir_;
};

/**
*/
class ViewPointGenAnnotation
{
public:
    ViewPointGenAnnotation(const std::string& name,
                           bool hidden = false);
    virtual ~ViewPointGenAnnotation() = default;

    void setSymbolColor(const QColor& color);

    void addFeature(std::unique_ptr<ViewPointGenFeature>&& feat);

    template<class T, typename... Arguments>
    T* addFeature(Arguments... args)
    {
        T* ptr = new T(args...);
        features_.push_back(std::unique_ptr<T>(ptr));
        return ptr;
    }

    ViewPointGenAnnotation* addAnnotation(const std::string& name, bool hidden = false);
    void addAnnotation(std::unique_ptr<ViewPointGenAnnotation>&& a);

    size_t size() const { return annotations_.size(); }
    size_t numFeatures() const { return features_.size(); }

    void toJSON(nlohmann::json& j) const;
    void print(std::ostream& strm, const std::string& prefix = "") const;

    static const std::string AnnotationFieldName;
    static const std::string AnnotationFieldHidden;
    static const std::string AnnotationFieldSymbolColor;
    static const std::string AnnotationFieldFeatures;
    static const std::string AnnotationFieldAnnotations;

private:
    std::string             name_;
    bool                    hidden_;
    boost::optional<QColor> symbol_color_;

    std::vector<std::unique_ptr<ViewPointGenFeature>>    features_;
    std::vector<std::unique_ptr<ViewPointGenAnnotation>> annotations_;
};

/**
*/
class ViewPointGenAnnotations
{
public:
    ViewPointGenAnnotations() = default;
    virtual ~ViewPointGenAnnotations() = default;

    ViewPointGenAnnotation* addAnnotation(const std::string& name, bool hidden = false);
    void addAnnotation(std::unique_ptr<ViewPointGenAnnotation>&& a);

    size_t size() const { return annotations_.size(); }
    const ViewPointGenAnnotation& annotation(size_t idx) const { return *annotations_.at(idx); }

    void toJSON(nlohmann::json& j) const;
    void print(std::ostream& strm, const std::string& prefix = "") const;

private:
    std::vector<std::unique_ptr<ViewPointGenAnnotation>> annotations_;
};

/**
*/
class ViewPointGenVP
{
public:
    enum class Status
    {
        Open = 0, 
        Closed,
        Todo
    };

    ViewPointGenVP(const std::string& name,
                   unsigned int id,
                   const std::string& type,
                   const QRectF& roi = QRectF(),
                   Status status = Status::Open);
    virtual ~ViewPointGenVP() = default;

    bool hasAnnotations() const { return (annotations_.size() > 0); }
    static bool hasAnnotations(const nlohmann::json& vp_json);

    void setROI(const QRectF& roi) { roi_ = roi; }

    void addCustomField(const std::string& name, const QVariant& value);

    ViewPointGenAnnotations& annotations() { return annotations_; }
    ViewPointGenFilters& filters() { return filters_; }

    void toJSON(nlohmann::json& j) const;
    void print(std::ostream& strm, const std::string& prefix = "") const;

    std::string statusString() const;

    static const std::string ViewPointFieldName;
    static const std::string ViewPointFieldID;
    static const std::string ViewPointFieldType;
    static const std::string ViewPointFieldStatus;
    static const std::string ViewPointFieldAnnotations;
    static const std::string ViewPointFieldFilters;
    static const std::string ViewPointFieldPosLat;
    static const std::string ViewPointFieldPosLon;
    static const std::string ViewPointFieldWinLat;
    static const std::string ViewPointFieldWinLon;

    static const std::string StatusNameOpen;
    static const std::string StatusNameClosed;
    static const std::string StatusNameTodo;

private:
    std::string  name_;
    unsigned int id_;
    std::string  type_;
    QRectF       roi_;
    Status       status_;

    ViewPointGenAnnotations annotations_;
    ViewPointGenFilters     filters_;

    std::map<std::string, QVariant> custom_fields_;
};

/**
*/
class ViewPointGenerator
{
public:
    ViewPointGenerator() = default;
    virtual ~ViewPointGenerator() = default;

    ViewPointGenVP* addViewPoint(const std::string& name,
                                 unsigned int id,
                                 const std::string& type,
                                 const QRectF& roi = QRectF(),
                                 ViewPointGenVP::Status status = ViewPointGenVP::Status::Open);
    void addViewPoint(std::unique_ptr<ViewPointGenVP>&& vp);

    size_t size() const { return view_points_.size(); }

    static const std::string Version;

    static const std::string ViewPointsFieldVersion;
    static const std::string ViewPointsFieldContentType;
    static const std::string ViewPointsFieldViewPoints;

    nlohmann::json toJSON(bool with_viewpoints_only = false,
                          bool with_annotations_only = false) const;

    static boost::optional<nlohmann::json> viewPointJSON(const nlohmann::json& vps_json, 
                                                         size_t idx, 
                                                         bool with_annotations_only = false);
private:
    void toJSON(nlohmann::json& j, 
                bool with_viewpoints_only,
                bool with_annotations_only) const;

    std::vector<std::unique_ptr<ViewPointGenVP>> view_points_;
};
