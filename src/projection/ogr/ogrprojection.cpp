#include "ogrprojection.h"
#include "projectionmanager.h"
#include "ogrcoordinatesystem.h"
#include "global.h"

#include "logger.h"

//#include "cpl_conv.h"

OGRProjection::OGRProjection(const std::string& class_id, const std::string& instance_id,
                               ProjectionManager& proj_manager)
    : Projection (class_id, instance_id, proj_manager)
{
    //registerParameter ("epsg_value", &epsg_value_, 31258); // 	MGI Austria GK M31.prj 	BMN – M31 	Greenwich

    //    OGRErr error = ogr_cart_.importFromEPSG(epsg_value_);

    //    if (error != OGRERR_NONE)
    //        throw std::runtime_error ("OGRProjection: createProjection: cartesian EPSG value "
    //                                  +std::to_string(epsg_value_)+" caused OGR error "
    //                                  +std::to_string(error));
}

OGRProjection::~OGRProjection()
{
}

void OGRProjection::generateSubConfigurable (const std::string& class_id, const std::string& instance_id)
{

}

void OGRProjection::checkSubConfigurables ()
{

}

bool OGRProjection::hasCoordinateSystem (unsigned int id)
{
    return coordinate_systems_.count(id);
}

void OGRProjection::addCoordinateSystem (unsigned int id, double latitude_deg, double longitude_deg, double altitude_m)
{
    assert (!hasCoordinateSystem(id));

    coordinate_systems_[id].reset(new OGRCoordinateSystem(id, latitude_deg, longitude_deg, altitude_m));
}


bool OGRProjection::polarToWGS84 (unsigned int id, double azimuth_rad, double slant_range_m, bool has_baro_altitude,
                           double baro_altitude_ft, double& latitude, double& longitude)
{
    assert (hasCoordinateSystem(id));

    double x1, y1, z1;
    bool ret;

//    Eigen::Vector3d pos;

    x1 = slant_range_m * sin(azimuth_rad);
    y1 = slant_range_m * cos(azimuth_rad);

    if (has_baro_altitude)
        z1 = baro_altitude_ft * FT2M;
    else
        z1 = 0.0;

    logdbg << "OGRProjection: polarToWGS84: local x " << x1 << " y " << y1 << " z " << z1;

    ret = coordinate_systems_.at(id)->ogrCart2Geo(x1, y1, latitude, longitude);

    // TODO altitude

    return ret;

//    if (ret)
//    {
//        logdbg << "OGRProjection: polarToWGS84: geoc x " << pos[0] << " y " << pos[1] << " z " << pos[2];

//        ret = RS2GCoordinateSystem::geocentric2Geodesic(pos);

//        latitude = pos [0];
//        longitude = pos [1];

//        logdbg << "OGRProjection: polarToWGS84: geod x " << pos[0] << " y " << pos[1];
//        //what to do with altitude?
//    }


}

//OGRSpatialReference* OGRProjection::wgs84() const
//{
//    return &wgs84_;
//}

//std::string OGRProjection::getWorldPROJ4Info ()
//{
//    char *tmp=0;
//    ogr_geo_.exportToProj4(&tmp);
//    std::string info = tmp;
//    CPLFree (tmp);

//    //loginf << "OGRProjection: getWorldPROJ4Info: '" << info << "'";

//    return info;
//}

//void OGRProjection::setNewCartesianEPSG (unsigned int epsg_value)
//{
//    epsg_value_=epsg_value;
//    init();
//}

//void OGRProjection::init ()
//{
//    OGRErr error = ogr_cart_.importFromEPSG(epsg_value_);

//    if (error != OGRERR_NONE)
//        throw std::runtime_error ("OGRProjection: createProjection: cartesian EPSG value "
//                                  +std::to_string(epsg_value_)+" caused OGR error "
//                                  +std::to_string(error));

//    ogr_geo2cart_ = nullptr;
//    ogr_cart2geo_ = nullptr;

//    ogr_geo2cart_.reset(OGRCreateCoordinateTransformation( &ogr_geo_, &ogr_cart_));
//    assert (ogr_geo2cart_);

//    ogr_cart2geo_.reset(OGRCreateCoordinateTransformation( &ogr_cart_, &ogr_geo_ ));
//    assert (ogr_cart2geo_);
//}

//std::string OGRProjection::getCartesianPROJ4Info ()
//{
//    char* tmp = nullptr;
//    ogr_cart_.exportToProj4(&tmp);
//    std::string info = tmp;
//    CPLFree (tmp);

//    logdbg << "OGRProjection: getCartesianPROJ4Info: '" << info << "'";

//    return info;
//}
