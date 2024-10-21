
#include "measurement.h"

namespace reconstruction
{

/**
*/
Eigen::Vector2d Measurement::position2D(CoordSystem cs) const
{
    if (cs == CoordSystem::WGS84)
        return Eigen::Vector2d(lat, lon);
    return Eigen::Vector2d(x, y);
}

/**
*/
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

/**
*/
double Measurement::distance(const Measurement& other, CoordSystem cs) const
{
    if (cs == CoordSystem::WGS84)
        return (Eigen::Vector2d(lat, lon) - Eigen::Vector2d(other.lat, other.lon)).norm();

    if (z.has_value() && other.z.has_value())
        return (Eigen::Vector3d(x, y, z.value()) - Eigen::Vector3d(other.x, other.y, other.z.value())).norm();

    return (Eigen::Vector2d(x, y) - Eigen::Vector2d(other.x, other.y)).norm();
}

/**
*/
double Measurement::distanceSqr(const Measurement& other, CoordSystem cs) const
{
    if (cs == CoordSystem::WGS84)
        return (Eigen::Vector2d(lat, lon) - Eigen::Vector2d(other.lat, other.lon)).squaredNorm();

    if (z.has_value() && other.z.has_value())
        return (Eigen::Vector3d(x, y, z.value()) - Eigen::Vector3d(other.x, other.y, other.z.value())).squaredNorm();

    return (Eigen::Vector2d(x, y) - Eigen::Vector2d(other.x, other.y)).squaredNorm();
}

/**
*/
bool Measurement::hasVelocity() const
{
    if (!vx.has_value() || !vy.has_value())
        return false;

    if (z.has_value() && !vz.has_value())
        return false;

    return true;
}

/**
*/
bool Measurement::hasAcceleration() const
{
    if (!ax.has_value() || !ay.has_value())
        return false;

    if (z.has_value() && !az.has_value())
        return false;

    return true;
}

/**
*/
bool Measurement::hasStdDevPosition() const
{
    return (x_stddev.has_value() && y_stddev.has_value());
}

/**
*/
dbContent::targetReport::Position Measurement::position() const
{
    return {lat, lon};
}

/**
*/
dbContent::targetReport::PositionAccuracy Measurement::positionAccuracy() const
{
    assert (hasStdDevPosition());

    if (xy_cov.has_value())
        return dbContent::targetReport::PositionAccuracy(*x_stddev, *y_stddev, *xy_cov);
    else
        return dbContent::targetReport::PositionAccuracy(*x_stddev, *y_stddev, 0);
}

/**
*/
bool Measurement::hasStdDevVelocity() const
{
    return (vx_stddev.has_value() && vy_stddev.has_value());
}

/**
*/
bool Measurement::hasStdDevAccel() const
{
    return (ax_stddev.has_value() && ay_stddev.has_value());
}

/**
*/
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

    std::string Q_var_str        = (Q_var.has_value() ? std::to_string(Q_var.value()) : "-");
    std::string Q_var_interp_str = (Q_var_interp.has_value() ? std::to_string(Q_var_interp.value()) : "-");

    ss << prefix << "source_id:    " << (source_id.has_value() ? std::to_string(source_id.value()) : "-") << std::endl;
    ss << prefix << "interp:       " << mm_interp << std::endl;
    ss << prefix << "pos wgs84:    " << lat << ", " << lon << std::endl;
    ss << prefix << "pos cart:     " << x << ", " << y << ", " << alt_str << " (" << x_stddev_str << ", " << y_stddev_str << ", " << xy_cov_str << ")" << std::endl;
    ss << prefix << "velocity:     " << vx_str << ", " << vy_str << " (" << vx_stddev_str << ", " << vy_stddev_str << ")" << std::endl;
    ss << prefix << "accel:        " << ax_str << ", " << ay_str << " (" << ax_stddev_str << ", " << ay_stddev_str << ")";
    ss << prefix << "Q_var:        " << Q_var_str;
    ss << prefix << "Q_var_interp: " << Q_var_interp_str;

    return ss.str();
}

