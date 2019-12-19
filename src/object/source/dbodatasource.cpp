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

#include "dbodatasource.h"
//#include "dbodatasourcewidget.h"
#include "dbobject.h"
#include "projectionmanager.h"
#include "rs2g.h"
#include "dboeditdatasourceaction.h"
#include "storeddbodatasource.h"

#include <cmath>

DBODataSource::DBODataSource(DBObject& object, unsigned int id, const std::string& name)
    : object_(&object), id_(id), name_(name)
{
}

DBODataSource& DBODataSource::operator=(StoredDBODataSource& other)
{
    //id_ = other.id(); not copied, keep own
    name_ = other.name();
    has_short_name_ = other.hasShortName();
    if (has_short_name_)
        short_name_ = other.shortName();
    has_sac_ = other.hasSac();
    if (has_sac_)
        sac_ = other.sac();
    has_sic_ = other.hasSic();
    if (has_sic_)
        sic_ = other.sic();
    has_latitude_ = other.hasLatitude();
    if (has_latitude_)
        latitude_ = other.latitude();
    has_longitude_ = other.hasLongitude();
    if (has_longitude_)
        longitude_ = other.longitude();
    has_altitude_ = other.hasAltitude();
    if (has_altitude_)
        altitude_ = other.altitude();

    loginf << "DBODataSource: operator=: name " << name_
           << " short name " << (has_short_name_ ? short_name_ : "false")
           << " sac " << (has_sac_ ? std::to_string(static_cast<int>(sac_)) : "false")
           << " sic " << (has_sic_ ? std::to_string(static_cast<int>(sic_)) : "false")
           << " lat " << (has_latitude_ ? std::to_string(latitude_) : "false")
           << " lon " << (has_longitude_ ? std::to_string(longitude_) : "false")
           << " alt " << (has_altitude_ ? std::to_string(altitude_) : "false");

    return *this;
}

DBODataSource& DBODataSource::operator=(DBODataSource&& other)
{
    loginf << "DBODataSource: move operator: moving";

    object_ = other.object_;
    id_ = other.id_;

    name_ = other.name_;
    has_short_name_ = other.has_short_name_;
    short_name_ = other.short_name_;
    has_sac_ = other.has_sac_;
    sac_ = other.sac_;
    has_sic_ = other.has_sic_;
    sic_ = other.sic_;
    has_latitude_ = other.has_latitude_;
    latitude_ = other.latitude_;
    has_longitude_ = other.has_longitude_;
    longitude_ = other.longitude_;
    has_altitude_ = other.has_altitude_;
    altitude_ = other.altitude_;

    return *this;
}

bool DBODataSource::operator==(StoredDBODataSource& other)
{
    logdbg << "DBODataSource: operator==: name " << (name_ == other.name())
           << " short " << (short_name_ == other.shortName())
           << " sac " << (sac_ == other.sac())
           << " sic " << (sic_ == other.sic())
           << " lat " << (fabs(latitude_ - other.latitude()) < 1e-10)
           << " long " << (fabs(longitude_ - other.longitude()) < 1e-10)
           << " alt " << (fabs(altitude_ - other.altitude()) < 1e-10);

    return (name_ == other.name()) &&
            (has_short_name_ == other.hasShortName()) &&
            (has_short_name_ ? short_name_ == other.shortName() : true) &&
            (has_sac_ == other.hasSac()) &&
            (has_sac_ ? sac_ == other.sac() : true) &&
            (has_sic_ == other.hasSic()) &&
            (has_sic_ ? sic_ == other.sic() : true) &&
            (has_latitude_ == other.hasLatitude()) &&
            (has_latitude_ ? fabs(latitude_ - other.latitude()) < 1e-10 : true) &&
            (has_longitude_ == other.hasLongitude()) &&
            (has_longitude_ ? fabs(longitude_ - other.longitude()) < 1e-10 : true) &&
            (has_altitude_ == other.hasAltitude()) &&
            (has_altitude_? fabs(altitude_ - other.altitude()) < 1e-10 : true);
}

