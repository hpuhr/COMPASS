#include "ogrprojection.h"

#include "global.h"
#include "logger.h"
#include "ogrcoordinatesystem.h"
#include "projectionmanager.h"

//#include "cpl_conv.h"

OGRProjection::OGRProjection(const std::string& class_id, const std::string& instance_id,
                             ProjectionManager& proj_manager)
    : Projection(class_id, instance_id, proj_manager)
{
    // registerParameter ("epsg_value", &epsg_value_, 31258); // 	MGI Austria GK M31.prj 	BMN –
    // M31 Greenwich

    //    OGRErr error = ogr_cart_.importFromEPSG(epsg_value_);

    //    if (error != OGRERR_NONE)
    //        throw std::runtime_error ("OGRProjection: createProjection: cartesian EPSG value "
    //                                  +std::to_string(epsg_value_)+" caused OGR error "
    //                                  +std::to_string(error));
}

OGRProjection::~OGRProjection() {}

void OGRProjection::generateSubConfigurable(const std::string& class_id,
                                            const std::string& instance_id)
{
}

void OGRProjection::checkSubConfigurables() {}

bool OGRProjection::hasCoordinateSystem(unsigned int id) { return coordinate_systems_.count(id); }

void OGRProjection::addCoordinateSystem(unsigned int id, double latitude_deg, double longitude_deg,
                                        double altitude_m)
{
    assert(!hasCoordinateSystem(id));

    coordinate_systems_[id].reset(
        new OGRCoordinateSystem(id, latitude_deg, longitude_deg, altitude_m));
}

bool OGRProjection::polarToWGS84(unsigned int id, double azimuth_rad, double slant_range_m,
                                 bool has_baro_altitude, double baro_altitude_ft,
                                 double& latitude_deg, double& longitude_deg)
{
    assert(hasCoordinateSystem(id));

    //    double x1, y1, z1;
    //    bool ret;

    //    x1 = slant_range_m * sin(azimuth_rad);
    //    y1 = slant_range_m * cos(azimuth_rad);

    //    if (has_baro_altitude)
    //        z1 = baro_altitude_ft * FT2M;
    //    else
    //        z1 = 0.0;

    //    logdbg << "OGRProjection: polarToWGS84: local x " << x1 << " y " << y1 << " z " << z1;

    //    ret = coordinate_systems_.at(id)->cartesian2WGS84(x1, y1, latitude_deg, longitude_deg);

    double x_pos_m, y_pos_m;
    bool ret = coordinate_systems_.at(id)->polarSlantToCartesian(
        azimuth_rad, slant_range_m, has_baro_altitude, baro_altitude_ft * FT2M, x_pos_m, y_pos_m);

    if (!ret)
        return false;

    ret =
        coordinate_systems_.at(id)->cartesian2WGS84(x_pos_m, y_pos_m, latitude_deg, longitude_deg);

    // TODO altitude

    return ret;
}
