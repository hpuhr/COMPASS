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

#include "rasterreference.h"
#include "colormap.h"

#include <map>

#include <Eigen/Core>

#include <boost/optional.hpp>

#include <QImage>

class ColorMap;

/**
*/
struct Grid2DLayer
{
    boost::optional<std::pair<double, double>> range() const; 

    std::string     name;
    Eigen::MatrixXd data;
    RasterReference ref;
};

/**
*/
class Grid2DLayers
{
public:
    Grid2DLayers();
    virtual ~Grid2DLayers();

    void clear();
    void addLayer(const std::string& name, 
                  const RasterReference& ref,
                  const Eigen::MatrixXd& data);

    const std::map<std::string, Grid2DLayer>& layers() const { return layers_; }

private:
    std::map<std::string, Grid2DLayer> layers_;
};

/**
*/
struct Grid2DRenderSettings
{
    ColorMap                color_map;
    boost::optional<double> min_value;
    boost::optional<double> max_value;
    int                     pixels_per_cell = 1;
};

/**
*/
class Grid2DLayerRenderer
{
public:
    static QImage render(const Grid2DLayer& layer,
                         const Grid2DRenderSettings& settings);
};
