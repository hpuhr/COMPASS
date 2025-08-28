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

#include "targetreportdefs.h"

#include <Eigen/Core>

#include <boost/optional.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace reconstruction
{

enum class CoordSystem
{
    Unknown = 0,
    WGS84,
    Cart
};

enum CovMatFlags
{
    CovMatPos = 1 << 0,
    CovMatVel = 1 << 1,
    CovMatAcc = 1 << 2,
    CovMatCov = 1 << 3
};

/**
*/
struct Measurement
{
    Eigen::Vector2d position2D(CoordSystem cs) const;

    void position2D(const Eigen::Vector2d& pos, CoordSystem cs);

    double distance(const Measurement& other, CoordSystem cs) const;
    double distanceSqr(const Measurement& other, CoordSystem cs) const;

    bool hasVelocity() const;
    bool hasAcceleration() const;
    bool hasStdDevPosition() const;

    dbContent::targetReport::Position position() const;
    dbContent::targetReport::PositionAccuracy positionAccuracy() const;

    bool hasStdDevVelocity() const;
    bool hasStdDevAccel() const;

    std::string asString(const std::string& prefix = "") const;

    Eigen::MatrixXd covMat(unsigned char flags = 255) const;
    unsigned char covMatFlags() const;
    bool setFromCovMat(const Eigen::MatrixXd& C, unsigned char flags = 255);

    std::pair<unsigned long, boost::posix_time::ptime> uniqueID() const;

    boost::optional<unsigned long> source_id;           // source of the measurement

    boost::posix_time::ptime t;                         // timestamp

    bool                     mm_interp         = false; // measurement has been interpolated (e.g. by spline interpolator)
    bool                     pos_acc_corrected = false; // position accuracy has been corrected due to invalid correlation coefficient

    double                   lat;                       // wgs84 latitude 
    double                   lon;                       // wgs84 longitude

    mutable double           x;                         // x position (hack: mutable because of on-demand projection in KalmanEstimator)
    mutable double           y;                         // y position (hack: mutable because of on-demand projection in KalmanEstimator)
    boost::optional<double>  z;                         // optional z position

    boost::optional<double>  vx;                        // speed vector x
    boost::optional<double>  vy;                        // speed vector y
    boost::optional<double>  vz;                        // speed vector z

    boost::optional<double>  ax;                        // accel vector x
    boost::optional<double>  ay;                        // accel vector y
    boost::optional<double>  az;                        // accel vector z

    boost::optional<double>  x_stddev;                  // position stddev x
    boost::optional<double>  y_stddev;                  // position stddev y
    boost::optional<double>  xy_cov;                    // position xy-covariance value

    boost::optional<double>  vx_stddev;                 // velocity stddev x
    boost::optional<double>  vy_stddev;                 // velocity stddev y

    boost::optional<double>  ax_stddev;                 // accel stddev x
    boost::optional<double>  ay_stddev;                 // accel stddev y

    boost::optional<float>   Q_var;                     // optional mm-specific process noise variance
    boost::optional<float>   Q_var_interp;              // optional mm-specific process noise variance for interpolation
};

}  // namespace reconstruction
