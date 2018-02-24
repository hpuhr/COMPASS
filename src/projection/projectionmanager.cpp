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

    registerParameter ("epsg_value", &epsg_value_, 31258); // 	MGI Austria GK M31.prj 	BMN – M31 	Greenwich

    geo_.SetWellKnownGeogCS("WGS84");
    createProjection();
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

bool ProjectionManager::geo2Cart (double latitude, double longitude, double& x_pos, double& y_pos)
{
    logdbg << "ProjectionManager: geo2Cart: lat " << latitude << " long " << longitude;

    x_pos = longitude;
    y_pos = latitude;

    bool ret = geo2cart_->Transform(1, &x_pos, &y_pos);

    if (!ret)
       logerr << "ProjectionManager: geo2Cart: error with longitude " << longitude << " latitude " << latitude;

    return ret;
}

bool ProjectionManager::cart2geo (double x_pos, double y_pos, double& latitude, double& longitude)
{
    logdbg << "ProjectionManager: cart2geo: x_pos " << x_pos << " y_pos " << y_pos;

    longitude = x_pos;
    latitude = y_pos;

    bool ret = cart2geo_->Transform(1, &longitude, &latitude);

    if (!ret)
       logerr << "ProjectionManager: cart2geo: error with x_pos " << x_pos << " y_pos " << y_pos;

    return ret;
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
        throw std::runtime_error ("ProjectionManager: createProjection: cartesian EPSG value "
                                  +std::to_string(epsg_value_)+" caused OGR error "
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
