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

#include "configurable.h"

#include <boost/thread/mutex.hpp>

class ProjectionManager;
class ProjectionCoordinateSystemBase;

class Projection : public Configurable
{
public:
    Projection(const std::string& class_id, const std::string& instance_id,
               ProjectionManager& proj_manager);
    virtual ~Projection();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    virtual bool hasCoordinateSystem(unsigned int id) = 0;
    virtual void addCoordinateSystem(unsigned int id, double latitude_deg, double longitude_deg,
                                     double altitude_m) = 0;

    virtual ProjectionCoordinateSystemBase& coordinateSystem(unsigned int id) = 0;

    virtual void clearCoordinateSystems() = 0;
    virtual bool polarToWGS84(unsigned int id, double azimuth_rad, double slant_range_m,
                              bool has_baro_altitude, double baro_altitude_ft, double& latitude,
                              double& longitude) = 0;

    double getGroundRange(unsigned int id, double slant_range_m,
                          bool has_altitude, double altitude_m);

    void addAllRadarCoordinateSystems(); // only adds if not already added

    std::string name() const;
    void name(const std::string& name);

    bool radarCoordinateSystemsAdded();

protected:
    ProjectionManager& proj_manager_;

    std::string name_;

    boost::mutex radar_coordinate_systems_mutex_;
    bool radar_coordinate_systems_added_ {false};

    virtual void checkSubConfigurables();
};


