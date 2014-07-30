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

#include "Configurable.h"
#include "Singleton.h"

#include "Global.h"

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
    /// System center altitude
    double center_altitude_;

    /// Radians to degrees conversion constant
    double degree_to_radian_;
    /// Degrees to radians conversion constant
    double radian_to_degree_;

    /// World scale plane width
    double projection_plane_width_;
    /// World scale constant
    double world_scale_;
    /// World height scale factor
    double height_scale_;
    /// World center point in x
    double world_center_x_;
    /// World center point in y
    double world_center_y_;

    /// @brief Constructor
    ProjectionManager();

    /// @brief Projects geo-coordinate to cartesian coordinate in WGS-84
    void projectPoint (double latitude, double longitude, double &x_pos, double &y_pos);

public:
    /// @brief Desctructor
    virtual ~ProjectionManager();
    /// @brief Return world plane size
    double getWorldSize (double size);

    /// @brief Scales/translates world position x
    float transformPositionX (float value);
    /// @brief Scales/translates world position y
    float transformPositionY (float value);
    /// @brief Scales world position height
    float transformHeight (float value);

    /// @brief Projects geo-coordinate to cartesian coordinate with scaling and translation
    void project(double latitude, double longitude, float &pos_x, float &pos_y);
    /// @brief Projects geo-coordinate to cartesian coordinate without hight
    void projectZeroHeight(double latitude, double longitude, float &pos_x, float &pos_y, float &pos_z);
    std::pair<double, double> projectZeroHeight(double latitude, double longitude);

    double getCenterLatitude () { return center_latitude_; }
    double getCenterLongitude () { return center_longitude_; }
    double getCenterAltitude () { return center_altitude_; }

    void setCenterLatitude (double value) { center_latitude_ = value; }
    void setCenterLongitude (double value) { center_longitude_ = value; }
    void setCenterAltitude (double value) { center_altitude_ = value; }

public:
    /// @brief Returns static instance
    static ProjectionManager& getInstance()
    {
        static ProjectionManager instance;
        return instance;
    }
};

#endif /* PROJECTIONMANAGER_H_ */
