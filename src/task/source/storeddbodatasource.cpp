/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "storeddbodatasource.h"

#include "dbcontent/dbcontent.h"
#include "dbodatasource.h"
#include "json.hpp"
#include "managedatasourcestask.h"

using namespace nlohmann;

StoredDBODataSource::StoredDBODataSource(const std::string& class_id,
                                         const std::string& instance_id,
                                         ManageDataSourcesTask& task)
    : Configurable(class_id, instance_id, &task)
{
    registerParameter("dbcontent_name", &dbcontent_name_, "");
    registerParameter("id", &id_, 0);
    registerParameter("name", &name_, "");
    registerParameter("has_short_name", &has_short_name_, false);
    registerParameter("short_name", &short_name_, "");
    registerParameter("has_sac", &has_sac_, false);
    registerParameter("sac", &sac_, 0);
    registerParameter("has_sic", &has_sic_, false);
    registerParameter("sic", &sic_, 0);

    registerParameter("has_latitude", &has_latitude_, false);
    registerParameter("latitude", &latitude_, 0);
    registerParameter("has_longitude", &has_longitude_, false);
    registerParameter("longitude", &longitude_, 0);
    registerParameter("has_altitude", &has_altitude_, false);
    registerParameter("altitude", &altitude_, 0);

    // psr
    registerParameter("has_primary_azimuth_stddev", &has_primary_azimuth_stddev_, false);
    registerParameter("primary_azimuth_stddev", &primary_azimuth_stddev_, 0);
    registerParameter("has_primary_range_stddev", &has_primary_range_stddev_, false);
    registerParameter("primary_range_stddev", &primary_range_stddev_, 0);

    registerParameter("has_primary_ir_min", &has_primary_ir_min_, false);
    registerParameter("primary_ir_min", &primary_ir_min_, 0);
    registerParameter("has_primary_ir_max", &has_primary_ir_max_, false);
    registerParameter("primary_ir_max", &primary_ir_max_, 0);

    // ssr
    registerParameter("has_secondary_azimuth_stddev", &has_secondary_azimuth_stddev_, false);
    registerParameter("secondary_azimuth_stddev", &secondary_azimuth_stddev_, 0);
    registerParameter("has_secondary_range_stddev", &has_secondary_range_stddev_, false);
    registerParameter("secondary_range_stddev", &secondary_range_stddev_, 0);

    registerParameter("has_secondary_ir_min", &has_secondary_ir_min_, false);
    registerParameter("secondary_ir_min", &secondary_ir_min_, 0);
    registerParameter("has_secondary_ir_max", &has_secondary_ir_max_, false);
    registerParameter("secondary_ir_max", &secondary_ir_max_, 0);

    // mode s
    registerParameter("has_mode_s_azimuth_stddev", &has_mode_s_azimuth_stddev_, false);
    registerParameter("mode_s_azimuth_stddev", &mode_s_azimuth_stddev_, 0);
    registerParameter("has_mode_s_range_stddev", &has_mode_s_range_stddev_, false);
    registerParameter("mode_s_range_stddev", &mode_s_range_stddev_, 0);

    registerParameter("has_mode_s_ir_min", &has_mode_s_ir_min_, false);
    registerParameter("mode_s_ir_min", &mode_s_ir_min_, 0);
    registerParameter("has_mode_s_ir_max", &has_mode_s_ir_max_, false);
    registerParameter("mode_s_ir_max", &mode_s_ir_max_, 0);
}

StoredDBODataSource::~StoredDBODataSource() {}

