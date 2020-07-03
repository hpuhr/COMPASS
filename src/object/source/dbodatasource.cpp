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
#include "dboeditdatasourceaction.h"
#include "projectionmanager.h"
#include "storeddbodatasource.h"

#include <cmath>
#include <sstream>

using namespace std;

DBODataSource::DBODataSource(DBObject& object, unsigned int id, const std::string& name)
    : object_(&object), id_(id), name_(name)
{
}

DBODataSource& DBODataSource::operator=(StoredDBODataSource& other)
{
    // id_ = other.id(); not copied, keep own
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

    // TODO radar stuff

    loginf << "DBODataSource: operator=: name " << name_ << " short name "
           << (has_short_name_ ? short_name_ : "false") << " sac "
           << (has_sac_ ? std::to_string(static_cast<int>(sac_)) : "false") << " sic "
           << (has_sic_ ? std::to_string(static_cast<int>(sic_)) : "false") << " lat "
           << (has_latitude_ ? std::to_string(latitude_) : "false") << " lon "
           << (has_longitude_ ? std::to_string(longitude_) : "false") << " alt "
           << (has_altitude_ ? std::to_string(altitude_) : "false");

    return *this;
}

// DBODataSource& DBODataSource::operator=(DBODataSource&& other)
//{
//    loginf << "DBODataSource: move operator: moving";

//    object_ = other.object_;
//    id_ = other.id_;

//    name_ = other.name_;
//    has_short_name_ = other.has_short_name_;
//    short_name_ = other.short_name_;
//    has_sac_ = other.has_sac_;
//    sac_ = other.sac_;
//    has_sic_ = other.has_sic_;
//    sic_ = other.sic_;
//    has_latitude_ = other.has_latitude_;
//    latitude_ = other.latitude_;
//    has_longitude_ = other.has_longitude_;
//    longitude_ = other.longitude_;
//    has_altitude_ = other.has_altitude_;
//    altitude_ = other.altitude_;

//    return *this;
//}

bool DBODataSource::operator==(const StoredDBODataSource& other) const
{
    logdbg << "DBODataSource: operator==: name " << (name_ == other.name()) << " short "
           << (short_name_ == other.shortName()) << " sac " << (sac_ == other.sac()) << " sic "
           << (sic_ == other.sic()) << " lat " << (fabs(latitude_ - other.latitude()) < 1e-10)
           << " long " << (fabs(longitude_ - other.longitude()) < 1e-10) << " alt "
           << (fabs(altitude_ - other.altitude()) < 1e-10);

    // TODO radar stuff

    return (name_ == other.name()) && (has_short_name_ == other.hasShortName())
            && (has_short_name_ ? short_name_ == other.shortName() : true)
            && (has_sac_ == other.hasSac()) && (has_sac_ ? sac_ == other.sac() : true)
            && (has_sic_ == other.hasSic()) && (has_sic_ ? sic_ == other.sic() : true)
            && (has_latitude_ == other.hasLatitude())
            && (has_latitude_ ? fabs(latitude_ - other.latitude()) < 1e-10 : true)
            && (has_longitude_ == other.hasLongitude())
            && (has_longitude_ ? fabs(longitude_ - other.longitude()) < 1e-10 : true)
            && (has_altitude_ == other.hasAltitude())
            && (has_altitude_ ? fabs(altitude_ - other.altitude()) < 1e-10 : true);
}

DBODataSource::~DBODataSource() { logdbg << "DBODataSource: dtor: id " << std::to_string(id_); }

bool DBODataSource::hasLatitude() const { return has_latitude_; }

double DBODataSource::altitude() const
{
    assert(has_altitude_);
    return altitude_;
}

// psr azm
bool DBODataSource::hasPrimaryAzimuthStdDev() const
{
    return has_primary_azimuth_stddev_;
}

void DBODataSource::removePrimaryAzimuthStdDev()
{
    has_primary_azimuth_stddev_ = false;
    primary_azimuth_stddev_ = 0;
}

void DBODataSource::primaryAzimuthStdDev(double value)
{
    has_primary_azimuth_stddev_ = true;
    primary_azimuth_stddev_ = value;
}

double DBODataSource::primaryAzimuthStdDev() const
{
    return primary_azimuth_stddev_;
}

// psr range
bool DBODataSource::hasPrimaryRangeStdDev() const
{
    return has_primary_range_stddev_;
}

void DBODataSource::removePrimaryRangeStdDev()
{
    has_primary_range_stddev_ = false;
    primary_range_stddev_ = 0;
}

void DBODataSource::primaryRangeStdDev(double value)
{
    has_primary_range_stddev_ = true;
    primary_range_stddev_ = value;
}

double DBODataSource::primaryRangeStdDev() const
{
    return primary_range_stddev_;
}

// ssr azm
bool DBODataSource::hasSecondaryAzimuthStdDev() const
{
    return has_secondary_azimuth_stddev_;
}

