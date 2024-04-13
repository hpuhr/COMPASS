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

#ifndef SINGLEEVALUATIONREQUIREMENTDETECTIONRESULT_H
#define SINGLEEVALUATIONREQUIREMENTDETECTIONRESULT_H

#include "eval/results/base.h"

#include "view/gridview/grid2d.h"
#include "view/gridview/grid2dlayer.h"

#include <vector>

#include <boost/optional.hpp>

#include <Eigen/Core>

const double OSGVIEW_POS_WINDOW_SCALE {1.8};

namespace EvaluationRequirementResult
{

using namespace std;

class Joined;

class Single : public Base
{
public:
    struct LayerDefinition
    {
        Grid2D::ValueType    value_type;
        Grid2DRenderSettings render_settings;
    };

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

    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) = 0;

    virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) = 0;

    unsigned int utn() const;
    const EvaluationTargetData* target() const;
    void addInterestFactor(double factor);

    void updateUseFromTarget ();

    std::unique_ptr<EvaluationDetails> generateDetails() const;

    const static std::string tr_details_table_name_;
    const static std::string target_table_name_;

    virtual void addAnnotations(nlohmann::json::object_t& viewable, bool overview, bool add_ok) = 0;

    virtual std::map<std::string, std::vector<LayerDefinition>> gridLayers() const { return {}; }
    virtual void addValuesToGrid(Grid2D& grid, const std::string& layer) const {}

protected:
    enum AnnotationType
    {
        TypeOk = 0,
        TypeError,
        TypeHighlight
    };

    std::map<AnnotationType, std::string> annotation_type_names_;

    unsigned int                utn_;    // used to generate result
    const EvaluationTargetData* target_; // used to generate result

    bool result_usable_ {true}; // whether valid data exists, changed in subclass

    std::string getTargetSectionID();
    std::string getTargetRequirementSectionID();

    virtual std::string getRequirementSectionID () override;

    void addCommonDetails (shared_ptr<EvaluationResultsReport::RootItem> root_item);

//    void addAnnotationFeatures(nlohmann::json::object_t& viewable,
//                               bool overview,
//                               bool add_highlight = false) const;

    void addAnnotationPos(nlohmann::json::object_t& viewable,
                          const EvaluationDetail::Position& pos, 
                          AnnotationType type) const;
    void addAnnotationLine(nlohmann::json::object_t& viewable,
                           const EvaluationDetail::Position& pos0, 
                           const EvaluationDetail::Position& pos1, 
                           AnnotationType type) const;

    nlohmann::json& annotationPointCoords(nlohmann::json::object_t& viewable, AnnotationType type, bool overview=false) const;
    nlohmann::json& annotationLineCoords(nlohmann::json::object_t& viewable, AnnotationType type, bool overview=false) const;
    nlohmann::json& getOrCreateAnnotation(nlohmann::json::object_t& viewable, AnnotationType type, bool overview) const;
    // creates if not existing

    void addValuesToGridBinary(Grid2D& grid, 
                               int detail_key, 
                               bool invert = false, 
                               bool use_ref_pos = true) const;
    void addValuesToGridBinary(Grid2D& grid, 
                               const EvaluationDetails& details, 
                               const std::function<bool(size_t)>& is_ok, 
                               bool use_ref_pos = true) const;
    LayerDefinition getGridLayerDefBinary() const;
};

}

#endif // SINGLEEVALUATIONREQUIREMENTDETECTIONRESULT_H
