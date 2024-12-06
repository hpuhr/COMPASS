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

#include "reconstruction_defs.h"
#include "kalman_defs.h"

#include <memory>

#include <Eigen/Core>

class FrameProjector;

namespace reconstruction
{

class KalmanInterface;

/**
 * Handles map projection systems for the kalman reconstructor, e.g.
 * - initialization of a new map projection
 * - projection and unprojection of data using the currently set map projection
 * - dynamic switching of map projections due to a maximum distance criterion
 * - transfer of a kalman state from one map projection system to another one
 */
class KalmanProjectionHandler
{
public:
    struct Settings
    {
        MapProjDistanceCheck proj_dist_check        = MapProjDistanceCheck::Cart; //distance checks performed
        double               proj_max_dist_cart_sqr = 0.0;                        //maximum squared distance threshold for MapProjectionMode::Dynamic
        double               proj_max_dist_wgs84    = 0.0;                        //maximum wgs84 distance threshold in degrees for MapProjectionMode::Dynamic
    };

    KalmanProjectionHandler();
    virtual ~KalmanProjectionHandler();

    bool valid() const;

    void initProjection(double lat, double lon);
    
    void project(double& x, 
                 double& y, 
                 double lat, 
                 double lon) const;
    void unproject(double& lat, 
                   double& lon, 
                   double x, 
                   double y,
                   const Eigen::Vector2d* proj_center = nullptr) const;
    
    //void unproject(std::vector<Reference>& references,
    //               const std::vector<Eigen::Vector2d>& proj_centers) const;

    bool inRangeCart(double x_cart, double y_cart) const;
    bool inRangeWGS84(double lat, double lon) const;
    
    bool changeProjectionIfNeeded(kalman::KalmanUpdate& update,
                                  const KalmanInterface& interface);
    
    void xReprojected(kalman::Vector& x_repr, 
                      const KalmanInterface& interface,
                      const kalman::Vector& x,
                      const Eigen::Vector2d& proj_center,
                      const Eigen::Vector2d& proj_center_new,
                      size_t* num_proj_center_changed = nullptr);
    kalman::XTransferFunc reprojectionTransform(const std::vector<kalman::KalmanUpdate>* updates,
                                                const KalmanInterface* interface,
                                                size_t offset = 0);
    Eigen::Vector2d projectionCenter() const;

    Settings& settings() { return settings_; }

private:
    bool needsReprojectionChange(kalman::KalmanUpdate& update,
                                 const KalmanInterface& interface) const;
    void changeProjection(kalman::KalmanUpdate& update,
                          const KalmanInterface& interface);

    std::unique_ptr<FrameProjector> proj_;
    Settings                        settings_;
};

} // namespace reconstruction
