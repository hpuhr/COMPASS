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

#include "property.h"
#include "logger.h"

#include <boost/assign/list_of.hpp>

#include <limits>

const std::map<PropertyDataType, std::string>& Property::dataTypes2Strings()
{
    static const auto* map = new std::map<PropertyDataType, std::string>
    {{PropertyDataType::BOOL, "BOOL"},
        {PropertyDataType::CHAR, "CHAR"},
        {PropertyDataType::UCHAR, "UCHAR"},
        {PropertyDataType::INT, "INT"},
        {PropertyDataType::UINT, "UINT"},
        {PropertyDataType::LONGINT, "LONGINT"},
        {PropertyDataType::ULONGINT, "ULONGINT"},
        {PropertyDataType::FLOAT, "FLOAT"},
        {PropertyDataType::DOUBLE, "DOUBLE"},
        {PropertyDataType::STRING, "STRING"},
        {PropertyDataType::JSON, "JSON"},
        {PropertyDataType::TIMESTAMP, "TIMESTAMP"}};
    return *map;
}

const std::map<PropertyDataType, std::string>& Property::dbDataTypes2Strings()
{
    static const auto* map = new std::map<PropertyDataType, std::string>
    {{PropertyDataType::BOOL, "TINYINT"},
        {PropertyDataType::CHAR, "TINYINT"},
        {PropertyDataType::UCHAR, "TINYINT"},
        {PropertyDataType::INT, "INT"},
        {PropertyDataType::UINT, "INT"},
        {PropertyDataType::LONGINT, "BIGINT"},
        {PropertyDataType::ULONGINT, "BIGINT"},
        {PropertyDataType::FLOAT, "FLOAT"},
        {PropertyDataType::DOUBLE, "DOUBLE"},
        {PropertyDataType::STRING, "TEXT"},
        {PropertyDataType::JSON, "TEXT"},
        {PropertyDataType::TIMESTAMP, "BIGINT"}};
    return *map;
}

const std::map<std::string, PropertyDataType>& Property::strings2DataTypes()
{
    static const auto* map = new std::map<std::string, PropertyDataType>
    {{"BOOL", PropertyDataType::BOOL},
        {"CHAR", PropertyDataType::CHAR},
        {"UCHAR", PropertyDataType::UCHAR},
        {"INT", PropertyDataType::INT},
        {"UINT", PropertyDataType::UINT},
        {"LONGINT", PropertyDataType::LONGINT},
        {"ULONGINT", PropertyDataType::ULONGINT},
        {"FLOAT", PropertyDataType::FLOAT},
        {"DOUBLE", PropertyDataType::DOUBLE},
        {"STRING", PropertyDataType::STRING},
        {"JSON", PropertyDataType::JSON},
        {"TIMESTAMP", PropertyDataType::TIMESTAMP}};
    return *map;
}

Property::Property(std::string id, PropertyDataType type) : data_type_(type), name_(id)
{
    data_type_str_ = asString(data_type_);
}

const std::string& Property::dbDataTypeString() const
{
    if (!dbDataTypes2Strings().count(data_type_))
    {
        std::cout << "Property: dbDataTypes2Strings: unkown type " << (unsigned int) data_type_ << std::endl;
        logerr << "Property: dbDataTypes2Strings: unkown type " << (unsigned int) data_type_;
    }

    assert(dbDataTypes2Strings().count(data_type_) > 0);
    return dbDataTypes2Strings().at(data_type_);
}

const std::string& Property::asString(PropertyDataType type)
{
    if (!dataTypes2Strings().count(type))
    {
        std::cout << "Property: asString: unkown type " << (unsigned int) type << std::endl;
        logerr << "Property: asString: unkown type " << (unsigned int) type;
    }

    assert(dataTypes2Strings().count(type) > 0);
    return dataTypes2Strings().at(type);
}

PropertyDataType Property::asDataType(const std::string& type)
{
    logdbg << "Property: asDataType: " << type;

    if (!strings2DataTypes().count(type))
    {
        std::cout << "Property: asDataType: unkown type " << type << std::endl;
        logerr << "Property: asDataType: unkown type " << type;
    }

    assert(strings2DataTypes().count(type) > 0);
    return strings2DataTypes().at(type);
}

