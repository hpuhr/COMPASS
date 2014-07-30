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
 * DataSource.cpp
 *
 *  Created on: Sep 30, 2013
 *      Author: sk
 */

#include "DataSource.h"
#include "Logger.h"
#include <cmath>

#include "String.h"

using namespace Utils::String;

DataSource::DataSource()
: finalized_(false), dbo_type_(DBO_UNDEFINED), id_(0), sac_(255), sic_(255), latitude_(360.0), longitude_(360), altitude_(1701),
 a11_(0), a12_(0), a13_(0), a21_(0), a22_(0), a23_(0), a31_(0), a32_(0), a33_(0), b1_(0), b2_(0), b3_(0),best_radius_(0),
 grande_normale_(0), sys_x_(0), sys_y_(0), sys_z_(0)
{

}

DataSource::~DataSource()
{

}

void DataSource::finalize ()
{
    assert (!finalized_);

    double alt;    // WGS-84 altitude of reference point; metres
    double f;      // Auxiliary
    double gn;     // The so-called "Grande Normale"; metres
    double lat;    // Geodetical latitude of reference point; radians
    double lon;    // Geodetical longitude of reference point; radians

    // Extract geodetical coordinates of reference point
    alt = 0.0;
    lat = latitude_ * 0.0174532925; //47.5
    lon = longitude_ * 0.0174532925; // 14

    const double earth_a =   6378137.00;
    // Semi major axis of earth; meters
    //const double earth_b = 6356752.3142;
    // Semi minor axis of earth; meters
    const double earth_e1sq = 0.0066943844;

    // Check geodetical latitude and longitude
    assert (-M_PI/2 <= lat && lat <= M_PI/2);
    assert (-M_PI <= lon && lon <= M_PI);

    // Compute the so-called "Grande Normale"
    f = 1.0 - earth_e1sq * ::sin (lat) * ::sin (lat);

    assert (f > 0.0);

    gn = earth_a / ::sqrt (f);

    // Set the coefficients of the rotation matrix
    a11_ = - ::sin (lon);
    a12_ = - ::sin (lat) * ::cos (lon);
    a13_ = ::cos (lat) * ::cos (lon);
    a21_ = ::cos (lon);
    a22_ = - ::sin (lat) * ::sin (lon);
    a23_ = ::cos (lat) * ::sin (lon);
    a31_ = 0.0;
    a32_ = ::cos (lat);
    a33_ = ::sin (lat);

    // Set the elements of the translation vector
    b1_ = (gn + alt) * ::cos (lat) * ::cos (lon);
    b2_ = (gn + alt) * ::cos (lat) * ::sin (lon);
    b3_ = ((1.0 - earth_e1sq) * gn + alt) * ::sin (lat);

    // Compute the local "best" earth radius
    best_radius_ = earth_a * (1.0 - 0.5 * earth_e1sq * ::cos (2.0 * lat));

    // Store the so-called "Grande Normale"
    grande_normale_ = gn;

    // Remember WGS-84 height of reference point

//    loginf << "DataSource: finalize: dbo " << DB_OBJECT_TYPE_STRINGS.at((DB_OBJECT_TYPE)dbo_type_) << " id " << id_ << " name " << name_
//            << " short name "  << short_name_ << " sac " << (int)sac_ << " sic " << (int)sic_ << " lat " << latitude_
//            << " long " << longitude_ << " alt " << altitude_;

    finalized_=true;
}