StoredDBODataSource& StoredDBODataSource::operator=(DBODataSource& other)
{
    // id_ = other.id(); not copied, keep own

    dbcontent_name_ = other.dbContentName();
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

    // psr
    has_primary_azimuth_stddev_ = other.hasPrimaryAzimuthStdDev();
    if (has_primary_azimuth_stddev_)
        primary_azimuth_stddev_ = other.primaryAzimuthStdDev();

    has_primary_range_stddev_ = other.hasPrimaryRangeStdDev();
    if (has_primary_range_stddev_)
        primary_range_stddev_ = other.primaryRangeStdDev();

    has_primary_ir_min_ = other.hasPrimaryRangeMin();
    if (has_primary_ir_min_)
        primary_ir_min_ = other.primaryRangeMin();

    has_primary_ir_max_ = other.hasPrimaryRangeMax();
    if (has_primary_ir_max_)
        primary_ir_max_ = other.primaryRangeMax();

    // ssr
    has_secondary_azimuth_stddev_ = other.hasSecondaryAzimuthStdDev();
    if (has_secondary_azimuth_stddev_)
        secondary_azimuth_stddev_ = other.secondaryAzimuthStdDev();

    has_secondary_range_stddev_ = other.hasSecondaryRangeStdDev();
    if (has_secondary_range_stddev_)
        secondary_range_stddev_ = other.secondaryRangeStdDev();

    has_secondary_ir_min_ = other.hasSecondaryRangeMin();
    if (has_secondary_ir_min_)
        secondary_ir_min_ = other.secondaryRangeMin();

    has_secondary_ir_max_ = other.hasSecondaryRangeMax();
    if (has_secondary_ir_max_)
        secondary_ir_max_ = other.secondaryRangeMax();

    // mode s
    has_mode_s_azimuth_stddev_ = other.hasModeSAzimuthStdDev();
    if (has_mode_s_azimuth_stddev_)
        mode_s_azimuth_stddev_ = other.modeSAzimuthStdDev();

    has_mode_s_range_stddev_ = other.hasModeSRangeStdDev();
    if (has_mode_s_range_stddev_)
        mode_s_range_stddev_ = other.modeSRangeStdDev();

    has_mode_s_ir_min_ = other.hasModeSRangeMin();
    if (has_mode_s_ir_min_)
        mode_s_ir_min_ = other.modeSRangeMin();

    has_mode_s_ir_max_ = other.hasModeSRangeMax();
    if (has_mode_s_ir_max_)
        mode_s_ir_max_ = other.modeSRangeMax();

    return *this;
}

