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

#include "grid2d.h"

#include "logger.h"

#include <iostream>

const double Grid2D::InvalidValue = std::numeric_limits<double>::max();

/**
*/
Grid2D::Grid2D()
{
    vtype_indices_.resize(grid2d::NumValueTypes);
    vtype_indices_[ grid2d::ValueTypeCount  ] = IndexCount;
    vtype_indices_[ grid2d::ValueTypeMin    ] = IndexMin;
    vtype_indices_[ grid2d::ValueTypeMax    ] = IndexMax;
    vtype_indices_[ grid2d::ValueTypeMean   ] = IndexMean;
    vtype_indices_[ grid2d::ValueTypeVar    ] = IndexVar;
    vtype_indices_[ grid2d::ValueTypeStddev ] = IndexStddev;
}

/**
*/
Grid2D::~Grid2D() = default;

/**
*/
bool Grid2D::valid() const
{
    return (n_cells_ > 0);
}

/**
*/
bool Grid2D::create(const QRectF& roi,
                    const grid2d::GridResolution& resolution,
                    const std::string& srs,
                    bool srs_is_north_up)
{
    clear();

    if (roi.isEmpty() || !resolution.valid())
        return false;

    //compute needed resolution values and adjusted roi
    QRectF roi_adj = resolution.resolution(n_cells_x_,
                                           n_cells_y_,
                                           cell_size_x_,
                                           cell_size_y_,
                                           roi);
    if (roi_adj.isEmpty())
        return false;

    cell_size_x_inv_ = 1.0 / cell_size_x_;
    cell_size_y_inv_ = 1.0 / cell_size_y_;
    n_cells_         = n_cells_x_ * n_cells_y_;
    x0_              = roi_adj.left();
    y0_              = roi_adj.top();
    x1_              = roi_adj.right();
    y1_              = roi_adj.bottom();

    layers_.resize(NumLayers, Eigen::MatrixXd(n_cells_y_, n_cells_x_));

    ref_.set(srs, roi_adj, n_cells_x_, n_cells_y_, srs_is_north_up);

    reset();

    return true;
}

/**
*/
void Grid2D::clear()
{
    layers_.resize(0);

    n_cells_x_ = 0;
    n_cells_y_ = 0;
    n_cells_   = 0;

    num_added_ = 0;
    num_oor_   = 0;
    num_inf_   = 0;
}

/**
*/
void Grid2D::reset()
{
    layers_[ IndexCount ].setConstant(0);
    layers_[ IndexMean  ].setConstant(0);
    layers_[ IndexMean2 ].setConstant(0);
    layers_[ IndexMin   ].setConstant(std::numeric_limits<double>::max());
    layers_[ IndexMax   ].setConstant(std::numeric_limits<double>::min());
 
    num_added_ = 0;
    num_oor_   = 0;
    num_inf_   = 0;
}

/**
*/
Grid2D::IndexError Grid2D::index(size_t& idx_x, size_t& idx_y, double x, double y) const
{
    if (!std::isfinite(x) || !std::isfinite(y))
        return IndexError::Inf;
    
    if (x < x0_ || x > x1_ || y < y0_ || y > y1_)
        return IndexError::OOR;

    idx_x = std::min(n_cells_x_ - 1, static_cast<size_t>((x - x0_) * cell_size_x_inv_));
    idx_y = std::min(n_cells_y_ - 1, static_cast<size_t>((y - y0_) * cell_size_y_inv_));
    
    return IndexError::NoError;
}

/**
*/
Grid2D::IndexError Grid2D::checkAdd(size_t& idx_x, size_t& idx_y, double x, double y, double v)
{
    assert(valid());

    if (!std::isfinite(v))
    {
        ++num_inf_;
        return IndexError::Inf;
    }

    auto err = index(idx_x, idx_y, x, y);

    if (err == IndexError::OOR)
        ++num_oor_;
    else if (err == IndexError::Inf)
        ++num_inf_;

    return err;
}

/**
*/
bool Grid2D::addValue(double x, double y, double v)
{
    size_t idx_x, idx_y;
    if (checkAdd(idx_x, idx_y, x, y, v) != IndexError::NoError)
        return false;

    auto& vmin  = layers_[ IndexMin   ](idx_y, idx_x);
    auto& vmax  = layers_[ IndexMax   ](idx_y, idx_x);
    auto& count = layers_[ IndexCount ](idx_y, idx_x);
    auto& mean  = layers_[ IndexMean  ](idx_y, idx_x);
    auto& mean2 = layers_[ IndexMean2 ](idx_y, idx_x);

    if (v < vmin) vmin = v;
    if (v > vmax) vmax = v;

    //welford's online algorithm
    //https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Welford's_online_algorithm
    count += 1;
    double delta = v - mean;
    mean += delta / count;
    double delta2 = v - mean;
    mean2 += delta * delta2;
    
    ++num_added_;

    return true;
}

