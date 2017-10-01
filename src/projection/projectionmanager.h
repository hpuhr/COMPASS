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

/*
 * ProjectionManager.h
 *
 *  Created on: Mar 12, 2013
 *      Author: sk
 */

#ifndef PROJECTIONMANAGER_H_
#define PROJECTIONMANAGER_H_

#include <ogr_spatialref.h>

#include "configurable.h"
#include "singleton.h"

//#include "Global.h"

/**
 * @brief Singleton for coordinate projection handling
 *
 * Currently handles projection from world coordinates to Cartesian coordinates using the WGS-84 method.
 */
class ProjectionManager : public Singleton, public Configurable
{
protected:
    /// Height offset
    float minimal_height_;
    /// System center latitude
    double center_latitude_;
    /// System center longitude
    double center_longitude_;

    double center_system_x_; //without transform
    double center_system_y_; //without transform

    /// World scale plane width
    double projection_plane_width_;
    /// World scale constant
    double world_scale_;
    /// World height scale factor
    double height_scale_;
    /// World center point in x

    double mult_factor_;
    double trans_x_factor_;
    double trans_y_factor_;

    unsigned int epsg_value_;

    OGRSpatialReference geo_;
    OGRSpatialReference cart_;

    OGRCoordinateTransformation *geo2cart_;
    OGRCoordinateTransformation *cart2geo_;

    /// @brief Constructor
    ProjectionManager();

    void createProjection ();

public:
    /// @brief Desctructor
    virtual ~ProjectionManager();
    /// @brief Return world plane size
    double getWorldSize (double size);

    /// @brief Scales world position height
    float transformHeight (float value);

    /// @brief Projects geo-coordinate in WGS-84 to cartesian coordinate, transform flag to ogre coordinates, else meters
    void geo2Cart (double latitude, double longitude, double &x_pos, double &y_pos, bool transform=true);
    /// @brief Projects cartesian coordinate to geo-coordinate in WGS-84
    void cart2geo (double x_pos, double y_pos, double &latitude, double &longitude, bool transform=true);

    double getCenterLatitude () { return center_latitude_; }
    double getCenterLongitude () { return center_longitude_; }

    double getCenterSystemX () { return center_system_x_; } // without transform
    double getCenterSystemY () { return center_system_y_; } // without transform

    void setCenterLatitude (double value) { center_latitude_ = value; }
    void setCenterLongitude (double value) { center_longitude_ = value; }

    //std::string getWorldWKTInfo ();
    std::string getWorldPROJ4Info ();
    void setNewCartesianEPSG (unsigned int epsg_value);
    //std::string getCartesianWKTInfo ();
    std::string getCartesianPROJ4Info ();

    unsigned int getEPSG () { return epsg_value_; }

    /// @brief Returns static instance
    static ProjectionManager& instance()
    {
        static ProjectionManager instance;
        return instance;
    }
};

#endif /* PROJECTIONMANAGER_H_ */