StoredDBODataSource& StoredDBODataSource::operator=(StoredDBODataSource&& other)
{
    loginf << "StoredDBODataSource: move operator: moving";

    //    object_ = other.object_;
    //    other.object_ = nullptr;

    dbcontent_name_ = other.dbcontent_name_;

    // id_ = other.id_; do not use others id, keep your own

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

    // psr
    has_primary_azimuth_stddev_ = other.has_primary_azimuth_stddev_;
    primary_azimuth_stddev_ = other.primary_azimuth_stddev_;
    has_primary_range_stddev_ = other.has_primary_range_stddev_;
    primary_range_stddev_ = other.primary_range_stddev_;

    has_primary_ir_min_ = other.has_primary_ir_min_;
    primary_ir_min_= other.primary_ir_min_;
    has_primary_ir_max_ = other.has_primary_ir_max_;
    primary_ir_max_ = other.primary_ir_max_;

    // ssr
    has_secondary_azimuth_stddev_ = other.has_secondary_azimuth_stddev_;
    secondary_azimuth_stddev_ = other.secondary_azimuth_stddev_;
    has_secondary_range_stddev_ = other.has_secondary_range_stddev_;
    secondary_range_stddev_ = other.secondary_range_stddev_;

    has_secondary_ir_min_ = other.has_secondary_ir_min_;
    secondary_ir_min_= other.secondary_ir_min_;
    has_secondary_ir_max_ = other.has_secondary_ir_max_;
    secondary_ir_max_ = other.secondary_ir_max_;

    // mode s
    has_mode_s_azimuth_stddev_ = other.has_mode_s_azimuth_stddev_;
    mode_s_azimuth_stddev_ = other.mode_s_azimuth_stddev_;
    has_mode_s_range_stddev_ = other.has_mode_s_range_stddev_;
    mode_s_range_stddev_ = other.mode_s_range_stddev_;

    has_mode_s_ir_min_ = other.has_mode_s_ir_min_;
    mode_s_ir_min_= other.mode_s_ir_min_;
    has_mode_s_ir_max_ = other.has_mode_s_ir_max_;
    mode_s_ir_max_ = other.mode_s_ir_max_;

    other.configuration().updateParameterPointer("dbcontent_name", &dbcontent_name_);
    other.configuration().updateParameterPointer("id", &id_);
    other.configuration().updateParameterPointer("name", &name_);
    other.configuration().updateParameterPointer("has_short_name", &has_short_name_);
    other.configuration().updateParameterPointer("short_name", &short_name_);
    other.configuration().updateParameterPointer("has_sac", &has_sac_);
    other.configuration().updateParameterPointer("sac", &sac_);
    other.configuration().updateParameterPointer("has_sic", &has_sic_);
    other.configuration().updateParameterPointer("sic", &sic_);
    other.configuration().updateParameterPointer("has_latitude", &has_latitude_);
    other.configuration().updateParameterPointer("latitude", &latitude_);
    other.configuration().updateParameterPointer("has_longitude", &has_longitude_);
    other.configuration().updateParameterPointer("longitude", &longitude_);
    other.configuration().updateParameterPointer("has_altitude", &has_altitude_);
    other.configuration().updateParameterPointer("altitude", &altitude_);

    // psr
    other.configuration().updateParameterPointer("has_primary_azimuth_stddev", &has_primary_azimuth_stddev_);
    other.configuration().updateParameterPointer("primary_azimuth_stddev", &primary_azimuth_stddev_);
    other.configuration().updateParameterPointer("has_primary_range_stddev", &has_primary_range_stddev_);
    other.configuration().updateParameterPointer("primary_range_stddev", &primary_range_stddev_);

    other.configuration().updateParameterPointer("has_primary_ir_min", &has_primary_ir_min_);
    other.configuration().updateParameterPointer("primary_ir_min", &primary_ir_min_);
    other.configuration().updateParameterPointer("has_primary_ir_max", &has_primary_ir_max_);
    other.configuration().updateParameterPointer("primary_ir_max", &primary_ir_max_);

    // ssr
    other.configuration().updateParameterPointer("has_secondary_azimuth_stddev", &has_secondary_azimuth_stddev_);
    other.configuration().updateParameterPointer("secondary_azimuth_stddev", &secondary_azimuth_stddev_);
    other.configuration().updateParameterPointer("has_secondary_range_stddev", &has_secondary_range_stddev_);
    other.configuration().updateParameterPointer("secondary_range_stddev", &secondary_range_stddev_);

    other.configuration().updateParameterPointer("has_secondary_ir_min", &has_secondary_ir_min_);
    other.configuration().updateParameterPointer("secondary_ir_min", &secondary_ir_min_);
    other.configuration().updateParameterPointer("has_secondary_ir_max", &has_secondary_ir_max_);
    other.configuration().updateParameterPointer("secondary_ir_max", &secondary_ir_max_);

    // mode s
    other.configuration().updateParameterPointer("has_mode_s_azimuth_stddev", &has_mode_s_azimuth_stddev_);
    other.configuration().updateParameterPointer("mode_s_azimuth_stddev", &mode_s_azimuth_stddev_);
    other.configuration().updateParameterPointer("has_mode_s_range_stddev", &has_mode_s_range_stddev_);
    other.configuration().updateParameterPointer("mode_s_range_stddev", &mode_s_range_stddev_);

    other.configuration().updateParameterPointer("has_mode_s_ir_min", &has_mode_s_ir_min_);
    other.configuration().updateParameterPointer("mode_s_ir_min", &mode_s_ir_min_);
    other.configuration().updateParameterPointer("has_mode_s_ir_max", &has_mode_s_ir_max_);
    other.configuration().updateParameterPointer("mode_s_ir_max", &mode_s_ir_max_);

    return static_cast<StoredDBODataSource&>(Configurable::operator=(std::move(other)));
}

