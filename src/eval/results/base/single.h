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

#include "eval/results/base/base.h"

#include "view/gridview/grid2d.h"
#include "view/gridview/grid2dlayer.h"

#include <vector>

#include <boost/optional.hpp>

#include <Eigen/Core>

#include <QColor>

const double OSGVIEW_POS_WINDOW_SCALE {1.8};

namespace EvaluationResultsReport
{
    class SectionContentTable;
    class RootItem;
}

namespace EvaluationRequirementResult
{

class Joined;

/**
*/
class Single : public Base
{
public:
    /**
    */
    struct LayerDefinition
    {
        grid2d::ValueType    value_type;
        Grid2DRenderSettings render_settings;
    };

    enum class TargetAnnotationType
    {
        SumOverview = 0,
        TargetOverview,
        Highlight
    };
    
    enum AnnotationType
    {
        TypeOk = 0,
        TypeError,
        TypeHighlight
    };

    enum class DetailNestingMode
    {
        Vector,      // details without child details
        Nested,      // details with child details
        SingleNested // single detail child details
    };

    typedef Info TargetInfo;

    typedef std::function<bool(const EvaluationDetail&)> DetailSkipFunc;
    typedef std::function<void(const EvaluationDetail&, const EvaluationDetail*, int, int)> DetailFunc;

    Single(const std::string& type, 
           const std::string& result_id,
           std::shared_ptr<EvaluationRequirement::Base> requirement, 
           const SectorLayer& sector_layer,
           unsigned int utn, 
           const EvaluationTargetData* target, 
           EvaluationManager& eval_man,
           const EvaluationDetails& details);
    virtual ~Single();

    virtual BaseType baseType() const override { return BaseType::Single; }

    bool hasViewableData (const EvaluationResultsReport::SectionContentTable& table, 
                          const QVariant& annotation) const override final;
    std::unique_ptr<nlohmann::json::object_t> viewableData(const EvaluationResultsReport::SectionContentTable& table, 
                                                           const QVariant& annotation) const override final;
    std::unique_ptr<nlohmann::json::object_t> viewableData() const override final;

    void createSumOverviewAnnotations(nlohmann::json& annotations_json,
                                      bool add_ok_details_to_overview = true) const;

    bool hasReference (const EvaluationResultsReport::SectionContentTable& table, 
                       const QVariant& annotation) const override final;
    std::string reference(const EvaluationResultsReport::SectionContentTable& table, 
                          const QVariant& annotation) const override final;

    unsigned int utn() const;
    const EvaluationTargetData* target() const;
    void setInterestFactor(double factor);

    void updateUseFromTarget ();

    //wip - on demand detail creation
    std::unique_ptr<EvaluationDetails> generateDetails() const;

    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) = 0;

    virtual std::map<std::string, std::vector<LayerDefinition>> gridLayers() const = 0;
    virtual void addValuesToGrid(Grid2D& grid, const std::string& layer) const = 0;

    void updateResult() override final;

    const static std::string tr_details_table_name_;
    const static std::string target_table_name_;

    static const int AnnotationPointSizeOverview;
    static const int AnnotationPointSizeHighlight;
    static const int AnnotationPointSizeError;
    static const int AnnotationPointSizeOk;

    static const int AnnotationLineWidthHighlight;
    static const int AnnotationLineWidthError;
    static const int AnnotationLineWidthOk;

    static const QColor AnnotationColorHighlight;
    static const QColor AnnotationColorError;
    static const QColor AnnotationColorOk;

protected:
    std::string getTargetSectionID();
    std::string getTargetRequirementSectionID();

    virtual std::string getRequirementSectionID () const override;

    /*report contents related*/
    virtual std::vector<std::string> targetTableHeadersCommon() const;
    virtual std::vector<std::string> targetTableHeadersOptional() const;
    std::vector<std::string> targetTableHeaders(unsigned int* sort_column = nullptr) const;
    virtual std::vector<QVariant> targetTableValuesCommon() const;
    virtual std::vector<QVariant> targetTableValuesOptional() const;
    std::vector<QVariant> targetTableValues() const;

    virtual std::vector<TargetInfo> targetInfosCommon() const;
    std::vector<TargetInfo> targetConditionInfos(bool& failed) const;
    
