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
    DB_OBJECT_TYPE getDBOType () const;
    double getLatitude() const;
    double getLongitude() const;
    std::string getName() const;
    unsigned char getSac() const;
    std::string getShortName() const;
    unsigned char getSic() const;

    void setAltitude(double altitude);
    void setId(unsigned int id);
    void setDBOType (DB_OBJECT_TYPE type);
    void setLatitude(double latitiude);
    void setLongitude(double longitude_);
    void setName(std::string name);
    void setSac(unsigned char sac);
    void setShortName(std::string short_name);
    void setSic(unsigned char sic);

    void finalize ();

    std::pair <double, double> calculateWorldCoordinates (double azimuth, double slant_range, double altitude);

protected:
    bool finalized_;
    unsigned int dbo_type_;
    unsigned int id_;
    std::string name_;
    std::string short_name_;
    unsigned char sac_;
    unsigned char sic_;
    double latitude_; //degrees
    double longitude_; // degrees
    double altitude_;  // meter above msl

    double a11_, a12_, a13_, a21_, a22_, a23_, a31_, a32_, a33_;
    /* Transformation coefficients */
    double b1_, b2_, b3_;
    /* Translation vector */
    double best_radius_;
    /* Local "best" earth radius; metres */
    double grande_normale_;
    /* "Grande Normale"; metres */

    double sys_x_, sys_y_, sys_z_;

    double calculateElevation (double range, double height);

};

#endif /* DATASOURCE_H_ */
