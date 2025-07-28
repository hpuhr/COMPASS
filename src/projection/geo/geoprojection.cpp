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

#include "geoprojection.h"
#include "global.h"
#include "logger.h"
#include "geocoordinatesystem.h"
#include "projectionmanager.h"

//#include "cpl_conv.h"

GeoProjection::GeoProjection(const std::string& class_id, const std::string& instance_id,
                             ProjectionManager& proj_manager)
    : Projection(class_id, instance_id, proj_manager)
{
}

GeoProjection::~GeoProjection() {}

void GeoProjection::generateSubConfigurable(const std::string& class_id,
                                            const std::string& instance_id)
{
}

void GeoProjection::checkSubConfigurables() {}

std::vector<unsigned int> GeoProjection::ids()
{
    std::vector<unsigned int> ids;

    for (auto& coord_sys : coordinate_systems_)
        ids.push_back(coord_sys.first);

    return ids;
}

bool GeoProjection::hasCoordinateSystem(unsigned int id) { return coordinate_systems_.count(id); }

void GeoProjection::addCoordinateSystem(unsigned int id, double latitude_deg, double longitude_deg,
                                        double altitude_m)
{
    if (hasCoordinateSystem(id))
        return;

    coordinate_systems_[id].reset(
        new GeoCoordinateSystem(id, latitude_deg, longitude_deg, altitude_m));
}

ProjectionCoordinateSystemBase& GeoProjection::coordinateSystem(unsigned int id)
{
    assert(hasCoordinateSystem(id));

    return *coordinate_systems_.at(id).get();
}

void GeoProjection::clearCoordinateSystems()
{
    coordinate_systems_.clear();
    radar_coordinate_systems_added_ = false;
}

bool GeoProjection::polarToWGS84(unsigned int id, double azimuth_rad, double slant_range_m,
                                 bool has_baro_altitude, double baro_altitude_ft,
                                 double& latitude_deg, double& longitude_deg)
{
    assert(hasCoordinateSystem(id));

    double x_pos_m, y_pos_m;
    bool ret = coordinate_systems_.at(id)->polarSlantToCartesian(
        azimuth_rad, slant_range_m, has_baro_altitude, baro_altitude_ft * FT2M,
        x_pos_m, y_pos_m);

    if (!ret || std::isnan(x_pos_m) || std::isnan(y_pos_m))
        return false;

    ret = coordinate_systems_.at(id)->cartesian2WGS84(x_pos_m, y_pos_m,
                                                      latitude_deg, longitude_deg);

    // TODO altitude?
    return ret;
}


bool GeoProjection::wgs842PolarHorizontal(unsigned int id, double latitude_deg, double longitude_deg,
                                          double& azimuth_deg, double& ground_range_m)
{
    assert(hasCoordinateSystem(id));

    return coordinate_systems_.at(id)->wgs842PolarHorizontal(latitude_deg, longitude_deg,
                                                             azimuth_deg, ground_range_m);
}