DBODataSource::~DBODataSource()
{
    logdbg << "DBODataSource: dtor: id " << std::to_string(id_);
}

void DBODataSource::finalize ()
{
    finalized_ = false;

    ProjectionManager& proj_man = ProjectionManager::instance();

    assert (proj_man.useOGRProjection() || proj_man.useSDLProjection() || proj_man.useRS2GProjection());

    if (proj_man.useOGRProjection())
    {
        bool ret = proj_man.ogrGeo2Cart(latitude_, longitude_, ogr_system_x_, ogr_system_y_);

        if (!ret)
        {
            logwrn << "DBODataSource: finalize: ogrGeo2Cart return false for " << short_name_
                   << " lat " << latitude_ << " lon " << longitude_ << " x " << ogr_system_x_ << " y " << ogr_system_y_;
            return;
        }

        initRS2G(); // used for elevation calculation
    }

    if (proj_man.useSDLProjection())
    {
        preset_cpos (&grs_pos_);
        preset_gpos (&geo_pos_);
        preset_mapping_info (&mapping_info_);

        geo_pos_.latitude = latitude_ * DEG2RAD;
        geo_pos_.longitude = longitude_ * DEG2RAD;
        geo_pos_.altitude = altitude_; // TODO check if exists
        geo_pos_.defined = true;

        t_Retc lrc;

        lrc = geo_calc_info (geo_pos_, &mapping_info_);
        assert (lrc == RC_OKAY);
    }

    if (proj_man.useRS2GProjection())
        initRS2G();

    loginf << "DBODataSource: finalize: " << short_name_ << " done";

    finalized_ = true;
}

// azimuth degrees, range nautical miles, altitude in meters
bool DBODataSource::calculateOGRSystemCoordinates (double azimuth_rad, double slant_range_m, bool has_baro_altitude,
                                                   double baro_altitude_ft, double &sys_x, double &sys_y)
{
    if (!finalized_)
        logerr << "DBODataSource: calculateOGRSystemCoordinates: " << short_name_ << " not finalized";

    assert (finalized_);

    //slant_range = 1852.0 * slant_range; // convert to meters

    double horizontal_range = 0.0;
    double altitude_m = 0.3048 * baro_altitude_ft;

    //            altitude_m -= data_sources.at(sensor_id).altitude(); //TODO use this?

    //    if (slant_range <= altitude)
    //    {
    //        logerr << "DataSource: calculateSystemCoordinates: a " << azimuth << " sr " << slant_range << " alt " << altitude
    //                << ", assuming range = slant range";
    //        range = slant_range; // TODO pure act of desperation
    //    }
    //    else
    //        range = sqrt (slant_range*slant_range-altitude*altitude); // TODO: flatland

    //    if (has_baro_altitude && slant_range_m > altitude_m)
    //        horizontal_range = sqrt (slant_range_m*slant_range_m-altitude_m*altitude_m);
    //    else
    //        horizontal_range = slant_range_m; // TODO pure act of desperation

    //    sys_x = horizontal_range * sin (azimuth_rad);
    //    sys_y = horizontal_range * cos (azimuth_rad);

    if (!has_baro_altitude)
        altitude_m = altitude_;

    double elevation = rs2gElevation(altitude_m, slant_range_m);

    sys_x = slant_range_m * cos(elevation) * sin(azimuth_rad);
    sys_y = slant_range_m * cos(elevation) * cos(azimuth_rad);

    sys_x += ogr_system_x_;
    sys_y += ogr_system_y_;

    if (sys_x != sys_x || sys_y != sys_y)
    {
        loginf << "DBODataSource: calculateOGRSystemCoordinates: error calculatign a " << azimuth_rad << " sr "
               << slant_range_m << " alt " << baro_altitude_ft << " hor.range " << horizontal_range
               << " sys_x " << sys_x << " sys_y " << sys_y;
        return false;
    }

    return true;
}