/**
*/
size_t Grid2D::addLineInternal(double x0, 
                               double y0, 
                               double x1, 
                               double y1, 
                               double v, 
                               std::set<std::pair<size_t, size_t>>& visited,
                               int subsampling)
{
    int cells_x = std::max(1, (int)std::ceil(std::fabs(x1 - x0) * cell_size_x_inv_));
    int cells_y = std::max(1, (int)std::ceil(std::fabs(y1 - y0) * cell_size_y_inv_));
    int samples = std::max(cells_x, cells_y) * 2 * subsampling;

    size_t idx_x, idx_y;
    size_t added = 0;
    for (int i = 0; i <= samples; ++i)
    {
        double t1 = (double)i / (double)samples;
        double t0 = (1.0 - t1);
        double  x = t0 * x0 + t1 * x1;
        double  y = t0 * y0 + t1 * y1;

        if (index(idx_x, idx_y, x, y) != Grid2D::IndexError::NoError)
            continue;

        if (visited.find(std::make_pair(idx_x, idx_y)) != visited.end())
            continue;

        if (!addValue(x, y, v))
            continue;

        visited.insert(std::make_pair(idx_x, idx_y));
        ++added;
    }

    return added;
}

/**
*/
size_t Grid2D::addLine(double x0, 
                       double y0, 
                       double x1, 
                       double y1, 
                       double v, 
                       int subsampling)
{
    std::set<std::pair<size_t, size_t>> visited;
    return addLineInternal(x0, y0, x1, y1, v, visited, subsampling);
}

/**
*/
size_t Grid2D::addPoly(const std::vector<Eigen::Vector2d>& positions,
                       double v, 
                       int subsampling)
{
    std::set<std::pair<size_t, size_t>> visited;
    size_t added = 0;
    for (size_t i = 1; i < positions.size(); ++i)
    {
        added += addLineInternal(positions[ i - 1 ][ 0 ],
                                 positions[ i - 1 ][ 1 ],
                                 positions[ i     ][ 0 ],
                                 positions[ i     ][ 1 ],
                                 v,
                                 visited,
                                 subsampling);
    }
    
    return added;
}

/**
*/
size_t Grid2D::addPoly(const std::function<void(double&, double&, size_t)>& pos_getter,
                       size_t n,
                       double v, 
                       int subsampling)
{
    assert(pos_getter);

    std::set<std::pair<size_t, size_t>> visited;
    double x0, y0, x1, y1;
    size_t added = 0;
    for (size_t i = 1; i < n; ++i)
    {
        pos_getter(x0, y0, i - 1);
        pos_getter(x1, y1, i    );
        added += addLineInternal(x0, y0, x1, y1, v, visited, subsampling);
    }

    return added;
}

/**
*/
bool Grid2D::setValue(double x, double y, double v)
{
    size_t idx_x, idx_y;
    if (checkAdd(idx_x, idx_y, x, y, v) != IndexError::NoError)
        return false;

    layers_[ IndexValue ](idx_y, idx_x) = v;
    layers_[ IndexCount ](idx_y, idx_x) = 1;

    return true;
}

/**
*/
bool Grid2D::setCount(double x, double y, size_t count)
{
    size_t idx_x, idx_y;
    if (checkAdd(idx_x, idx_y, x, y, 0.0) != IndexError::NoError)
        return false;

    layers_[ IndexCount ](idx_y, idx_x) = (double)count;

    return true;
}

/**
*/
Eigen::MatrixXd Grid2D::getValues(grid2d::ValueType vtype) const
{
    ValueIndex vindex = vtype_indices_[ vtype ];

    Eigen::MatrixXd l = layers_[ vindex ];
    double* d = l.data();

    const auto&   count  = layers_[ IndexCount ];
    const double* dcount = count.data();

    size_t n = (size_t)l.size();

    for (size_t i = 0; i < n; ++i)
        if(dcount[ i ] == 0)
            d[ i ] = InvalidValue;
    
    if (vtype == grid2d::ValueType::ValueTypeVar)
    {
        for (size_t i = 0; i < n; ++i)
            if (d[ i ] != InvalidValue)
                d[ i ] /= dcount[ i ];
    }
    else if (vtype == grid2d::ValueType::ValueTypeStddev)
    {
        for (size_t i = 0; i < n; ++i)
            if (d[ i ] != InvalidValue)
                d[ i ] = std::sqrt(d[ i ] / dcount[ i ]);
    }

    return l;
}

/**
*/
const RasterReference& Grid2D::getReference() const
{
    return ref_;
}
