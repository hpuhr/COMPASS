#include "ogrcoordinatesystem.h"
#include "ogrprojection.h"
#include "logger.h"

OGRCoordinateSystem::OGRCoordinateSystem(unsigned int id, double latitude_deg, double longitude_deg, double altitude_m)
    : id_(id), latitude_deg_(latitude_deg), longitude_deg_(longitude_deg), altitude_m_(altitude_m)
{
    wgs84_.SetWellKnownGeogCS("WGS84");
    local_.SetStereographic(latitude_deg, longitude_deg_, 1.0, 0.0, 0.0);

    ogr_geo2cart_.reset(OGRCreateCoordinateTransformation (&wgs84_, &local_));
    assert (ogr_geo2cart_);

    ogr_cart2geo_.reset(OGRCreateCoordinateTransformation (&local_, &wgs84_));
    assert (ogr_cart2geo_);
}

OGRCoordinateSystem::~OGRCoordinateSystem()
{
}

bool OGRCoordinateSystem::ogrGeo2Cart (double latitude, double longitude, double& x_pos, double& y_pos)
{
    logdbg << "OGRCoordinateSystem: ogrGeo2Cart: lat " << latitude << " long " << longitude;

    x_pos = longitude;
    y_pos = latitude;

    bool ret = ogr_geo2cart_->Transform(1, &x_pos, &y_pos);

    if (!ret)
        logerr << "OGRCoordinateSystem: ogrGeo2Cart: error with longitude " << longitude << " latitude " << latitude;

    return ret;
}

bool OGRCoordinateSystem::ogrCart2Geo (double x_pos, double y_pos, double& latitude, double& longitude)
{
    logdbg << "OGRCoordinateSystem: ogrCart2Geo: x_pos " << x_pos << " y_pos " << y_pos;

    longitude = x_pos;
    latitude = y_pos;

    bool ret = ogr_cart2geo_->Transform(1, &longitude, &latitude);

    if (!ret)
        logerr << "OGRCoordinateSystem: ogrCart2Geo: error with x_pos " << x_pos << " y_pos " << y_pos;

    return ret;
}
