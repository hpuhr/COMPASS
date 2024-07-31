#include "measurement.h"

namespace reconstruction
{

Eigen::Vector2d Measurement::position2D(CoordSystem cs) const
{
    if (cs == CoordSystem::WGS84)
        return Eigen::Vector2d(lat, lon);
    return Eigen::Vector2d(x, y);
}

void Measurement::position2D(const Eigen::Vector2d& pos, CoordSystem cs)
{
    if (cs == CoordSystem::WGS84)
    {
        lat = pos[ 0 ];
        lon = pos[ 1 ];
    }
    x = pos[ 0 ];
    y = pos[ 1 ];
}

double Measurement::distance(const Measurement& other, CoordSystem cs) const
{
    if (cs == CoordSystem::WGS84)
        return (Eigen::Vector2d(lat, lon) - Eigen::Vector2d(other.lat, other.lon)).norm();

    if (z.has_value() && other.z.has_value())
        return (Eigen::Vector3d(x, y, z.value()) - Eigen::Vector3d(other.x, other.y, other.z.value())).norm();

    return (Eigen::Vector2d(x, y) - Eigen::Vector2d(other.x, other.y)).norm();
}

double Measurement::distanceSqr(const Measurement& other, CoordSystem cs) const
{
    if (cs == CoordSystem::WGS84)
        return (Eigen::Vector2d(lat, lon) - Eigen::Vector2d(other.lat, other.lon)).squaredNorm();

    if (z.has_value() && other.z.has_value())
        return (Eigen::Vector3d(x, y, z.value()) - Eigen::Vector3d(other.x, other.y, other.z.value())).squaredNorm();

    return (Eigen::Vector2d(x, y) - Eigen::Vector2d(other.x, other.y)).squaredNorm();
}

bool Measurement::hasVelocity() const
{
    if (!vx.has_value() || !vy.has_value())
        return false;

    if (z.has_value() && !vz.has_value())
        return false;

    return true;
}

bool Measurement::hasAcceleration() const
{
    if (!ax.has_value() || !ay.has_value())
        return false;

    if (z.has_value() && !az.has_value())
        return false;

    return true;
}
bool Measurement::hasStdDevPosition() const
{
    return (x_stddev.has_value() && y_stddev.has_value());
}

dbContent::targetReport::Position Measurement::position() const
{
    return {lat, lon};
}

dbContent::targetReport::PositionAccuracy Measurement::positionAccuracy() const
{
    assert (hasStdDevPosition());

    if (xy_cov.has_value())
        return dbContent::targetReport::PositionAccuracy(*x_stddev, *y_stddev, *xy_cov);
    else
        return dbContent::targetReport::PositionAccuracy(*x_stddev, *y_stddev, 0);
}


bool Measurement::hasStdDevVelocity() const
{
    return (vx_stddev.has_value() && vy_stddev.has_value());
}
bool Measurement::hasStdDevAccel() const
{
    return (ax_stddev.has_value() && ay_stddev.has_value());
}

std::string Measurement::asString(const std::string& prefix) const
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

}  // namespace reconstruction
