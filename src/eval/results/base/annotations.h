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

#include <string>
#include <vector>
#include <map>

#include "eval/results/base/result_defs.h"

#include "histogram.h"
#include "grid2d_defs.h"
#include "grid2dlayer.h"
#include "colormap.h"

namespace EvaluationRequirementResult
{

/**
*/
class AnnotationDefinitions
{
public:
    /**
    */
    struct AnnotationDefinition
    {
        typedef std::function<double(const EvaluationDetail&)> DetailValueFunc;

        /**
        */
        bool isValid() const
        {
            return value_source.isValid();
        }

        std::string       name;
        DetailValueSource value_source;
    };

    /**
    */
    struct HistogramDefinition : public AnnotationDefinition
    {
        bool force_range_histogram = false;
    };

    /**
    */
    struct GridDefinition : public AnnotationDefinition
    {
        enum class AddDetailMode
        {
            AddEvtPosition = 0,    // add event position 
            AddEvtRefPosition,     // add event ref position
            AddPositionsAsPolygon  // add all event positions as polygon
        };

        /// how to add details to the grid
        AddDetailMode add_detail_mode = AddDetailMode::AddEvtPosition;
        /// how to retrieve detail positions
        DetailValuePositionMode pos_mode = DetailValuePositionMode::EventPosition;
        /// value types to extract form the grid's cells (min, max, mean, etc.)
        std::vector<grid2d::ValueType> value_types;
        /// settings for rendering the grid
        Grid2DRenderSettings render_settings;
    };

    AnnotationDefinitions() = default;
    virtual ~AnnotationDefinitions() = default;

    AnnotationDefinitions& addHistogram(const std::string& annotation_name,
                                        const std::string& layer_name,
                                        const DetailValueSource& value_source,
                                        bool force_range_histogram = false);
    AnnotationDefinitions& addValueGrid(const std::string& annotation_name,
                                        const std::string& layer_name,
                                        const DetailValueSource& value_source,
                                        const std::vector<grid2d::ValueType>& value_types,
                                        GridDefinition::AddDetailMode add_detail_mode,
                                        const boost::optional<double>& render_value_min,
                                        const boost::optional<double>& render_value_max,
                                        ColorMap::ColorScale render_color_scale = ColorScaleDefault,
                                        unsigned int render_color_steps = NumColorStepsDefault);
    AnnotationDefinitions& addBinaryGrid(const std::string& annotation_name,
                                         const std::string& layer_name,
                                         const DetailValueSource& value_source,
                                         GridDefinition::AddDetailMode add_detail_mode,
                                         const QColor& render_false_color = Qt::green,
                                         const QColor& render_true_color = Qt::red);

    const std::map<std::string, std::vector<HistogramDefinition>>& histograms() const { return histograms_; }
    const std::map<std::string, std::vector<GridDefinition>>& grids() const { return grids_; }

    static const ColorMap::ColorScale ColorScaleDefault;
    static const unsigned int         NumColorStepsDefault;

protected:
    std::map<std::string, std::vector<HistogramDefinition>> histograms_;
    std::map<std::string, std::vector<GridDefinition>>      grids_;
};

}