bool StoredDBODataSource::operator==(const DBODataSource& other) const
{
    logdbg << "StoredDBODataSource: operator==: name " << (name_ == other.name()) << " short "
           << (short_name_ == other.shortName()) << " sac " << (sac_ == other.sac()) << " sic "
           << (sic_ == other.sic()) << " lat " << (fabs(latitude_ - other.latitude()) < 1e-10)
           << " long " << (fabs(longitude_ - other.longitude()) < 1e-10) << " alt "
           << (fabs(altitude_ - other.altitude()) < 1e-10);

    return (dbcontent_name_ == other.dbContentName()) && (name_ == other.name())
            &&(has_short_name_ == other.hasShortName())
            && (has_short_name_ ? short_name_ == other.shortName() : true)
            && (has_sac_ == other.hasSac()) && (has_sac_ ? sac_ == other.sac() : true)
            && (has_sic_ == other.hasSic()) && (has_sic_ ? sic_ == other.sic() : true)
            && (has_latitude_ == other.hasLatitude())
            && (has_latitude_ ? fabs(latitude_ - other.latitude()) < 1e-10 : true)
            && (has_longitude_ == other.hasLongitude())
            && (has_longitude_ ? fabs(longitude_ - other.longitude()) < 1e-10 : true)
            && (has_altitude_ == other.hasAltitude())
            && (has_altitude_ ? fabs(altitude_ - other.altitude()) < 1e-10 : true)

            // psr
            && (has_primary_azimuth_stddev_ == other.hasPrimaryAzimuthStdDev())
            && (has_primary_azimuth_stddev_ ? fabs(primary_azimuth_stddev_ - other.primaryAzimuthStdDev()) < 1e-10 : true)

            && (has_primary_range_stddev_ == other.hasPrimaryRangeStdDev())
            && (has_primary_range_stddev_ ? fabs(primary_range_stddev_ - other.primaryRangeStdDev()) < 1e-10 : true)

            && (has_primary_ir_min_ == other.hasPrimaryRangeMin())
            && (has_primary_ir_min_ ? fabs(primary_ir_min_ - other.primaryRangeMin()) == 0 : true)

            && (has_primary_ir_max_ == other.hasPrimaryRangeMax())
            && (has_primary_ir_max_ ? fabs(primary_ir_max_ - other.primaryRangeMax()) == 0 : true)

            // ssr
            && (has_secondary_azimuth_stddev_ == other.hasSecondaryAzimuthStdDev())
            && (has_secondary_azimuth_stddev_ ? fabs(secondary_azimuth_stddev_ - other.secondaryAzimuthStdDev()) < 1e-10 : true)

            && (has_secondary_range_stddev_ == other.hasSecondaryRangeStdDev())
            && (has_secondary_range_stddev_ ? fabs(secondary_range_stddev_ - other.secondaryRangeStdDev()) < 1e-10 : true)

            && (has_secondary_ir_min_ == other.hasSecondaryRangeMin())
            && (has_secondary_ir_min_ ? fabs(secondary_ir_min_ - other.secondaryRangeMin()) == 0 : true)

            && (has_secondary_ir_max_ == other.hasSecondaryRangeMax())
            && (has_secondary_ir_max_ ? fabs(secondary_ir_max_ - other.secondaryRangeMax()) == 0 : true)

            // mode s
            && (has_mode_s_azimuth_stddev_ == other.hasModeSAzimuthStdDev())
            && (has_mode_s_azimuth_stddev_ ? fabs(mode_s_azimuth_stddev_ - other.modeSAzimuthStdDev()) < 1e-10 : true)

            && (has_mode_s_range_stddev_ == other.hasModeSRangeStdDev())
            && (has_mode_s_range_stddev_ ? fabs(mode_s_range_stddev_ - other.modeSRangeStdDev()) < 1e-10 : true)

            && (has_mode_s_ir_min_ == other.hasModeSRangeMin())
            && (has_mode_s_ir_min_ ? fabs(mode_s_ir_min_ - other.modeSRangeMin()) == 0 : true)

            && (has_mode_s_ir_max_ == other.hasModeSRangeMax())
            && (has_mode_s_ir_max_ ? fabs(mode_s_ir_max_ - other.modeSRangeMax()) == 0 : true);
}

