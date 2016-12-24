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
 * Property.h
 *
 *  Created on: Dec 7, 2011
 *      Author: sk
 */

#ifndef PROPERTY_H_
#define PROPERTY_H_

#include "Global.h"

enum class PropertyDataType { BOOL, CHAR, UCHAR, INT, UINT, LONGINT, ULONGINT,
    FLOAT, DOUBLE, STRING }; // P_TYPE_POINTER and SENTINEL removed

/**
 * @brief Base class for a data item identifier
 *
 * Contains a data type (based on PROPERTY_DATA_TYPE) and a string identifier. Also a size in bytes is stored,
 * which is based on the data type.
 *
 */
class Property
{
public:
  /// @brief Default constructor. Use only if members are overwritten.
  //Property();
  /// @brief Constructor
  Property(std::string id, PropertyDataType type);
  /// @brief Destructor
  virtual ~Property() {};

  PropertyDataType getDataType() const { return data_type_; };

  const std::string &asString() const { return Property::asString(data_type_); };

  std::string getId() const { return id_; };

  //unsigned int getSize() const;

  static const std::string &asString (PropertyDataType type)
  {
      assert (data_types_2_strings_.count(type) > 0);
      return data_types_2_strings_[type];
  };

  static PropertyDataType &asDataType (const std::string &type)
  {
      assert (strings_2_data_types_.count(type) > 0);
      return strings_2_data_types_[type];
  };


protected:
  /// Data type
  PropertyDataType data_type_;
  /// String identifier
  std::string id_;
  /// Size of data item in bytes
//  unsigned int size_;

  /// Mappings from PropertyDataType to strings, and back.
  static std::map<PropertyDataType,std::string> data_types_2_strings_;
  static std::map<std::string, PropertyDataType> strings_2_data_types_;
};

#endif /* PROPERTY_H_ */
