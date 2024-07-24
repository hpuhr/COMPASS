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

#include "eval/results/base/featuredefinition.h"
#include "eval/results/base/featuredefinition_t.h"

#include "view/points/viewpointgenerator.h"
#include "view/scatterplotview/scatterseries.h"

#include "dbcontent/dbcontent.h"

#include "evaluationmanager.h"

namespace EvaluationRequirementResult
{

/**
 * Scatter series feature definition.
*/
class FeatureDefinitionScatterSeries : public FeatureDefinition
{
public:
    typedef FeatureDefinitionDataSeries<double> ScatterDataSeries;

    FeatureDefinitionScatterSeries(const EvaluationManager& eval_manager,
                                   const std::string& feature_description,
                                   const std::string& x_axis_label,
                                   const std::string& y_axis_label) 
    :   FeatureDefinition(eval_manager, feature_description, x_axis_label, y_axis_label) {}

    virtual ~FeatureDefinitionScatterSeries() = default;

    /**
    */
    FeatureDefinitionScatterSeries& addDataSeries(const std::string& name, 
                                                  const ValueSource<double>& value_source_x, 
                                                  const ValueSource<double>& value_source_y, 
                                                  const QColor& color = DataSeriesColorDefault)
    {
        ScatterDataSeries ds_x;
        ds_x.series_name         = name;
        ds_x.series_color        = color;
        ds_x.series_value_source = value_source_x;

        ScatterDataSeries ds_y;
        ds_y.series_name         = name;
        ds_y.series_color        = color;
        ds_y.series_value_source = value_source_y;

        data_series_x_.push_back(ds_x);
        data_series_y_.push_back(ds_y);
        
        return *this;
    }

    /**
    */
    bool isValid() const override final
    {
        if (data_series_x_.size() != data_series_y_.size())
            return false;
        
        //check if all data series are valid
        for (const auto& ds : data_series_x_)
            if (!ds.isValid())
                return false;

        for (const auto& ds : data_series_y_)
            if (!ds.isValid())
                return false;

        return true;
    }

    /**
    */
    std::unique_ptr<ViewPointGenFeature> createFeature_impl(const Base* result) const override final
    {
        assert(isValid());

        ScatterSeriesCollection collection;

        //handle data series
        size_t n = data_series_x_.size();

        for (size_t i = 0; i < n; ++i)
        {
            const auto& data_series_x = data_series_x_[ i ];
            const auto& data_series_y = data_series_y_[ i ];

            auto values_x = data_series_x.getOptionalValues(result);
            auto values_y = data_series_y.getOptionalValues(result);

            assert(values_x.size() == values_y.size());

            size_t n = values_x.size();

            ScatterSeries series;
            series.points.reserve(n);

            for (size_t i = 0; i < n; ++i)
                if (values_x[ i ].has_value() && values_y[ i ].has_value())
                    series.points.emplace_back(values_x[ i ].value(), values_y[ i ].value());

            series.points.shrink_to_fit();

            collection.addDataSeries(series, data_series_x.series_name, data_series_x.series_color);
        }

        auto feat = new ViewPointGenFeatureScatterSeries(collection);

        return std::unique_ptr<ViewPointGenFeature>(feat);
    }

private:
    std::vector<ScatterDataSeries> data_series_x_;
    std::vector<ScatterDataSeries> data_series_y_;
};

/**
 * Custom scatter series feature definition.
*/
class FeatureDefinitionCustomScatterSeries : public FeatureDefinition
{
public:
    FeatureDefinitionCustomScatterSeries(const EvaluationManager& eval_manager,
                                         const std::string& feature_description,
                                         const std::string& x_axis_label,
                                         const std::string& y_axis_label)
    :   FeatureDefinition(eval_manager, feature_description, x_axis_label, y_axis_label) 
    {
    }
    virtual ~FeatureDefinitionCustomScatterSeries() = default;

    /**
     * Add static counts to histogram.
    */
    FeatureDefinitionCustomScatterSeries& addDataSeries(const std::string& name, 
                                                        const std::vector<Eigen::Vector2d>& positions,
                                                        const QColor& color = Qt::blue)
    {
        ScatterDataSeries ds;
        ds.name      = name;
        ds.color     = color;
        ds.positions = positions;

        data_series_.push_back(ds);
        
        return *this;
    }

    /**
    */
    bool isValid() const override final
    {
        return true;
    }

    /**
    */
    std::unique_ptr<ViewPointGenFeature> createFeature_impl(const Base* result) const override final
    {
        assert(isValid());

        ScatterSeriesCollection collection;

        //handle data series
        for (const auto& ds : data_series_)
        {
            ScatterSeries series;
            series.points = ds.positions;

            collection.addDataSeries(series, ds.name, ds.color);
        }

        auto feat = new ViewPointGenFeatureScatterSeries(collection);

        return std::unique_ptr<ViewPointGenFeature>(feat);
    }

private:
    /**
    */
    struct ScatterDataSeries
    {
        std::string                  name;
        QColor                       color;
        std::vector<Eigen::Vector2d> positions;
    };

    std::vector<ScatterDataSeries> data_series_;
};

/**
 * Scatter series feature definition.
*/
class FeatureDefinitionTimedScatterSeries : public FeatureDefinition
{
public:
    typedef FeatureDefinitionDataSeries<double> ScatterDataSeries;

    FeatureDefinitionTimedScatterSeries(const EvaluationManager& eval_manager,
                                        const std::string& feature_description,
                                        const std::string& y_axis_label) 
    :   FeatureDefinition(eval_manager, feature_description, DBContent::meta_var_timestamp_.name(), y_axis_label) {}

    virtual ~FeatureDefinitionTimedScatterSeries() = default;

    /**
    */
    FeatureDefinitionTimedScatterSeries& addDataSeries(const std::string& name, 
                                                       const ValueSource<double>& value_source, 
                                                       const QColor& color = DataSeriesColorDefault)
    {
        ScatterDataSeries ds;
        ds.series_name         = name;
        ds.series_color        = color;
        ds.series_value_source = value_source;

        data_series_.push_back(ds);
        
        return *this;
    }

    /**
    */
    bool isValid() const override final
    {
        //check if all data series are valid
        for (const auto& ds : data_series_)
            if (!ds.isValid())
                return false;

        return true;
    }

    /**
    */
    std::unique_ptr<ViewPointGenFeature> createFeature_impl(const Base* result) const override final
    {
        assert(isValid());

        ScatterSeriesCollection collection;

        //handle data series
        size_t n = data_series_.size();

        for (size_t i = 0; i < n; ++i)
        {
            const auto& data_series = data_series_[ i ];

            auto values = data_series.getMSecTimedValues(result);

            ScatterSeries series;
            series.points.reserve(n);
            series.data_type_x = ScatterSeries::DataTypeTimestamp;

            for (const auto& v : values)
                series.points.emplace_back(v.t_msecs, v.value);

            series.points.shrink_to_fit();

            collection.addDataSeries(series, data_series.series_name, data_series.series_color);
        }

        auto feat = new ViewPointGenFeatureScatterSeries(collection);

        return std::unique_ptr<ViewPointGenFeature>(feat);
    }

private:
    std::vector<ScatterDataSeries> data_series_;
};

}
