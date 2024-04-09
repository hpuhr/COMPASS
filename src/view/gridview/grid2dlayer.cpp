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

#include "grid2dlayer.h"
#include "grid2d.h"

#include "logger.h"

#include <iostream>

/*********************************************************************************
 * Grid2DLayer
 *********************************************************************************/

/**
*/
boost::optional<std::pair<double, double>> Grid2DLayer::range() const
{
    double vmin = std::numeric_limits<double>::max();
    double vmax = std::numeric_limits<double>::min();

    const double* d = data.data();

    bool has_range = false;

    size_t n = (size_t)data.size();

    for (size_t i = 0; i < n; ++i)
    {
        if (d[ i ] == Grid2D::InvalidValue)
            continue;

        if (d[ i ] < vmin) vmin = d[ i ];
        if (d[ i ] > vmax) vmax = d[ i ];

        has_range = true;
    }

    if (!has_range)
        return {};

    return std::make_pair(vmin, vmax);
}

/*********************************************************************************
 * Grid2DLayers
 *********************************************************************************/

/**
*/
Grid2DLayers::Grid2DLayers() = default;

/**
*/
Grid2DLayers::~Grid2DLayers() = default;

/**
*/
void Grid2DLayers::clear()
{
    layers_.clear();
}

/**
*/
void Grid2DLayers::addLayer(const std::string& name, 
                            const RasterReference& ref,
                            const Eigen::MatrixXd& data)
{
    assert(layers_.count(name) == 0);

    auto& l = layers_[ name ];

    l.name = name;
    l.ref  = ref;
    l.data = data;
}

/*********************************************************************************
 * Grid2DLayerRenderer
 *********************************************************************************/

/**
*/
std::pair<QImage,RasterReference> Grid2DLayerRenderer::render(const Grid2DLayer& layer,
                                                              const Grid2DRenderSettings& settings)
{
    const auto& data = layer.data;

    if (data.size() == 0)
        return {};

    auto range = layer.range();

    double vmin, vmax;

    if (settings.min_value.has_value() && settings.max_value.has_value())
    {
        vmin = settings.min_value.value();
        vmax = settings.max_value.value();
    }
    else if (range.has_value())
    {
        vmin = range.value().first;
        vmax = range.value().second;
    }
    else
    {
        return {};
    }

    //std::cout << "vmin: " << vmin << ", vmax: " << vmax << std::endl;

    int pixels_per_cell = std::max(1, settings.pixels_per_cell);

    size_t nrows  = layer.data.rows();
    size_t ncols  = layer.data.cols();

    size_t nrows_out = nrows * pixels_per_cell;
    size_t ncols_out = ncols * pixels_per_cell;

    QImage img(ncols_out, nrows_out, QImage::Format_ARGB32);
    img.fill(QColor(0, 0, 0));

    double vrange = vmax - vmin;
    
    auto getColor = [ & ] (size_t cx, size_t cy)
    {
        //flip y-index if north is up
        if (layer.ref.is_north_up)
            cy = nrows - 1 - cy;

        if (data(cy, cx) == Grid2D::InvalidValue)
            return QColor(0, 0, 0, 0);

        double t = (data(cy, cx) - vmin) / vrange;

        return settings.color_map.sample(t);
    };

    for (size_t y = 0, y0 = 0; y < nrows; ++y, y0 += pixels_per_cell)
    {
        for (size_t x = 0, x0 = 0; x < ncols; ++x, x0 += pixels_per_cell)
        {
            QColor col = getColor(x, y);

            for (int y2 = 0; y2 < pixels_per_cell; ++y2)
                for (int x2 = 0; x2 < pixels_per_cell; ++x2)
                    img.setPixelColor(x0 + x2, y0 + y2, col);
        }
    }

    RasterReference ref = layer.ref;

    ref.img_pixel_size_x /= pixels_per_cell;
    ref.img_pixel_size_y /= pixels_per_cell;

    return std::make_pair(img, ref);
}
