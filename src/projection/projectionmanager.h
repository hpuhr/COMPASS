/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PROJECTIONMANAGER_H_
#define PROJECTIONMANAGER_H_

#include <ogr_spatialref.h>

#include "configurable.h"
#include "singleton.h"

class ProjectionManagerWidget;

/**
 * @brief Singleton for coordinate projection handling
 *
 * Currently handles projection from world coordinates to Cartesian coordinates using the WGS-84 method.
 */
class ProjectionManager : public Singleton, public Configurable
{
protected:
    float sdl_system_latitude_;
    float sdl_system_longitude_;

    unsigned int epsg_value_;

    OGRSpatialReference ogr_geo_;
    OGRSpatialReference ogr_cart_;

    OGRCoordinateTransformation* ogr_geo2cart_ {nullptr};
    OGRCoordinateTransformation* ogr_cart2geo_ {nullptr};

    ProjectionManagerWidget* widget_ {nullptr};

    /// @brief Constructor
    ProjectionManager();

public:
    /// @brief Desctructor
    virtual ~ProjectionManager();
    /// @brief Return world plane size
    //double getWorldSize (double size);

    /// @brief Scales world position height
    //float transformHeight (float value);

    /// @brief Projects geo-coordinate in WGS-84 to cartesian coordinate, returns false on error
    bool ogrGeo2Cart (double latitude, double longitude, double& x_pos, double& y_pos);
    /// @brief Projects cartesian coordinate to geo-coordinate in WGS-84, returns false on error
    bool ogrCart2Geo (double x_pos, double y_pos, double& latitude, double& longitude);

    std::string getWorldPROJ4Info ();
    void setNewCartesianEPSG (unsigned int epsg_value);
    std::string getCartesianPROJ4Info ();

    void createOGRProjection ();

    unsigned int getEPSG () { return epsg_value_; }

    ProjectionManagerWidget* widget ();

    void shutdown ();

    /// @brief Returns static instance
    static ProjectionManager& instance()
    {
        static ProjectionManager instance;
        return instance;
    }
    float sdlSystemLatitude() const;
    void sdlSystemLatitude(float sdl_system_latitude);

    float sdlSystemLongitude() const;
    void sdlSystemLongitude(float sdl_system_longitude);
};

#endif /* PROJECTIONMANAGER_H_ */
