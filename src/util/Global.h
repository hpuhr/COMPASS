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

#ifndef DBGLOBAL_H_
#define DBGLOBAL_H_

#include <cassert>
#include <map>
#include <string>

/**
 * Property data type
 *
 * If new data type is added, add to PROPERTY_DATA_TYPE and at least ArrayTemplateManager.
 *
 */

/// C struct data type, if any new data types are added also add them to the conversion mechanism in StructureConverter.cpp
enum SE_DATA_TYPE {
  SE_TYPE_BOOL=0, SE_TYPE_TINYINT, SE_TYPE_SMALLINT, SE_TYPE_INT, SE_TYPE_UTINYINT, SE_TYPE_USMALLINT, SE_TYPE_UINT, SE_TYPE_VARCHAR, SE_TYPE_VARCHAR_ARRAY,
  SE_TYPE_FLOAT, SE_TYPE_DOUBLE, SE_TYPE_SENTINEL
};

/// SQL data type
enum DB_DATA_TYPE { DB_TYPE_BOOL=0, DB_TYPE_INT, DB_TYPE_DOUBLE, DB_TYPE_STRING, DB_TYPE_SENTINEL};

/// DBO types
//enum DB_OBJECT_TYPE {DBO_UNDEFINED=0, DBO_PLOTS, DBO_SYSTEM_TRACKS, DBO_ADS_B, DBO_MLAT, DBO_REFERENCE_TRAJECTORIES, DBO_SENSOR_INFORMATION};
/// Mappings for DBO type to strings, for debug output, defined in util.cpp. slightly outdated
//extern std::map<DB_OBJECT_TYPE,std::string> DB_OBJECT_TYPE_STRINGS;

/// Special string representation types. Slighly outdated
enum STRING_REPRESENTATION { R_STANDARD, R_TIME_SECONDS, R_OCTAL, R_FLIGHT_LEVEL, R_SENSOR_NAME, R_HEX };
/// Mappings for STRING_REPRESENTATION to strings, defined in util.cpp
extern std::map<STRING_REPRESENTATION,std::string> STRING_REPRESENTATION_STRINGS;

/// If an old ogre library (1.6.x) is used.
#define USE_OLD_OGRE 0
/// If set, disables ogre logging.
#define DISABLE_OGRE_LOG 1

/// Definition of different presentation modes
enum PRESENTATION_MODE
{
  PRESENTATION_MODE_POINT_DETECTION,
  PRESENTATION_MODE_POINT_RADAR,
  PRESENTATION_MODE_SYMBOL_DETECTION,
  PRESENTATION_MODE_SYMBOL_RADAR
};

/// Definition of different view modes
enum VIEW_MODE
{
  VIEW_MODE_2D, VIEW_MODE_3D
};

/// Definition if different display object types
enum DOType {
  DO_UNKNOWN,
  DO_POINTS,
  DO_TILEDPOINTS,
  DO_LINES
};

/// Definition of different display object query flags
enum DOQueryFlag {
  QUERY_NONE        = 0,
  QUERY_POINTS      = 1<<1,
  QUERY_TILEDPOINTS = 1<<2,
  QUERY_LINES       = 1<<3,
  QUERY_RECTS       = 1<<4
};

#endif
