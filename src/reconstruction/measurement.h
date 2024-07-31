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

}  // namespace reconstruction

