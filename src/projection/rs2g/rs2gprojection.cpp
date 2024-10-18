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

ProjectionCoordinateSystemBase& RS2GProjection::coordinateSystem(unsigned int id)
{
    assert(hasCoordinateSystem(id));

    return *coordinate_systems_.at(id).get();
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
    if (!hasCoordinateSystem(id))
        logerr << "RS2GProjection: polarToWGS84: no coord system for " << id;

    assert(hasCoordinateSystem(id));

    bool ret {false};

    Eigen::Vector3d geodetic_pos;

    // logdbg << "RS2GProjection: polarToWGS84: local x " << x1 << " y " << y1 << " z " << z1;

    ret = coordinate_systems_.at(id)->calculateRadSlt2Geocentric(
        azimuth_rad, slant_range_m, has_baro_altitude, baro_altitude_ft * FT2M, geodetic_pos);

    if (ret)
    {
        logdbg << "RS2GProjection: polarToWGS84: geoc x " << geodetic_pos[0] << " y " << geodetic_pos[1] << " z "
               << geodetic_pos[2];

        ret = RS2GCoordinateSystem::geocentric2Geodesic(geodetic_pos);

        latitude = geodetic_pos[0];
        longitude = geodetic_pos[1];

        logdbg << "RS2GProjection: polarToWGS84: geod x " << geodetic_pos[0] << " y " << geodetic_pos[1];
        // what to do with altitude?
    }

    return ret;
}

