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

#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>

#include "cpl_conv.h"

#include "projectionmanager.h"
#include "projectionmanagerwidget.h"

ProjectionManager::ProjectionManager()
: Configurable ("ProjectionManager", "ProjectionManager0", 0, "projection.xml")
{
    loginf  << "ProjectionManager: constructor";

//    registerParameter("center_latitude", &center_latitude_, 47.5);
//    registerParameter("center_longitude", &center_longitude_, 14.0);

//    registerParameter("minimal_height", &minimal_height_, 0.0);
//    registerParameter("projection_plane_width", &projection_plane_width_, 1e6);
//    registerParameter("world_scale", &world_scale_, 2000);
//    registerParameter("height_scale", &height_scale_, 2000);

    registerParameter ("epsg_value", &epsg_value_, 31258); // 	MGI Austria GK M31.prj 	BMN – M31 	Greenwich

    geo_.SetWellKnownGeogCS("WGS84");
    createProjection();

//    mult_factor_ = 1.0;

//    geo2Cart(center_latitude_, center_longitude_, center_system_x_, center_system_y_, false);

//    double center_pos_x, center_pos_y;
//    double center_lat, center_long;

//    mult_factor_ = world_scale_/projection_plane_width_;
//    trans_x_factor_=0;
//    trans_x_factor_=0;

//    loginf << "ProjectionManager: constructor: center lat " << center_latitude_ << " long " << center_longitude_;

//    geo2Cart(center_latitude_, center_longitude_, center_pos_x, center_pos_y);
//    loginf << "ProjectionManager: constructor: center there x " << center_pos_x << " y " << center_pos_y;

//    trans_x_factor_ = -center_pos_x;
//    trans_y_factor_ = -center_pos_y;

//    geo2Cart(center_latitude_, center_longitude_, center_pos_x, center_pos_y);
//    cart2geo(center_pos_x, center_pos_y, center_lat, center_long);
//    loginf << "ProjectionManager: constructor: center back again lat " << center_lat << " long " << center_long;

//    geo2Cart(center_latitude_, center_longitude_, center_pos_x, center_pos_y);
//    assert (center_pos_x < 0.0001);
//    assert (center_pos_y < 0.0001);
    //loginf << "ProjectionManager: constructor: 2 got x " << center_pos_x << " y " << center_pos_y;
}

ProjectionManager::~ProjectionManager()
{
    assert (geo2cart_);
    delete geo2cart_;
    geo2cart_=0;

    assert (cart2geo_);
    delete cart2geo_;
    cart2geo_=0;

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }
}

void ProjectionManager::shutdown ()
{
    loginf  << "ProjectionManager: shutdown";

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }
}

//double  ProjectionManager::getWorldSize (double size)
//{
//    return size*mult_factor_;
//}

//float ProjectionManager::transformHeight (float value)
//{
//    return value*height_scale_;///projection_plane_width_;
//}

void ProjectionManager::geo2Cart (double latitude, double longitude, double &x_pos, double &y_pos)
{
    //loginf << "ProjectionManager: geo2Cart: lat " << latitude << " long " << longitude;

    x_pos = longitude;
    y_pos = latitude;

    bool ret = geo2cart_->Transform(1, &x_pos, &y_pos);
    assert (ret);

//    if (transform)
//    {
//        x_pos = (x_pos*mult_factor_) + trans_x_factor_;
//        y_pos = (-y_pos*mult_factor_) + trans_y_factor_;
//    }
}

void ProjectionManager::cart2geo (double x_pos, double y_pos, double &latitude, double &longitude)
{
//    if (transform)
//    {
//        x_pos = (x_pos-trans_x_factor_)/mult_factor_;
//        y_pos = -(y_pos-trans_y_factor_)/mult_factor_;
//    }
    longitude = x_pos;
    latitude = y_pos;

    bool ret = cart2geo_->Transform(1, &longitude, &latitude);

    if (!ret)
       logerr << "ProjectionManager: cart2geo: x_pos " << x_pos << " y_pos " << y_pos;

    assert (ret);
}

std::string ProjectionManager::getWorldPROJ4Info ()
{
    char *tmp=0;
    geo_.exportToProj4(&tmp);
    std::string info = tmp;
    CPLFree (tmp);

    //loginf << "ProjectionManager: getWorldPROJ4Info: '" << info << "'";

    return info;
}

void ProjectionManager::setNewCartesianEPSG (unsigned int epsg_value)
{
    epsg_value_=epsg_value;
    createProjection();
}

void ProjectionManager::createProjection ()
{
    OGRErr error = cart_.importFromEPSG(epsg_value_);
    if (error != OGRERR_NONE)
        throw std::runtime_error ("ProjectionManager: createProjection: cartesian EPSG value "+std::to_string(epsg_value_)+" caused ORG error "
        +std::to_string((int)error));

    if (geo2cart_)
    {
        delete geo2cart_;
        geo2cart_=0;
    }
    geo2cart_ = OGRCreateCoordinateTransformation( &geo_, &cart_ );
    assert (geo2cart_);

    if (cart2geo_)
    {
        delete cart2geo_;
        cart2geo_=0;
    }
    cart2geo_ = OGRCreateCoordinateTransformation( &cart_, &geo_ );
    assert (cart2geo_);
}

std::string ProjectionManager::getCartesianPROJ4Info ()
{
    char *tmp=0;
    cart_.exportToProj4(&tmp);
    std::string info = tmp;
    CPLFree (tmp);

    //loginf << "ProjectionManager: getCartesianPROJ4Info: '" << info << "'";

    return info;
}

ProjectionManagerWidget *ProjectionManager::widget ()
{
    if (!widget_)
    {
        widget_ = new ProjectionManagerWidget ();
    }
    assert (widget_);
    return widget_;
}
