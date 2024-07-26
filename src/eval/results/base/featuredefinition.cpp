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

#include "eval/results/base/featuredefinition.h"

#include "view/points/viewpointgenerator.h"

namespace EvaluationRequirementResult
{

/**************************************************************************************
 * FeatureDefinition
 **************************************************************************************/

const QColor FeatureDefinition::DataSeriesColorDefault = Qt::blue;

const unsigned int           FeatureDefinition::NumColorStepsDefault = 5;
const colorscale::ColorScale FeatureDefinition::ColorScaleDefault    = colorscale::ColorScale::Green2Red;

/**
*/
FeatureDefinition::FeatureDefinition() = default;

/**
*/
FeatureDefinition::FeatureDefinition(const EvaluationManager& eval_manager,
                                     const std::string& feature_description,
                                     const std::string& x_axis_label,
                                     const std::string& y_axis_label)
:   eval_manager_       (&eval_manager      )
,   feature_description_(feature_description)
,   x_axis_label_       (x_axis_label       )
,   y_axis_label_       (y_axis_label       )
{
}

/**
*/
FeatureDefinition::~FeatureDefinition() = default;

/**
*/
std::unique_ptr<ViewPointGenFeature> FeatureDefinition::createFeature(const Base* result,
                                                                      const std::vector<EvaluationDetail>* details) const
{
    assert(result);

    auto feat = createFeature_impl(result, details);
    if (!feat)
        return {};

    feat->setName(feature_description_);

    return feat;
}

/**
*/
const std::string& FeatureDefinition::featureDescription() const
{
    return feature_description_;
}

/**
*/
const std::string& FeatureDefinition::xAxisLabel() const
{
    return x_axis_label_;
}

/**
*/
const std::string& FeatureDefinition::yAxisLabel() const
{
    return y_axis_label_;
}

/**
*/
const EvaluationManager& FeatureDefinition::evalManager() const
{
    return *eval_manager_;
}

/**************************************************************************************
 * FeatureDefinitions
 **************************************************************************************/

/**
*/
FeatureDefinitions::FeatureDefinitions() = default;

/**
*/
FeatureDefinitions::~FeatureDefinitions() = default;

/**
*/
FeatureDefinitions& FeatureDefinitions::addDefinition(const std::string& value_description,
                                                      std::unique_ptr<FeatureDefinition>&& def)
{
    assert(def);
    feature_defs_[ value_description ].push_back(std::move(def));

    return *this;
}

/**
*/
FeatureDefinitions& FeatureDefinitions::addDefinition(const std::string& value_description,
                                                      FeatureDefinition* def)
{
    assert(def);
    feature_defs_[ value_description ].emplace_back(def);

    return *this;
}

/**
*/
const FeatureDefinitions::DefinitionMap& FeatureDefinitions::definitions() const
{
    return feature_defs_;
}

}