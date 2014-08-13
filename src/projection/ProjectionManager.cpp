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
 * ProjectionManager.cpp
 *
 *  Created on: Mar 12, 2013
 *      Author: sk
 */

#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>

#include "Config.h"
#include "ProjectionManager.h"

//static const double earth_a = 6378137.00;
//                   // Semi major axis of earth; meters
//static const double earth_b = 6356752.3142;
//                   // Semi minor axis of earth; meters
//static const double earth_e1sq = 0.0066943844;
//                   // Square of first excentricity of earth

//proj +proj=lcc +lat_0=47 +lat_1=46 +lat_2=49 +lon_0=13d20 +ellps=WGS84 | perl
//circle_inter.pl |
//proj +proj=lcc +lat_0=47 +lat_1=46 +lat_2=49 +lon_0=13d20 +ellps=WGS84 -I -f "%.6f"

ProjectionManager::ProjectionManager()
: Configurable ("ProjectionManager", "ProjectionManager0", 0), geo2cart_(0), cart2geo_(0)
{
    logdbg  << "ProjectionManager: constructor";

    registerParameter("center_latitude", &center_latitude_, 47.5);
    registerParameter("center_longitude", &center_longitude_, 14.0);
    registerParameter("center_altitude", &center_altitude_, 0.0);

    registerParameter("minimal_height", &minimal_height_, 0.0);
    registerParameter("projection_plane_width", &projection_plane_width_, 1e6);
    registerParameter("world_scale", &world_scale_, 2000);
    registerParameter("height_scale", &height_scale_, 2000);
    registerParameter("world_center_x", &world_center_x_, 1000);
    registerParameter("world_center_y", &world_center_y_, 1000);

    mult_factor_ = world_scale_/projection_plane_width_;

    degree_to_radian_ = 2.0*M_PI/360.0;
    radian_to_degree_ = 360.0/2.0*M_PI;

    loginf << "ProjectionManager: constructor: center lat " << center_latitude_ << " long " << center_longitude_
           << " alt " << center_altitude_;

    geo_.SetWellKnownGeogCS("WGS84");
    //cart_.SetWellKnownGeogCS( "EPSG:31258" );
    cart_.importFromEPSG(31258);

    geo2cart_ = OGRCreateCoordinateTransformation( &geo_, &cart_ );
    assert (geo2cart_);
    cart2geo_ = OGRCreateCoordinateTransformation( &cart_, &geo_ );
    assert (cart2geo_);

    double center_pos_x, center_pos_y;

    geo2Cart(center_latitude_, center_longitude_, center_pos_x, center_pos_y);
    loginf << "got x " << center_pos_x << " y " << center_pos_y;

    geo2Cart(center_longitude_, center_latitude_, center_pos_x, center_pos_y);
    loginf << "got reverse x " << center_pos_x << " y " << center_pos_y;
}

ProjectionManager::~ProjectionManager()
{
    assert (geo2cart_);
    delete geo2cart_;
    geo2cart_=0;

    assert (cart2geo_);
    delete cart2geo_;
    cart2geo_=0;
}

double  ProjectionManager::getWorldSize (double size)
{
    return size*mult_factor_;
}

float ProjectionManager::transformPositionX (float value)
{
    return value*mult_factor_+world_center_x_;
}
float ProjectionManager::transformPositionY (float value)
{
    return -value*mult_factor_+world_center_y_;
}
float ProjectionManager::transformHeight (float value)
{
    return value*height_scale_;///projection_plane_width_;
}

//void ProjectionManager::project(double latitude, double longitude, float &pos_x, float &pos_y)
//{
//    double x=0.;
//    double y=0.;
//    projectPoint(latitude, longitude, x, y);
//
//    pos_x = x*world_scale_/projection_plane_width_+world_center_x_;
//    pos_y = -y*world_scale_/projection_plane_width_+world_center_y_;
//}
//
//void ProjectionManager::projectZeroHeight(double latitude, double longitude, float &pos_x, float &pos_y, float &pos_z)
//{
//    double x=0.;
//    double y=0.;
//    projectPoint(latitude, longitude, x, y);
//    pos_x = x*world_scale_/projection_plane_width_+world_center_x_;
//    pos_y=0;
//    pos_z = -y*world_scale_/projection_plane_width_+world_center_y_;
//}
//
//std::pair<double, double> ProjectionManager::projectZeroHeight(double latitude, double longitude)
//{
//    std::pair<double, double> result;
//
//    double x=0.;
//    double y=0.;
//    projectPoint(latitude, longitude, x, y);
//    result.first = x*world_scale_/projection_plane_width_+world_center_x_;
//    //pos_y=0;
//    result.second = -y*world_scale_/projection_plane_width_+world_center_y_;
//
//    return result;
//}

void ProjectionManager::geo2Cart (double latitude, double longitude, double &x_pos, double &y_pos)
{
    x_pos = longitude;
    y_pos = latitude;

    bool ret = geo2cart_->Transform(1, &x_pos, &y_pos);
    assert (ret);

    x_pos *= mult_factor_;
    y_pos *= mult_factor_;
}

void ProjectionManager::cart2geo (double x_pos, double y_pos, double &latitude, double &longitude)
{
    longitude = x_pos/mult_factor_;
    latitude = y_pos/mult_factor_;

    bool ret = cart2geo_->Transform(1, &longitude, &latitude);
    assert (ret);
}

