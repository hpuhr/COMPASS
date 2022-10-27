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

#include "histogramgenerator.h"
#include "histogram.h"
#include "nullablevector.h"

#include <map>
#include <type_traits>

#include <boost/optional.hpp>

/**
 * Scans data and uses the collected information to initialize histograms in a suitable way.
 */
template <typename T>
class HistogramInitializer
{
public:
    typedef std::map<std::string, HistogramT<T>> Histograms;

    HistogramInitializer() = default;
    virtual ~HistogramInitializer() = default;

    /**
     * Checks validity, e.g. if any data was scanned.
     */
    bool valid() const 
    {
        return (data_min_.has_value() && data_max_.has_value());
    }

    /**
     * Resets all scanned information.
     */
    void reset()
    {
        data_min_        = {};
        data_max_        = {};
        distinct_values_ = {};
    }

    /**
     * Scans a buffer.
     */
    bool scan(NullableVector<T>& data)
    {
        //keep track of min max values
        bool min_max_set = true;
        T data_min, data_max;

        std::tie(min_max_set, data_min, data_max) = data.minMaxValues();

        if (!min_max_set)
            return false;
            
        if (!data_min_.has_value() || data_min < data_min_.value())
            data_min_ = data_min;
        if (!data_max_.has_value() || data_max > data_max_.value())
            data_max_ = data_max;

        //collect distinct values if supported/desired for template type
        if (!std::is_floating_point<T>::value &&
            !std::is_same<boost::posix_time::ptime, T>::value)
        {
            auto distinct_values = distinctValues(data);
            if (!distinct_values.empty())
            {
                if (distinct_values_.has_value())
                    distinct_values_.value().insert(distinct_values.begin(), distinct_values.end());
                else
                    distinct_values_ = distinct_values;
            }
        }
        
        return true;
    }

    /**
     * Scans a data vector.
     */
    bool scan(const std::vector<T>& data)
    {
        //keep track of min max values
        boost::optional<T> data_min;
        boost::optional<T> data_max;

        for (const T& v : data)
        {
            if (!data_min.has_value() || v < data_min.value())
                data_min = v;
            if (!data_max.has_value() || v > data_max.value())
                data_max = v;
        }

        bool min_max_set = (data_min.has_value() && data_max.has_value());
        if (!min_max_set)
            return false;
        
        if (!data_min_.has_value() || data_min < data_min_.value())
            data_min_ = data_min;
        if (!data_max_.has_value() || data_max > data_max_.value())
            data_max_ = data_max;

        //collect distinct values if supported/desired for template type
        if (!std::is_floating_point<T>::value &&
            !std::is_same<boost::posix_time::ptime, T>::value)
        {
            auto distinct_values = distinctValues(data);
            if (!distinct_values.empty())
            {
                if (distinct_values_.has_value())
                    distinct_values_.value().insert(distinct_values.begin(), distinct_values.end());
                else
                    distinct_values_ = distinct_values;
            }
        }
        
        return true;
    }

    /**
     * Generates a suitable histogram configuration depending on the collected information 
     * (e.g. distinct values, extrema, etc.).
     */
    HistogramConfig currentConfiguration() const
    {
        //no data range or no distinct values stored -> return default config
        if (!valid() || 
            !distinct_values_.has_value() || 
             distinct_values_.value().empty())
            return HistogramConfig();

        //default config also for floating point numbers and timestamps
        if (std::is_floating_point<T>::value ||
            std::is_same<boost::posix_time::ptime, T>::value)
            return HistogramConfig();

        HistogramConfig config;

        unsigned int num_distinct_values = distinct_values_.value().size();

        //distinct values in data less than bins -> reduce bins and generate histogram from discrete categories
        if (std::is_same<T, std::string>::value || num_distinct_values < config.num_bins)
        {
            config.num_bins = num_distinct_values;
            config.type     = HistogramConfig::Type::Category;
        }

        return config;
    }

    /**
     * Initializes the given histogram.
     */
    bool initHistogram(HistogramT<T>& h, const HistogramConfig& config)
    {
        if (!valid())
            return false;

        //initialize histogram using config
        if (config.type == HistogramConfig::Type::Range)
        {
            h.createFromRange(config.num_bins, data_min_.value(), data_max_.value());
        }
        else
        {
            std::vector<T> categories(distinct_values_.value().begin(), distinct_values_.value().end());
            h.createFromCategories(categories);
        }

        return true;
    }

protected:
    /**
     * Returns the buffers distinct (unique) values.
     */
    std::set<T> distinctValues(NullableVector<T>& data) const
    {
        //extract distinct values by default
        return data.distinctValues();
    }

    /**
     * Returns the vectors distinct (unique) values.
     */
    std::set<T> distinctValues(const std::vector<T>& data) const
    {
        return std::set<T>(data.begin(), data.end());
    }

    boost::optional<T>           data_min_;
    boost::optional<T>           data_max_;
    boost::optional<std::set<T>> distinct_values_;
};

/**
 * No distinct value generation for these data types.
 */
template<>
inline std::set<boost::posix_time::ptime> HistogramInitializer<boost::posix_time::ptime>::distinctValues(NullableVector<boost::posix_time::ptime>& data) const
{
    return {};
}
template<>
inline std::set<boost::posix_time::ptime> HistogramInitializer<boost::posix_time::ptime>::distinctValues(const std::vector<boost::posix_time::ptime>& data) const
{
    return {};
}
