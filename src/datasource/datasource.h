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
 * DataSource.h
 *
 *  Created on: Sep 30, 2013
 *      Author: sk
 */

#ifndef DATASOURCE_H_
#define DATASOURCE_H_

#include "Global.h"

class DataSource
{
public:
    DataSource();
    virtual ~DataSource();

    double getAltitude() const;
    unsigned int getId() const;
    const std::string &getDBOType () const;
    double getLatitude() const;
    double getLongitude() const;
    const std::string &getName() const;
    unsigned char getSac() const;
    const std::string &getShortName() const;
    unsigned char getSic() const;

    void setAltitude(double altitude);
    void setId(unsigned int id);
    void setDBOType (const std::string &type);
    void setLatitude(double latitiude);
    void setLongitude(double longitude_);
    void setName(const std::string &name);
    void setSac(unsigned char sac);
    void setShortName(const std::string &short_name);
    void setSic(unsigned char sic);

    void finalize ();

    // azimuth degrees, range & altitude in meters
    void calculateSystemCoordinates (double azimuth, double slant_range, double altitude, bool has_altitude, double &sys_x, double &sys_y);

protected:
    bool finalized_;
    std::string dbo_type_;
    unsigned int id_;
    std::string name_;
    std::string short_name_;
    unsigned char sac_;
    unsigned char sic_;
    double latitude_; //degrees
    double longitude_; // degrees
    double altitude_;  // meter above msl

    double system_x_;
    double system_y_;

//    double local_trans_x_;
//    double local_trans_y_;

    double deg2rad_;
};

#endif /* DATASOURCE_H_ */
