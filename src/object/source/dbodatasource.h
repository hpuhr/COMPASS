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

#ifndef DBODATASOURCE_H
#define DBODATASOURCE_H

#include <memory>
#include <QWidget>

#include "rs2g.h"
#include "geomap.h"
#include "dboeditdatasourceactionoptions.h"

class DBObject;
class DBODataSourceWidget;
class QGridLayout;

class DBODataSource
{
public:
    DBODataSource(DBObject& object, unsigned int id, const std::string& name);
    DBODataSource() = default;

    // copy from dbds, everything but id
    DBODataSource& operator=(StoredDBODataSource& other);
    DBODataSource& operator=(DBODataSource&& other);

    // comparison
    bool operator==(StoredDBODataSource& other);
    bool operator!=(StoredDBODataSource& other) { return !(*this == other); }

    virtual ~DBODataSource();

    unsigned int id() const;
    //void id(unsigned int id);

    const std::string &name() const;
    void name(const std::string &name);

    bool hasShortName() const;
    void shortName(const std::string &short_name);
    const std::string &shortName() const;

    bool hasSac() const;
    void sac(unsigned char sac);
    unsigned char sac() const;

    bool hasSic() const;
    void sic(unsigned char sic);
    unsigned char sic() const;

    bool hasLatitude() const;
    void latitude(double latitiude);
    double latitude() const;

    bool hasLongitude() const;
    void longitude(double longitude_);
    double longitude() const;

    bool hasAltitude() const;
    void altitude(double altitude);
    double altitude() const;

    DBODataSourceWidget* widget (bool add_headers=false, QWidget* parent=0, Qt::WindowFlags f=0);

    void finalize ();

    bool isFinalized () { return finalized_; } // returns false if projection can not be made because of error

    // azimuth degrees, range & altitude in meters
    bool calculateOGRSystemCoordinates (double azimuth_rad, double slant_range_m, bool has_baro_altitude,
                                        double baro_altitude_ft, double &sys_x, double &sys_y);

    bool calculateSDLGRSCoordinates (double azimuth_rad, double slant_range_m, bool has_baro_altitude,
                                     double baro_altitude_ft, t_CPos& grs_pos);

    bool calculateRadSlt2Geocentric (double x, double y, double z, Eigen::Vector3d& geoc_pos);

protected:
    DBObject* object_;
    unsigned int id_{0};

    std::string name_;

    bool has_short_name_{false};
    std::string short_name_;

    bool has_sac_;
    unsigned char sac_;

    bool has_sic_{false};
    unsigned char sic_ {0};

    bool has_latitude_{false};
    double latitude_ {0}; //degrees

    bool has_longitude_ {false};
    double longitude_ {0}; // degrees

    bool has_altitude_ {false};
    double altitude_ {0};  // meter above msl

    std::unique_ptr<DBODataSourceWidget> widget_;

    bool finalized_ {false};

    double ogr_system_x_ {0};
    double ogr_system_y_ {0};

    t_CPos grs_pos_;
    t_GPos geo_pos_;
    t_Mapping_Info mapping_info_;

    MatA rs2g_A_;

    MatA rs2g_T_Ai_; // transposed matrix (depends on radar)
    VecB rs2g_bi_;  // vector (depends on radar)
    double rs2g_hi_; // height of selected radar
    double rs2g_Rti_; // earth radius of tangent sphere at the selected radar
    double rs2g_ho_; // height of COP
    double rs2g_Rto_; // earth radius of tangent sphere at the COP

    MatA rs2g_A_p0q0_;
    VecB rs2g_b_p0q0_;

    void initRS2G ();
    double rs2gAzimuth(double x, double y);
    double rs2gElevation(double z, double rho);
    void radarSlant2LocalCart(VecB& local);
    void sysCart2SysStereo(VecB& b, double* x, double* y);
    void localCart2Geocentric(VecB& input);

};

#endif // DBODATASOURCE_H
