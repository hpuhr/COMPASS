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

#include <ostream>

#include <boost/optional.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <Eigen/Core>

namespace reconstruction
{

/**
*/
struct Measurement
{
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
    bool hasStdDevVelocity() const
    {
        return (vx_stddev.has_value() && vy_stddev.has_value());
    }

    void print(std::ostream& strm) const
    {
        strm << "source_id: " << source_id << std::endl;
        //strm << "t:         " << Utils::Time::toString(t) << std::endl;

        strm << "interp:    " << interpolated << std::endl;

        strm << "lat:       " << lat << std::endl;
        strm << "lon:       " << lon << std::endl;

        strm << "x:         " << x << std::endl;
        strm << "y:         " << y << std::endl;
        strm << "z:         " << (z.has_value() ? std::to_string(z.value()) : "-") << std::endl;

        strm << "vx:        " << (vx.has_value() ? std::to_string(vx.value()) : "-") << std::endl;
        strm << "vy:        " << (vy.has_value() ? std::to_string(vy.value()) : "-") << std::endl;
        strm << "vz:        " << (vz.has_value() ? std::to_string(vz.value()) : "-") << std::endl;

        strm << "ax:        " << (ax.has_value() ? std::to_string(ax.value()) : "-") << std::endl;
        strm << "ay:        " << (ay.has_value() ? std::to_string(ay.value()) : "-") << std::endl;
        strm << "az:        " << (az.has_value() ? std::to_string(az.value()) : "-") << std::endl;

        strm << "pos stddev x:  " << (x_stddev.has_value() ? std::to_string(x_stddev.value()) : "-") << std::endl;
        strm << "pos stddev y:  " << (y_stddev.has_value() ? std::to_string(y_stddev.value()) : "-") << std::endl;
        strm << "pos cov xy:    " << (xy_cov.has_value() ? std::to_string(xy_cov.value()) : "-") << std::endl;

        strm << "vel stddev x:  " << (vx_stddev.has_value() ? std::to_string(vx_stddev.value()) : "-") << std::endl;
        strm << "vel stddev y:  " << (vy_stddev.has_value() ? std::to_string(vy_stddev.value()) : "-") << std::endl;
    }

    uint32_t                 source_id;            // source of the measurement
    boost::posix_time::ptime t;                    // timestamp

    bool                     interpolated = false; //measurement has been interpolated (e.g. by spline interpolator)

    double                   lat;                  // wgs84 latitude
    double                   lon;                  // wgs84 longitude

    double                   x;                    // x position
    double                   y;                    // y position
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
};

/**
*/
struct Reference : public Measurement
{
    bool reset_pos    = false; // is this a position where kalman filter was resetted (e.g. due to long time offset)
    bool nospeed_pos  = false; // did this position not obtain speed information
    bool noaccel_pos  = false; // did this position not obtain acceleration information
    bool nostddev_pos = false; // did this position not obtain stddev information

    Eigen::MatrixXd cov; //covariance matrix
};

/**
*/
struct Uncertainty
{
    double pos_var   = 100.0;
    double speed_var = 100.0;
    double acc_var   = 100.0;
};

} // namespace reconstruction
