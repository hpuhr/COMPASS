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
#include "histogram_raw.h"
#include "nullablevector.h"

#include <map>
#include <type_traits>

#include <boost/optional.hpp>

/**
 * Scans data and uses the collected information to initialize histograms in a suitable way.
 */
template <typename T>
class HistogramInitializerT
{
public:
    typedef std::map<std::string, HistogramT<T>> Histograms;
    typedef std::function<T(size_t)>             GetValueFunc;

    HistogramInitializerT() = default;
    virtual ~HistogramInitializerT() = default;

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
    */
    bool isFinite(const T& v) const
    {
        return std::isfinite(v);
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

        for (const auto& v : data)
        {
            if (!isFinite(v))
                continue;

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
     * Scans generic data.
    */
    bool scan(const GetValueFunc& get_value_func, size_t n) 
    {
        assert(get_value_func);

        //keep track of min max values
        boost::optional<T> data_min;
        boost::optional<T> data_max;

        for (size_t i = 0; i < n; ++i)
        {
            T v = get_value_func(i);

            if (!isFinite(v))
                continue;

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
            auto distinct_values = distinctValues(get_value_func, n);
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
    HistogramConfig generateConfiguration(const boost::optional<unsigned int>& num_bins = boost::optional<unsigned int>(),
                                          const boost::optional<unsigned int>& distinct_values_min = boost::optional<unsigned int>()) const
    {
        HistogramConfig config;

        //override default num bins?
        if (num_bins.has_value())
            config.num_bins = num_bins.value();

        //no data range or no distinct values stored -> return default config
        if (!valid() || 
            !distinct_values_.has_value() || 
             distinct_values_.value().empty())
            return config;

        //default config also for floating point numbers and timestamps
        if (std::is_floating_point<T>::value ||
            std::is_same<boost::posix_time::ptime, T>::value)
            return config;

        unsigned int num_distinct_values     = distinct_values_.value().size();
        unsigned int num_distinct_values_min = distinct_values_min.has_value() ? distinct_values_min.value() : config.num_bins;

        if (std::is_same<T, std::string>::value)
        {
            //always create an unsorted category histogram for strings
            config.num_bins    = num_distinct_values;
            config.type        = HistogramConfig::Type::Category;
            config.sorted_bins = false;
        }
        else if(num_distinct_values < num_distinct_values_min)
        {
            //distinct values in data less than min threshold -> reduce bins and generate histogram from discrete categories
            config.num_bins    = num_distinct_values;
            config.type        = HistogramConfig::Type::Category;
            config.sorted_bins = std::is_arithmetic<T>::value;
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
            h.createFromCategories(categories, config.sorted_bins);
        }

        return true;
    }

    /**
    */
    bool createRAW(RawHistogram& histogram, 
                   const std::vector<T>& data, 
                   bool add_invalid_bin = true,
                   const boost::optional<unsigned int>& num_bins = boost::optional<unsigned int>(),
                   const boost::optional<unsigned int>& distinct_values_min = boost::optional<unsigned int>(),
                   dbContent::Variable* data_var_repr = nullptr)
    {
        histogram = {};

        reset();

        if (!scan(data))
            return false;

        HistogramT<T> h;
        if (!initHistogram(h, generateConfiguration()))
            return false;

        h.add(data);

        histogram = h.toRaw(add_invalid_bin, data_var_repr);

        return true;
    }

    /**
    */
    bool createRAW(RawHistogram& histogram, 
                   const GetValueFunc& get_value_func, 
                   size_t n,
                   bool add_invalid_bin = true,
                   const boost::optional<unsigned int>& num_bins = boost::optional<unsigned int>(),
                   const boost::optional<unsigned int>& distinct_values_min = boost::optional<unsigned int>(),
                   dbContent::Variable* data_var_repr = nullptr)
    {
        assert(get_value_func);

        histogram = {};

        reset();

        if (n == 0)
            return true;

        std::vector<T> data(n);
        for (size_t i = 0; i < n; ++i)
            data[ i ] = get_value_func(i);

        if (!scan(data))
            return false;

        HistogramT<T> h;
        if (!initHistogram(h, generateConfiguration()))
            return false;

        h.add(data);

        histogram = h.toRaw(add_invalid_bin, data_var_repr);

        return true;
    }

    /**
    */
    bool createRAW(RawHistogram& histogram, 
                   const std::vector<T>& category_values,
                   const std::vector<uint32_t>& category_counts,
                   bool categories_are_sorted = false,
                   dbContent::Variable* data_var_repr = nullptr)
    {
        assert(category_values.size() == category_counts.size());

        histogram = {};

        reset();

        if (category_values.size() == 0)
            return true;

        HistogramT<T> h;
        h.createFromCategories(category_values, categories_are_sorted);

        for (size_t i = 0; i < category_values.size(); ++i)
            h.add(category_values[ i ], category_counts[ i ]);

        histogram = h.toRaw(false, data_var_repr);

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
        std::vector<T> data_cpy = data;
        auto it = std::remove_if(data_cpy.begin(), data_cpy.end(), [] (const T& v) { return !histogram_helpers::checkFinite<T>(v); });
        data_cpy.erase(it, data_cpy.end());
        return std::set<T>(data_cpy.begin(), data_cpy.end());
    }

    /**
     * Returns the vectors distinct (unique) values.
     */
    std::set<T> distinctValues(const GetValueFunc& get_value_func, size_t n) const
    {
        assert(get_value_func);

        std::set<T> distinct_values;

        for (size_t i = 0; i < n; ++i)
        {
            auto v = get_value_func(i);

            if (!histogram_helpers::checkFinite<T>(v))
                continue;

            distinct_values.insert(v);
        }

        return distinct_values;
    }

    boost::optional<T>           data_min_;
    boost::optional<T>           data_max_;
    boost::optional<std::set<T>> distinct_values_;
};

/**
 * No distinct value generation for these data types.
 */
template<>
inline std::set<boost::posix_time::ptime> HistogramInitializerT<boost::posix_time::ptime>::distinctValues(NullableVector<boost::posix_time::ptime>& data) const
{
    return {};
}
template<>
inline std::set<boost::posix_time::ptime> HistogramInitializerT<boost::posix_time::ptime>::distinctValues(const std::vector<boost::posix_time::ptime>& data) const
{
    return {};
}
template<>
inline std::set<boost::posix_time::ptime> HistogramInitializerT<boost::posix_time::ptime>::distinctValues(const std::function<boost::posix_time::ptime(size_t)>& get_value_func, size_t n) const
{
    return {};
}
/**
 * Disable finite check.
*/
template<>
inline bool HistogramInitializerT<std::string>::isFinite(const std::string& v) const
{
    return true;
}
