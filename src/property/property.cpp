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
 * Property.cpp
 *
 *  Created on: Nov 19, 2012
 *      Author: sk
 */

#include <limits>
#include <boost/assign/list_of.hpp>

#include "property.h"
#include "logger.h"

std::map<PropertyDataType,std::string> Property::data_types_2_strings_ = boost::assign::map_list_of
        (PropertyDataType::BOOL,       "BOOL")
        (PropertyDataType::CHAR,       "CHAR")
        (PropertyDataType::UCHAR,      "UCHAR")
        (PropertyDataType::INT,        "INT")
        (PropertyDataType::UCHAR,      "UINT")
        (PropertyDataType::LONGINT,    "LONGINT")
        (PropertyDataType::ULONGINT,   "ULONGINT")
        (PropertyDataType::FLOAT,      "FLOAT")
        (PropertyDataType::DOUBLE,     "DOUBLE")
        (PropertyDataType::STRING,     "STRING");

std::map<std::string, PropertyDataType> Property::strings_2_data_types_ = boost::assign::map_list_of
        ("BOOL", PropertyDataType::BOOL)
        ("CHAR", PropertyDataType::CHAR)
        ("UCHAR", PropertyDataType::UCHAR)
        ("INT", PropertyDataType::INT)
        ("UINT", PropertyDataType::UCHAR)
        ("LONGINT", PropertyDataType::LONGINT)
        ("ULONGINT", PropertyDataType::ULONGINT)
        ("FLOAT", PropertyDataType::FLOAT)
        ("DOUBLE", PropertyDataType::DOUBLE)
        ("STRING", PropertyDataType::STRING);

Property::Property(std::string id, PropertyDataType type)
    : data_type_(type), name_(id)
{
    //size_ = MemoryManager::getInstance().getBaseSizesInBytes(data_type_int_);
}

const std::string &Property::asString (PropertyDataType type)
{
    assert (data_types_2_strings_.count(type) > 0);
    return data_types_2_strings_.at(type);
}

PropertyDataType &Property::asDataType (const std::string &type)
{
    logdbg << "Property: asDataType: " << type;
    assert (strings_2_data_types_.count(type) > 0);
    return strings_2_data_types_.at(type);
}


//PROPERTY_DATA_TYPE Property::getDataType() const
//{
//    return (PROPERTY_DATA_TYPE) data_type_int_;
//}

//unsigned int Property::getDataTypeInt() const
//{
//    return data_type_int_;
//}

//std::string &Property::getDataTypeStr() const
//{
//    assert (data_type_int_ > 0 && data_type_int_ < P_TYPE_SENTINEL);
//    return data_type_strings[(PROPERTY_DATA_TYPE) data_type_int_];
//}

//std::string Property::getID() const
//{
//    return id_;
//}

//unsigned int Property::getSize() const
//{
//    return size_;
//}

//bool Property::isNan (void *ptr)
//{
//  switch (data_type_int_)
//  {
//  case P_TYPE_BOOL:
//  {
//    return false;
//  }
//  break;
//  case P_TYPE_UCHAR:
//  {
//    return false;
//  }
//  break;
//  case P_TYPE_CHAR:
//  {
//    return false;
//  }
//  break;
//  case P_TYPE_INT:
//  {
//    return *((int*)ptr) == std::numeric_limits<int>::max();
//  }
//  break;
//  case P_TYPE_UINT:
//  {
//    return *((unsigned int*)ptr) == std::numeric_limits<unsigned int>::max();
//  }
//  break;
//  case P_TYPE_STRING:
//  {
//    return false;
//  }
//  break;
//  case P_TYPE_FLOAT:
//  {
//    return *((float*)ptr) != *((float*)ptr);    //TODO: Too compiler/platform dependent!
//  }
//  break;
//  case P_TYPE_DOUBLE:
//  {
//    return *((float*)ptr) != *((float*)ptr);    //TODO: Too compiler/platform dependent!
//  }
//  break;
//  case P_TYPE_POINTER:
//  {
//    return *((void**)ptr) != NULL;
//  }
//  break;
//  default:
//    logerr  <<  "Property: isNan: unknown property type";
//    throw std::runtime_error ("Property: isNan: unknown property type");
//    return false;
//  }
//  return isNan ((PROPERTY_DATA_TYPE) data_type_int_, ptr);
//}


//void Property::setNan (void *ptr)
//{
//  switch (data_type_int_)
//  {
//  case P_TYPE_BOOL:
//  {
//  }
//  break;
//  case P_TYPE_UCHAR:
//  {
//  }
//  break;
//  case P_TYPE_CHAR:
//  {
//  }
//  break;
//  case P_TYPE_INT:
//  {
//    *((int*)ptr) = std::numeric_limits<int>::max();
//  }
//  break;
//  case P_TYPE_UINT:
//  {
//    *((unsigned int*)ptr) = std::numeric_limits<unsigned int>::max();
//  }
//  break;
//  case P_TYPE_STRING:
//  {
//  }
//  break;
//  case P_TYPE_FLOAT:
//  {
//    *((float*)ptr) = std::numeric_limits<float>::quiet_NaN();
//  }
//  break;
//  case P_TYPE_DOUBLE:
//  {
//    *((double*)ptr) = std::numeric_limits<double>::quiet_NaN();
//  }
//  break;
//  case P_TYPE_POINTER:
//  {
//    *((void**)ptr) = NULL;
//  }
//  break;
//  default:
//    logerr  <<  "Property: setNan: unknown property type";
//    throw std::runtime_error ("Property: setNan: unknown property type");
//  }
//  setNan ((PROPERTY_DATA_TYPE) data_type_int_, ptr);
//}

//void Property::setValue (void *source, void *target, bool null)
//{
//  switch (data_type_int_)
//  {
//  case P_TYPE_BOOL:
//  {
//    if (null)
//      *(bool *)target = false;
//    else
//      *(bool *)target = *(bool *)source;
//  }
//  break;
//  case P_TYPE_UCHAR:
//  {
//    if (null)
//      *(unsigned char *)target = 0;
//    else
//      *(unsigned char *)target = *(unsigned *)source;
//  }
//  break;
//  case P_TYPE_CHAR:
//  {
//    if (null)
//      *(char *)target = 0;
//    else
//      *(char *)target = *(char *)source;
//  }
//  break;
//  case P_TYPE_INT:
//  {
//    if (null)
//      *(int *)target = std::numeric_limits<int>::max();
//    else
//      *(int *)target = *(int *)source;
//  }
//  break;
//  case P_TYPE_UINT:
//  {
//    if (null)
//      *(unsigned int *)target = std::numeric_limits<unsigned int>::max();
//    else
//      *(unsigned int *)target = *(unsigned int *)source;
//  }
//  break;
//  case P_TYPE_STRING:
//  {
//    if (null)
//      *(std::string *)target = std::string();
//    else
//      *(std::string *)target = *(std::string *)source;
//  }
//  break;
//  case P_TYPE_FLOAT:
//  {
//    if (null)
//      *(float *)target = std::numeric_limits<float>::quiet_NaN();
//    else
//      *(float *)target = *(float *)source;
//  }
//  break;
//  case P_TYPE_DOUBLE:
//  {
//    if (null)
//      *(double *)target = std::numeric_limits<double>::quiet_NaN();
//    else
//      *(double *)target = *(double *)source;
//  }
//  break;
//  default:
//    logerr  <<  "Property: setValue: unknown property type";
//    throw std::runtime_error ("Property: setValue: unknown property type");
//  }
//}
