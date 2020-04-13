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

//
//#ifndef STRUCTUREDESCRIPTIONSENSOR_H_
//#define STRUCTUREDESCRIPTIONSENSOR_H_
//
//#include <cstddef>
//#include "StructureDescription.h"
//
///**
// * @brief Description for Radar sensor data struct t_Radar_Description
// */
// class StructureDescriptionSensor : public StructureDescription
//{
// public:
//  StructureDescriptionSensor () : StructureDescription (DBO_SENSOR_INFORMATION, "Sensor", "RDL
//  sensor structure", 0)
//  {
//    create ();
//  }
//  virtual ~StructureDescriptionSensor() {}
//
// protected:
//  void create ()
//  {
//    addStructureVariable("altitude", SE_TYPE_DOUBLE, 1, "Geographical altitude; metres",
//    offsetof(t_Radar_Description,altitude)); addStructureVariable("context", SE_TYPE_INT, 1,
//    "Context", offsetof(t_Radar_Description,context)); addStructureVariable("latitude",
//    SE_TYPE_DOUBLE, 1, "Geographical latitude; degrees", offsetof(t_Radar_Description,latitude));
//    addStructureVariable("longitude", SE_TYPE_DOUBLE, 1, "Geographical longitude; degrees",
//    offsetof(t_Radar_Description,longitude)); addStructureVariable("max_psr_range",
//    SE_TYPE_DOUBLE, 1, "PSR range meters", offsetof(t_Radar_Description,max_psr_range));
//    addStructureVariable("max_ssr_range", SE_TYPE_DOUBLE, 1, "SSR range meters",
//    offsetof(t_Radar_Description,max_ssr_range)); addStructureVariable("radar_number",
//    SE_TYPE_SMALLINT, 1, "Radar number (within the defined context; 0 otherwise)",
//    offsetof(t_Radar_Description,radar_number)); addStructureVariable("remarks", SE_TYPE_VARCHAR,
//    50, "Pointer to remarks (e. g. full name)", offsetof(t_Radar_Description,remarks));
//    addStructureVariable("sensor_short_name", SE_TYPE_VARCHAR_ARRAY, M_SENSOR_SHORT_NAME_LENGTH +
//    1, "Short name of sensor", offsetof(t_Radar_Description,sensor_short_name));
//  }
//};
//
//
//#endif /* STRUCTUREDESCRIPTIONSENSOR_H_ */