void DBODataSource::removeSecondaryAzimuthStdDev()
{
    has_secondary_azimuth_stddev_ = false;
    secondary_azimuth_stddev_ = 0;
}

void DBODataSource::secondaryAzimuthStdDev(double value)
{
    has_secondary_azimuth_stddev_ = true;
    secondary_azimuth_stddev_ = value;
}

double DBODataSource::secondaryAzimuthStdDev() const
{
    return secondary_azimuth_stddev_;
}

// ssr range
bool DBODataSource::hasSecondaryRangeStdDev() const
{
    return has_secondary_range_stddev_;
}

void DBODataSource::removeSecondaryRangeStdDev()
{
    has_secondary_range_stddev_ = false;
    secondary_range_stddev_ = 0;
}

void DBODataSource::secondaryRangeStdDev(double value)
{
    has_secondary_range_stddev_ = true;
    secondary_range_stddev_ = value;
}

double DBODataSource::secondaryRangeStdDev() const
{
    return secondary_range_stddev_;
}

// mode s azm
bool DBODataSource::hasModeSAzimuthStdDev() const
{
    return has_mode_s_azimuth_stddev_;
}

void DBODataSource::removeModeSAzimuthStdDev()
{
    has_mode_s_azimuth_stddev_ = false;
    mode_s_azimuth_stddev_ = 0;
}

void DBODataSource::modeSAzimuthStdDev(double value)
{
    has_mode_s_azimuth_stddev_ = true;
    mode_s_azimuth_stddev_ = value;
}

double DBODataSource::modeSAzimuthStdDev() const
{
    return mode_s_azimuth_stddev_;
}

// mode s range
bool DBODataSource::hasModeSRangeStdDev() const
{
    return has_mode_s_range_stddev_;
}

void DBODataSource::removeModeSRangeStdDev()
{
    has_mode_s_range_stddev_ = false;
    mode_s_range_stddev_ = 0;
}

void DBODataSource::modeSRangeStdDev(double value)
{
    has_mode_s_range_stddev_ = true;
    mode_s_range_stddev_ = value;
}

double DBODataSource::modeSRangeStdDev() const
{
    return mode_s_range_stddev_;
}


const std::string DBODataSource::dboName() const
{
    assert(object_);
    return object_->name();
}

unsigned int DBODataSource::id() const { return id_; }

double DBODataSource::latitude() const
{
    assert(has_latitude_);
    return latitude_;
}

double DBODataSource::longitude() const
{
    assert(has_longitude_);
    return longitude_;
}

bool DBODataSource::hasShortName() const { return has_short_name_; }

bool DBODataSource::hasSac() const { return has_sac_; }

bool DBODataSource::hasSic() const { return has_sic_; }

bool DBODataSource::hasLongitude() const { return has_longitude_; }

bool DBODataSource::hasAltitude() const { return has_altitude_; }

const std::string& DBODataSource::name() const { return name_; }

void DBODataSource::name(const std::string& name) { this->name_ = name; }

unsigned char DBODataSource::sac() const
{
    assert(has_sac_);
    return sac_;
}

const std::string& DBODataSource::shortName() const
{
    assert(has_short_name_);
    return short_name_;
}

unsigned char DBODataSource::sic() const
{
    assert(has_sic_);
    return sic_;
}

void DBODataSource::altitude(double altitude)
{
    has_altitude_ = true;
    this->altitude_ = altitude;
}

void DBODataSource::latitude(double latitiude)
{
    has_latitude_ = true;
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

void DBODataSource::shortName(const std::string& short_name)
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

DBObject& DBODataSource::object()
{
    assert(object_);
    return *object_;
}

void DBODataSource::updateInDatabase()
{
    assert(object_);
    object_->updateDataSource(id_);
}

void DBODataSource::print() const
{
    assert(object_);

    stringstream ss;

    ss << "dbo " << object_->name() << " id " << id_ << " name " << name_;

    if (has_short_name_)
        ss << " short name " << short_name_;
    else
        ss << " no short name";

    if (has_sac_)
        ss << " sac " << (int) sac_;
    else
        ss << " no sac";

    if (has_sic_)
        ss << " sic " << (int) sic_;
    else
        ss << " no sic";

    if (has_latitude_)
        ss << " latitude " << latitude_;
    else
        ss << " no latitude";

    if (has_longitude_)
        ss << " longitude " << longitude_;
    else
        ss << " no longitude";

    if (has_altitude_)
        ss << " altitude " << altitude_;
    else
        ss << " no altitude";

    //ss << " json '" << db_content_.dump() << "'";

    loginf << "DBODataSource: print: "  << ss.str();
}

//nlohmann::json DBODataSource::dbContent() const
//{
//    return db_content_;
//}

//void DBODataSource::dbContent(const nlohmann::json& db_content)
//{
//    db_content_ = db_content;
//}
