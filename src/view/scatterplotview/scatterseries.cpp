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

#include "scatterseries.h"

/*******************************************************************************************
 * ScatterSeries
 *******************************************************************************************/

/**
*/
bool ScatterSeries::fromJSON(const nlohmann::json& data, bool binary)
{
    *this = {};

    if (binary)
    {
        if (!data.is_string())
            return false;

        //read binary
        std::string str = data;

        QByteArray ba_base64(str.data(), str.size());
        QByteArray ba = QByteArray::fromBase64(ba_base64);

        unsigned int* n    = (unsigned int*)(ba.data());
        const double* data = (const double*)(ba.data() + sizeof(unsigned int));

        unsigned int n_points = *n;

        points.resize(*n);

        size_t cnt = 0;
        for (unsigned int i = 0; i < n_points; ++i, cnt += 2)
            points[ i ] = Eigen::Vector2d(data[ cnt ], data[ cnt + 1 ]);
    }
    else
    {
        //read as json array
        if (!data.is_array())
            return false;

        points.reserve(data.size());

        for (size_t i = 0; i < data.size(); ++i)
        {
            const auto& jpoint = data[ i ];
            if (!jpoint.is_array() || jpoint.size() != 2)
                return false;

            double x = jpoint[ 0 ];
            double y = jpoint[ 1 ];

            points.emplace_back(x, y);
        }
    }

    return true;
}

/**
*/
nlohmann::json ScatterSeries::toJSON(bool binary) const
{
    nlohmann::json jpoints;

    if (binary)
    {
        //write binary
        unsigned int n      = points.size();
        unsigned int n_flat = 2 * n;

        QByteArray ba;

        //add size
        ba.append((const char*)&n, sizeof(unsigned int));

        std::vector<double> points_flat(n_flat);
        size_t cnt = 0;
        for (unsigned int i = 0; i < n; ++i, cnt += 2)
        {
            points_flat[ cnt     ] = points[ i ].x();
            points_flat[ cnt + 1 ] = points[ i ].y();
        }
        
        //add array
        ba.append((const char*)points_flat.data(), n_flat * sizeof(double));

        //code base 64
        QString byte_str(ba.toBase64());

        jpoints = byte_str.toStdString();
    }
    else
    {
        //write as json array
        jpoints = nlohmann::json::array();

        for (const auto& pos : points)
        {
            nlohmann::json jpoint = nlohmann::json::array();
            jpoint.push_back(pos.x());
            jpoint.push_back(pos.y());

            jpoints.push_back(jpoint);
        }
    }

    return jpoints;
}

/*******************************************************************************************
 * ScatterSeriesCollection
 *******************************************************************************************/

const std::string ScatterSeriesCollection::TagDataSeries = "dataseries";
const std::string ScatterSeriesCollection::TagData       = "data";
const std::string ScatterSeriesCollection::TagDataRaw    = "data_raw";
const std::string ScatterSeriesCollection::TagDataTypeX  = "data_type_x";
const std::string ScatterSeriesCollection::TagDataTypeY  = "data_type_y";
const std::string ScatterSeriesCollection::TagName       = "name";
const std::string ScatterSeriesCollection::TagColor      = "color";
const std::string ScatterSeriesCollection::TagMarkerSize = "marker_size";

/**
*/
ScatterSeriesCollection::ScatterSeriesCollection() = default;

/**
*/
ScatterSeriesCollection::~ScatterSeriesCollection() = default;

/**
*/
void ScatterSeriesCollection::clear()
{
    data_series_.clear();
}

/**
*/
void ScatterSeriesCollection::addDataSeries(const DataSeries& series)
{
    data_series_.push_back(series);
}

/**
*/
void ScatterSeriesCollection::addDataSeries(const ScatterSeries& scatter_series,
                                            const std::string& name,
                                            const QColor& color,
                                            double marker_size)
{
    DataSeries series;
    series.scatter_series = scatter_series;
    series.name           = name;
    series.color          = color;
    series.marker_size    = marker_size;

    data_series_.push_back(series);
}

/**
*/
size_t ScatterSeriesCollection::numDataSeries() const
{
    return data_series_.size();
}

/**
*/
const std::vector<ScatterSeriesCollection::DataSeries>& ScatterSeriesCollection::dataSeries() const
{
    return data_series_;
}

/**
*/
ScatterSeries::DataType ScatterSeriesCollection::commonDataTypeX() const
{
    if (data_series_.empty())
        return ScatterSeries::DataTypeFloatingPoint;

    ScatterSeries::DataType data_type = data_series_[ 0 ].scatter_series.data_type_x;

    for (size_t i = 1; i < data_series_.size(); ++i)
        if (data_series_[ i ].scatter_series.data_type_x != data_type)
            return ScatterSeries::DataTypeFloatingPoint;

    return data_type;
}

/**
*/
ScatterSeries::DataType ScatterSeriesCollection::commonDataTypeY() const
{
    if (data_series_.empty())
        return ScatterSeries::DataTypeFloatingPoint;

    ScatterSeries::DataType data_type = data_series_[ 0 ].scatter_series.data_type_y;

    for (size_t i = 1; i < data_series_.size(); ++i)
        if (data_series_[ i ].scatter_series.data_type_y != data_type)
            return ScatterSeries::DataTypeFloatingPoint;

    return data_type;
}

/**
*/
bool ScatterSeriesCollection::fromJSON(const nlohmann::json& data)
{
    clear();

    if (!data.is_object() || !data.contains(TagDataSeries))
        return false;

    const auto& jseries = data[ TagDataSeries ];
    if (!jseries.is_array())
        return false;

    for (const auto& js : jseries)
    {
        if (!js.is_object() || (!js.contains(TagData) && !js.contains(TagDataRaw)))
            return false;

        DataSeries dseries;

        bool read_raw = js.contains(TagDataRaw);

        if (!dseries.scatter_series.fromJSON(js[ read_raw ? TagDataRaw : TagData ], read_raw))
            return false;

        if (js.contains(TagDataTypeX))
            dseries.scatter_series.data_type_x = js[ TagDataTypeX ];

        if (js.contains(TagDataTypeY))
            dseries.scatter_series.data_type_y = js[ TagDataTypeY ];

        if (js.contains(TagName))
            dseries.name = js[ TagName ];

        if (js.contains(TagColor))
        {
            std::string cname = js[ TagColor ];
            dseries.color = QColor(QString::fromStdString(cname));
        }

        if (js.contains(TagMarkerSize))
            dseries.marker_size = js[ TagMarkerSize ];

        data_series_.push_back(dseries);
    }

    return true;
}

/**
*/
nlohmann::json ScatterSeriesCollection::toJSON(bool binary) const
{
    nlohmann::json obj;
    nlohmann::json jseries = nlohmann::json::array();

    for (const auto& dseries : data_series_)
    {
        nlohmann::json js;

        js[ binary ? TagDataRaw : TagData ] = dseries.scatter_series.toJSON(binary);

        js[ TagDataTypeX  ] = dseries.scatter_series.data_type_x;
        js[ TagDataTypeY  ] = dseries.scatter_series.data_type_y;
        js[ TagName       ] = dseries.name;
        js[ TagColor      ] = dseries.color.name().toStdString();
        js[ TagMarkerSize ] = dseries.marker_size;

        jseries.push_back(js);
    }

    obj[ TagDataSeries ] = jseries;

    return obj;
}
