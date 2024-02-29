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
    if (hasCoordinateSystem(id))
        return;

    coordinate_systems_[id].reset(
        new RS2GCoordinateSystem(id, latitude_deg, longitude_deg, altitude_m));
}

void RS2GProjection::clearCoordinateSystems()
{
    coordinate_systems_.clear();
    radar_coordinate_systems_added_ = false;
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
