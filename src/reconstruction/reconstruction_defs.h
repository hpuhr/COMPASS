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

#include "kalman_defs.h"
#include "targetreportdefs.h"

#include <ostream>

#include <boost/optional.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <Eigen/Core>

namespace reconstruction
{

enum class CoordSystem
{
    Unknown = 0,
    WGS84,
    Cart
};

enum MapProjectionMode
{
    MapProjectNone = 0, //no projection (cartesian coords pre-stored in the measurements are used,
                        //unprojection to geodetic should happen outside of reconstructor)

    MapProjectStatic,   //a single map projection is used for the whole track (origin of projection is set to
                        //center of data bounds, might result in inaccuracies in locations far away from the projection center)
    
    MapProjectDynamic   //the map projection varies dynamically based on a maximum distance threshold (most accurate,
                        //but results in some computation overhead)
};

enum class StateInterpMode
{
    BlendHalf,      // time-projected kalman states will be blended halfways
    BlendLinear,    // time-projected kalman states will be blended linearly with time
    BlendStdDev,    // states will be weighted by time-projected stddevs
    BlendVar        // states will be weighted by time-projected variances
};

/**
*/
struct Measurement
{
    Eigen::Vector2d position2D(CoordSystem cs) const
    {
        if (cs == CoordSystem::WGS84)
            return Eigen::Vector2d(lat, lon);
        return Eigen::Vector2d(x, y);
    }

    void position2D(const Eigen::Vector2d& pos, CoordSystem cs)
    {
        if (cs == CoordSystem::WGS84)
        {
            lat = pos[ 0 ];
            lon = pos[ 1 ];
        }
        x = pos[ 0 ];
        y = pos[ 1 ];
    }

    double distance(const Measurement& other, CoordSystem cs) const
    {
        if (cs == CoordSystem::WGS84)
            return (Eigen::Vector2d(lat, lon) - Eigen::Vector2d(other.lat, other.lon)).norm();

        if (z.has_value() && other.z.has_value())
            return (Eigen::Vector3d(x, y, z.value()) - Eigen::Vector3d(other.x, other.y, other.z.value())).norm();
        
        return (Eigen::Vector2d(x, y) - Eigen::Vector2d(other.x, other.y)).norm();
    }

    double distanceSqr(const Measurement& other, CoordSystem cs) const
    {
        if (cs == CoordSystem::WGS84)
            return (Eigen::Vector2d(lat, lon) - Eigen::Vector2d(other.lat, other.lon)).squaredNorm();

        if (z.has_value() && other.z.has_value())
            return (Eigen::Vector3d(x, y, z.value()) - Eigen::Vector3d(other.x, other.y, other.z.value())).squaredNorm();
        
        return (Eigen::Vector2d(x, y) - Eigen::Vector2d(other.x, other.y)).squaredNorm();
    }

    bool hasVelocity() const
    {
        if (!vx.has_value() || !vy.has_value())
            return false;

        if (z.has_value() && !vz.has_value())
            return false;

        return true;
    }
    bool hasAcceleration() const
    {
        if (!ax.has_value() || !ay.has_value())
            return false;

        if (z.has_value() && !az.has_value())
            return false;

        return true;
    }
    bool hasStdDevPosition() const
    {
        return (x_stddev.has_value() && y_stddev.has_value());
    }

    dbContent::targetReport::Position position() const
    {
        return {lat, lon};
    }

    dbContent::targetReport::PositionAccuracy positionAccuracy() const
    {
        assert (hasStdDevPosition());

        if (xy_cov.has_value())
            return dbContent::targetReport::PositionAccuracy(*x_stddev, *y_stddev, *xy_cov);
        else
            return dbContent::targetReport::PositionAccuracy(*x_stddev, *y_stddev, 0);
    }


    bool hasStdDevVelocity() const
    {
        return (vx_stddev.has_value() && vy_stddev.has_value());
    }
    bool hasStdDevAccel() const
    {
        return (ax_stddev.has_value() && ay_stddev.has_value());
    }