    /// derive to obtain custom target table header strings
    virtual std::vector<std::string> targetTableHeadersCustom() const = 0;
    /// derive to obtain custom target table row values (size must match targetTableHeadersDerived())
    virtual std::vector<QVariant> targetTableValuesCustom() const = 0;
    /// derive to obtain a custom sort column for the target table (!index relative to custom values!)
    virtual int targetTableCustomSortColumn() const { return -1; }
    /// derive to obtain a custom sort order for the target table
    virtual Qt::SortOrder targetTableSortOrder() const;
    /// derive to obtain items for the target details overview table
    virtual std::vector<TargetInfo> targetInfos() const = 0;
    /// derive to obtain header strings for the target details table
    virtual std::vector<std::string> detailHeaders() const = 0;
    /// derive to obtain values for the target details table (size must match detailHeaders())
    virtual std::vector<QVariant> detailValues(const EvaluationDetail& detail, 
                                               const EvaluationDetail* parent_detail) const = 0;

    virtual void addTargetToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    virtual void addTargetToOverviewTable(EvaluationResultsReport::Section& section, 
                                          const std::string& table_name);
    virtual void addTargetDetailsToReport(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    virtual void generateDetailsTable(EvaluationResultsReport::Section& utn_req_section);

    /*details related*/

    /// detail nesting mode
    virtual DetailNestingMode detailNestingMode() const { return DetailNestingMode::Vector; } 

    boost::optional<DetailIndex> detailIndex(const QVariant& annotation) const;
    void iterateDetails(const DetailFunc& func,
                        const DetailSkipFunc& skip_func = DetailSkipFunc()) const;

    /*viewable + annotation related*/
    std::unique_ptr<nlohmann::json::object_t> createBaseViewable() const override final;
    ViewableInfo createViewableInfo(const AnnotationOptions& options) const override final;
    void createAnnotations(nlohmann::json& annotations_json, 
                           const AnnotationOptions& options) const override final;

    /// if yes the overview annotations are added if a detail is highlighted (default)
    virtual bool addOverviewAnnotationsToDetail() const { return true; }
    /// derive to implement detail validity checks
    virtual bool detailIsOk(const EvaluationDetail& detail) const = 0;
    /// derive to implement detail annotation creation 
    virtual void addAnnotationForDetail(nlohmann::json& annotations_json, 
                                        const EvaluationDetail& detail, 
                                        TargetAnnotationType type,
                                        bool is_ok) const = 0;

    void createTargetAnnotations(nlohmann::json& annotations_json,
                                 TargetAnnotationType type,
                                 const boost::optional<DetailIndex>& detail_index = boost::optional<DetailIndex>(),
                                 bool add_ok_details_to_overview = true) const;
    
    void addAnnotationPos(nlohmann::json& annotations_json,
                          const EvaluationDetail::Position& pos, 
                          AnnotationType type) const;
    void addAnnotationLine(nlohmann::json& annotations_json,
                           const EvaluationDetail::Position& pos0, 
                           const EvaluationDetail::Position& pos1,
                           AnnotationType type) const;
    void addAnnotationDistance(nlohmann::json& annotations_json,
                               const EvaluationDetail& detail,
                               AnnotationType type,
                               bool add_line = true,
                               bool add_ref = true) const;

    /// creates if not existing
    nlohmann::json& annotationPointCoords(nlohmann::json& annotations_json, AnnotationType type, bool overview=false) const;
    nlohmann::json& annotationLineCoords(nlohmann::json& annotations_json, AnnotationType type, bool overview=false) const;
    nlohmann::json& getOrCreateAnnotation(nlohmann::json& annotations_json, AnnotationType type, bool overview) const;
    
    void addValuesToGridBinary(Grid2D& grid, 
                               int detail_key, 
                               bool invert = false, 
                               bool use_ref_pos = true) const;
    void addValuesToGridBinary(Grid2D& grid, 
                               const EvaluationDetails& details, 
                               const std::function<bool(size_t)>& is_ok, 
                               bool use_ref_pos = true) const;
    LayerDefinition getGridLayerDefBinary() const;

    std::map<AnnotationType, std::string> annotation_type_names_;

    unsigned int                utn_;    // used to generate result
    const EvaluationTargetData* target_; // used to generate result

    double interest_factor_ {0};
};

}
