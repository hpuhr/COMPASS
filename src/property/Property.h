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

/**
 * @brief Base class for a data item identifier
 *
 * Contains a data type (based on PROPERTY_DATA_TYPE) and a string identifier. Also a size in bytes is stored,
 * which is based on the data type.
 *
 * Uses public members for fast access
 *
 * \todo Public members should be changed into getter and setter methods
 */
class Property
{
public:
  /// Data type (based on PROPERTY_DATA_TYPE)
  unsigned int data_type_int_;
  /// String identifier
  std::string id_;
  /// Size of data item in bytes
  unsigned int size_;

  /// @brief Default constructor. Use only if members are overwritten.
  Property();
  /// @brief Constructor
  Property(std::string id, PROPERTY_DATA_TYPE type);
  /// @brief Destructor
  virtual ~Property() {};
};

#endif /* PROPERTY_H_ */