    std::string asString(const std::string& prefix = "") const
    {
        std::stringstream ss;

        std::string alt_str = (z.has_value() ? std::to_string(z.value()) : "-");

        std::string vx_str  = (vx.has_value() ? std::to_string(vx.value()) : "-");
        std::string vy_str  = (vy.has_value() ? std::to_string(vy.value()) : "-");
        std::string vz_str  = (vz.has_value() ? std::to_string(vz.value()) : "-");

        std::string ax_str  = (ax.has_value() ? std::to_string(ax.value()) : "-");
        std::string ay_str  = (ay.has_value() ? std::to_string(ay.value()) : "-");
        std::string az_str  = (az.has_value() ? std::to_string(az.value()) : "-");

        std::string x_stddev_str = (x_stddev.has_value() ? std::to_string(x_stddev.value()) : "-");
        std::string y_stddev_str = (y_stddev.has_value() ? std::to_string(y_stddev.value()) : "-");
        std::string xy_cov_str   = (  xy_cov.has_value() ? std::to_string(xy_cov.value())   : "-");

        std::string vx_stddev_str = (vx_stddev.has_value() ? std::to_string(vx_stddev.value()) : "-");
        std::string vy_stddev_str = (vy_stddev.has_value() ? std::to_string(vy_stddev.value()) : "-");

        std::string ax_stddev_str = (ax_stddev.has_value() ? std::to_string(ax_stddev.value()) : "-");
        std::string ay_stddev_str = (ay_stddev.has_value() ? std::to_string(ay_stddev.value()) : "-");

        ss << prefix << "source_id: " << source_id << std::endl;
        ss << prefix << "interp:    " << mm_interp << std::endl;
        ss << prefix << "pos wgs84: " << lat << ", " << lon << std::endl;
        ss << prefix << "pos cart:  " << x << ", " << y << ", " << alt_str << " (" << x_stddev_str << ", " << y_stddev_str << ", " << xy_cov_str << ")" << std::endl;
        ss << prefix << "velocity:  " << vx_str << ", " << vy_str << " (" << vx_stddev_str << ", " << vy_stddev_str << ")" << std::endl;
        ss << prefix << "accel:     " << ax_str << ", " << ay_str << " (" << ax_stddev_str << ", " << ay_stddev_str << ")";

        return ss.str();
    }

    unsigned long            source_id;            // source of the measurement
    boost::posix_time::ptime t;                    // timestamp

    bool                     mm_interp = false;    // measurement has been interpolated (e.g. by spline interpolator)

    double                   lat;                  // wgs84 latitude
    double                   lon;                  // wgs84 longitude

    mutable double           x;                    // x position (hack: mutable because of on-demand projection in KalmanEstimator)
    mutable double           y;                    // y position (hack: mutable because of on-demand projection in KalmanEstimator)
    boost::optional<double>  z;                    // optional z position

    boost::optional<double>  vx;                   // speed vector x
    boost::optional<double>  vy;                   // speed vector y
    boost::optional<double>  vz;                   // speed vector z

    boost::optional<double>  ax;                   // accel vector x
    boost::optional<double>  ay;                   // accel vector y
    boost::optional<double>  az;                   // accel vector z

    boost::optional<double>  x_stddev;             // position stddev x
    boost::optional<double>  y_stddev;             // position stddev y
    boost::optional<double>  xy_cov;               // position xy-covariance value

    boost::optional<double>  vx_stddev;            // velocity stddev x
    boost::optional<double>  vy_stddev;            // velocity stddev y

    boost::optional<double>  ax_stddev;            // accel stddev x
    boost::optional<double>  ay_stddev;            // accel stddev y
};

/**
*/
struct Reference : public Measurement
{
    bool reset_pos      = false; // is this a position where kalman filter was resetted (e.g. due to long time offset)
    bool nospeed_pos    = false; // did this position not obtain speed information
    bool noaccel_pos    = false; // did this position not obtain acceleration information
    bool nostddev_pos   = false; // did this position not obtain stddev information
    bool projchange_pos = false; // position where a change of map projection happened 

    bool ref_interp     = false; // reference has been interpolated (e.g. from kalman samples)

    Eigen::MatrixXd cov; // covariance matrix
    double          dt;  // last timestep (only set for non-interpolated)
};

/**
*/
struct MeasurementInterp : public Measurement
{
    MeasurementInterp() = default;
    MeasurementInterp(const Measurement& mm, bool is_interp, bool is_corrected) : Measurement(mm), corrected(is_corrected) 
    {
        mm_interp = is_interp;
    }

    bool corrected = false; //interpolation failed, so the measurement was interpolated linearly
};

/**
*/
struct Uncertainty
{
    double pos_var   = 100.0;
    double speed_var = 100.0;
    double acc_var   = 100.0;
};

/**
*/
struct InterpOptions
{
    double sample_dt = 1.0;
    double max_dt    = 30.0;
};

/**
*/
struct UpdateStats
{
    bool   set         = false;
    size_t num_fresh   = 0;
    size_t num_updated = 0;
    size_t num_valid   = 0;
    size_t num_failed  = 0;
    size_t num_skipped = 0;
};

/**
*/
struct PredictionStats
{
    size_t num_predictions = 0;
    size_t num_failed      = 0;
    size_t num_fixed       = 0;
};

} // namespace reconstruction
