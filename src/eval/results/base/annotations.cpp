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

#include "eval/results/base/annotations.h"

using namespace std;

namespace EvaluationRequirementResult
{

const ColorMap::ColorScale AnnotationDefinitions::ColorScaleDefault    = ColorMap::ColorScale::Green2Red;
const unsigned int         AnnotationDefinitions::NumColorStepsDefault = 5;

namespace
{
    DetailValuePositionMode positionMode(AnnotationDefinitions::GridDefinition::AddDetailMode add_mode)
    {
        if (add_mode == AnnotationDefinitions::GridDefinition::AddDetailMode::AddEvtPosition)
            return DetailValuePositionMode::EventPosition;
        else if (add_mode == AnnotationDefinitions::GridDefinition::AddDetailMode::AddEvtRefPosition)
            return DetailValuePositionMode::EventRefPosition;
        else if (add_mode == AnnotationDefinitions::GridDefinition::AddDetailMode::AddPositionsAsPolygon)
            return DetailValuePositionMode::AllPositions;
        
        return DetailValuePositionMode::EventPosition;
    }
}

/**
*/
AnnotationDefinitions& AnnotationDefinitions::addHistogram(const std::string& annotation_name,
                                                           const std::string& layer_name,
                                                           const DetailValueSource& value_source,
                                                           bool force_range_histogram)
{
    HistogramDefinition def;
    def.name                  = layer_name;
    def.value_source          = value_source;
    def.force_range_histogram = force_range_histogram;

    histograms_[ annotation_name ].push_back(def);

    return *this;
}

/**
*/
AnnotationDefinitions& AnnotationDefinitions::addValueGrid(const std::string& annotation_name,
                                                           const std::string& layer_name,
                                                           const DetailValueSource& value_source,
                                                           const std::vector<grid2d::ValueType>& value_types,
                                                           GridDefinition::AddDetailMode add_detail_mode,
                                                           const boost::optional<double>& render_value_min,
                                                           const boost::optional<double>& render_value_max,
                                                           ColorMap::ColorScale render_color_scale,
                                                           unsigned int render_color_steps)
{
    GridDefinition def;

    def.name            = layer_name;
    def.value_source    = value_source;
    def.value_types     = value_types;
    def.add_detail_mode = add_detail_mode;
    def.pos_mode        = positionMode(add_detail_mode);

    def.render_settings.show_selected = false;
    def.render_settings.min_value     = render_value_min;
    def.render_settings.max_value     = render_value_max;

    def.render_settings.color_map.create(render_color_scale, render_color_steps);
    
    grids_[ annotation_name ].push_back(def);

    return *this;
}

/**
*/
AnnotationDefinitions& AnnotationDefinitions::addBinaryGrid(const std::string& annotation_name,
                                                            const std::string& layer_name,
                                                            const DetailValueSource& value_source,
                                                            GridDefinition::AddDetailMode add_detail_mode,
                                                            const QColor& render_false_color,
                                                            const QColor& render_true_color)
{
    GridDefinition def;

    def.name            = layer_name;
    def.value_source    = value_source;
    def.value_types     = { grid2d::ValueType::ValueTypeMax };
    def.add_detail_mode = add_detail_mode;
    def.pos_mode        = positionMode(add_detail_mode);

    def.render_settings.show_selected = false;
    def.render_settings.min_value     = 0.0;
    def.render_settings.max_value     = 1.0;

    def.render_settings.color_map.create(render_false_color, render_true_color, 2, ColorMap::Type::Binary);
    
    grids_[ annotation_name ].push_back(def);

    return *this;
}

}