std::pair <double, double> DataSource::calculateWorldCoordinates (double azimuth, double slant_range, double altitude)
{
    assert (finalized_);

    double calc_altitude = calculateElevation (slant_range, altitude);

    //loginf << "calc alt " << calc_altitude *57.3 ;

    double azimuth_rad = azimuth * 0.01745329251;

    //    t_Retc c_Geomap :: map_lpc_to_lcl
    //       (t_Real range, t_Real azimuth, t_Real elevation, t_Cpos lcl_pos)
    //   {
    double lx;     // Local x coordinate; metres
    double ly;     // Local y coordinate; metres
    double lz;     // Local z coordinate; metres

    // Check parameters
    assert (slant_range > 0.0);
    assert (0.0 <= azimuth_rad && azimuth_rad < 2*M_PI);
    assert (-M_PI <= calc_altitude && calc_altitude <= M_PI);
//    {
//        logerr << "DataSource: calculateWorldCoordinates: got wrong altitude " << calc_altitude;
//        return std::pair <double, double> (0,0);
//    }

    // Compute local coordinates
    lx = slant_range * ::sin (azimuth_rad) * ::cos (calc_altitude);
    ly = slant_range * ::cos (azimuth_rad) * ::cos (calc_altitude);
    lz = slant_range * ::sin (calc_altitude);

    //loginf << " lx " << lx << " ly " << ly << " lz " << lz;

    //    t_Retc c_Geomap :: map_lcl_to_ecef // UGA 2
    //       (t_Mapping_Info *info_ptr, t_Cpos lcl_pos, t_Cpos ecef_pos) // lcl local pos in radar coord, ecef result
    //   {
    double ex;     // ECEF x coordinate; metres
    double ey;     // ECEF y coordinate; metres
    double ez;     // ECEF z coordinate; metres
    //    double lx;     // Local x coordinate; metres
    //    double ly;     // Local y coordinate; metres
    //    double lz;     // Local z coordinate; metres

    // Extract the local coordinates
    //    lx = lcl_pos[M_CPOS_X];
    //    ly = lcl_pos[M_CPOS_Y];
    //    lz = lcl_pos[M_CPOS_Z];
    //
    //    Assert (info_ptr->defined, "Mapping information not defined");

    // Multiply with rotation matrix
    ex = a11_ * lx + a12_ * ly + a13_ * lz;
    ey = a21_ * lx + a22_ * ly + a23_ * lz;
    ez = a31_ * lx + a32_ * ly + a33_ * lz;

    // Add translation vector, results in ECEF coordinates
    ex = ex + b1_;
    ey = ey + b2_;
    ez = ez + b3_;

    //loginf << " ex " << ex << " ey " << ey << " ez " << ez;

    //    t_Retc c_Geomap :: map_ecef_to_llh (t_Cpos ecef_pos, t_Geo_Pos *geo_ptr) // uga 3
    //   {
    //double alt;    // Geographical altitude; metres
    double cos_phi;
    // Auxiliary
    //double ex, ey, ez;
    // Auxiliaries
    double f, f1, f2, f3;
    // Auxiliaries
    //double gn;     // So-called "grande normale"; metres
    double lat;    // Geodetical latitude; radians
    double lon;    // Geodetical longitude; radians
    double phi;    // Auxiliary
    double rho;    // Auxiliary

    double sin_lat;
    // Auxiliary
    double sin_phi;
    // Auxiliary

    const double earth_a = 6378137.00;
    // Semi major axis of earth; meters
    const double earth_b = 6356752.3142;
    // Semi minor axis of earth; meters
    const double earth_e1sq = 0.0066943844;

    // We are using Bowring's approximation

    rho = ::sqrt (ex * ex + ey * ey);

    //phi = utl_azimuth (ez * earth_a, rho * earth_b);

    phi = atan (ez * earth_a / rho * earth_b);

    sin_phi = ::sin (phi);
    cos_phi = ::cos (phi);

    f3 = earth_a / earth_b;
    f3 = f3 * f3 * earth_e1sq;

    f1 = ez + earth_b * f3 * sin_phi * sin_phi * sin_phi;
    f2 = rho - earth_a * earth_e1sq * cos_phi * cos_phi * cos_phi;

    //lat = utl_azimuth (f1, f2);

    lat = atan (f1 / f2);

    //lon = utl_azimuth (ey, ex);

    lon = atan (ey / ex);

    if (lat > M_PI/2)
    {
        lat -= 2*M_PI;
    }

    if (lon > M_PI)
    {
        lon -= 2*M_PI;
    }

    sin_lat = ::sin (lat);
    f = 1.0 - earth_e1sq * sin_lat * sin_lat;
    assert (f > 0.0);
//    {
//        logerr << "DataSource: calculateWorldCoordinates: f is " << f;
//        return std::pair <double, double> (0,0);
//    }
    //gn = earth_a / ::sqrt (f);

    //alt = rho / ::cos (lat) - gn;

    //    geo_ptr->latitude = lat;
    //    geo_ptr->longitude = lon;
    //    geo_ptr->altitude = alt;

    // Set return code
    //         ret = RC_OKAY;


    return std::pair<double, double> ((360.0 * lat / (2*M_PI))-0.0663, 360.0 * lon/ (2*M_PI));
}

