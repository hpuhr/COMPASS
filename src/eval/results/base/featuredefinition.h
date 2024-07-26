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

#include "colormap_defs.h"

#include <string>
#include <memory>
#include <vector>
#include <map>

#include <QColor>

class EvaluationManager;
class ViewPointGenFeature;
class EvaluationDetail;

namespace EvaluationRequirementResult
{

class Base;

/**
 * Defines how a certain annotation feature is created, which can be attached to an evaluation viewable.
 */
class FeatureDefinition
{
public:
    FeatureDefinition();
    FeatureDefinition(const EvaluationManager& eval_manager,
                      const std::string& feature_description,
                      const std::string& x_axis_label,
                      const std::string& y_axis_label);
    virtual ~FeatureDefinition();

    /// checks if the feature definition is valid
    virtual bool isValid() const = 0;

    std::unique_ptr<ViewPointGenFeature> createFeature(const Base* result,
                                                       const std::vector<EvaluationDetail>* details = nullptr) const;

    const std::string& featureDescription() const;
    const std::string& xAxisLabel() const;
    const std::string& yAxisLabel() const;

    static const QColor                 DataSeriesColorDefault;
    static const unsigned int           NumColorStepsDefault;
    static const colorscale::ColorScale ColorScaleDefault;

protected:
    /// creates a feature which can be part of an annotation
    virtual std::unique_ptr<ViewPointGenFeature> createFeature_impl(const Base* result,
                                                                    const std::vector<EvaluationDetail>* details) const = 0;
    const EvaluationManager& evalManager() const;

private:
    const EvaluationManager* eval_manager_ = nullptr;

    std::string feature_description_;
    std::string x_axis_label_;
    std::string y_axis_label_;
};

/**
 * Governs all feature definitions of an evaluation result.
*/
class FeatureDefinitions
{
public:
    typedef std::vector<std::unique_ptr<FeatureDefinition>> DefinitionVector;
    typedef std::map<std::string, DefinitionVector>         DefinitionMap;

    FeatureDefinitions();
    virtual ~FeatureDefinitions();
    FeatureDefinitions(FeatureDefinitions&&) = default;
    FeatureDefinitions(const FeatureDefinitions&) = delete;

    FeatureDefinitions& addDefinition(const std::string& value_description, 
                                      std::unique_ptr<FeatureDefinition>&& def);
    FeatureDefinitions& addDefinition(const std::string& value_description, 
                                      FeatureDefinition* def);

    const DefinitionMap& definitions() const;

    template <class T, typename... Targs>
    T& addDefinition(const std::string& value_description, Targs&&... args)
    {
        T* def = new T(std::forward<Targs>(args)...);

        addDefinition(value_description, def);

        return *def;
    }

private:
    DefinitionMap feature_defs_;
};

}
 