// azimuth degrees, range nautical miles, altitude in meters
bool DBODataSource::calculateSDLGRSCoordinates (double azimuth_rad, double slant_range_m, bool has_baro_altitude,
                                                double baro_altitude_ft, t_CPos& grs_pos)
{
    if (!finalized_)
        logerr << "DBODataSource: calculateSDLSystemCoordinates: " << short_name_ << " not finalized";

    assert (finalized_);

    t_Retc lrc;
    t_Real hgt = 0.0;    // Height for projection; metres
    bool hgt_defined = false;    // Height for projection defined

    if (has_baro_altitude)
    {
        double baro_altitude_m = 0.3048 * baro_altitude_ft;
        hgt = map_mch_to_hae (baro_altitude_m);
        hgt_defined = true;
    }

    // check height
    if (!hgt_defined)
    {
        t_Real default_height;
        // Default height; metres
        t_Real default_height_gradient;
        // Default height gradient

        default_height = 100.0 * M_FL2MTR;
        // Beware: hard-coded value
        // Beware: no mapping by map_mch_to_hae()
        default_height_gradient = 1.0 / (20.0 * M_NMI2MTR);
        // Beware: hard-coded value

        //        if (msg_ptr->rtgt.reported_ppos.present)
        //        {
        t_Real f;
        // Factor
        t_Real rng;
        // Slant range; metres

        rng = slant_range_m;
        //if (msg_ptr->base.data_source_identifier.value == 0x7801) rng -= 235.0;

        // Compute factor
        f = 1.0;
        if (rng > 0.0)
        {
            f = default_height_gradient * rng;
            if (f > 1.0)
            {
                f = 1.0;
            }
        }

        // Set assumed height
        hgt = f * default_height;
        hgt_defined = true;

        // Remember this height
        //        msg_ptr->proc.assumed_height.present = true;
        //        msg_ptr->proc.assumed_height.value = hgt;
        //        msg_ptr->proc.assumed_height.value_in_feet =
        //                (t_Si32) (hgt / M_FT2MTR);

        //#if DBGAID
        //            dbg_printf (103225, " using height=%.2f [mtr]", hgt);
        //#endif
        //        }
        //        else
        //        {
        //            // No polar position

        //            // Set assumed height
        //            hgt = default_height;
        //            hgt_defined = true;

        //            // Remember this height
        //            msg_ptr->proc.assumed_height.present = true;
        //            msg_ptr->proc.assumed_height.value = hgt;
        //            msg_ptr->proc.assumed_height.value_in_feet =
        //                    (t_Si32) (hgt / M_FT2MTR);

        //#if DBGAID
        //            dbg_printf (103225, " using height=%.2f [mtr]", hgt);
        //#endif
        //        }
    }

    bool elevation_present = false;
    double elevation = 0.0;

    // calculate elevation angle
    if (hgt_defined) // must be at this point
    {
        t_Real elv;
        // Elevation (above radar horizon); degrees
        t_Real rng;
        // Slant range; metres

        rng = slant_range_m;
        //if (msg_ptr->base.data_source_identifier.value == 0x7801) rng -= 235.0;

        if (rng > 0.0)
        {
            lrc = geo_calc_elv (&mapping_info_, rng, hgt, &elv);

            if (lrc == RC_OKAY)
            {
                elevation_present = true;
                elevation = M_DEG2RAD * elv;
            }
        }
    }

    if (!elevation_present)
    {
        loginf << "DBODataSource: calculateDSLSystemCoordinates: a " << azimuth_rad << " sr " << slant_range_m
               << " alt " << baro_altitude_ft << " elevation not present";
        return false;
    }

    //    if (elevation_present)
    //    {
    t_Real azm;
    // Azimuth; radians
    t_Real elv;
    // Elevation (above radar horizon); radians
    t_Real rng;
    // Slant range; metres

    azm = azimuth_rad;
    elv = elevation;
    rng = slant_range_m;
    //if (msg_ptr->base.data_source_identifier.value == 0x7801) rng -= 235.0;

    assert (0.0 <= azm && azm < M_TWO_PI); //, "Invalid azimuth");
    assert (-M_PI_HALF <= elv && elv <= +M_PI_HALF); //,"Invalid elevation");
    assert (rng > 0.0); //, "Invalid slant range");

    t_CPos lcl_pos;
    // Local Cartesian position

    lrc = geo_lpc_to_lcl (rng, azm, elv, &lcl_pos);
    assert (lrc == RC_OKAY); //, "Cannot map to local Cartesian position");

    t_CPos tmp_grs_pos;
    // GRS position

    lrc = geo_lcl_to_grs (&mapping_info_, lcl_pos, &tmp_grs_pos);
    assert (lrc == RC_OKAY); //, "Cannot map to GRS position");

    grs_pos = tmp_grs_pos;

    //        t_CPos sys_pos;
    //        // Position relative to system reference point

    //        lrc = geo_grs_to_lcl (&(gp_areas->srp_mapping_info),
    //                              grs_pos, &sys_pos);
    //        Assert (lrc == RC_OKAY, "Cannot map to system coordinates");

    //        msg_ptr->proc.uvh_position.defined = true;
    //        msg_ptr->proc.uvh_position.value[M_CPOS_U] =
    //                sys_pos.value[M_CPOS_X];
    //        msg_ptr->proc.uvh_position.value[M_CPOS_V] =
    //                sys_pos.value[M_CPOS_Y];
    //        msg_ptr->proc.uvh_position.value[M_CPOS_H] =
    //                hgt;
    //    }

    return true;
}

