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

#include "grid2d_defs.h"
#include "rasterreference.h"

#include <vector>
#include <set>
#include <limits>
#include <math.h>

#include <boost/optional.hpp>

#include <QRectF>
#include <QImage>

#include <Eigen/Core>
#include <Eigen/Dense>

struct Grid2DLayer;
class  Grid2DLayers;

/**
*/
class Grid2D
{
public:
    Grid2D();
    virtual ~Grid2D();

    bool valid() const;

    size_t numCellsX() const { return n_cells_x_; }
    size_t numCellsY() const { return n_cells_y_; }
    size_t numCells() const { return n_cells_; }

    bool create(const QRectF& roi,
                const grid2d::GridResolution& resolution,
                const std::string& srs = "wgs84",
                bool srs_is_north_up = true);
    void clear();
    void reset();

    bool addValue(double x, double y, double v);
    size_t addLine(double x0, 
                   double y0, 
                   double x1, 
                   double y1, 
                   double v, 
                   int subsampling = 10);
    size_t addPoly(const std::vector<Eigen::Vector2d>& positions,
                   double v, 
                   int subsampling = 10);
    size_t addPoly(const std::function<void(double&, double&, size_t)>& pos_getter,
                   size_t n,
                   double v, 
                   int subsampling = 10);
    bool setValue(double x, double y, double v);
    bool setCount(double x, double y, size_t count);

    void select(size_t x, size_t y, bool ok);
    void select(const QRectF& roi, bool ok);

    Eigen::MatrixXd getValues(grid2d::ValueType vtype) const;
    const RasterReference& getReference() const;
    const Eigen::MatrixXi& getFlags() const;

    std::unique_ptr<Grid2DLayer> createLayer(const std::string& layer_name,
                                             grid2d::ValueType vtype) const;
    void addToLayers(Grid2DLayers& layers, 
                     const std::string& layer_name,
                     grid2d::ValueType vtype) const;

    QRectF gridBounds() const;
    static void cropGrid(QRectF& roi, 
                         QRect& region, 
                         const QRectF& crop_rect,
                         const QRectF& grid_roi,
                         int grid_cells_x,
                         int grid_cells_y,
                         bool grid_north_up,
                         bool region_in_image_space,
                         int pixels_per_cell = 1);

    size_t numAdded() const { return num_added_; }
    size_t numOutOfRange() const { return num_oor_; }
    size_t numInf() const { return num_inf_; }

    static const double InvalidValue;

private:
    enum class IndexError
    {
        NoError = 0,
        Inf,
        OOR
    };

    enum ValueIndex
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

    IndexError index(size_t& idx_x, size_t& idx_y, double x, double y, bool clamp = false) const;
    IndexError checkAdd(size_t& idx_x, size_t& idx_y, double x, double y, double v);

    void indices(std::vector<std::pair<size_t, size_t>>& indices, const QRectF& roi) const;

    size_t addLineInternal(double x0, 
                           double y0, 
                           double x1, 
                           double y1, 
                           double v,
                           std::set<std::pair<size_t, size_t>>& visited,
                           int subsampling = 10);

    void setFlags(size_t x, size_t y, unsigned char flags, bool ok);
    void setFlags(const QRectF& roi, unsigned char flags, bool ok);
    void resetFlags();

    size_t n_cells_x_       = 0;
    size_t n_cells_y_       = 0;
    size_t n_cells_         = 0;

    double x0_              = 0.0;
    double y0_              = 0.0;
    double x1_              = 1.0;
    double y1_              = 1.0;
    double cell_size_x_     = 1.0;
    double cell_size_y_     = 1.0;
    double cell_size_x_inv_ = 1.0;
    double cell_size_y_inv_ = 1.0;

    size_t num_added_   = 0;
    size_t num_oor_     = 0;
    size_t num_inf_     = 0;

    RasterReference ref_;

    std::vector<Eigen::MatrixXd>  layers_;
    Eigen::MatrixXi flags_;
    std::vector<ValueIndex>       vtype_indices_;
};