double StoredDBODataSource::altitude() const
{
    assert(has_altitude_);
    return altitude_;
}

// psr azm
bool StoredDBODataSource::hasPrimaryAzimuthStdDev() const
{
    return has_primary_azimuth_stddev_;
}

void StoredDBODataSource::removePrimaryAzimuthStdDev()
{
    has_primary_azimuth_stddev_ = false;
    primary_azimuth_stddev_ = 0;
}

void StoredDBODataSource::primaryAzimuthStdDev(double value)
{
    has_primary_azimuth_stddev_ = true;
    primary_azimuth_stddev_ = value;
}

double StoredDBODataSource::primaryAzimuthStdDev() const
{
    return primary_azimuth_stddev_;
}

// psr range
bool StoredDBODataSource::hasPrimaryRangeStdDev() const
{
    return has_primary_range_stddev_;
}

void StoredDBODataSource::removePrimaryRangeStdDev()
{
    has_primary_range_stddev_ = false;
    primary_range_stddev_ = 0;
}

void StoredDBODataSource::primaryRangeStdDev(double value)
{
    has_primary_range_stddev_ = true;
    primary_range_stddev_ = value;
}

double StoredDBODataSource::primaryRangeStdDev() const
{
    return primary_range_stddev_;
}

// psr min range

bool StoredDBODataSource::hasPrimaryRangeMin() const
{
    return has_primary_ir_min_;
}
void StoredDBODataSource::removePrimaryRangeMin()
{
    has_primary_ir_min_ = false;
    primary_ir_min_ = 0;
}
void StoredDBODataSource::primaryRangeMin(int value)
{
    has_primary_ir_min_ = true;
    primary_ir_min_ = value;
}
int StoredDBODataSource::primaryRangeMin() const
{
    assert (has_primary_ir_min_);
    return primary_ir_min_;
}

// psr max range

bool StoredDBODataSource::hasPrimaryRangeMax() const
{
    return has_primary_ir_max_;
}
void StoredDBODataSource::removePrimaryRangeMax()
{
    has_primary_ir_max_ = false;
    primary_ir_max_ = 0;
}
void StoredDBODataSource::primaryRangeMax(int value)
{
    has_primary_ir_max_ = true;
    primary_ir_max_ = value;
}
int StoredDBODataSource::primaryRangeMax() const
{
    assert (has_primary_ir_max_);
    return primary_ir_max_;
}

// ssr azm
bool StoredDBODataSource::hasSecondaryAzimuthStdDev() const
{
    return has_secondary_azimuth_stddev_;
}

void StoredDBODataSource::removeSecondaryAzimuthStdDev()
{
    has_secondary_azimuth_stddev_ = false;
    secondary_azimuth_stddev_ = 0;
}

void StoredDBODataSource::secondaryAzimuthStdDev(double value)
{
    has_secondary_azimuth_stddev_ = true;
    secondary_azimuth_stddev_ = value;
}

double StoredDBODataSource::secondaryAzimuthStdDev() const
{
    return secondary_azimuth_stddev_;
}

// ssr range
bool StoredDBODataSource::hasSecondaryRangeStdDev() const
{
    return has_secondary_range_stddev_;
}

void StoredDBODataSource::removeSecondaryRangeStdDev()
{
    has_secondary_range_stddev_ = false;
    secondary_range_stddev_ = 0;
}

void StoredDBODataSource::secondaryRangeStdDev(double value)
{
    has_secondary_range_stddev_ = true;
    secondary_range_stddev_ = value;
}

double StoredDBODataSource::secondaryRangeStdDev() const
{
    return secondary_range_stddev_;
}

// ssr min range

