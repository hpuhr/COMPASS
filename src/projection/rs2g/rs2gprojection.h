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

#ifndef RS2GPROJECTION_H
#define RS2GPROJECTION_H

#include "projection.h"

class ProjectionManager;
class RS2GCoordinateSystem;

class RS2GProjection : public Projection
{
  public:
    RS2GProjection(const std::string& class_id, const std::string& instance_id,
                   ProjectionManager& proj_manager);
    virtual ~RS2GProjection();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    virtual bool hasCoordinateSystem(unsigned int id);
    virtual void addCoordinateSystem(unsigned int id, double latitude_deg, double longitude_deg,
                                     double altitude_m);
    virtual bool polarToWGS84(unsigned int id, double azimuth_rad, double slant_range_m,
                              bool has_baro_altitude, double baro_altitude_ft, double& latitude,
                              double& longitude);

  protected:
    std::map<unsigned int, std::unique_ptr<RS2GCoordinateSystem>> coordinate_systems_;

    virtual void checkSubConfigurables();
};

#endif  // RS2GPROJECTION_H
