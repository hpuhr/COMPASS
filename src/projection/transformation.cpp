#include "transformation.h"
#include "logger.h"
#include "compass.h"

#include <ogr_spatialref.h>

#include <iomanip>

//bool Transformation::in_appimage_ = COMPASS::isAppImage();
const double Transformation::max_wgs_dist_ {0.1};

Transformation::Transformation()
    : wgs84_{new OGRSpatialReference()}, local_{new OGRSpatialReference()}
{
    wgs84_->SetWellKnownGeogCS("WGS84");
}

Transformation::Transformation (const Transformation& a)
    : wgs84_{new OGRSpatialReference()}, local_{new OGRSpatialReference()}
{
    wgs84_->SetWellKnownGeogCS("WGS84");
}


Transformation::~Transformation()
{
}


std::tuple<bool, double, double> Transformation::distanceCart (double lat1, double long1, double lat2, double long2)
{
    logdbg << "Transformation: distanceCart: lat1 " << lat1 << " long1 " << long1
           << " lat2 " << lat2 << " long2 " << long2;

    updateCenter(lat1, long1);

    double x_pos1, y_pos1, x_pos2, y_pos2;
    bool ok;
    std::tuple<bool, double, double> ret {false, 0, 0};

    x_pos1 = long1;
    y_pos1 = lat1;

    x_pos2 = long2;
    y_pos2 = lat2;

    ok = ogr_geo2cart_->Transform(1, &x_pos1, &y_pos1); // wgs84 to cartesian offsets

    if (!ok)
    {
        return ret;
    }

    ok = ogr_geo2cart_->Transform(1, &x_pos2, &y_pos2); // wgs84 to cartesian offsets

    if (!ok)
    {
        return ret;
    }

    ret = std::tuple<bool, double, double>(true, x_pos2-x_pos1, y_pos2-y_pos1);

    logdbg << "Transformation: distanceCart: p1 " << lat1 << " / " << long1
           << " p2 " << lat2 << " / " << long2 << " d " << x_pos2-x_pos1 << " / " << y_pos2-y_pos1;

    return ret;
}

std::tuple<bool, double, double> Transformation::wgsAddCartOffset (
        double lat1, double long1, double x_pos2, double y_pos2)
{
    logdbg << "Transformation: wgsAddCartOffset: lat1 " << lat1 << " long1 " << long1
           << " x_pos2 " << x_pos2 << " y_pos2 " << y_pos2;

    updateCenter(lat1, long1);

    bool ok;
    std::tuple<bool, double, double> ret {false, 0, 0};

    // calc pos 1 cart
    double x_pos1, y_pos1;

        x_pos1 = long1;
        y_pos1 = lat1;


    ok = ogr_geo2cart_->Transform(1, &x_pos1, &y_pos1); // wgs84 to cartesian offsets

    if (!ok)
    {
        //loginf << "UGA1";
        return ret;
    }

    logdbg << "Transformation: wgsAddCartOffset: x_pos1 " << x_pos1 << " y_pos1 " << y_pos1
           << " x_pos2 " << x_pos2 << " y_pos2 " << y_pos2;

    // add origin offset
    x_pos2 += x_pos1;
    y_pos2 += y_pos1;

    ok = ogr_cart2geo_->Transform(1, &x_pos2, &y_pos2);

    if (!ok)
    {
        //loginf << "UGA2";
        return ret;
    }

    double lat2, long2;

    lat2 = y_pos2;
    long2 = x_pos2;


    ret = std::tuple<bool, double, double>(true, lat2, long2);

    return ret;
}

void Transformation::updateCenter(double lat1, double long1)
{
    if (!has_pos1_ || sqrt(pow(lat1_-lat1, 2)+pow(long1_-long1, 2)) > max_wgs_dist_) // set
    {
        has_pos1_ = true;
        lat1_ = lat1;
        long1_ = long1;

        local_->SetStereographic(lat1_, long1_, 1.0, 0.0, 0.0);

        ogr_geo2cart_.reset(OGRCreateCoordinateTransformation(wgs84_.get(), local_.get()));
        ogr_cart2geo_.reset(OGRCreateCoordinateTransformation(local_.get(), wgs84_.get()));
    }
}


//////////////////////////////////////////////
//bool FixedTransformation::in_appimage_ = true; //COMPASS::isAppImage();
//const double FixedTransformation::max_wgs_dist_ {0.5};

FixedTransformation::FixedTransformation(double lat1, double long1)
    : lat1_(lat1), long1_(long1), wgs84_{new OGRSpatialReference()}, local_{new OGRSpatialReference()}
{
    wgs84_->SetWellKnownGeogCS("WGS84");

    local_->SetStereographic(lat1, long1, 1.0, 0.0, 0.0);

    ogr_geo2cart_.reset(OGRCreateCoordinateTransformation(wgs84_.get(), local_.get()));
    ogr_cart2geo_.reset(OGRCreateCoordinateTransformation(local_.get(), wgs84_.get()));
}

FixedTransformation::~FixedTransformation()
{
}

std::tuple<bool, double, double> FixedTransformation::distanceCart (double lat2, double long2)
{
    logdbg << "FixedTransformation: distanceCart: lat2 " << lat2 << " long2 " << long2;

    double x_pos2, y_pos2;
    bool ok;
    std::tuple<bool, double, double> ret {false, 0, 0};

//    if (in_appimage_) // inside appimage
//    {
        x_pos2 = long2;
        y_pos2 = lat2;
//    }
//    else
//    {
//        x_pos2 = lat2;
//        y_pos2 = long2;
//    }

    ok = ogr_geo2cart_->Transform(1, &x_pos2, &y_pos2); // wgs84 to cartesian offsets

    if (!ok)
    {
        return ret;
    }

    ret = std::tuple<bool, double, double>(true, x_pos2, y_pos2);

    logdbg << "Transformation: distanceCart: p1 "
           << std::setprecision(14) << lat1_ << " / " << std::setprecision(14) << long1_
           << " p2 "
           << std::setprecision(14) << lat2 << " / " << std::setprecision(14) << long2
           << " d " << x_pos2 << " / " << y_pos2;

    return ret;
}

std::tuple<bool, double, double> FixedTransformation::wgsAddCartOffset (double x_pos2, double y_pos2)
{
    logdbg << "FixedTransformation: wgsAddCartOffset: x_pos2 " << x_pos2 << " y_pos2 " << y_pos2;

    bool ok;
    std::tuple<bool, double, double> ret {false, 0, 0};

    ok = ogr_cart2geo_->Transform(1, &x_pos2, &y_pos2);

    if (!ok)
    {
        //loginf << "UGA2";
        return ret;
    }

    double lat2, long2;

//    if (in_appimage_) // inside appimage
//    {
        lat2 = y_pos2;
        long2 = x_pos2;
//    }
//    else
//    {
//        lat2 = x_pos2;
//        long2 = y_pos2;
//    }

    ret = std::tuple<bool, double, double>(true, lat2, long2);

    return ret;
}