bool StoredDBODataSource::hasSecondaryRangeMin() const
{
    return has_secondary_ir_min_;
}
void StoredDBODataSource::removeSecondaryRangeMin()
{
    has_secondary_ir_min_ = false;
    secondary_ir_min_ = 0;
}
void StoredDBODataSource::secondaryRangeMin(int value)
{
    has_secondary_ir_min_ = true;
    secondary_ir_min_ = value;
}
int StoredDBODataSource::secondaryRangeMin() const
{
    assert (has_secondary_ir_min_);
    return secondary_ir_min_;
}

// ssr max range

bool StoredDBODataSource::hasSecondaryRangeMax() const
{
    return has_secondary_ir_max_;
}
void StoredDBODataSource::removeSecondaryRangeMax()
{
    has_secondary_ir_max_ = false;
    secondary_ir_max_ = 0;
}
void StoredDBODataSource::secondaryRangeMax(int value)
{
    has_secondary_ir_max_ = true;
    secondary_ir_max_ = value;
}
int StoredDBODataSource::secondaryRangeMax() const
{
    assert (has_secondary_ir_max_);
    return secondary_ir_max_;
}

// mode s azm
bool StoredDBODataSource::hasModeSAzimuthStdDev() const
{
    return has_mode_s_azimuth_stddev_;
}

void StoredDBODataSource::removeModeSAzimuthStdDev()
{
    has_mode_s_azimuth_stddev_ = false;
    mode_s_azimuth_stddev_ = 0;
}

void StoredDBODataSource::modeSAzimuthStdDev(double value)
{
    has_mode_s_azimuth_stddev_ = true;
    mode_s_azimuth_stddev_ = value;
}

double StoredDBODataSource::modeSAzimuthStdDev() const
{
    return mode_s_azimuth_stddev_;
}

// mode s range
bool StoredDBODataSource::hasModeSRangeStdDev() const
{
    return has_mode_s_range_stddev_;
}

void StoredDBODataSource::removeModeSRangeStdDev()
{
    has_mode_s_range_stddev_ = false;
    mode_s_range_stddev_ = 0;
}

void StoredDBODataSource::modeSRangeStdDev(double value)
{
    has_mode_s_range_stddev_ = true;
    mode_s_range_stddev_ = value;
}

double StoredDBODataSource::modeSRangeStdDev() const
{
    return mode_s_range_stddev_;
}

// mode s min range

bool StoredDBODataSource::hasModeSRangeMin() const
{
    return has_mode_s_ir_min_;
}
void StoredDBODataSource::removeModeSRangeMin()
{
    has_mode_s_ir_min_ = false;
    mode_s_ir_min_ = 0;
}
void StoredDBODataSource::modeSRangeMin(int value)
{
    has_mode_s_ir_min_ = true;
    mode_s_ir_min_ = value;
}
int StoredDBODataSource::modeSRangeMin() const
{
    assert (has_mode_s_ir_min_);
    return mode_s_ir_min_;
}

// mode s max range

bool StoredDBODataSource::hasModeSRangeMax() const
{
    return has_mode_s_ir_max_;
}
void StoredDBODataSource::removeModeSRangeMax()
{
    has_mode_s_ir_max_ = false;
    mode_s_ir_max_ = 0;
}
void StoredDBODataSource::modeSRangeMax(int value)
{
    has_mode_s_ir_max_ = true;
    mode_s_ir_max_ = value;
}
int StoredDBODataSource::modeSRangeMax() const
{
    assert (has_mode_s_ir_max_);
    return mode_s_ir_max_;
}

std::string StoredDBODataSource::dbContentName() const { return dbcontent_name_; }

unsigned int StoredDBODataSource::id() const { return id_; }

double StoredDBODataSource::latitude() const
{
    assert(hasLatitude());
    return latitude_;
}

double StoredDBODataSource::longitude() const
{
    assert(hasLongitude());
    return longitude_;
}

const std::string& StoredDBODataSource::name() const { return name_; }

unsigned char StoredDBODataSource::sac() const
{
    assert(has_sac_);
    return sac_;
}

