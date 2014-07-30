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

static const double earth_a = 6378137.00;
                   // Semi major axis of earth; meters
static const double earth_b = 6356752.3142;
                   // Semi minor axis of earth; meters
static const double earth_e1sq = 0.0066943844;
                   // Square of first excentricity of earth

ProjectionManager::ProjectionManager()
: Configurable ("ProjectionManager", "ProjectionManager0", 0)
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

    degree_to_radian_ = 2.0*M_PI/360.0;
    radian_to_degree_ = 360.0/2.0*M_PI;

    loginf << "ProjectionManager: constructor: center lat " << center_latitude_ << " long " << center_longitude_
           << " alt " << center_altitude_;
}

ProjectionManager::~ProjectionManager()
{
}

double  ProjectionManager::getWorldSize (double size)
{
    return size*world_scale_/projection_plane_width_;
}

float ProjectionManager::transformPositionX (float value)
{
    return value*world_scale_/projection_plane_width_+world_center_x_;
}
float ProjectionManager::transformPositionY (float value)
{
    return -value*world_scale_/projection_plane_width_+world_center_y_;
}
float ProjectionManager::transformHeight (float value)
{
    return value*height_scale_;///projection_plane_width_;
}

void ProjectionManager::project(double latitude, double longitude, float &pos_x, float &pos_y)
{
    double x=0.;
    double y=0.;
    projectPoint(latitude, longitude, x, y);

    pos_x = x*world_scale_/projection_plane_width_+world_center_x_;
    pos_y = -y*world_scale_/projection_plane_width_+world_center_y_;
}

void ProjectionManager::projectZeroHeight(double latitude, double longitude, float &pos_x, float &pos_y, float &pos_z)
{
    double x=0.;
    double y=0.;
    projectPoint(latitude, longitude, x, y);
    pos_x = x*world_scale_/projection_plane_width_+world_center_x_;
    pos_y=0;
    pos_z = -y*world_scale_/projection_plane_width_+world_center_y_;
}

std::pair<double, double> ProjectionManager::projectZeroHeight(double latitude, double longitude)
{
    std::pair<double, double> result;

    double x=0.;
    double y=0.;
    projectPoint(latitude, longitude, x, y);
    result.first = x*world_scale_/projection_plane_width_+world_center_x_;
    //pos_y=0;
    result.second = -y*world_scale_/projection_plane_width_+world_center_y_;

    return result;
}

void ProjectionManager::projectPoint (double latitude, double longitude, double &x_pos, double &y_pos)
{
    double a11, a21, a31;
                  // Transformation coefficients
    double c;        // Auxiliary
    double cplat;    // Cosine of center latitude
    double cprlong;  // Cosine of difference of longitudes
    double cpsi;     // Cosine of geocentric latitude
    double crlat;    // Cosine of intermediate latitude
    double f1;       // Auxiliary
    double f2;       // Auxiliary
    double gn;       // "Grande Normale"; meters
    double prlong;   // Difference of longitudes; degrees
    double psi;      // Geocentric latitude of projection point; radians
    double rlat;     // Intermediate latitude; radians
    double rrad;     // Radial distance of projection point; meters
    double splat;    // Sine of center latitude
    double sprlong;  // Sine of difference of longitudes
    double spsi;     // Sine of geocentric latitude
    double srlat;    // Sine of intermediate latitude

                  // Preset return value(s)
//   if (xpos_ptr != NULL)
//       *xpos_ptr = 0.0;
//   if (ypos_ptr != NULL)
//       *ypos_ptr = 0.0;

                  // Check parameter(s)
   //TODO include
//  Assert (-90.0 <= center.latitude && center.latitude <= 90.0,
//          "Invalid parameter");
//  Assert (-180.0 <= center.longitude && center.longitude <= 180.0,
//            "Invalid parameter");
//  Assert (-90.0 <= latitude && latitude <= 90.0, "Invalid parameter");
//  Assert (-180.0 <= longitude && longitude <= 180.0, "Invalid parameter");
//  Assert (xpos_ptr != NULL, "Invalid parameter");
//  Assert (ypos_ptr != NULL, "Invalid parameter");

                  // Compute geocentric latitude of projection point
   f1 = (1.0 - earth_e1sq) * std::sin (degree_to_radian_ * latitude);
   f2 = std::cos (degree_to_radian_ * latitude);
   psi = std::atan2 (f1, f2);
   //cout<<"f1:"<<f1<<" f2:"<<f2<<" psi:"<<psi<<endl;
                  // Is in radians, not in degrees

                  // Sine and cosine of geocentric latitude
   spsi = std::sin (psi);
   cpsi = std::cos (psi);
   //cout<<"spsi:"<<spsi<<" cpsi:"<<cpsi<<endl;
                  // Compute distance from projection point to
                  // the center of earth
   f1 = 1.0 - earth_e1sq * cpsi * cpsi;
   //cout<<"f1:"<<f1<<endl;
      //Assume (f1 > 0.0, "Domain error");
       if (f1 < 0.0) {
         throw std::runtime_error("Domain error");
       }
   rrad = earth_b / std::sqrt (f1);
   //cout<<"rrad:"<<rrad<<endl;
                  // Sine and cosine of center latitude
   f1 = degree_to_radian_ * center_latitude_;
   splat = std::sin (f1);
   cplat = std::cos (f1);
   //cout<<"f1:"<<f1<<" splat:"<<splat<<" cplat:"<<cplat<<endl;

                  // Compute "Grande Normale"
   f1 = 1.0 - earth_e1sq * splat * splat;
   //cout<<"f1:"<<f1<<endl;
      //Assume (f1 > 0.0, "Domain error");
       if (f1 < 0.0) {
         throw std::runtime_error("Domain error");
       }
   gn = earth_a / std::sqrt (f1);

                  // Compute some intermediate value
   c = earth_e1sq * gn * splat;

                  // Compute intermediate latitude
   f1 = c + rrad * spsi;
   f2 = rrad * cpsi;
   rlat = std::atan2 (f1, f2);
                  // Is in radians, not in degrees

                  // Sine and cosine of intermediate latitude
   srlat = std::sin (rlat);
   crlat = std::cos (rlat);

                  // Compute difference of longitudes
   prlong = center_longitude_ - longitude;

                  // Sine and cosine of difference of longitudes
   sprlong = std::sin (degree_to_radian_ * prlong);
   cprlong = std::cos (degree_to_radian_ * prlong);

                  // Compute some coefficients of transformation matrix
   a11 = crlat * cplat * cprlong + srlat * splat;
   a21 = - crlat * sprlong;
   a31 = srlat * cplat - crlat * splat * cprlong;

                  // Stereographic projection of position onto
                  // plane tangential to earth at reference point

   f1 = 2.0 * gn / (1.0 + a11);

   x_pos = a21 * f1;
   y_pos = a31 * f1;

//  done:          // We are done
   return;
}
