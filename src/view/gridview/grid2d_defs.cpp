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

#include "grid2d_defs.h"

#include <math.h>
#include <limits>

#include <boost/optional.hpp>

#include <QRectF>

namespace grid2d
{

const double GridResolution::DefaultBorderFactor = 0.01; //1 % border by default

/**
*/
GridResolution& GridResolution::setCellCount(size_t nx,
                                             size_t ny)
{
    type        = Type::CellCount;
    num_cells_x = nx;
    num_cells_y = ny;

    return *this;
}

/**
*/
GridResolution& GridResolution::setCellSize(double sx,
                                            double sy)
{
    type        = Type::CellSize;
    cell_size_x = sx;
    cell_size_y = sy;

    return *this;
}

/**
*/
GridResolution& GridResolution::setBorder(double b)
{
    border = b;

    return *this;
}

/**
*/
bool GridResolution::validResolution(double cell_size)
{
    return (std::isfinite(cell_size) && cell_size >= 1e-09);
}

/**
*/
bool GridResolution::valid() const
{
    if (type == Type::CellCount)
        return (num_cells_x > 0 && num_cells_y > 0);
    else if (type == Type::CellSize)
        return GridResolution::validResolution(cell_size_x) && GridResolution::validResolution(cell_size_y);

    return false;
}

/**
*/
QRectF GridResolution::addBorder(const QRectF& roi,
                                 const boost::optional<double>& border,
                                 const boost::optional<double>& xmin,
                                 const boost::optional<double>& xmax,
                                 const boost::optional<double>& ymin,
                                 const boost::optional<double>& ymax)
{
    if (roi.isEmpty())
        return QRectF();

    double b = border.has_value() ? border.value() : DefaultBorderFactor;

    if (b <= 0.0)
        return roi;

    double w = roi.width();
    double h = roi.height();

    double x_border = w * b * 0.5;
    double y_border = h * b * 0.5;

    double roi_x_min = std::max(xmin.has_value() ? xmin.value() : std::numeric_limits<double>::lowest(), roi.left()   - x_border);
    double roi_x_max = std::min(xmax.has_value() ? xmax.value() : std::numeric_limits<double>::max()   , roi.right()  + x_border);
    double roi_y_min = std::max(ymin.has_value() ? ymin.value() : std::numeric_limits<double>::lowest(), roi.top()    - y_border);
    double roi_y_max = std::min(ymax.has_value() ? ymax.value() : std::numeric_limits<double>::max()   , roi.bottom() + y_border);

    return QRectF(roi_x_min, roi_y_min, roi_x_max - roi_x_min, roi_y_max - roi_y_min);
}

/**
*/
QRectF GridResolution::resolution(size_t& grid_cells_x,
                                  size_t& grid_cells_y,
                                  double& grid_cell_size_x,
                                  double& grid_cell_size_y,
                                  const QRectF& roi) const
{
    if (!valid())
        return QRectF();

    QRectF roi_bordered = GridResolution::addBorder(roi, border);

    if (type == Type::CellCount)
    {
        grid_cells_x = num_cells_x;
        grid_cells_y = num_cells_y;

        grid_cell_size_x = roi_bordered.width()  / num_cells_x;
        grid_cell_size_y = roi_bordered.height() / num_cells_y;

        if (!validResolution(grid_cell_size_x) || 
            !validResolution(grid_cell_size_y))
            return QRectF();

        return roi;
    }
    else if (type == Type::CellSize)
    {
        grid_cell_size_x = cell_size_x;
        grid_cell_size_y = cell_size_y;

        //@TODO: upper cell limit?
        grid_cells_x = std::max((size_t)1, (size_t)std::ceil(roi_bordered.width()  / cell_size_x));
        grid_cells_y = std::max((size_t)1, (size_t)std::ceil(roi_bordered.height() / cell_size_y));

        //center adjusted region on original roi's center
        double region_w = grid_cells_x * grid_cell_size_x;
        double region_h = grid_cells_y * grid_cell_size_y;

        double x0 = roi.center().x() - region_w / 2;
        double y0 = roi.center().y() - region_h / 2;

        return QRectF(x0, y0, region_w, region_h);
    }

    return QRectF();
}

} // grid2d
