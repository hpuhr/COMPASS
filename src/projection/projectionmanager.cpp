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

bool ProjectionManager::useSDLProjection() const
{
    return use_sdl_projection_;
}

void ProjectionManager::useSDLProjection(bool use_sdl_projection)
{
    use_sdl_projection_ = use_sdl_projection;

    use_ogr_projection_ = !use_sdl_projection_;
}

bool ProjectionManager::useOGRProjection() const
{
    return use_ogr_projection_;
}

void ProjectionManager::useOGRProjection(bool use_ogr_projection)
{
    use_ogr_projection_ = use_ogr_projection;

    use_sdl_projection_ = !use_ogr_projection_;
}

ProjectionManager::ProjectionManager()
    : Configurable ("ProjectionManager", "ProjectionManager0", 0, "projection.xml")
{
    loginf  << "ProjectionManager: constructor";

    registerParameter ("use_sdl_projection", &use_sdl_projection_, true);
    registerParameter ("use_ogr_projection", &use_ogr_projection_, false);

    registerParameter ("sdl_system_latitude", &sdl_system_latitude_, 37.5);
    registerParameter ("sdl_system_longitude", &sdl_system_longitude_, 14.0);

    registerParameter ("epsg_value", &epsg_value_, 31258); // 	MGI Austria GK M31.prj 	BMN – M31 	Greenwich

    ogr_geo_.SetWellKnownGeogCS("WGS84");
    createOGRProjection();
}

ProjectionManager::~ProjectionManager()
{
    assert (ogr_geo2cart_);
    delete ogr_geo2cart_;
    ogr_geo2cart_=0;

    assert (ogr_cart2geo_);
    delete ogr_cart2geo_;
    ogr_cart2geo_=0;

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

bool ProjectionManager::ogrGeo2Cart (double latitude, double longitude, double& x_pos, double& y_pos)
{
    logdbg << "ProjectionManager: geo2Cart: lat " << latitude << " long " << longitude;

    x_pos = longitude;
    y_pos = latitude;

    bool ret = ogr_geo2cart_->Transform(1, &x_pos, &y_pos);

    if (!ret)
       logerr << "ProjectionManager: geo2Cart: error with longitude " << longitude << " latitude " << latitude;

    return ret;
}

bool ProjectionManager::ogrCart2Geo (double x_pos, double y_pos, double& latitude, double& longitude)
{
    logdbg << "ProjectionManager: cart2geo: x_pos " << x_pos << " y_pos " << y_pos;

    longitude = x_pos;
    latitude = y_pos;

    bool ret = ogr_cart2geo_->Transform(1, &longitude, &latitude);

    if (!ret)
       logerr << "ProjectionManager: cart2geo: error with x_pos " << x_pos << " y_pos " << y_pos;

    return ret;
}

std::string ProjectionManager::getWorldPROJ4Info ()
{
    char *tmp=0;
    ogr_geo_.exportToProj4(&tmp);
    std::string info = tmp;
    CPLFree (tmp);

    //loginf << "ProjectionManager: getWorldPROJ4Info: '" << info << "'";

    return info;
}

void ProjectionManager::setNewCartesianEPSG (unsigned int epsg_value)
{
    epsg_value_=epsg_value;
    createOGRProjection();
}

void ProjectionManager::createOGRProjection ()
{
    OGRErr error = ogr_cart_.importFromEPSG(epsg_value_);
    if (error != OGRERR_NONE)
        throw std::runtime_error ("ProjectionManager: createProjection: cartesian EPSG value "
                                  +std::to_string(epsg_value_)+" caused OGR error "
                                  +std::to_string((int)error));

    if (ogr_geo2cart_)
    {
        delete ogr_geo2cart_;
        ogr_geo2cart_=0;
    }
    ogr_geo2cart_ = OGRCreateCoordinateTransformation( &ogr_geo_, &ogr_cart_ );
    assert (ogr_geo2cart_);

    if (ogr_cart2geo_)
    {
        delete ogr_cart2geo_;
        ogr_cart2geo_=0;
    }
    ogr_cart2geo_ = OGRCreateCoordinateTransformation( &ogr_cart_, &ogr_geo_ );
    assert (ogr_cart2geo_);
}

std::string ProjectionManager::getCartesianPROJ4Info ()
{
    char *tmp=0;
    ogr_cart_.exportToProj4(&tmp);
    std::string info = tmp;
    CPLFree (tmp);

    logdbg << "ProjectionManager: getCartesianPROJ4Info: '" << info << "'";

    return info;
}

ProjectionManagerWidget* ProjectionManager::widget ()
{
    if (!widget_)
    {
        widget_ = new ProjectionManagerWidget ();
    }
    assert (widget_);
    return widget_;
}

float ProjectionManager::sdlSystemLatitude() const
{
    return sdl_system_latitude_;
}

void ProjectionManager::sdlSystemLatitude(float sdl_system_latitude)
{
    sdl_system_latitude_ = sdl_system_latitude;
}

float ProjectionManager::sdlSystemLongitude() const
{
    return sdl_system_longitude_;
}

void ProjectionManager::sdlSystemLongitude(float sdl_system_longitude)
{
    sdl_system_longitude_ = sdl_system_longitude;
}