/**
*/
Eigen::MatrixXd Measurement::covMat(unsigned char flags) const
{
    if (hasStdDevPosition() && (flags & CovMatFlags::CovMatPos))
    {
        if (hasStdDevVelocity() && (flags & CovMatFlags::CovMatVel))
        {
            if (hasStdDevAccel() && (flags & CovMatFlags::CovMatAcc))
            {
                //generate 6x6 from position, velocity and acceleration
                Eigen::MatrixXd C = Eigen::MatrixXd::Zero(6, 6);
                C(0, 0) =  x_stddev.value() *  x_stddev.value();
                C(1, 1) = vx_stddev.value() * vx_stddev.value();
                C(2, 2) = ax_stddev.value() * ax_stddev.value();
                C(3, 3) =  y_stddev.value() *  y_stddev.value();
                C(4, 4) = vy_stddev.value() * vy_stddev.value();
                C(5, 5) = ay_stddev.value() * ay_stddev.value();

                if (xy_cov.has_value())
                {
                    C(0, 3) = xy_cov.value();
                    C(3, 0) = xy_cov.value();
                }
            }
            else
            {
                //generate 4x4 from position and velocity
                Eigen::MatrixXd C = Eigen::MatrixXd::Zero(4, 4);
                C(0, 0) =  x_stddev.value() *  x_stddev.value();
                C(1, 1) = vx_stddev.value() * vx_stddev.value();
                C(2, 2) =  y_stddev.value() *  y_stddev.value();
                C(3, 3) = vy_stddev.value() * vy_stddev.value();

                if (xy_cov.has_value())
                {
                    C(0, 2) = xy_cov.value();
                    C(2, 0) = xy_cov.value();
                }
            }
        }
        else
        {
            //generate 2x2 from position
            Eigen::MatrixXd C = Eigen::MatrixXd::Zero(2, 2);
            C(0, 0) =  x_stddev.value() *  x_stddev.value();
            C(1, 1) =  y_stddev.value() *  y_stddev.value();

            if (xy_cov.has_value())
            {
                C(0, 1) = xy_cov.value();
                C(1, 0) = xy_cov.value();
            }
        }
    }

    return Eigen::MatrixXd();
}

/**
*/
unsigned char Measurement::covMatFlags() const
{
    unsigned char flags = 0;

    if (hasStdDevPosition())
        flags |= CovMatFlags::CovMatPos;
    if (hasStdDevVelocity())
        flags |= CovMatFlags::CovMatVel;
    if (hasStdDevAccel())
        flags |= CovMatFlags::CovMatAcc;
    if (xy_cov.has_value())
        flags |= CovMatFlags::CovMatCov;

    return flags;
}

namespace covmat
{
    void setMMPosAcc(Measurement& mm, const Eigen::MatrixXd& C, int idx0, int idx1)
    {
        mm.x_stddev = std::sqrt(C(idx0, idx0));
        mm.y_stddev = std::sqrt(C(idx1, idx1));
    }
    void setMMPosCov(Measurement& mm, const Eigen::MatrixXd& C, int idx0, int idx1)
    {
        mm.xy_cov = C(idx0, idx1);
    }
    void setMMVelAcc(Measurement& mm, const Eigen::MatrixXd& C, int idx0, int idx1)
    {
        mm.vx_stddev = std::sqrt(C(idx0, idx0));
        mm.vy_stddev = std::sqrt(C(idx1, idx1));
    }
    void setMMAccAcc(Measurement& mm, const Eigen::MatrixXd& C, int idx0, int idx1)
    {
        mm.ax_stddev = std::sqrt(C(idx0, idx0));
        mm.ay_stddev = std::sqrt(C(idx1, idx1));
    }
}

/**
*/
bool Measurement::setFromCovMat(const Eigen::MatrixXd& C, unsigned char flags)
{
    if (C.cols() == 2 && C.rows() == 2)
    {
        if (flags & CovMatFlags::CovMatPos) covmat::setMMPosAcc(*this, C, 0, 1);
        if (flags & CovMatFlags::CovMatCov) covmat::setMMPosCov(*this, C, 0, 1);

        return true;
    }
    else if (C.cols() == 4 && C.rows() == 4)
    {
        if (flags & CovMatFlags::CovMatPos) covmat::setMMPosAcc(*this, C, 0, 2);
        if (flags & CovMatFlags::CovMatCov) covmat::setMMPosCov(*this, C, 0, 2);
        if (flags & CovMatFlags::CovMatVel) covmat::setMMVelAcc(*this, C, 1, 3);

        return true;
    }
    else if (C.cols() == 6 && C.rows() == 6)
    {
        if (flags & CovMatFlags::CovMatPos) covmat::setMMPosAcc(*this, C, 0, 3);
        if (flags & CovMatFlags::CovMatCov) covmat::setMMPosCov(*this, C, 0, 3);
        if (flags & CovMatFlags::CovMatVel) covmat::setMMVelAcc(*this, C, 1, 4);
        if (flags & CovMatFlags::CovMatAcc) covmat::setMMAccAcc(*this, C, 2, 5);

        return true;
    }

    //weird covmat size
    return false;
}

/**
*/
std::pair<unsigned long, boost::posix_time::ptime> Measurement::uniqueID() const
{
    assert(source_id.has_value());
    return std::pair<unsigned long, boost::posix_time::ptime>(source_id.value(), t);
}

}  // namespace reconstruction