void DBODataSource::initRS2G()
{
    loginf << "DBODataSource: initRS2G: name " << name_ << " lat " << latitude_ << " long " << longitude_
           << " altitude " << altitude_;

    double lat_rad = latitude_ * DEG2RAD;
    double long_rad = longitude_ * DEG2RAD;

    rs2gFillMat(rs2g_A_, lat_rad, long_rad);

    rs2g_T_Ai_ = rs2g_A_.transpose();

    MatA A_p0(3,3), A_q0(3,3);
    VecB b_p0(3), b_q0(3);

    rs2g_Rti_ = EE_A * (1 - EE_E2) / sqrt(pow(1 - EE_E2 * pow(sin(lat_rad), 2), 3));
    //       printf(" - best earth radius:%g\n", (*it).second.rad->Rti());

    rs2gFillMat(A_p0, lat_rad, long_rad);
    //mult(A_q0, trans(A_p0), *A_p0q0_); // A_p0q0 = A_q0 * Transpose(A_p0) in doxygen ...
    rs2g_A_p0q0_ = A_q0 * A_p0.transpose();
    //(*it).second.A_p0q0 = A_p0q0_;

    // #if defined(DEBUG_ARTAS_TRF)
    //       printf(" - radar matrix final values\n");
    //       print_all_matrix(*A_p0q0_);
    // #endif

    rs2gFillVec(b_p0, lat_rad, long_rad, altitude_);

    //add(scaled(b_q0, -1.0), b_p0, b_p0); // b_p0 = b_p0 - b_q0 in doxygen ...
    b_p0 = b_p0 - b_q0;

    //mult(A_q0, b_p0, *b_p0q0);    // b_p0q0 = A_q0 * (b_p0 - b_q0) in doxygen ...
    rs2g_b_p0q0_ = A_q0 * b_p0;

    // #if defined(DEBUG_ARTAS_TRF)
    //       printf(" - radar vector final values\n");
    //       print_vector(*b_p0q0);
    // #endif
    //    }

    // from setradar
    rs2gFillVec(rs2g_bi_, lat_rad, long_rad, altitude_);

    rs2g_hi_ = altitude_;
}

