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
 * DataManipulation.h
 *
 *  Created on: Mar 3, 2014
 *      Author: sk
 */

#ifndef DATA_H_
#define DATA_H_

#include "Global.h"
#include "DBOVariableSet.h"

class Buffer;

namespace Utils
{

namespace Data
{

struct Point { double x_; double y_; };
struct Line { Point p1_; Point p2_; };

/// @brief Sets a NaN value
extern void setNan (unsigned int data_type, void *ptr);
/// @brief Checks if value is NaN
extern bool isNan (unsigned int data_type, void *ptr);
/// @brief Sets special null values to NaNs
extern void clear (unsigned int data_type, void *ptr);
extern void copy (void *src, void *dest, unsigned int data_type, unsigned int num_bytes, bool reverse, bool verbose);
extern void add (void *data, unsigned int data_type, int constant);
extern void memcpy_reverse (char *dest, char* src, unsigned int num_bytes, bool verbose);
extern void check_reverse (char *data, char* check_value, unsigned int num_bytes); // throws exception if not
extern void setSpecialNullsNan (Buffer *buffer, unsigned int column, unsigned int data_type, std::string special_null);
/// @brief Multiplies given data with a factor
extern void multiplyData (void *ptr, PropertyDataType data_type, double factor);
/// @brief Copies data from src to target
extern void copyPropertyData (void *src, void *target, PropertyDataType type);

extern void finalizeDBData (DB_OBJECT_TYPE type, Buffer *buffer, DBOVariableSet read_list);
}
}
#endif



