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

//#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>

#include "cpl_conv.h"

#include "projectionmanager.h"
#include "projectionmanagerwidget.h"
#include "rs2gprojection.h"
#include "global.h"
#include "logger.h"

ProjectionManager::ProjectionManager()
    : Configurable ("ProjectionManager", "ProjectionManager0", 0, "projection.json")
{
    loginf  << "ProjectionManager: constructor";

    registerParameter ("current_projection_name", &current_projection_name_, "RS2G");

    //    registerParameter ("use_sdl_projection", &use_sdl_projection_, false);
    //    registerParameter ("use_ogr_projection", &use_ogr_projection_, true);
    //    registerParameter ("use_rs2g_projection_", &use_rs2g_projection_, false);

    //    registerParameter ("sdl_system_latitude", &sdl_system_latitude_, 47.5);
    //    registerParameter ("sdl_system_longitude", &sdl_system_longitude_, 14.0);

    //    loginf  << "ProjectionManager: constructor: using sdl lat " << sdl_system_latitude_
    //            << " long " << sdl_system_longitude_;

    //    // init sdl
    //    t_GPos geo_pos;

    //    preset_gpos (&geo_pos);
    //    preset_mapping_info (&sdl_mapping_info_);

    //    geo_pos.latitude = sdl_system_latitude_ * DEG2RAD;
    //    geo_pos.longitude = sdl_system_longitude_ * DEG2RAD;
    //    geo_pos.altitude = 0.0; // TODO check if exists
    //    geo_pos.defined = true;

    //    t_Retc lrc;

    //    lrc = geo_calc_info (geo_pos, &sdl_mapping_info_);

    //    assert (lrc == RC_OKAY);

    createSubConfigurables();

    registerParameter ("epsg_value", &epsg_value_, 31258); // 	MGI Austria GK M31.prj 	BMN – M31 	Greenwich

    // init ogr
    ogr_geo_.SetWellKnownGeogCS("WGS84");
    createOGRProjection();

    // init radSlt2Geo
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

void ProjectionManager::generateSubConfigurable (const std::string& class_id, const std::string& instance_id)
{
    if (class_id == "RS2GProjection")
    {
        std::string name = configuration().getSubConfiguration(
                    class_id, instance_id).getParameterConfigValueString("name");

        assert (!projections_.count(name));

        projections_[name].reset (new RS2GProjection (class_id, instance_id, *this));
    }
    else
        throw std::runtime_error ("DBObject: generateSubConfigurable: unknown class_id "+class_id );
}

void ProjectionManager::checkSubConfigurables ()
{
    if (!projections_.count("RS2G"))
    {
        Configuration& configuration = addNewSubConfiguration ("RS2GProjection");

        configuration.addParameterString ("name", "RS2G");
        generateSubConfigurable ("RS2GProjection", configuration.getInstanceId());
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

std::string ProjectionManager::currentProjectionName() const
{
    return current_projection_name_;
}

void ProjectionManager::currentProjectionName(const std::string& name)
{
    loginf << "ProjectionManager: currentProjectionName: name " << name;
    current_projection_name_ = name;
}

bool ProjectionManager::hasProjection (const std::string& name)
{
    return projections_.count(name);
}

bool ProjectionManager::hasCurrentProjection ()
{
    return hasProjection(current_projection_name_);
}

Projection& ProjectionManager::currentProjection ()
{
    return *projections_.at(current_projection_name_);
}

bool ProjectionManager::ogrGeo2Cart (double latitude, double longitude, double& x_pos, double& y_pos)
{
    logdbg << "ProjectionManager: ogrGeo2Cart: lat " << latitude << " long " << longitude;

    x_pos = longitude;
    y_pos = latitude;

    bool ret = ogr_geo2cart_->Transform(1, &x_pos, &y_pos);

    if (!ret)
        logerr << "ProjectionManager: ogrGeo2Cart: error with longitude " << longitude << " latitude " << latitude;

    return ret;
}

bool ProjectionManager::ogrCart2Geo (double x_pos, double y_pos, double& latitude, double& longitude)
{
    logdbg << "ProjectionManager: ogrCart2Geo: x_pos " << x_pos << " y_pos " << y_pos;

    longitude = x_pos;
    latitude = y_pos;

    bool ret = ogr_cart2geo_->Transform(1, &longitude, &latitude);

    if (!ret)
        logerr << "ProjectionManager: ogrCart2Geo: error with x_pos " << x_pos << " y_pos " << y_pos;

    return ret;
}

//bool ProjectionManager::sdlGRS2Geo (t_CPos grs_pos, t_GPos& geo_pos)
//{
//    //logdbg << "ProjectionManager: sdlGRS2Geo: x_pos " << x_pos << " y_pos " << y_pos;

//    t_Retc lrtc;
//    //    t_CPos lcl_pos;

//    //    lrtc = geo_grs_to_lcl (sdl_mapping_info_, grs_pos, &lcl_pos);

//    t_GPos tmp_geo_pos;

//    lrtc = geo_grs_to_llh (grs_pos, &tmp_geo_pos);

//    assert (lrtc == RC_OKAY);
//    geo_pos = tmp_geo_pos;

//    return true;
//}

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
        widget_ = new ProjectionManagerWidget (*this);
    }
    assert (widget_);
    return widget_;
}

//float ProjectionManager::sdlSystemLatitude() const
//{
//    return sdl_system_latitude_;
//}

//void ProjectionManager::sdlSystemLatitude(float sdl_system_latitude)
//{
//    sdl_system_latitude_ = sdl_system_latitude;
//}

//float ProjectionManager::sdlSystemLongitude() const
//{
//    return sdl_system_longitude_;
//}

//void ProjectionManager::sdlSystemLongitude(float sdl_system_longitude)
//{
//    sdl_system_longitude_ = sdl_system_longitude;
//}

//bool ProjectionManager::useSDLProjection() const
//{
//    return use_sdl_projection_;
//}

//void ProjectionManager::useSDLProjection(bool use_sdl_projection)
//{
//    use_sdl_projection_ = use_sdl_projection;

//    if (use_sdl_projection_)
//    {
//        use_ogr_projection_ = false;
//        use_rs2g_projection_ = false;
//    }
//}

//bool ProjectionManager::useOGRProjection() const
//{
//    return use_ogr_projection_;
//}

//void ProjectionManager::useOGRProjection(bool use_ogr_projection)
//{
//    use_ogr_projection_ = use_ogr_projection;

//    if (use_ogr_projection_)
//    {
//        use_sdl_projection_ = false;
//        use_rs2g_projection_ = false;
//    }
//}

//bool ProjectionManager::useRS2GProjection() const
//{
//    return use_rs2g_projection_;
//}

//void ProjectionManager::useRS2GProjection(bool use_rs2g_projection)
//{
//    use_rs2g_projection_ = use_rs2g_projection;

//    if (use_rs2g_projection_)
//    {
//        use_sdl_projection_ = false;
//        use_ogr_projection_ = false;
//    }
//}