const std::string& StoredDBODataSource::shortName() const
{
    assert(has_short_name_);
    return short_name_;
}

unsigned char StoredDBODataSource::sic() const
{
    assert(has_sic_);
    return sic_;
}

void StoredDBODataSource::name(const std::string& name)
{
    loginf << "StoredDBODataSource " << id_ << ": name: " << name;
    name_ = name;
}

void StoredDBODataSource::shortName(const std::string& short_name)
{
    loginf << "StoredDBODataSource " << id_ << ": shortName: " << short_name;
    has_short_name_ = true;
    this->short_name_ = short_name;
}

void StoredDBODataSource::altitude(double altitude)
{
    loginf << "StoredDBODataSource " << id_ << ": altitude: " << altitude;
    has_altitude_ = true;
    this->altitude_ = altitude;
}

void StoredDBODataSource::latitude(double latitude)
{
    loginf << "StoredDBODataSource " << id_ << ": latitude: " << latitude;
    has_latitude_ = true;
    this->latitude_ = latitude;
}

void StoredDBODataSource::longitude(double longitude)
{
    loginf << "StoredDBODataSource " << id_ << ": longitude: " << longitude;
    has_longitude_ = true;
    this->longitude_ = longitude;
}

void StoredDBODataSource::sac(unsigned char sac)
{
    loginf << "StoredDBODataSource " << id_ << ": sac: " << static_cast<int>(sac);
    has_sac_ = true;
    this->sac_ = sac;
}

void StoredDBODataSource::sic(unsigned char sic)
{
    loginf << "StoredDBODataSource " << id_ << ": sic: " << static_cast<int>(sic);
    has_sic_ = true;
    this->sic_ = sic;
}

bool StoredDBODataSource::hasSac() const { return has_sac_; }

bool StoredDBODataSource::hasSic() const { return has_sic_; }

bool StoredDBODataSource::hasLatitude() const { return has_latitude_; }

bool StoredDBODataSource::hasLongitude() const { return has_longitude_; }

bool StoredDBODataSource::hasAltitude() const { return has_altitude_; }

bool StoredDBODataSource::hasShortName() const { return has_short_name_; }

void StoredDBODataSource::removeShortName()
{
    has_short_name_ = false;
    short_name_ = "";
}

void StoredDBODataSource::removeSac()
{
    has_sac_ = false;
    sac_ = 0;
}

void StoredDBODataSource::removeSic()
{
    has_sic_ = false;
    sic_ = 0;
}

void StoredDBODataSource::removeLatitude()
{
    has_latitude_ = false;
    latitude_ = 0;
}

void StoredDBODataSource::removeLongitude()
{
    has_longitude_ = false;
    longitude_ = 0;
}

void StoredDBODataSource::removeAltitude()
{
    has_altitude_ = false;
    altitude_ = 0;
}

json StoredDBODataSource::getAsJSON()
{
    json j;

    j["dbcontent_name"] = dbcontent_name_;
    j["name"] = name_;

    if (has_short_name_)
        j["short_name"] = short_name_;
    if (has_sac_)
        j["sac"] = sac_;
    if (has_sic_)
        j["sic"] = sic_;
    if (has_latitude_)
        j["latitude"] = latitude_;
    if (has_longitude_)
        j["longitude"] = longitude_;
    if (has_altitude_)
        j["altitude"] = altitude_;

    // psr
    if (has_primary_azimuth_stddev_)
        j["primary_azimuth_stddev"] = primary_azimuth_stddev_;
    if (has_primary_range_stddev_)
        j["primary_range_stddev"] = primary_range_stddev_;

    if (has_primary_ir_min_)
        j["primary_ir_min"] = primary_ir_min_;
    if (has_primary_ir_max_)
        j["primary_ir_max"] = primary_ir_max_;

    // ssr
    if (has_secondary_azimuth_stddev_)
        j["secondary_azimuth_stddev"] = secondary_azimuth_stddev_;
    if (has_secondary_range_stddev_)
        j["secondary_range_stddev"] = secondary_range_stddev_;

    if (has_secondary_ir_min_)
        j["secondary_ir_min"] = secondary_ir_min_;
    if (has_secondary_ir_max_)
        j["secondary_ir_max"] = secondary_ir_max_;

    // mode s
    if (has_mode_s_azimuth_stddev_)
        j["mode_s_azimuth_stddev"] = mode_s_azimuth_stddev_;
    if (has_mode_s_range_stddev_)
        j["mode_s_range_stddev"] = mode_s_range_stddev_;

    if (has_mode_s_ir_min_)
        j["mode_s_ir_min"] = mode_s_ir_min_;
    if (has_mode_s_ir_max_)
        j["mode_s_ir_max"] = mode_s_ir_max_;

    return j;
}