//return angle above horizon
double DataSource::calculateElevation (double range, double height)
{
    assert (finalized_);
    double elv;    // Elevation of plot above radar plane; radians
    double er;     // Local "best" earth radius; metres
    double f;      // Auxiliary
    double f1;     // Auxiliary
    double f2;     // Auxiliary
    double hp;     // Height of plot; metres above WGS-84 ellipsoid
    double hs;     // Height of sensor; metres above WGS-84 ellipsoid

    assert (range > 0.0);

    // Set the "best" local earth radius
    er = best_radius_;

    // Set the plot (or whatever) height
    hp = height;

    // Set the reference height
    hs = altitude_;

    //loginf << " UGA best rad " << best_radius_ << " hp " << hp << " has " << hs << " range " << range;

    // Compute auxiliaries
    f1 = (2.0 * er + hp + hs) * (hp - hs) - range * range;
    f2 = 2.0 * (er + hs) * range;

    if (f2 <= 0.0)
        throw std::runtime_error ("DataSource: calculateElevation: f2 <= 0, value "+doubleToString(f2));;

//    {
//        throw std::runtime_error ("DataSource: calculateElevation: f2 is zero");
//    }

    f = f1 / f2;

    // Check against allowable range [-1.0, +1.0]
    if (::fabs (f) > 1.0)
    {
        throw std::runtime_error ("DataSource: calculateElevation: |f| > 1, value "+doubleToString(f));
    }

    // Compute elevation
    elv = ::asin (f);

    // Store result value
    return elv;
}

double DataSource::getAltitude() const
{
    return altitude_;
}

DB_OBJECT_TYPE DataSource::getDBOType () const
{
    return (DB_OBJECT_TYPE) dbo_type_;
}

unsigned int DataSource::getId() const
{
    return id_;
}

double DataSource::getLatitude() const
{
    return latitude_;
}

double DataSource::getLongitude() const
{
    return longitude_;
}

std::string DataSource::getName() const
{
    return name_;
}

unsigned char DataSource::getSac() const
{
    return sac_;
}

std::string DataSource::getShortName() const
{
    return short_name_;
}

unsigned char DataSource::getSic() const
{
    return sic_;
}

void DataSource::setAltitude(double altitude)
{
    this->altitude_ = altitude;
}

void DataSource::setDBOType (DB_OBJECT_TYPE type)
{
    assert (type != DBO_UNDEFINED);
    dbo_type_=type;
}

void DataSource::setId(unsigned int id)
{
    this->id_ = id;
}

void DataSource::setLatitude(double latitiude)
{
    this->latitude_ = latitiude;
}

void DataSource::setLongitude(double longitude)
{
    this->longitude_ = longitude;
}

void DataSource::setName(std::string name)
{
    this->name_ = name;
}

void DataSource::setSac(unsigned char sac)
{
    this->sac_ = sac;
}

void DataSource::setShortName(std::string short_name)
{
    this->short_name_ = short_name;
}

void DataSource::setSic(unsigned char sic)
{
    this->sic_ = sic;
}


