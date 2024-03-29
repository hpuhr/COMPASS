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

#include <rasterreference.h>

#include <vector>
#include <limits>
#include <math.h>

#include <boost/optional.hpp>

#include <QRectF>
#include <QImage>

#include <Eigen/Core>

/**
*/
class Grid2D
{
public:
    /**
    */
    struct Cell
    {
        void reset();

        double value = 0.0;
        size_t count;
    };

    /**
    */
    struct CellStats
    {
        void reset();

        double value_min = std::numeric_limits<double>::max();
        double value_max = std::numeric_limits<double>::min();
        double mean      = 0.0;
        double mean2     = 0.0;
    };

    enum Value
    {
        IndexValue    = 0,
        IndexCount    = 0,
        IndexMin      = 1,
        IndexMax      = 2,
        IndexMean     = 3,
        IndexMean2    = 4,
        IndexVar      = 4,
        IndexStddev   = 4,
        NumLayers     = 5
    };

    Grid2D();
    virtual ~Grid2D();

    bool valid() const;

    size_t numCellsX() const { return n_cells_x_; }
    size_t numCellsY() const { return n_cells_y_; }
    size_t numCells() const { return n_cells_; }

    bool create(const QRectF& roi,
                double cell_size_x,
                double cell_size_y,
                const std::string& srs = "wgs84",
                bool srs_is_north_up = true);
    bool create(const QRectF& roi,
                size_t num_cells_x,
                size_t num_cells_y,
                const std::string& srs = "wgs84",
                bool srs_is_north_up = true);
    void clear();
    void reset();

    bool addValue(double x, double y, double v);
    bool setValue(double x, double y, double v);
    bool setCount(double x, double y, size_t count);

    Eigen::MatrixXd getValues(Value vtype) const;
    const RasterReference& getReference() const;

    static const double InvalidValue;

private:
    enum class IndexError
    {
        NoError = 0,
        Inf,
        OOR
    };

    IndexError index(size_t& idx_x, size_t& idx_y, double x, double y) const;
    IndexError checkAdd(size_t& idx_x, size_t& idx_y, double x, double y, double v);

    size_t n_cells_x_       = 0;
    size_t n_cells_y_       = 0;
    size_t n_cells_         = 0;

    double x0_              = 0.0;
    double y0_              = 0.0;
    double x1_              = 1.0;
    double y1_              = 1.0;
    double cell_size_x_inv_ = 1.0;
    double cell_size_y_inv_ = 1.0;

    size_t num_added_   = 0;
    size_t num_oor_     = 0;
    size_t num_inf_     = 0;

    RasterReference ref_;

    std::vector<Eigen::MatrixXd> layers_;
};