#include "rs2gprojection.h"

#include "global.h"
#include "logger.h"
#include "projectionmanager.h"
#include "rs2gcoordinatesystem.h"

RS2GProjection::RS2GProjection(const std::string& class_id, const std::string& instance_id,
                               ProjectionManager& proj_manager)
    : Projection(class_id, instance_id, proj_manager)
{
}

RS2GProjection::~RS2GProjection() {}

void RS2GProjection::generateSubConfigurable(const std::string& class_id,
                                             const std::string& instance_id)
{
}

void RS2GProjection::checkSubConfigurables() {}

bool RS2GProjection::hasCoordinateSystem(unsigned int id) { return coordinate_systems_.count(id); }

void RS2GProjection::addCoordinateSystem(unsigned int id, double latitude_deg, double longitude_deg,
                                         double altitude_m)
{
    assert(!hasCoordinateSystem(id));

    coordinate_systems_[id].reset(
        new RS2GCoordinateSystem(id, latitude_deg, longitude_deg, altitude_m));
}

bool RS2GProjection::polarToWGS84(unsigned int id, double azimuth_rad, double slant_range_m,
                                  bool has_baro_altitude, double baro_altitude_ft, double& latitude,
                                  double& longitude)
{
    assert(hasCoordinateSystem(id));

    double x1, y1, z1;
    bool ret;

    Eigen::Vector3d pos;

    x1 = slant_range_m * sin(azimuth_rad);
    y1 = slant_range_m * cos(azimuth_rad);

    if (has_baro_altitude)
        z1 = baro_altitude_ft * FT2M;
    else
        z1 = 0.0;

    logdbg << "RS2GProjection: polarToWGS84: local x " << x1 << " y " << y1 << " z " << z1;

    ret =
        coordinate_systems_.at(id)->calculateRadSlt2Geocentric(x1, y1, z1, pos, has_baro_altitude);

    if (ret)
    {
        logdbg << "RS2GProjection: polarToWGS84: geoc x " << pos[0] << " y " << pos[1] << " z "
               << pos[2];

        ret = RS2GCoordinateSystem::geocentric2Geodesic(pos);

        latitude = pos[0];
        longitude = pos[1];

        logdbg << "RS2GProjection: polarToWGS84: geod x " << pos[0] << " y " << pos[1];
        // what to do with altitude?
    }

    return ret;
}
