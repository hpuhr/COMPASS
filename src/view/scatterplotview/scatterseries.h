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

#include "json.hpp"

#include <string>
#include <vector>

#include <Eigen/Core>

#include <QColor>

/**
*/
struct ScatterSeries
{
    enum DataType
    {
        DataTypeFloatingPoint = 0,  // general floating point values
        DataTypeTimestamp           // timestamp as ms since epoch
    };

    typedef std::vector<Eigen::Vector2d> Points;

    bool fromJSON(const nlohmann::json& data, bool binary);
    nlohmann::json toJSON(bool binary) const;

    std::vector<Eigen::Vector2d> points;
    DataType                     data_type_x = DataTypeFloatingPoint;
    DataType                     data_type_y = DataTypeFloatingPoint;
};

/**
 * A raw histogram.
 */
class ScatterSeriesCollection
{
public:
    struct DataSeries
    {
        ScatterSeries scatter_series;
        
        std::string   name;
        QColor        color;
        double        marker_size;
    };

    ScatterSeriesCollection();
    virtual ~ScatterSeriesCollection();

    void clear();
    void addDataSeries(const DataSeries& data_series);
    void addDataSeries(const ScatterSeries& scatter_series,
                       const std::string& name = "",
                       const QColor& color = Qt::blue,
                       double marker_size = 8.0);

    size_t numDataSeries() const;

    const std::vector<DataSeries>& dataSeries() const;
    std::vector<DataSeries>& dataSeries();

    ScatterSeries::DataType commonDataTypeX() const;
    ScatterSeries::DataType commonDataTypeY() const; 

    bool fromJSON(const nlohmann::json& data);
    nlohmann::json toJSON(bool binary = false) const;

    static const std::string TagDataSeries;
    static const std::string TagData;
    static const std::string TagDataRaw;
    static const std::string TagDataTypeX;
    static const std::string TagDataTypeY;
    static const std::string TagName;
    static const std::string TagColor;
    static const std::string TagMarkerSize;

private:
    std::vector<DataSeries> data_series_;
};
