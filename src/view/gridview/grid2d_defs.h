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

#include <stddef.h>
#include <string>

#include <boost/optional.hpp>

class QRectF;

namespace grid2d
{

/**
*/
struct GridResolution
{
    enum class Type
    {
        CellCount = 0,
        CellSize
    };

    GridResolution() = default;
    ~GridResolution() = default;

    GridResolution& setCellCount(size_t nx,
                                 size_t ny);
    GridResolution& setCellSize(double sx,
                                double sy);
    GridResolution& setBorder(double b);

    bool valid() const;
    QRectF resolution(size_t& grid_cells_x,
                      size_t& grid_cells_y,
                      double& grid_cell_size_x,
                      double& grid_cell_size_y,
                      const QRectF& roi) const;

    static bool validResolution(double cell_size);
    static QRectF addBorder(const QRectF& roi,
                            const boost::optional<double>& border = boost::optional<double>(),
                            const boost::optional<double>& xmin = boost::optional<double>(),
                            const boost::optional<double>& xmax = boost::optional<double>(),
                            const boost::optional<double>& ymin = boost::optional<double>(),
                            const boost::optional<double>& ymax = boost::optional<double>());

    static const double DefaultBorderFactor;

    Type   type        = Type::CellCount;
    size_t num_cells_x = 0;
    size_t num_cells_y = 0;
    double cell_size_x = 0.0;
    double cell_size_y = 0.0;
    double border      = DefaultBorderFactor; 
};

enum ValueType
{
    ValueTypeCount = 0,
    ValueTypeMin,
    ValueTypeMax,
    ValueTypeMean,
    ValueTypeVar,
    ValueTypeStddev,
    NumValueTypes
};

enum CellFlags
{
    CellSelected = 1 << 0
};

/**
*/
inline std::string valueTypeToString(ValueType vtype)
{
    switch (vtype)
    {
        case ValueTypeCount:
            return "Count";
        case ValueTypeMin:
            return "Min";
        case ValueTypeMax:
            return "Max";
        case ValueTypeMean:
            return "Mean";
        case ValueTypeVar:
            return "Var";
        case ValueTypeStddev:
            return "Stddev";
        default:
            return "";
    }
    return "";
}

} // grid2d