double DBODataSource::rs2gAzimuth(double x, double y)
{
    double azimuth = 0.0;

    if (x == 0.0 && y == 0.0)
        return azimuth;

    //    this is the implementation in ARTAS/COMSOFT
    //    IRS document Appendix A2 v6.5 2002/06/28
    //
    //    if (x != 0.0 || y != 0.0) {
    //       if (y == 0.0) {
    //          if (x > 0.0)
    //             azimuth = M_PI / 2.0;
    //          else
    //             azimuth = 3.0 * M_PI / 2.0;
    //       }
    //       else {
    //          azimuth = atan(x/y);
    //          if (y < 0.0)
    //             azimuth += M_PI;
    //          else {
    //             if (x < 0.0)
    //                azimuth += 2.0 * M_PI;
    //          }
    //       }
    //    }

    // this is an equivalent implementation to the above
    // and was taken from the TRANSLIB library function
    // 'azimuth' in file 'artas_trans.c'
    if (fabs(y) < ALMOST_ZERO)
        azimuth = (x / fabs(x)) * M_PI / 2.0;
    else
        azimuth = atan2(x, y);

    if (azimuth < 0.0)
        azimuth += 2.0 * M_PI;

    return azimuth;
}

double DBODataSource::rs2gElevation(double z, double rho)
{
    double elevation = 0.0;

    if (rho >= ALMOST_ZERO) {
       double x = (2 * rs2g_Rti_ * (z - rs2g_hi_) + pow(z, 2) - pow(rs2g_hi_, 2) - pow(rho, 2)) /
               (2 * rho * (rs2g_Rti_ + rs2g_hi_));
       if (fabs(x) <= 1.0)
          elevation = asin(x);
    }

//    if (rho >= ALMOST_ZERO)
//    {
//        //        elevation = asin((2 * rs2g_Rti_ * (z - rs2g_hi_) + pow(z, 2) - pow(rs2g_hi_, 2) - pow(rho, 2)) /
//        //                         (2 * rho * (rs2g_Rti_ + rs2g_hi_)));
//        elevation = asin((z - rs2g_hi_)/rho);

//        //        if (rho < 50000)
//        //            loginf << "DBODataSource: rs2gElevation: z " << z << " rho " << rho << " elev " << elevation;
//    }

    return elevation;
}


void DBODataSource::radarSlant2LocalCart(VecB& local)
{
    logdbg << "radarSlant2LocalCart: in x: " << local[0] << " y: " << local[1] << " z: " << local[2];

    double z = local[2];
    if (z == -1000.0)
        z = rs2g_hi_; // the Z value has not been filled so use at least the radar height

    //double rho = sqrt(pow(local[0], 2) + pow(local[1], 2) + pow(z, 2));
    double rho = sqrt(pow(local[0], 2) + pow(local[1], 2));
    double elevation = rs2gElevation(z, rho);
    double azimuth = rs2gAzimuth(local[0], local[1]);

    //    if (rho < 50000)
    //    {
    //        loginf << "radarSlant2LocalCart: in x: " << local[0] << " y: " << local[1] << " z: " << local[2];
    //        loginf << "radarSlant2LocalCart: rho: " << rho << " elevation: " << elevation << " azimuth: " << azimuth;
    //    }

    local[0] = rho * cos(elevation) * sin(azimuth);
    local[1] = rho * cos(elevation) * cos(azimuth);
    local[2] = rho * sin(elevation);

    logdbg << "radarSlant2LocalCart: out x: " << local[0] << " y: " << local[1] << " z: " << local[2];
}

void DBODataSource::sysCart2SysStereo(VecB& b, double* x, double* y)
{
    double H = sqrt(pow(b[0], 2) + pow(b[1], 2) + pow(b[2] + rs2g_ho_ + rs2g_Rto_, 2)) - rs2g_Rto_;
    double k = 2 * rs2g_Rto_ / (2 * rs2g_Rto_ + rs2g_ho_ + b[2] + H);

    *x = k * b[0];
    *y = k * b[1];
}

