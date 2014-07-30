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
 * StructureDescriptionPlot.h
 *
 *  Created on: Jul 26, 2012
 *      Author: sk
 */

#ifndef STRUCTUREDESCRIPTIONPLOT_H_
#define STRUCTUREDESCRIPTIONPLOT_H_

#include "StructureDescription.h"
#include "RadarPlot.h"

#include <cstddef>

/**
 * @brief Description for Radar plot data struct t_Rtgt
 *
 * Creates a temporary description, and adds flattened contents to itself
 */
class StructureDescriptionPlot : public StructureDescription
{
public:
  StructureDescriptionPlot () : StructureDescription (DBO_PLOTS, "Plot", "RDL t_Rtgt structure", 0)
  {
    create ();
  }
  virtual ~StructureDescriptionPlot() {}

protected:
  void create ()
  {
    StructureDescription *rdl_plot_description = new StructureDescription (DBO_PLOTS, "", "rdl t_Rtgt structure", 0);

  //  //t_ACAS_Resolution_Advisory_Report acas_resolution_advisory_report;
  //                   /* ACAS resolution advisory report */
  //  size_t offset = offsetof (t_Rtgt, acas_resolution_advisory_report);
  //  StructureDescription* acas = rdl_plot_description->addStructureDescription("acas_resolution_advisory_report", "ACAS resolution advisory report", offset);
  //  acas->addStructureVariable("present", TYPE_BOOL, 1, "ACAS resolution advisory report present", offset+offsetof(t_ACAS_Resolution_Advisory_Report,present));
  //  acas->addStructureVariable("value", TYPE_UTINYINT, M_ACAS_RESOLUTION_ADVISORY_LENGTH, "Currently active resolution advisory, if any  Comm-B message data of BDS register 3,0", offset+offsetof(t_ACAS_Resolution_Advisory_Report,value));

    //t_Aircraft_Identification aircraft_identification;
    //                 /* Aircraft identification */
    size_t offset = offsetof (t_Rtgt, aircraft_identification);
    StructureDescription* air_id = rdl_plot_description->addStructureDescription("aircraft_identification", "Aircraft identification", offset);
    air_id->addPresentStructureVariable("present", SE_TYPE_BOOL, 1, "Aircraft identification present", offset+offsetof(t_Aircraft_Identification, present));
    air_id->addStructureVariable("sti_present", SE_TYPE_BOOL, 1, "Source of target identification present", offset+offsetof(t_Aircraft_Identification,sti_present));
    air_id->addStructureVariable("value_idt", SE_TYPE_VARCHAR_ARRAY, M_AIRCRAFT_IDENTIFICATION_LENGTH + 1, "Aircraft identification", offset+offsetof(t_Aircraft_Identification,value_idt));
    air_id->addStructureVariable("value_sti", SE_TYPE_UTINYINT, 1, "Source of target identification", offset+offsetof(t_Aircraft_Identification,value_sti));

    //t_Tres alarm;
                     /* (UVD) alarm flag */
    //rdl_plot_description->addStructureVariable("alarm", TYPE_UTINYINT, 1, "(UVD) alarm flag  e_is_undefined = 0 e_is_false = 1 e_is_true = 2", offsetof(t_Rtgt,alarm));

    //t_Antenna_Number antenna_number;
                     /* Antenna number */
  //  offset = offsetof (t_Rtgt, antenna_number);
  //  StructureDescription* antenna_number = rdl_plot_description->addStructureDescription("antenna_number", "Antenna number", offset);
  //  antenna_number->addStructureVariable("present", TYPE_BOOL, 1, "Antenna number present", offset+offsetof(t_Antenna_Number,present));
  //  antenna_number->addStructureVariable("value", TYPE_USMALLINT, 1, "Antenna number  0 means: undefined or unknown", offset+offsetof(t_Antenna_Number,value));

    //t_Assumed_Height assumed_height;
                     /* Assumed height */
  //  offset = offsetof (t_Rtgt, assumed_height);
  //  StructureDescription* assumed_height = rdl_plot_description->addStructureDescription("assument_height", "Assumed height", offset);
  //  assumed_height->addStructureVariable("present", TYPE_BOOL, 1, "Assumed height present", offset+offsetof(t_Assumed_Height,present));
  //  assumed_height->addStructureVariable("value", TYPE_DOUBLE, 1, "Assumed height; metres", offset+offsetof(t_Assumed_Height,value));

    //t_Byte asterix_category;
                     /* ASTERIX category */
    rdl_plot_description->addStructureVariable("asterix", SE_TYPE_UTINYINT, 1, "ASTERIX category", offsetof(t_Rtgt,asterix_category));

  //  //t_Azimuth_Difference azimuth_difference;
  //                   /* Azimuth difference between PSR and SSR plot */
  //  offset = offsetof (t_Rtgt, azimuth_difference);
  //  StructureDescription* azimuth_difference = rdl_plot_description->addStructureDescription("azimuth_difference", "Azimuth difference between PSR and SSR plot", offset);
  //  azimuth_difference->addStructureVariable("present", TYPE_BOOL, 1, "Azimuth difference present", offset+offsetof(t_Azimuth_Difference,present));
  //  azimuth_difference->addStructureVariable("value", TYPE_DOUBLE, 1, "Azimuth difference; radians", offset+offsetof(t_Azimuth_Difference,value));


    //t_BDS_Register bds_registers[M_MAX_BDS_REGISTERS];
                     /* BDS registers */
  //  for (int cnt=0; cnt < M_MAX_BDS_REGISTERS; cnt++)
  //  {
  //    std::string tmp;
  //    tmp="bds_register_"+intToString(cnt);
  //
  //    offset = offsetof (t_Rtgt, bds_registers)+cnt*sizeof(t_BDS_Register);
  //    StructureDescription* bds_register = rdl_plot_description->addStructureDescription(tmp, "BDS registers", offset);
  //    bds_register->addStructureVariable("number", TYPE_UTINYINT, 1, "Number of BDS register", offset+offsetof(t_BDS_Register,number));
  //    bds_register->addStructureVariable("present", TYPE_BOOL, 1, "BDS register present", offset+offsetof(t_BDS_Register,present));
  //    bds_register->addStructureVariable("value", TYPE_UTINYINT, M_BDS_REGISTER_LENGTH, "Value of BDS register", offset+offsetof(t_BDS_Register,value));
  //  }

    //t_Board_Number board_number;
                     /* Aircraft board number */
  //  offset = offsetof (t_Rtgt, board_number);
  //  StructureDescription* board_number = rdl_plot_description->addStructureDescription("board_number", "Aircraft board number", offset);
  //  board_number->addStructureVariable("present", TYPE_BOOL, 1, "Board number present", offset+offsetof(t_Board_Number,present));
  //  board_number->addStructureVariable("value", TYPE_UINT, 1, "Board number", offset+offsetof(t_Board_Number,value));

  //  //t_Doppler_Speed calculated_doppler_speed;
  //                   /* Calculated Doppler speed */
  //  offset = offsetof (t_Rtgt, calculated_doppler_speed);
  //  StructureDescription* calculated_doppler_speed = rdl_plot_description->addStructureDescription("calculated_doppler_speed", "Calculated Doppler speed", offset);
  //  calculated_doppler_speed->addStructureVariable("is_doubtful", TYPE_UTINYINT, 1, "Doppler speed is doubtful e_is_undefined = 0 e_is_false = 1 e_is_true = 2", offset+offsetof(t_Doppler_Speed,is_doubtful));
  //  calculated_doppler_speed->addStructureVariable("present", TYPE_BOOL, 1, "Doppler speed present", offset+offsetof(t_Doppler_Speed,present));
  //  calculated_doppler_speed->addStructureVariable("value", TYPE_DOUBLE, 1, "Doppler speed; metres/second", offset+offsetof(t_Doppler_Speed,value));

    //t_Tres civil_emergency;
                     /* Civil emergency indication */
    rdl_plot_description->addStructureVariable("civil_emergency", SE_TYPE_UTINYINT, 1, "Civil emergency indication:  e_is_undefined = 0 e_is_false = 1 e_is_true = 2", offsetof (t_Rtgt, civil_emergency));

    //t_Computed_Position computed_position;
                     /* Computed (Cartesian) position */
    offset = offsetof (t_Rtgt, computed_position);
    StructureDescription* computed_position = rdl_plot_description->addStructureDescription("computed_position", "Computed (Cartesian) position", offset);
    computed_position->addPresentStructureVariable("present", SE_TYPE_BOOL, 1, "Computed position present", offset+offsetof(t_Computed_Position,present));
    computed_position->addStructureVariable("value_x", SE_TYPE_DOUBLE, 1, "Computed x coordinate; metres", offset+offsetof(t_Computed_Position,value_x));
    computed_position->addStructureVariable("value_y", SE_TYPE_DOUBLE, 1, "Computed y coordinate; metres", offset+offsetof(t_Computed_Position,value_y));

    //t_Data_Format data_format;
                     /* Data format (origin) */
    //rdl_plot_description->addStructureVariable("data_format", TYPE_UTINYINT, 1, "Data format (origin) TODO BIG ENUM", offsetof(t_Rtgt,data_format));

    //t_Data_Source_Identifier data_source_identifier;
                     /* Data source identifier */
  //  offset = offsetof (t_Rtgt, data_source_identifier);
  //  StructureDescription* data_source_identifier = rdl_plot_description->addStructureDescription("data_source_identifier", "Data source identifier", offset);
  //  data_source_identifier->addStructureVariable("present", TYPE_BOOL, 1, "Data source identifier present", offset+offsetof(t_Data_Source_Identifier,present));
  //  data_source_identifier->addStructureVariable("supplemented", TYPE_BOOL, 1, "Data source identifier supplemented", offset+offsetof(t_Data_Source_Identifier,supplemented));
  //  data_source_identifier->addStructureVariable("value", TYPE_USMALLINT, 1, "Data source identifier (SAC/SIC)", offset+offsetof(t_Data_Source_Identifier,value));

    //t_Partial_Time_of_Day detection_ptod;
                     /* Measured detection time (as partial time of day) */
  //  offset = offsetof (t_Rtgt, detection_ptod);
  //  StructureDescription* detection_ptod = rdl_plot_description->addStructureDescription("detection_ptod", "Measured detection time (as partial time of day)", offset);
  //  detection_ptod->addStructureVariable("present", TYPE_BOOL, 1, "Partial time of day present", offset+offsetof(t_Partial_Time_of_Day,present));
  //  detection_ptod->addStructureVariable("value", TYPE_USMALLINT, 1, "Partial time of day; 1/128 seconds", offset+offsetof(t_Partial_Time_of_Day,value));

    //t_Time_of_Day detection_time;
                     /* Measured detection time */
    offset = offsetof (t_Rtgt, detection_time);
    StructureDescription* detection_time = rdl_plot_description->addStructureDescription("detection_time", "Measured detection time", offset);
    detection_time->addPresentStructureVariable("present", SE_TYPE_BOOL, 1, "Time of day present", offset+offsetof(t_Time_of_Day,present));
    detection_time->addStructureVariable("value", SE_TYPE_DOUBLE, 1, "Time of day; seconds", offset+offsetof(t_Time_of_Day,value));

    //t_Detection_Type detection_type;
                     /* Detection type */
    offset = offsetof (t_Rtgt, detection_type);
    StructureDescription* detection_type = rdl_plot_description->addStructureDescription("detection_type", "Detection type", offset);
    detection_type->addStructureVariable("from_fixed_field_transponder", SE_TYPE_UTINYINT, 1, "Target report originating from Fixed Field Transponder (FFT) which is also called a \"parrot\" or RABM (range and bearing monitor) sometimes", offset+offsetof(t_Detection_Type,from_fixed_field_transponder));
    detection_type->addStructureVariable("is_ctr_target", SE_TYPE_UTINYINT, 1, "Is a control target (some type of RTQC as used on Russian radar lines)", offset+offsetof(t_Detection_Type,from_fixed_field_transponder));
    detection_type->addStructureVariable("is_radar_track", SE_TYPE_UTINYINT, 1, "Is a radar track (not a radar plot)", offset+offsetof(t_Detection_Type,is_radar_track));
    detection_type->addStructureVariable("is_raw_plot", SE_TYPE_UTINYINT, 1, "Is a so-called \"raw\" plot, i. e. rather unprocessed target from an early stage", offset+offsetof(t_Detection_Type,is_raw_plot));
    detection_type->addStructureVariable("is_rbs_target", SE_TYPE_UTINYINT, 1, "Is a so-called ATCRBS target report, i. e. Western-style report on Russian radar lines", offset+offsetof(t_Detection_Type,is_rbs_target));
    detection_type->addStructureVariable("is_uvd_target", SE_TYPE_UTINYINT, 1, "Is a VRL UVD target report", offset+offsetof(t_Detection_Type,is_uvd_target));
    detection_type->addStructureVariable("mode_s_all_call", SE_TYPE_UTINYINT, 1, "Response to an SSR mode S all-call", offset+offsetof(t_Detection_Type,mode_s_all_call));
    detection_type->addStructureVariable("mode_s_roll_call", SE_TYPE_UTINYINT, 1, "Response to an SSR mode S roll-call", offset+offsetof(t_Detection_Type,mode_s_roll_call));
    detection_type->addStructureVariable("monopulse", SE_TYPE_UTINYINT, 1, "Monopulse detection (not sliding window)", offset+offsetof(t_Detection_Type,monopulse));
    detection_type->addStructureVariable("passive_reply", SE_TYPE_BOOL, 1, "Passive reply", offset+offsetof(t_Detection_Type,passive_reply));
    detection_type->addPresentStructureVariable("present", SE_TYPE_BOOL, 1, "Detection type present", offset+offsetof(t_Detection_Type,present));
    detection_type->addStructureVariable("reported_from_ads", SE_TYPE_UTINYINT, 1, "Reported from an ADS-B groundstation", offset+offsetof(t_Detection_Type,reported_from_ads));
    detection_type->addStructureVariable("reported_from_mds", SE_TYPE_UTINYINT, 1, "Reported from a Mode S sensor (tracker)", offset+offsetof(t_Detection_Type,reported_from_mds));
    detection_type->addStructureVariable("reported_from_mlt", SE_TYPE_UTINYINT, 1, "Reported from a multilateration system", offset+offsetof(t_Detection_Type,reported_from_mlt));
    detection_type->addStructureVariable("reported_from_psr", SE_TYPE_UTINYINT, 1, "Reported from a Primary Surveillance Radar", offset+offsetof(t_Detection_Type,reported_from_psr));
    detection_type->addStructureVariable("reported_from_ssr", SE_TYPE_UTINYINT, 1, "Reported from a Secondary Surveillance Radar", offset+offsetof(t_Detection_Type,reported_from_ssr));
    detection_type->addStructureVariable("sector_crossing", SE_TYPE_BOOL, 1, "Sector crossing", offset+offsetof(t_Detection_Type,sector_crossing));
    detection_type->addStructureVariable("simulated", SE_TYPE_UTINYINT, 1, "Simulated (not actual) target", offset+offsetof(t_Detection_Type,simulated));
    detection_type->addStructureVariable("test_target", SE_TYPE_UTINYINT, 1, "(Internal or external) test target", offset+offsetof(t_Detection_Type,test_target));


    //t_Tres emergency_1;
                     /* Emergency indication (A7500) */
    rdl_plot_description->addStructureVariable("emergency_1", SE_TYPE_UTINYINT, 1, "Emergency indication (A7500):  e_is_undefined = 0 e_is_false = 1 e_is_true = 2", offsetof(t_Rtgt,emergency_1));

    //t_Tres emergency_2;
                     /* Emergency indication (A7600) */
    rdl_plot_description->addStructureVariable("emergency_2", SE_TYPE_UTINYINT, 1, "Emergency indication (A7600):  e_is_undefined = 0 e_is_false = 1 e_is_true = 2", offsetof(t_Rtgt,emergency_2));

    //t_Tres emergency_3;
                     /* Emergency indication (A7700) */
    rdl_plot_description->addStructureVariable("emergency_3", SE_TYPE_UTINYINT, 1, "Emergency indication (A7700):  e_is_undefined = 0 e_is_false = 1 e_is_true = 2", offsetof(t_Rtgt,emergency_3));

    //t_Time_of_Day estimated_time;
                     /* Estimated detection time */
    offset = offsetof (t_Rtgt, estimated_time);
    StructureDescription* estimated_time = rdl_plot_description->addStructureDescription("estimated_time", "Estimated detection time", offset);
    estimated_time->addPresentStructureVariable("present", SE_TYPE_BOOL, 1, "Time of day present", offset+offsetof(t_Time_of_Day,present));
    estimated_time->addStructureVariable("value", SE_TYPE_DOUBLE, 1, "Time of day; seconds", offset+offsetof(t_Time_of_Day,value));

    //t_Bool excessive_dt_fd;
                     /* Excessive difference of time between frame
                        time and detection time */
    //rdl_plot_description->addStructureVariable("excessive_dt_fd", TYPE_BOOL, 1, "Excessive difference of time between frame time and detection time", offsetof(t_Rtgt,excessive_dt_fd));

    //t_Bool excessive_dt_fe;
                     /* Excessive difference of time between frame
                        time and estimated time */
    //rdl_plot_description->addStructureVariable("excessive_dt_fe", TYPE_BOOL, 1, "Excessive difference of time between frame time and estimated time", offsetof(t_Rtgt,excessive_dt_fe));

    //t_Frame_Date frame_date;
                     /* Frame date */
  //  offset = offsetof (t_Rtgt, frame_date);
  //  StructureDescription* frame_date = rdl_plot_description->addStructureDescription("frame_date", "Frame date", offset);
  //  frame_date->addStructureVariable("present", TYPE_BOOL, 1, "Frame date present", offset+offsetof(t_Frame_Date,present));
  //  offset = offset+offsetof (t_Frame_Date, value);
  //  StructureDescription* value = frame_date->addStructureDescription("value", "Frame date", offset);
  //  value->addStructureVariable("year", TYPE_INT, 1, "Frame date year", offset+offsetof(t_Date,year));
  //  value->addStructureVariable("month", TYPE_INT, 1, "Frame date month", offset+offsetof(t_Date,month));
  //  value->addStructureVariable("day", TYPE_INT, 1, "Frame date day", offset+offsetof(t_Date,day));

    //t_Frame_Time frame_time;
                     /* Frame time */
    offset = offsetof (t_Rtgt, frame_time);
    StructureDescription* frame_time = rdl_plot_description->addStructureDescription("frame_time", "Frame time", offset);
    frame_time->addPresentStructureVariable("present", SE_TYPE_BOOL, 1, "Frame time present", offset+offsetof(t_Frame_Time,present));
    frame_time->addStructureVariable("value", SE_TYPE_DOUBLE, 1, "Frame time (of day); seconds", offset+offsetof(t_Frame_Time,value));

    //t_Bool from_rdp_chain_1;
                     /* From RDP chain 1 */
    //rdl_plot_description->addStructureVariable("from_rdp_chain_1", TYPE_BOOL, 1, "From RDP chain 1", offsetof(t_Rtgt,from_rdp_chain_1));

    //t_Bool from_rdp_chain_2;
                     /* From RDP chain 2 */
    //rdl_plot_description->addStructureVariable("from_rdp_chain_2", TYPE_BOOL, 1, "From RDP chain 2", offsetof(t_Rtgt,from_rdp_chain_2));

    //t_Fuel_Residue fuel_residue;
                     /* Fuel residue indication */
  //  offset = offsetof (t_Rtgt, fuel_residue);
  //  StructureDescription* fuel_residue = rdl_plot_description->addStructureDescription("fuel_residue", "Fuel residue indication", offset);
  //  fuel_residue->addStructureVariable("present", TYPE_BOOL, 1, "Fuel residue indication present", offset+offsetof(t_Fuel_Residue,present));
  //  fuel_residue->addStructureVariable("value", TYPE_USMALLINT, 1, "Fuel residue indication", offset+offsetof(t_Fuel_Residue,value));

    //t_Ground_Vector ground_vector;
                     /* Ground speed and heading */
  //  offset = offsetof (t_Rtgt, ground_vector);
  //  StructureDescription* ground_vector = rdl_plot_description->addStructureDescription("ground_vector", "Ground speed and heading", offset);
  //  ground_vector->addStructureVariable("present", TYPE_BOOL, 1, "Ground vector present", offset+offsetof(t_Ground_Vector,present));
  //  ground_vector->addStructureVariable("value_gsp", TYPE_DOUBLE, 1, "Ground speed; metres/second", offset+offsetof(t_Ground_Vector,value_gsp));
  //  ground_vector->addStructureVariable("value_hdg", TYPE_DOUBLE, 1, "Ground speed; Heading; radians", offset+offsetof(t_Ground_Vector,value_hdg));

    //t_Height_3D height_3d;
                     /* 3D radar height */

  //  offset = offsetof (t_Rtgt, height_3d);
  //  StructureDescription* height_3d = rdl_plot_description->addStructureDescription("height_3D", "3D radar height", offset);
  //  height_3d->addStructureVariable("present", TYPE_BOOL, 1, "Height from 3D radar present", offset+offsetof(t_Height_3D,present));
  //  height_3d->addStructureVariable("valid", TYPE_UTINYINT, 1, "Height from 3D radar valid", offset+offsetof(t_Height_3D,valid));
  //  height_3d->addStructureVariable("value", TYPE_DOUBLE, 1, "Height from 3D radar; metres", offset+offsetof(t_Height_3D,value));
  //  height_3d->addStructureVariable("value_in_feet", TYPE_INT, 1, "Height from 3D radar; feet", offset+offsetof(t_Height_3D,value_in_feet));

    //t_Byte line_number;
                     /* Board/line number */
    //rdl_plot_description->addStructureVariable("line_number", TYPE_UTINYINT, 1, "Board/line number", offsetof(t_Rtgt,line_number));

    //t_Mapped_Position mapped_position;
                     /* Mapped (stereographic) position */
    offset = offsetof (t_Rtgt, mapped_position);
    StructureDescription* mapped_position = rdl_plot_description->addStructureDescription("mapped_position", "Mapped (stereographic) position", offset);
    mapped_position->addPresentStructureVariable("present", SE_TYPE_BOOL, 1, "Mapped position present", offset+offsetof(t_Mapped_Position,present));
    mapped_position->addStructureVariable("value_h", SE_TYPE_DOUBLE, 1, "Height used for mapping; metres", offset+offsetof(t_Mapped_Position,value_h));
    mapped_position->addStructureVariable("value_u", SE_TYPE_DOUBLE, 1, "Mapped position component; metres", offset+offsetof(t_Mapped_Position,value_u));
    mapped_position->addStructureVariable("value_v", SE_TYPE_DOUBLE, 1, "Mapped position component; metres", offset+offsetof(t_Mapped_Position,value_v));

    //t_Measured_Azimuth measured_azm;
                     /* Measured azimuth */
  //  offset = offsetof (t_Rtgt, measured_azm);
  //  StructureDescription* measured_azm = rdl_plot_description->addStructureDescription("measured_azm", "Measured azimuth", offset);
  //  measured_azm->addStructureVariable("present", TYPE_BOOL, 1, "Measured azimuth present", offset+offsetof(t_Measured_Azimuth,present));
  //  measured_azm->addStructureVariable("value", TYPE_DOUBLE, 1, "Measured azimuth; radians", offset+offsetof(t_Measured_Azimuth,value));

    //t_Measured_Range measured_rng;
                     /* Measured range */
  //  offset = offsetof (t_Rtgt, measured_rng);
  //  StructureDescription* measured_rng = rdl_plot_description->addStructureDescription("measured_rng", "Measured range", offset);
  //  measured_rng->addStructureVariable("present", TYPE_BOOL, 1, "Measured range present", offset+offsetof(t_Measured_Range,present));
  //  measured_rng->addStructureVariable("value", TYPE_DOUBLE, 1, "Measured range; meters", offset+offsetof(t_Measured_Range,value));

  //#if gaga
  //  t_Polar_Position measured_position;
  //                   /* Measured polar position */
  //#endif

    //t_Metric_Height metric_height;
                     /* Metric height */
  //  offset = offsetof (t_Rtgt, metric_height);
  //  StructureDescription* metric_height = rdl_plot_description->addStructureDescription("metric_height", "Metric height", offset);
  //  metric_height->addStructureVariable("is_relative", TYPE_UTINYINT, 1, "Metric height is relative to (some) aerodrome", offset+offsetof(t_Metric_Height,is_relative));
  //  metric_height->addStructureVariable("present", TYPE_BOOL, 1, "Metric height present", offset+offsetof(t_Metric_Height,present));
  //  metric_height->addStructureVariable("value", TYPE_DOUBLE, 1, "Metric height; metres", offset+offsetof(t_Metric_Height,value));

    //t_Tres military_emergency;
                     /* Military emergency indication */
    //rdl_plot_description->addStructureVariable("military_emergency", TYPE_UTINYINT, 1, "Military emergency indication", offsetof(t_Rtgt,military_emergency));

    //t_Tres military_ident;
                     /* Military ident indication */
    //rdl_plot_description->addStructureVariable("military_ident", TYPE_UTINYINT, 1, "Military ident indication", offsetof(t_Rtgt,military_ident));

    //t_Mode_Info mode_1_info;
                     /* SSR mode 1 information */
  //  offset = offsetof (t_Rtgt, mode_1_info);
  //  StructureDescription* mode_1_info = rdl_plot_description->addStructureDescription("mode_1_info", "SSR mode 1 information", offset);
  //  mode_1_info->addStructureVariable("code", TYPE_USMALLINT, 1, "SSR mode 1, 2, or 3/A code", offset+offsetof(t_Mode_Info,code));
  //  mode_1_info->addStructureVariable("code_confidence", TYPE_USMALLINT, 1, "SSR mode 1, 2, or 3/A code confidence  Not inverted, i. e. 0=high confidence", offset+offsetof(t_Mode_Info,code_confidence));
  //  mode_1_info->addStructureVariable("code_confidence_present", TYPE_BOOL, 1, "SSR mode 1, 2, or 3/A code confidence present", offset+offsetof(t_Mode_Info,code_confidence_present));
  //  mode_1_info->addStructureVariable("code_garbled", TYPE_UTINYINT, 1, "SSR mode 1, 2, or 3/A code garbled", offset+offsetof(t_Mode_Info,code_garbled));
  //  mode_1_info->addStructureVariable("code_invalid", TYPE_UTINYINT, 1, "SSR mode 1, 2, or 3/A code invalid", offset+offsetof(t_Mode_Info,code_invalid));
  //  mode_1_info->addStructureVariable("code_smoothed", TYPE_UTINYINT, 1, "SSR mode 1, 2, or 3/A code smoothed", offset+offsetof(t_Mode_Info,code_smoothed));
  //  mode_1_info->addStructureVariable("present", TYPE_BOOL, 1, "SSR mode 1, 2, or 3/A information present", offset+offsetof(t_Mode_Info,present));
  //  mode_1_info->addStructureVariable("replies", TYPE_USMALLINT, 1, "Number of SSR mode 1, 2, or 3/A replies", offset+offsetof(t_Mode_Info,replies));

    //t_Mode_Info mode_2_info;
                     /* SSR mode 2 information */
  //  offset = offsetof (t_Rtgt, mode_2_info);
  //  StructureDescription* mode_2_info = rdl_plot_description->addStructureDescription("mode_2_info", "SSR mode 2 information", offset);
  //  mode_2_info->addStructureVariable("code", TYPE_USMALLINT, 1, "SSR mode 1, 2, or 3/A code", offset+offsetof(t_Mode_Info,code));
  //  mode_2_info->addStructureVariable("code_confidence", TYPE_USMALLINT, 1, "SSR mode 1, 2, or 3/A code confidence  Not inverted, i. e. 0=high confidence", offset+offsetof(t_Mode_Info,code_confidence));
  //  mode_2_info->addStructureVariable("code_confidence_present", TYPE_BOOL, 1, "SSR mode 1, 2, or 3/A code confidence present", offset+offsetof(t_Mode_Info,code_confidence_present));
  //  mode_2_info->addStructureVariable("code_garbled", TYPE_UTINYINT, 1, "SSR mode 1, 2, or 3/A code garbled", offset+offsetof(t_Mode_Info,code_garbled));
  //  mode_2_info->addStructureVariable("code_invalid", TYPE_UTINYINT, 1, "SSR mode 1, 2, or 3/A code invalid", offset+offsetof(t_Mode_Info,code_invalid));
  //  mode_2_info->addStructureVariable("code_smoothed", TYPE_UTINYINT, 1, "SSR mode 1, 2, or 3/A code smoothed", offset+offsetof(t_Mode_Info,code_smoothed));
  //  mode_2_info->addStructureVariable("present", TYPE_BOOL, 1, "SSR mode 1, 2, or 3/A information present", offset+offsetof(t_Mode_Info,present));
  //  mode_2_info->addStructureVariable("replies", TYPE_USMALLINT, 1, "Number of SSR mode 1, 2, or 3/A replies", offset+offsetof(t_Mode_Info,replies));


    //t_Mode_Info mode_3a_info;
                     /* SSR mode 3/A information */
    offset = offsetof (t_Rtgt,  mode_3a_info);
    StructureDescription* mode_3a_info = rdl_plot_description->addStructureDescription("mode_3a_info", "SSR mode 1 information", offset);
    mode_3a_info->addStructureVariable("code", SE_TYPE_USMALLINT, 1, "SSR mode 1, 2, or 3/A code", offset+offsetof(t_Mode_Info,code));
    mode_3a_info->addStructureVariable("code_confidence", SE_TYPE_USMALLINT, 1, "SSR mode 1, 2, or 3/A code confidence  Not inverted, i. e. 0=high confidence", offset+offsetof(t_Mode_Info,code_confidence));
    mode_3a_info->addStructureVariable("code_confidence_present", SE_TYPE_BOOL, 1, "SSR mode 1, 2, or 3/A code confidence present", offset+offsetof(t_Mode_Info,code_confidence_present));
    mode_3a_info->addStructureVariable("code_garbled", SE_TYPE_UTINYINT, 1, "SSR mode 1, 2, or 3/A code garbled", offset+offsetof(t_Mode_Info,code_garbled));
    mode_3a_info->addStructureVariable("code_invalid", SE_TYPE_UTINYINT, 1, "SSR mode 1, 2, or 3/A code invalid", offset+offsetof(t_Mode_Info,code_invalid));
    mode_3a_info->addStructureVariable("code_smoothed", SE_TYPE_UTINYINT, 1, "SSR mode 1, 2, or 3/A code smoothed", offset+offsetof(t_Mode_Info,code_smoothed));
    mode_3a_info->addPresentStructureVariable("present", SE_TYPE_BOOL, 1, "SSR mode 1, 2, or 3/A information present", offset+offsetof(t_Mode_Info,present));
    mode_3a_info->addStructureVariable("replies", SE_TYPE_USMALLINT, 1, "Number of SSR mode 1, 2, or 3/A replies", offset+offsetof(t_Mode_Info,replies));

    //t_Mode_4_Info mode_4_info;
                     /* SSR mode 4 information */
  //  offset = offsetof (t_Rtgt, mode_4_info);
  //  StructureDescription* mode_4_info = rdl_plot_description->addStructureDescription("mode_4_info", "SSR mode 4 information", offset);
  //  mode_4_info->addStructureVariable("invalid", TYPE_UTINYINT, 1, "SSR mode 4 information invalid", offset+offsetof(t_Mode_4_Info,invalid));
  //  mode_4_info->addStructureVariable("present", TYPE_UTINYINT, 1, "SSR mode 4 information present", offset+offsetof(t_Mode_4_Info,present));
  //  mode_4_info->addStructureVariable("value", TYPE_USMALLINT, 1, "SSR mode 4 information  0 ... no SSR mode 4 interrogation,  1 ... friendly target,  2 ... unknown target,  3 ... no reply", offset+offsetof(t_Mode_4_Info,value));

    //t_Mode_C_Code mode_c_code;
                     /* SSR mode C code */
  //  offset = offsetof (t_Rtgt, mode_c_code);
  //  StructureDescription* mode_c_code = rdl_plot_description->addStructureDescription("mode_c_code", "SSR mode C code", offset);
  //  mode_c_code->addStructureVariable("code", TYPE_USMALLINT, 1, "SSR mode C code  This is the SSR mode C code as sent down by the transponder", offset+offsetof(t_Mode_C_Code,code));
  //  mode_c_code->addStructureVariable("code_confidence", TYPE_USMALLINT, 1, "SSR mode C code confidence  Not inverted, i.e. 0=high confidence", offset+offsetof(t_Mode_C_Code,code_confidence));
  //  mode_c_code->addStructureVariable("code_confidence_present", TYPE_BOOL, 1, "SSR mode C code confidence present", offset+offsetof(t_Mode_C_Code,code_confidence_present));
  //  mode_c_code->addStructureVariable("code_garbled", TYPE_UTINYINT, 1, "SSR mode C code garbled", offset+offsetof(t_Mode_C_Code,code_garbled));
  //  mode_c_code->addStructureVariable("code_invalid", TYPE_UTINYINT, 1, "SSR mode C invalid", offset+offsetof(t_Mode_C_Code,code_invalid));
  //  mode_c_code->addStructureVariable("present", TYPE_BOOL, 1, "SSR mode C information present", offset+offsetof(t_Mode_C_Code,present));

    //t_Mode_C_Height mode_c_height;
                     /* SSR mode C height */
    offset = offsetof (t_Rtgt, mode_c_height);
    StructureDescription* mode_c_height = rdl_plot_description->addStructureDescription("mode_c_height", "SSR mode C height", offset);
    mode_c_height->addStructureVariable("garbled", SE_TYPE_UTINYINT, 1, "Garbled SSR mode C height", offset+offsetof(t_Mode_C_Height,garbled));
    mode_c_height->addStructureVariable("height_in_error", SE_TYPE_UTINYINT, 1, "SSR mode C height is not a valid altitude, i.e. it is out of (regular) bounds", offset+offsetof(t_Mode_C_Height,height_in_error));
    mode_c_height->addStructureVariable("in_25_feet", SE_TYPE_UTINYINT, 1, "SSR mode C height in 25 feet resolution", offset+offsetof(t_Mode_C_Height,in_25_feet));
    mode_c_height->addStructureVariable("invalid", SE_TYPE_UTINYINT, 1, "Invalid SSR mode C height", offset+offsetof(t_Mode_C_Height,invalid));
    mode_c_height->addPresentStructureVariable("present", SE_TYPE_BOOL, 1, "SSR mode C information present", offset+offsetof(t_Mode_C_Height,present));
    mode_c_height->addStructureVariable("value", SE_TYPE_DOUBLE, 1, "SSR mode C height; metres", offset+offsetof(t_Mode_C_Height,value));
    mode_c_height->addStructureVariable("value_in_feet", SE_TYPE_INT, 1, "SSR mode C height; feet", offset+offsetof(t_Mode_C_Height,value_in_feet));

    //t_Mode_S_Flags mode_s_flags;
                     /* SSR mode S flags */
  //  offset = offsetof (t_Rtgt, mode_s_flags);
  //  StructureDescription* mode_s_flags = rdl_plot_description->addStructureDescription("mode_s_flags", "SSR mode S flags", offset);
  //  mode_s_flags->addStructureVariable("present", TYPE_BOOL, 1, "SSR mode S flags present", offset+offsetof(t_Mode_S_Flags,present));
  //  mode_s_flags->addStructureVariable("value_aic", TYPE_UTINYINT, 1, "AIC", offset+offsetof(t_Mode_S_Flags,value_aic));
  //  mode_s_flags->addStructureVariable("value_arc", TYPE_UTINYINT, 1, "ARC (0 = 100 feet, 1 = 25 feet)", offset+offsetof(t_Mode_S_Flags,value_arc));
  //  mode_s_flags->addStructureVariable("value_b1a", TYPE_UTINYINT, 1, "B1A", offset+offsetof(t_Mode_S_Flags,value_b1a));
  //  mode_s_flags->addStructureVariable("value_b1b", TYPE_UTINYINT, 1, "B1B", offset+offsetof(t_Mode_S_Flags,value_b1b));
  //  mode_s_flags->addStructureVariable("value_cc", TYPE_UTINYINT, 1, "CC", offset+offsetof(t_Mode_S_Flags,value_cc));
  //  mode_s_flags->addStructureVariable("value_fs", TYPE_UTINYINT, 1, "FS", offset+offsetof(t_Mode_S_Flags,value_fs));
  //  mode_s_flags->addStructureVariable("value_mssc", TYPE_UTINYINT, 1, "MSSC", offset+offsetof(t_Mode_S_Flags,value_mssc));
  //  mode_s_flags->addStructureVariable("value_si", TYPE_UTINYINT, 1, "SI", offset+offsetof(t_Mode_S_Flags,value_si));

    //t_Bool possible_split;
                     /* Possibly a split radar target report */
    //rdl_plot_description->addStructureVariable("possible_split", TYPE_UTINYINT, 1, "Possibly a split radar target report", offsetof(t_Rtgt,possible_split));

    //t_Measured_Amplitude psr_amplitude;
                     /* Amplitude of PSR reply */
  //  offset = offsetof (t_Rtgt, psr_amplitude);
  //  StructureDescription* psr_amplitude = rdl_plot_description->addStructureDescription("psr_amplitude", "Amplitude of PSR reply", offset);
  //  psr_amplitude->addStructureVariable("present", TYPE_BOOL, 1, "Measured amplitude present", offset+offsetof(t_Measured_Amplitude,present));
  //  psr_amplitude->addStructureVariable("value", TYPE_SMALLINT, 1, "Measured amplitude; dBm", offset+offsetof(t_Measured_Amplitude,value));

    //t_Runlength psr_runlength;
                     /* PSR plot runlength */
  //  offset = offsetof (t_Rtgt, psr_runlength);
  //  StructureDescription* psr_runlength = rdl_plot_description->addStructureDescription("psr_runlength", "PSR plot runlength", offset);
  //  psr_runlength->addStructureVariable("present", TYPE_BOOL, 1, "Runlength present", offset+offsetof(t_Runlength,present));
  //  psr_runlength->addStructureVariable("value", TYPE_DOUBLE, 1, "Runlength; degrees", offset+offsetof(t_Runlength,value));
  //  psr_runlength->addStructureVariable("value_in_acps", TYPE_UINT, 1, "Runlength; ACPs", offset+offsetof(t_Runlength,value_in_acps));

    //t_Radial_Speed radial_speed;
                     /* Measured radial Doppler speed */
  //  offset = offsetof (t_Rtgt, radial_speed);
  //  StructureDescription* radial_speed = rdl_plot_description->addStructureDescription("radial_speed", "Measured radial Doppler speed", offset);
  //  radial_speed->addStructureVariable("present", TYPE_BOOL, 1, "Measured radial Doppler speed", offset+offsetof(t_Radial_Speed,present));
  //  radial_speed->addStructureVariable("value", TYPE_DOUBLE, 1, "Radial speed; metres/second", offset+offsetof(t_Radial_Speed,value));

    //t_Range_Difference range_difference;
                     /* Range difference between PSR and SSR plot */
  //  offset = offsetof (t_Rtgt, range_difference);
  //  StructureDescription* range_difference = rdl_plot_description->addStructureDescription("range_difference", "Range difference between PSR and SSR plot", offset);
  //  range_difference->addStructureVariable("present", TYPE_BOOL, 1, "Range difference present", offset+offsetof(t_Range_Difference,present));
  //  range_difference->addStructureVariable("value", TYPE_DOUBLE, 1, "Range difference; metres", offset+offsetof(t_Range_Difference,value));

    //t_Raw_Doppler_Speed raw_doppler_speed;
                     /* Raw Doppler speed */
  //  offset = offsetof (t_Rtgt, raw_doppler_speed);
  //  StructureDescription* raw_doppler_speed = rdl_plot_description->addStructureDescription("raw_doppler_speed", "Raw Doppler speed", offset);
  //  raw_doppler_speed->addStructureVariable("ambiguity_range", TYPE_DOUBLE, 1, "Raw Doppler speed ambiguity range; metres/second", offset+offsetof(t_Raw_Doppler_Speed,ambiguity_range));
  //  raw_doppler_speed->addStructureVariable("present", TYPE_BOOL, 1, "Raw Doppler speed present", offset+offsetof(t_Raw_Doppler_Speed,present));
  //  raw_doppler_speed->addStructureVariable("transmitter_frequency", TYPE_USMALLINT, 1, "Transmitter frequency; MHz", offset+offsetof(t_Raw_Doppler_Speed,transmitter_frequency));
  //  raw_doppler_speed->addStructureVariable("value", TYPE_DOUBLE, 1, "Raw Doppler speed; metres/second", offset+offsetof(t_Raw_Doppler_Speed,value));

    //t_Received_Power received_power;
                     /* Received power */
  //  offset = offsetof (t_Rtgt, received_power);
  //  StructureDescription* received_power = rdl_plot_description->addStructureDescription("received_power", "Range difference between PSR and SSR plot", offset);
  //  received_power->addStructureVariable("present", TYPE_BOOL, 1, "Received power present", offset+offsetof(t_Received_Power,present));
  //  received_power->addStructureVariable("value", TYPE_SMALLINT, 1, "Received power present", offset+offsetof(t_Received_Power,value));

    //t_Reported_Track_Quality reported_track_quality;
                     /* Reported (radar) track quality */
  //  offset = offsetof (t_Rtgt, reported_track_quality);
  //  StructureDescription* reported_track_quality = rdl_plot_description->addStructureDescription("reported_track_quality", "Reported (radar) track quality", offset);
  //  reported_track_quality->addStructureVariable("length", TYPE_USMALLINT, 1, "Length", offset+offsetof(t_Reported_Track_Quality,length));
  //  reported_track_quality->addStructureVariable("present", TYPE_BOOL, 1, "Reported track quality present", offset+offsetof(t_Reported_Track_Quality,present));
  //  reported_track_quality->addStructureVariable("value", TYPE_USMALLINT, 1, "Reported track quality  Encoded values are NOT standardized", offset+offsetof(t_Reported_Track_Quality,value));

    //t_Reported_Track_Status reported_track_status;
                     /* Reported (radar) track status */
  //  offset = offsetof (t_Rtgt, reported_track_status);
  //  StructureDescription* reported_track_status = rdl_plot_description->addStructureDescription("reported_track_status", "Reported (radar) track quality", offset);
  //  reported_track_status->addStructureVariable("attitude", TYPE_USMALLINT, 1, "Vertical attitude or trend  0 = maintaining, 1 = climbing,  2 = descending, 3 = invalid", offset+offsetof(t_Reported_Track_Status,attitude));
  //  reported_track_status->addStructureVariable("coasted", TYPE_USMALLINT, 1, "Coasted (extrapolated) track", offset+offsetof(t_Reported_Track_Status,coasted));
  //  reported_track_status->addStructureVariable("confirmed", TYPE_USMALLINT, 1, "Confirmed track", offset+offsetof(t_Reported_Track_Status,confirmed));
  //  reported_track_status->addStructureVariable("doubtful_association", TYPE_USMALLINT, 1, "Doubtful Association", offset+offsetof(t_Reported_Track_Status,doubtful_association));
  //  reported_track_status->addStructureVariable("ghost", TYPE_USMALLINT, 1, "Ghost track", offset+offsetof(t_Reported_Track_Status,ghost));
  //  reported_track_status->addStructureVariable("horizontal_manoeuvre", TYPE_USMALLINT, 1, "Horizontal Manoeuvre", offset+offsetof(t_Reported_Track_Status,horizontal_manoeuvre));
  //  reported_track_status->addStructureVariable("present", TYPE_BOOL, 1, "Reported track quality present", offset+offsetof(t_Reported_Track_Status,present));
  //  reported_track_status->addStructureVariable("primary_track", TYPE_USMALLINT, 1, "Primary track", offset+offsetof(t_Reported_Track_Status,primary_track));
  //  reported_track_status->addStructureVariable("secondary_track", TYPE_USMALLINT, 1, "Second track", offset+offsetof(t_Reported_Track_Status,secondary_track));
  //  reported_track_status->addStructureVariable("smoothed", TYPE_USMALLINT, 1, "Smoothed position", offset+offsetof(t_Reported_Track_Status,smoothed));
  //  reported_track_status->addStructureVariable("tre", TYPE_USMALLINT, 1, "Last report for track", offset+offsetof(t_Reported_Track_Status,tre));

    //t_Sensor_Number sensor_number;
                     /* Sensor number */
    offset = offsetof (t_Rtgt, sensor_number);
    StructureDescription* sensor_number = rdl_plot_description->addStructureDescription("sensor_number", "Sensor number", offset);
    sensor_number->addPresentStructureVariable("present", SE_TYPE_BOOL, 1, "Sensor number present", offset+offsetof(t_Sensor_Number,present));
    sensor_number->addStructureVariable("value", SE_TYPE_INT, 1, "Sensor number  Must be from range 1 ... MAX_NUMBER_OF_RADARS", offset+offsetof(t_Sensor_Number,value));

    //t_Tres special_position_indication;
                     /* Special Position Indication flag */
    rdl_plot_description->addStructureVariable("special_position_indication", SE_TYPE_UTINYINT, 1, "Special Position Indication flag", offsetof(t_Rtgt,special_position_indication));

    //t_Measured_Amplitude ssr_amplitude;
                     /* Amplitude of SSR reply */
  //  offset = offsetof (t_Rtgt, ssr_amplitude);
  //  StructureDescription* ssr_amplitude = rdl_plot_description->addStructureDescription("ssr_amplitude", "Amplitude of SSR reply", offset);
  //  ssr_amplitude->addStructureVariable("present", TYPE_BOOL, 1, "Measured amplitude present", offset+offsetof(t_Measured_Amplitude,present));
  //  ssr_amplitude->addStructureVariable("value", TYPE_SMALLINT, 1, "Measured amplitude; dBm", offset+offsetof(t_Measured_Amplitude,value));

    //t_Replies_Count ssr_replies;
                     /* Number of SSR replies */
  //  offset = offsetof (t_Rtgt, ssr_replies);
  //  StructureDescription* ssr_replies = rdl_plot_description->addStructureDescription("ssr_replies", "Number of SSR replies", offset);
  //  ssr_replies->addStructureVariable("present", TYPE_BOOL, 1, "Replies count present", offset+offsetof(t_Replies_Count,present));
  //  ssr_replies->addStructureVariable("value", TYPE_USMALLINT, 1, "Replies count", offset+offsetof(t_Replies_Count,value));

    //t_Runlength ssr_runlength;
                     /* SSR plot runlength */
  //  offset = offsetof (t_Rtgt, ssr_runlength);
  //  StructureDescription* ssr_runlength = rdl_plot_description->addStructureDescription("ssr_runlength", "SSR plot runlength", offset);
  //  ssr_runlength->addStructureVariable("present", TYPE_BOOL, 1, "Runlength present", offset+offsetof(t_Runlength,present));
  //  ssr_runlength->addStructureVariable("value", TYPE_DOUBLE, 1, "Runlength; degrees", offset+offsetof(t_Runlength,value));
  //  ssr_runlength->addStructureVariable("value_in_acps", TYPE_USMALLINT, 1, "Runlength; ACPs", offset+offsetof(t_Runlength,value_in_acps));

    //t_Sweep_Angle sweep_angle;
                     /* Radar sweep angle */
  //  offset = offsetof (t_Rtgt, sweep_angle);
  //  StructureDescription* sweep_angle = rdl_plot_description->addStructureDescription("sweep_angle", "Radar sweep angle", offset);
  //  sweep_angle->addStructureVariable("present", TYPE_BOOL, 1, "Sweep angle present", offset+offsetof(t_Sweep_Angle,present));
  //  sweep_angle->addStructureVariable("value", TYPE_USMALLINT, 1, "Sweep angle; specific units", offset+offsetof(t_Sweep_Angle,value));

    //t_Aircraft_Address target_address;
                     /* Target address */
    offset = offsetof (t_Rtgt, target_address);
    StructureDescription* target_address = rdl_plot_description->addStructureDescription("target_address", "Target address", offset);
    target_address->addPresentStructureVariable("present", SE_TYPE_BOOL, 1, "Aircraft address present", offset+offsetof(t_Aircraft_Address,present));
    target_address->addStructureVariable("value", SE_TYPE_UINT, 1, "Aircraft address", offset+offsetof(t_Aircraft_Address,value));

    //t_Time_Delay time_delay;
                     /* Time delay, i. e., difference between receiving and
                        detection time */
  //  offset = offsetof (t_Rtgt, time_delay);
  //  StructureDescription* time_delay = rdl_plot_description->addStructureDescription("time_delay", "Time delay, i. e., difference between receiving and detection time", offset);
  //  time_delay->addStructureVariable("present", TYPE_BOOL, 1, "Time delay present", offset+offsetof(t_Time_Delay,present));
  //  time_delay->addStructureVariable("value", TYPE_UINT, 1, "Time delay, i. e., difference between receiving and detection time", offset+offsetof(t_Time_Delay,value));

    //t_TRI_Info tri_info;
                     /* Target Report Identifier (TRI) */
  //  offset = offsetof (t_Rtgt, tri_info);
  //  StructureDescription* tri_info = rdl_plot_description->addStructureDescription("tri_info", "Target Report Identifier (TRI)", offset);
  //  tri_info->addStructureVariable("digest", TYPE_UTINYINT, 16, "Message-digest (from MD5)", offset+offsetof(t_TRI_Info,digest));
  //  tri_info->addStructureVariable("present", TYPE_BOOL, 1, "Target Report Identifier present", offset+offsetof(t_TRI_Info,present));
  //  tri_info->addStructureVariable("value", TYPE_UINT, 1, "Value of TRI", offset+offsetof(t_TRI_Info,value));

    //t_Tres to_be_cancelled;
                     /* Radar track to be cancelled */
    //rdl_plot_description->addStructureVariable("to_be_cancelled", TYPE_UTINYINT, 1, "Radar track to be cancelled", offsetof(t_Rtgt,to_be_cancelled));

    //t_Bool to_be_listed;
                     /* Radar target report to be listed */
    //rdl_plot_description->addStructureVariable("to_be_listed", TYPE_BOOL, 1, "Radar target report to be listed", offsetof(t_Rtgt,to_be_listed));

    //t_Track_Number track_number;
                     /* Track number */
    offset = offsetof (t_Rtgt, track_number);
    StructureDescription* track_number = rdl_plot_description->addStructureDescription("track_number", "Track number", offset);
    track_number->addPresentStructureVariable("present", SE_TYPE_BOOL, 1, "Track number present", offset+offsetof(t_Track_Number,present));
    track_number->addStructureVariable("value", SE_TYPE_USMALLINT, 1, "Track number", offset+offsetof(t_Track_Number,value));

    //t_WEC_List wec_list;
                     /* List of warning/error conditions */
  //  offset = offsetof (t_Rtgt, wec_list);
  //  StructureDescription* wec_list = rdl_plot_description->addStructureDescription("wec_list", "List of warning/error conditions", offset);
  //  wec_list->addStructureVariable("count", TYPE_USMALLINT, 1, "Number of warning/error conditions", offset+offsetof(t_WEC_List,count));
  //  wec_list->addStructureVariable("list", TYPE_USMALLINT, M_MAX_NUMBER_OF_WEC, "List of warning/error conditions", offset+offsetof(t_WEC_List,list));
  //  wec_list->addStructureVariable("present", TYPE_BOOL, 1, "List of warning/error conditions present", offset+offsetof(t_WEC_List,present));

    //t_X_Pulses x_pulses;
                     /* X pulses */
  //  offset = offsetof (t_Rtgt, x_pulses);
  //  StructureDescription* x_pulses = rdl_plot_description->addStructureDescription("x_pulses", "X pulses", offset);
  //  x_pulses->addStructureVariable("present", TYPE_BOOL, 1, "X pulse(s) present", offset+offsetof(t_X_Pulses,present));
  //  x_pulses->addStructureVariable("value_m2", TYPE_USMALLINT, 1, "X pulse from SSR mode 2", offset+offsetof(t_X_Pulses,value_m2));
  //  x_pulses->addStructureVariable("value_m3a", TYPE_USMALLINT, 1, "X pulse from SSR mode 3/A", offset+offsetof(t_X_Pulses,value_m3a));
  //  x_pulses->addStructureVariable("value_mc", TYPE_USMALLINT, 1, "X pulse from SSR mode C", offset+offsetof(t_X_Pulses,value_mc));

    rdl_plot_description->addToFlatStructureDescription(this, "");
    delete rdl_plot_description;
  }
};


#endif /* STRUCTUREDESCRIPTIONPLOT_H_ */
