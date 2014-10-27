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
#include "ProjectionManager.h"
#include <cmath>

#include "String.h"

using namespace Utils::String;

DataSource::DataSource()
: finalized_(false), dbo_type_(DBO_UNDEFINED), id_(0), sac_(255), sic_(255), latitude_(360.0), longitude_(360), altitude_(1701),
  system_x_(0), system_y_(0), local_trans_x_(0), local_trans_y_(0)
{
    deg2rad_ = 2*M_PI/360.0;
}

DataSource::~DataSource()
{

}

void DataSource::finalize ()
{
    assert (!finalized_);

    ProjectionManager::getInstance().geo2Cart(latitude_, longitude_, system_x_, system_y_, false);
    double center_system_x = ProjectionManager::getInstance().getCenterSystemX();
    double center_system_y = ProjectionManager::getInstance().getCenterSystemY();

    local_trans_x_ = center_system_x-system_x_;
    local_trans_y_ = center_system_y-system_y_;

    finalized_=true;
}

// azimuth degrees, range & altitude in meters
void DataSource::calculateSystemCoordinates (double azimuth, double slant_range, double altitude, double &sys_x, double &sys_y)
{
    if (!finalized_)
        finalize ();

    assert (finalized_);

    double range;

    if (slant_range <= altitude)
    {
        logerr << "DataSource: calculateSystemCoordinates: a " << azimuth << " sr " << slant_range << " alt " << altitude
                << ", assuming range = slant range";
        range = slant_range; // TODO pure act of desperation
    }
    else
        range = sqrt (slant_range*slant_range-altitude*altitude); // TODO: flatland

    azimuth *= deg2rad_;

    sys_x = range * cos (azimuth);
    sys_y = range * sin (azimuth);

    sys_x += local_trans_x_;
    sys_y += local_trans_y_;

    if (sys_x != sys_x || sys_y != sys_y)
    {
        logerr << "DataSource: calculateSystemCoordinates: a " << azimuth << " sr " << slant_range << " alt " << altitude
                << " range " << range << " sys_x " << sys_x << " sys_y " << sys_y;
        assert (false);
    }
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