void DBODataSource::localCart2Geocentric(VecB& input)
{
    logdbg << "localCart2Geocentric: in x: " << input[0] << " y:" << input[1] << " z:" << input[2];

    VecB Xinput(3);

    // local cartesian to geocentric
    //mult(T_Ai_, input, Xinput); // Xinput = Transposed(Ai_) * input
    Xinput = rs2g_T_Ai_ * input;

    //add(bi_, Xinput, input); // Xinput = Xinput + bi_
    input = Xinput + rs2g_bi_;

    logdbg << "localCart2Geocentric: out x: " << input[0] << " y:" << input[1] << " z:" << input[2];
}

bool DBODataSource::calculateRadSlt2Geocentric (double x, double y, double z, Eigen::Vector3d& geoc_pos)
{
    if (!finalized_)
        logerr << "DBODataSource: calculateRadSlt2Geocentric: " << short_name_ << " not finalized";

    assert (finalized_);

    VecB Xlocal(3);  //, Xsystem(3);

    // the coordinates are in radar slant coordinates
    Xlocal[0] = x;
    Xlocal[1] = y;
    Xlocal[2] = z;

    // radar slant to local cartesian
    radarSlant2LocalCart(Xlocal);

    // local cartesian to geocentric
    localCart2Geocentric(Xlocal);

    geoc_pos = Xlocal;

    // geocentric to geodesic
    //Geocentric2Geodesic(Xlocal);
    // done later

    return true;
}

bool DBODataSource::hasLatitude() const
{
    return has_latitude_;
}

double DBODataSource::altitude() const
{
    assert (has_altitude_);
    return altitude_;
}

unsigned int DBODataSource::id() const
{
    return id_;
}

double DBODataSource::latitude() const
{
    assert (has_latitude_);
    return latitude_;
}

double DBODataSource::longitude() const
{
    assert (has_longitude_);
    return longitude_;
}

bool DBODataSource::hasShortName() const
{
    return has_short_name_;
}

bool DBODataSource::hasSac() const
{
    return has_sac_;
}

bool DBODataSource::hasSic() const
{
    return has_sic_;
}

bool DBODataSource::hasLongitude() const
{
    return has_longitude_;
}

bool DBODataSource::hasAltitude() const
{
    return has_altitude_;
}

const std::string &DBODataSource::name() const
{
    return name_;
}

void DBODataSource::name(const std::string &name)
{
    this->name_ = name;
}

unsigned char DBODataSource::sac() const
{
    assert (has_sac_);
    return sac_;
}

const std::string &DBODataSource::shortName() const
{
    assert (has_short_name_);
    return short_name_;
}

unsigned char DBODataSource::sic() const
{
    assert (has_sic_);
    return sic_;
}

void DBODataSource::altitude(double altitude)
{
    has_altitude_ = true;
    this->altitude_ = altitude;
}

void DBODataSource::latitude(double latitiude)
{
    has_latitude_= true;
    this->latitude_ = latitiude;
}

void DBODataSource::longitude(double longitude)
{
    has_longitude_ = true;
    this->longitude_ = longitude;
}

void DBODataSource::sac(unsigned char sac)
{
    has_sac_ = true;
    this->sac_ = sac;
}

void DBODataSource::shortName(const std::string &short_name)
{
    has_short_name_ = true;
    this->short_name_ = short_name;
}

void DBODataSource::sic(unsigned char sic)
{
    has_sic_ = true;
    this->sic_ = sic;
}

void DBODataSource::removeShortName()
{
    has_short_name_ = false;
    short_name_ = "";
}

void DBODataSource::removeSac()
{
    has_sac_ = false;
    sac_ = 0;
}

void DBODataSource::removeSic()
{
    has_sic_ = false;
    sic_ = 0;
}

void DBODataSource::removeLatitude()
{
    has_latitude_ = false;
    latitude_ = 0;
}

void DBODataSource::removeLongitude()
{
    has_longitude_ = false;
    longitude_ = 0;
}

void DBODataSource::removeAltitude()
{
    has_altitude_ = false;
    altitude_ = 0;
}
void DBODataSource::updateInDatabase ()
{
    assert (object_);
    object_->updateDataSource(id_);
}