void StoredDBODataSource::setFromJSON(json& j)
{
    dbcontent_name_ = j.at("dbcontent_name");
    name_ = j.at("name");

    has_short_name_ = j.contains("short_name");
    if (has_short_name_)
        short_name_ = j.at("short_name");

    has_sac_ = j.contains("sac");
    if (has_sac_)
        sac_ = j.at("sac");

    has_sic_ = j.contains("sic");
    if (has_sic_)
        sic_ = j.at("sic");

    has_latitude_ = j.contains("latitude");
    if (has_latitude_)
        latitude_ = j.at("latitude");

    has_longitude_ = j.contains("longitude");
    if (has_longitude_)
        longitude_ = j.at("longitude");

    has_altitude_ = j.contains("altitude");
    if (has_altitude_)
        altitude_ = j.at("altitude");

    // psr
    has_primary_azimuth_stddev_ = j.contains("primary_azimuth_stddev");
    if (has_primary_azimuth_stddev_)
        primary_azimuth_stddev_ = j.at("primary_azimuth_stddev");

    has_primary_range_stddev_ = j.contains("primary_range_stddev");
    if (has_primary_range_stddev_)
        primary_range_stddev_ = j.at("primary_range_stddev");

    has_primary_ir_min_ = j.contains("primary_ir_min");
    if (has_primary_ir_min_)
        primary_ir_min_ = j.at("primary_ir_min");

    has_primary_ir_max_ = j.contains("primary_ir_max");
    if (has_primary_ir_max_)
        primary_ir_max_ = j.at("primary_ir_max");

    // ssr
    has_secondary_azimuth_stddev_ = j.contains("secondary_azimuth_stddev");
    if (has_secondary_azimuth_stddev_)
        secondary_azimuth_stddev_ = j.at("secondary_azimuth_stddev");

    has_secondary_range_stddev_ = j.contains("secondary_range_stddev");
    if (has_secondary_range_stddev_)
        secondary_range_stddev_ = j.at("secondary_range_stddev");

    has_secondary_ir_min_ = j.contains("secondary_ir_min");
    if (has_secondary_ir_min_)
        secondary_ir_min_ = j.at("secondary_ir_min");

    has_secondary_ir_max_ = j.contains("secondary_ir_max");
    if (has_secondary_ir_max_)
        secondary_ir_max_ = j.at("secondary_ir_max");

    // mode s
    has_mode_s_azimuth_stddev_ = j.contains("mode_s_azimuth_stddev");
    if (has_mode_s_azimuth_stddev_)
        mode_s_azimuth_stddev_ = j.at("mode_s_azimuth_stddev");

    has_mode_s_range_stddev_ = j.contains("mode_s_range_stddev");
    if (has_mode_s_range_stddev_)
        mode_s_range_stddev_ = j.at("mode_s_range_stddev");

    has_mode_s_ir_min_ = j.contains("mode_s_ir_min");
    if (has_mode_s_ir_min_)
        mode_s_ir_min_ = j.at("mode_s_ir_min");

    has_mode_s_ir_max_ = j.contains("mode_s_ir_max");
    if (has_mode_s_ir_max_)
        mode_s_ir_max_ = j.at("mode_s_ir_max");
}
