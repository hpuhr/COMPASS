#include "ogrcoordinatesystem.h"
#include "ogrprojection.h"
#include "logger.h"
#include "global.h"

using namespace std;

OGRCoordinateSystem::OGRCoordinateSystem(unsigned int id, double latitude_deg, double longitude_deg, double altitude_m)
    : id_(id), latitude_deg_(latitude_deg), longitude_deg_(longitude_deg), altitude_m_(altitude_m)
{
    wgs84_.SetWellKnownGeogCS("WGS84");
    wgs84_ellispoid_semi_major_ = wgs84_.GetSemiMajor();
    wgs84_ellispoid_semi_minor_ = wgs84_.GetSemiMinor();

    double scale = 1.0;
    double radius_at_radar = getRadiusAt (latitude_deg_);

    // scaling trial
    scale = (radius_at_radar+altitude_m_) / radius_at_radar;

    local_.SetStereographic(latitude_deg, longitude_deg_, scale, 0.0, 0.0);

    ogr_geo2cart_.reset(OGRCreateCoordinateTransformation (&wgs84_, &local_));
    assert (ogr_geo2cart_);

    ogr_cart2geo_.reset(OGRCreateCoordinateTransformation (&local_, &wgs84_));
    assert (ogr_cart2geo_);
}

OGRCoordinateSystem::~OGRCoordinateSystem()
{
}

bool OGRCoordinateSystem::polarSlantToCartesian (double azimuth_rad, double slant_range_m,
                                            bool has_altitude, double altitude_baro_m, bool do_slant_range_correction,
                                            double& x_pos_m, double& y_pos_m)
{
    if (!do_slant_range_correction)
        return polarHorizontalToCartesian (azimuth_rad, slant_range_m, x_pos_m, y_pos_m);

    // calulated lat/lon of target for radius estimation
    double h_e_t; // height of ellipsoid at target location, slight error here

    {
        // cartesian without slant range correction
        double x_pos_non_sr_cor, y_pos_non_sr_cor;
        bool ret = polarHorizontalToCartesian (azimuth_rad, slant_range_m, x_pos_non_sr_cor, y_pos_non_sr_cor);
        assert (ret);

        // wgs84 without slant range correction
        double lat_non_sr_cor, lon_non_sr_cor;
        ret = cartesian2WGS84 (x_pos_non_sr_cor, y_pos_non_sr_cor, lat_non_sr_cor, lon_non_sr_cor);
        assert (ret);

        h_e_t = getRadiusAt(lat_non_sr_cor * DEG2RAD);
    }

    double h_e_r = getRadiusAt(latitude_deg_ * DEG2RAD); // height of ellipsoid at radar location
    double h_r = h_e_r + altitude_m_; // height of radar, msl error

    logdbg << "OGRCoordinateSystem: polarSlantToCartesian: h_e_r " << fixed << h_e_r << " h_r " << h_r;

    double h_t;

    if (has_altitude) // if altitude is given
        h_t = h_e_t + altitude_baro_m; // baro = geo error
    else
        h_t = h_e_t;

    logdbg << "OGRCoordinateSystem: polarSlantToCartesian: h_e_t " << fixed << h_e_t << " h_t " << h_t;

    double r_s = slant_range_m;

    double alpha = acos((-pow(r_s, 2) + pow(h_r, 2) + pow (h_t, 2))/(2*h_r*h_t));

    logdbg << "OGRCoordinateSystem: polarSlantToCartesian: alpha " << fixed << alpha*RAD2DEG;

    double r_h = sqrt(pow(h_r, 2) + pow (h_r, 2) - 2*h_r*h_r*cos(alpha)); // horizontal range at radar height

    return polarHorizontalToCartesian (azimuth_rad, r_h, x_pos_m, y_pos_m);
}

bool OGRCoordinateSystem::polarHorizontalToCartesian (double azimuth_rad, double horizontal_range_m,
                                                      double& x_pos_m, double& y_pos_m)
{
    x_pos_m = horizontal_range_m * sin(azimuth_rad);
    y_pos_m = horizontal_range_m * cos(azimuth_rad);

    return true;
}

bool OGRCoordinateSystem::wgs842Cartesian (double latitude_deg, double longitude_deg, double& x_pos, double& y_pos)
{
    logdbg << "OGRCoordinateSystem: wgs842Cartesian: lat " << latitude_deg << " long " << longitude_deg;

    x_pos = longitude_deg;
    y_pos = latitude_deg;

    bool ret = ogr_geo2cart_->Transform(1, &x_pos, &y_pos);

    if (!ret)
        logerr << "OGRCoordinateSystem: wgs842Cartesian: error with latitude "
               << latitude_deg << " longitude " << longitude_deg;

    return ret;
}

bool OGRCoordinateSystem::cartesian2WGS84 (double x_pos, double y_pos, double& latitude, double& longitude)
{
    logdbg << "OGRCoordinateSystem: cartesian2WGS84: x_pos " << x_pos << " y_pos " << y_pos;

    longitude = x_pos;
    latitude = y_pos;

    bool ret = ogr_cart2geo_->Transform(1, &longitude, &latitude);

    if (!ret)
        logerr << "OGRCoordinateSystem: cartesian2WGS84: error with x_pos " << x_pos << " y_pos " << y_pos;

    return ret;
}

double OGRCoordinateSystem::getRadiusAt (double latitude_rad)
{
    // see https://en.wikipedia.org/wiki/Reference_ellipsoid
    //  a and b are the equatorial radius (semi-major axis) and the polar radius (semi-minor axis),
    // N(lat) = a^2 / sqrt(a^2 * cos^2 (lat) + b^2 * sin^2 (lat))

    return pow(wgs84_ellispoid_semi_major_, 2) / sqrt (
                pow(wgs84_ellispoid_semi_major_, 2) * pow(cos(latitude_rad), 2)
                + pow(wgs84_ellispoid_semi_minor_, 2) * pow(sin(latitude_rad), 2));
}
