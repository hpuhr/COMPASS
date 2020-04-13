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

#ifndef STRUCTUREDESCRIPTIONREFERENCETRAJECTORY_H_
#define STRUCTUREDESCRIPTIONREFERENCETRAJECTORY_H_

#include <cstddef>

#include "StructureDescription.h"
#include "SystemTrack.h"

/**
 * @brief Description for Reference trajectory data struct t_Strck
 *
 * Creates a temporary description, and adds flattened contents to itself
 */
class StructureDescriptionReferenceTrajectory : public StructureDescription
{
  public:
    StructureDescriptionReferenceTrajectory()
        : StructureDescription(DBO_REFERENCE_TRAJECTORIES, "ReferenceTrajectory",
                               "RDL t_Strck structure", 0)
    {
        create();
    }
    virtual ~StructureDescriptionReferenceTrajectory() {}

  protected:
    void create()
    {
        StructureDescription* ref_traj_desc =
            new StructureDescription(DBO_REFERENCE_TRAJECTORIES, "", "rdl t_Strk structure", 0);

        // t_Computed_Velocity calculated_cartesian_velocity;
        /* Calculated Cartesian velocity */
        size_t offset = offsetof(t_Strk, calculated_cartesian_velocity);
        StructureDescription* calc_vel = ref_traj_desc->addStructureDescription(
            "calculated_cartesian_velocity", "Calculated Cartesian velocity", offset);
        calc_vel->addPresentStructureVariable("present", SE_TYPE_BOOL, 1,
                                              "Computed velocity present",
                                              offset + offsetof(t_Computed_Velocity, present));
        calc_vel->addStructureVariable("value_vx", SE_TYPE_DOUBLE, 1,
                                       "Computed x component; metres/second",
                                       offset + offsetof(t_Computed_Velocity, value_vx));
        calc_vel->addStructureVariable("value_vy", SE_TYPE_DOUBLE, 1,
                                       "Computed y component; metres/second",
                                       offset + offsetof(t_Computed_Velocity, value_vy));

        // t_Strk_Vertical_Rate calculated_rate_of_climb_descent;
        /* Calculated rate of climb/descent */
        offset = offsetof(t_Strk, calculated_rate_of_climb_descent);
        StructureDescription* calc_climb = ref_traj_desc->addStructureDescription(
            "calculated_rate_of_climb_descent", "Calculated rate of climb/descent", offset);
        calc_climb->addPresentStructureVariable("present", SE_TYPE_BOOL, 1,
                                                "Calculated vertical rate present",
                                                offset + offsetof(t_Strk_Vertical_Rate, present));
        calc_climb->addStructureVariable("value", SE_TYPE_DOUBLE, 1,
                                         "Calculated vertical rate; metres/second",
                                         offset + offsetof(t_Strk_Vertical_Rate, value));

        // t_Computed_Position calculated_position;
        /* Calculated Cartesian position */
        offset = offsetof(t_Strk, calculated_position);
        StructureDescription* calc_pos = ref_traj_desc->addStructureDescription(
            "calculated_position", "Calculated Cartesian position", offset);
        calc_pos->addPresentStructureVariable("present", SE_TYPE_BOOL, 1,
                                              "Computed position present",
                                              offset + offsetof(t_Computed_Position, present));
        calc_pos->addStructureVariable("value_x", SE_TYPE_DOUBLE, 1,
                                       "Computed x coordinate; metres",
                                       offset + offsetof(t_Computed_Position, value_x));
        calc_pos->addStructureVariable("value_y", SE_TYPE_DOUBLE, 1,
                                       "Computed y coordinate; metres",
                                       offset + offsetof(t_Computed_Position, value_y));

        // t_Bool observed_by_psr;
        /* Observed by PSR */
        ref_traj_desc->addStructureVariable("observed_by_psr", SE_TYPE_BOOL, 1, "Observed by PSR",
                                            offsetof(t_Strk, observed_by_psr));

        // t_Bool observed_by_ssr;
        /* Observed by SSR */
        ref_traj_desc->addStructureVariable("observed_by_ssr", SE_TYPE_BOOL, 1, "Observed by SSR",
                                            offsetof(t_Strk, observed_by_ssr));

        // t_Bool track_terminated;
        /* Track terminated */
        ref_traj_desc->addStructureVariable("track_terminated", SE_TYPE_BOOL, 1, "Track terminated",
                                            offsetof(t_Strk, track_terminated));

        // t_Bool track_created;
        /* Track created */
        ref_traj_desc->addStructureVariable("track_created", SE_TYPE_BOOL, 1, "Track created",
                                            offsetof(t_Strk, track_created));

        // t_Track_Number track_number;
        /* Track number */
        offset = offsetof(t_Strk, track_number);
        StructureDescription* track_num =
            ref_traj_desc->addStructureDescription("track_number", "Track number", offset);
        track_num->addPresentStructureVariable("present", SE_TYPE_BOOL, 1, "Track number present",
                                               offset + offsetof(t_Track_Number, present));
        track_num->addStructureVariable("value", SE_TYPE_USMALLINT, 1, "Track number",
                                        offset + offsetof(t_Track_Number, value));

        // t_Time_of_Day time_of_last_update;
        /* Time of last update */
        offset = offsetof(t_Strk, time_of_last_update);
        StructureDescription* time_up = ref_traj_desc->addStructureDescription(
            "time_of_last_update", "Time of last update", offset);
        time_up->addPresentStructureVariable("present", SE_TYPE_BOOL, 1, "Time of day present",
                                             offset + offsetof(t_Time_of_Day, present));
        time_up->addStructureVariable("value", SE_TYPE_DOUBLE, 1, "Time of day; seconds",
                                      offset + offsetof(t_Time_of_Day, value));

        // t_Strk_Mode_Info mode_3a_info;
        /* SSR mode 3A information */
        offset = offsetof(t_Strk, mode_3a_info);
        StructureDescription* mode_3a_info =
            ref_traj_desc->addStructureDescription("mode_3a_info", "Time of last update", offset);
        mode_3a_info->addStructureVariable("code", SE_TYPE_USMALLINT, 1, "SSR mode code",
                                           offset + offsetof(t_Strk_Mode_Info, code));
        mode_3a_info->addStructureVariable("code_changed", SE_TYPE_BOOL, 1, "SSR mode code changed",
                                           offset + offsetof(t_Strk_Mode_Info, code_changed));
        mode_3a_info->addStructureVariable("code_garbled", SE_TYPE_BOOL, 1, "SSR mode code garbled",
                                           offset + offsetof(t_Strk_Mode_Info, code_garbled));
        mode_3a_info->addStructureVariable("code_invalid", SE_TYPE_BOOL, 1, "SSR mode code invalid",
                                           offset + offsetof(t_Strk_Mode_Info, code_invalid));
        mode_3a_info->addPresentStructureVariable("present", SE_TYPE_BOOL, 1,
                                                  "SSR mode information present",
                                                  offset + offsetof(t_Strk_Mode_Info, present));

        // t_Flight_Level measured_track_mode_c_height;
        /* Measured track mode C height */
        offset = offsetof(t_Strk, measured_track_mode_c_height);
        StructureDescription* measured_track_mode_c_height = ref_traj_desc->addStructureDescription(
            "measured_track_mode_c_height", "Measured track mode C height", offset);
        measured_track_mode_c_height->addPresentStructureVariable(
            "present", SE_TYPE_BOOL, 1, "Flight level present",
            offset + offsetof(t_Flight_Level, present));
        measured_track_mode_c_height->addStructureVariable(
            "value_in_feet", SE_TYPE_INT, 1, "Flight level; metres",
            offset + offsetof(t_Flight_Level, value_in_feet));

        // t_Frame_Time frame_time;
        /* Frame time */
        offset = offsetof(t_Strk, frame_time);
        StructureDescription* frame_time =
            ref_traj_desc->addStructureDescription("frame_time", "Frame time", offset);
        frame_time->addPresentStructureVariable("present", SE_TYPE_BOOL, 1, "Frame time present",
                                                offset + offsetof(t_Frame_Time, present));
        frame_time->addStructureVariable("value", SE_TYPE_DOUBLE, 1, "Frame time (of day); seconds",
                                         offset + offsetof(t_Frame_Time, value));

        // t_Bool callsign_present;
        /* Callsign present */
        // ref_traj_desc->addStructureVariable("callsign_present", TYPE_BOOL, 1, "Callsign present",
        // offsetof(t_Strk,callsign_present));

        // t_Callsign callsign;
        /* Callsign */
        offset = offsetof(t_Strk, callsign);
        StructureDescription* callsign =
            ref_traj_desc->addStructureDescription("callsign", "Callsign", offset);
        callsign->addPresentStructureVariable("present", SE_TYPE_BOOL, 1, "Callsign present",
                                              offset + offsetof(t_Callsign, present));
        callsign->addStructureVariable("value", SE_TYPE_VARCHAR_ARRAY, 8, "Callsign",
                                       offset + offsetof(t_Callsign, value));

        // t_Bool special_position_indication;
        /* Special position indication */
        ref_traj_desc->addStructureVariable("special_position_indication", SE_TYPE_BOOL, 1,
                                            "Special position indication",
                                            offsetof(t_Strk, special_position_indication));

        // t_Aircraft_Address aircraft_address;
        /* Technical SSR mode S address */
        offset = offsetof(t_Strk, aircraft_address);
        StructureDescription* aircraft_address = ref_traj_desc->addStructureDescription(
            "aircraft_address", "Technical SSR mode S address", offset);
        aircraft_address->addPresentStructureVariable("present", SE_TYPE_BOOL, 1, "address present",
                                                      offset + offsetof(t_Callsign, present));
        aircraft_address->addStructureVariable("value", SE_TYPE_UINT, 1, "Aircraft address",
                                               offset + offsetof(t_Callsign, value));

        ref_traj_desc->addToFlatStructureDescription(this, "");
        delete ref_traj_desc;
    }
};

#endif /* STRUCTUREDESCRIPTIONREFERENCETRAJECTORY_H_ */
