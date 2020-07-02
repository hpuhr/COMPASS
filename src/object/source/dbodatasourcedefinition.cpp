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

#include "dbodatasourcedefinition.h"

#include "dbobject.h"
#include "dbodatasourcedefinitionwidget.h"

DBODataSourceDefinition::DBODataSourceDefinition(const std::string& class_id,
                                                 const std::string& instance_id, DBObject* object)
    : Configurable(class_id, instance_id, object), object_(object)
{
    registerParameter("schema", &schema_, "");
    registerParameter("local_key", &local_key_, "");
    registerParameter("meta_table", &meta_table_, "");
    registerParameter("foreign_key", &foreign_key_, "");
    registerParameter("short_name_column", &short_name_column_, "");
    registerParameter("name_column", &name_column_, "");
    registerParameter("sac_column", &sac_column_, "");
    registerParameter("sic_column", &sic_column_, "");

    registerParameter("latitude_column", &latitude_column_, "");
    registerParameter("longitude_column", &longitude_column_, "");
    registerParameter("altitude_column", &altitude_column_, "");

    registerParameter("primary_azimuth_stddev_column", &primary_azimuth_stddev_column_, "");
    registerParameter("primary_range_stddev_column", &primary_range_stddev_column_, "");

    registerParameter("secondary_azimuth_stddev_column", &secondary_azimuth_stddev_column_, "");
    registerParameter("secondary_range_stddev_column", &secondary_range_stddev_column_, "");

    registerParameter("mode_s_azimuth_stddev_column", &mode_s_azimuth_stddev_column_, "");
    registerParameter("mode_s_range_stddev_column", &mode_s_range_stddev_column_, "");
}

DBODataSourceDefinition::~DBODataSourceDefinition()
{
    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }
}

DBODataSourceDefinitionWidget* DBODataSourceDefinition::widget()
{
    if (!widget_)
    {
        widget_ = new DBODataSourceDefinitionWidget(*object_, *this);
    }

    assert(widget_);
    return widget_;
}

std::string DBODataSourceDefinition::latitudeColumn() const { return latitude_column_; }

std::string DBODataSourceDefinition::longitudeColumn() const { return longitude_column_; }

void DBODataSourceDefinition::longitudeColumn(const std::string& longitude_column)
{
    loginf << "DBODataSourceDefinition: localKey: value " << longitude_column;
    longitude_column_ = longitude_column;
    emit definitionChangedSignal();
}

std::string DBODataSourceDefinition::shortNameColumn() const { return short_name_column_; }

void DBODataSourceDefinition::shortNameColumn(const std::string& short_name_column)
{
    loginf << "DBODataSourceDefinition: shortNameColumn: value " << short_name_column;
    short_name_column_ = short_name_column;
    emit definitionChangedSignal();
}

std::string DBODataSourceDefinition::sacColumn() const { return sac_column_; }

void DBODataSourceDefinition::sacColumn(const std::string& sac_column)
{
    loginf << "DBODataSourceDefinition: sacColumn: value " << sac_column;
    sac_column_ = sac_column;
    emit definitionChangedSignal();
}

std::string DBODataSourceDefinition::sicColumn() const { return sic_column_; }

void DBODataSourceDefinition::sicColumn(const std::string& sic_column)
{
    loginf << "DBODataSourceDefinition: sicColumn: value " << sic_column;
    sic_column_ = sic_column;
    emit definitionChangedSignal();
}

std::string DBODataSourceDefinition::altitudeColumn() const { return altitude_column_; }

void DBODataSourceDefinition::altitudeColumn(const std::string& altitude_column)
{
    loginf << "DBODataSourceDefinition: altitudeColumn: value " << altitude_column;
    altitude_column_ = altitude_column;
    emit definitionChangedSignal();
}

std::string DBODataSourceDefinition::primaryAzimuthStdDevColumn() const
{
    return primary_azimuth_stddev_column_;
}

void DBODataSourceDefinition::primaryAzimuthStdDevColumn(const std::string& value)
{
    loginf << "DBODataSourceDefinition: primaryAzimuthStdDevColumn: value " << value;
    primary_azimuth_stddev_column_ = value;
    emit definitionChangedSignal();
}

std::string DBODataSourceDefinition::primaryRangeStdDevColumn() const
{
    return primary_range_stddev_column_;
}

void DBODataSourceDefinition::primaryRangeStdDevColumn(const std::string& value)
{
    loginf << "DBODataSourceDefinition: primaryRangeStdDevColumn: value " << value;
    primary_range_stddev_column_ = value;
    emit definitionChangedSignal();
}

std::string DBODataSourceDefinition::secondaryAzimuthStdDevColumn() const
{
    return secondary_azimuth_stddev_column_;
}

void DBODataSourceDefinition::secondaryAzimuthStdDevColumn(const std::string& value)
{
    loginf << "DBODataSourceDefinition: secondaryAzimuthStdDevColumn: value " << value;
    secondary_azimuth_stddev_column_ = value;
    emit definitionChangedSignal();
}

std::string DBODataSourceDefinition::secondaryRangeStdDevColumn() const
{
    return secondary_range_stddev_column_;
}

void DBODataSourceDefinition::secondaryRangeStdDevColumn(const std::string& value)
{
    loginf << "DBODataSourceDefinition: secondaryRangeStdDevColumn: value " << value;
    secondary_range_stddev_column_ = value;
    emit definitionChangedSignal();
}

std::string DBODataSourceDefinition::modeSAzimuthStdDevColumn() const
{
    return mode_s_azimuth_stddev_column_;
}

void DBODataSourceDefinition::modeSAzimuthStdDevColumn(const std::string& value)
{
    loginf << "DBODataSourceDefinition: modeSAzimuthStdDevColumn: value " << value;
    mode_s_azimuth_stddev_column_ = value;
    emit definitionChangedSignal();
}

std::string DBODataSourceDefinition::modeSRangeStdDevColumn() const
{
    return mode_s_range_stddev_column_;
}

void DBODataSourceDefinition::modeSRangeStdDevColumn(const std::string& value)
{
    loginf << "DBODataSourceDefinition: modeSRangeStdDevColumn: value " << value;
    mode_s_range_stddev_column_ = value;
    emit definitionChangedSignal();
}

void DBODataSourceDefinition::latitudeColumn(const std::string& latitude_column)
{
    loginf << "DBODataSourceDefinition: latitudeColumn: value " << latitude_column;
    latitude_column_ = latitude_column;
    emit definitionChangedSignal();
}

void DBODataSourceDefinition::localKey(const std::string& local_key)
{
    loginf << "DBODataSourceDefinition: localKey: value " << local_key;
    local_key_ = local_key;
    emit definitionChangedSignal();
}

void DBODataSourceDefinition::metaTable(const std::string& meta_table)
{
    loginf << "DBODataSourceDefinition: metaTable: value " << meta_table;
    meta_table_ = meta_table;
    emit definitionChangedSignal();
}

void DBODataSourceDefinition::foreignKey(const std::string& foreign_key)
{
    loginf << "DBODataSourceDefinition: foreignKey: value " << foreign_key;
    foreign_key_ = foreign_key;
    emit definitionChangedSignal();
}

void DBODataSourceDefinition::nameColumn(const std::string& name_column)
{
    loginf << "DBODataSourceDefinition: nameColumn: value " << name_column;
    name_column_ = name_column;
    emit definitionChangedSignal();
}
