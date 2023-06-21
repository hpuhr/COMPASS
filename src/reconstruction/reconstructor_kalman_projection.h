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

#include "reconstructor_defs.h"
#include "kalman_defs.h"

#include <memory>

#include <Eigen/Core>

class FrameProjector;

namespace reconstruction
{

class ReconstructorKalman;
struct KalmanChain;

/**
 * Handles map projection systems for the kalman reconstructor, e.g.
 * - initialization of a new map projection
 * - projection and unprojection of data using the currently set map projection
 * - dynamic switching of map projections due to a maximum distance criterion
 * - transfer of a kalman state from one map projection system to another one
 */
class RecKalmanProjectionHandler
{
public:
    struct Settings
    {
        MapProjectionMode map_proj_mode          = MapProjectionMode::None; //projection mode
        double            proj_max_dist_cart_sqr = 0.0;                     //maximum squared distance threshold for MapProjectionMode::Dynamic
    };

    RecKalmanProjectionHandler(ReconstructorKalman* rec);
    virtual ~RecKalmanProjectionHandler();

    bool valid() const;

    void initProjection(Measurement& mm, bool project_mm);
    
    void project(Measurement& mm) const;
    void unproject(Reference& ref) const;
    void unproject(double& lat, double& lon, const Reference& ref) const;
    void unproject(double& lat, 
                   double& lon, 
                   double x, 
                   double y, 
                   const Eigen::Vector2d& proj_center) const;
    void unproject(std::vector<Reference>& references,
                   const std::vector<Eigen::Vector2d>& proj_centers) const;
    
    bool changeProjectionIfNeeded(Reference& ref, kalman::KalmanState& state);

    kalman::XTransferFunc reprojectionTransform(const KalmanChain& chain);

    Eigen::Vector2d projectionCenter() const;

    Settings& settings() { return settings_; }

private:
    void initProjection(double lat, double lon);
    bool projectionValid(const Reference& ref) const;
    void changeProjection(Reference& ref, kalman::KalmanState& state);

    void xReprojected(kalman::Vector& x_repr, 
                      const kalman::Vector& x,
                      const Eigen::Vector2d& proj_center,
                      const Eigen::Vector2d& proj_center_new);

    ReconstructorKalman* rec_ = nullptr;

    std::unique_ptr<FrameProjector> proj_;
    Settings                        settings_;
};

} // namespace reconstruction
