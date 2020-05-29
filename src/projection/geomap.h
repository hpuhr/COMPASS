/*----------------------------------------------------------------------------*/
/* Project:        Analysis of Surveillance Systems (Alyss)                   */
/* File:           src/geomap.hpp                                             */
/* Contents:       Geodetical mapping function definitions                    */
/* Author(s):      kb                                                         */
/* Last change:    2014-03-02                                                 */
/*----------------------------------------------------------------------------*/
/* Copyright (c) 2012 - 2014 by Helmut Kobelbauer, Sinabelkirchen (Austria).  */
/* All rights reserved. Do not copy or distribute without expressed prior     */
/* written permission by the owner of this software.                          */
/*----------------------------------------------------------------------------*/

#ifndef INCLUDED_GEOMAP_HPP
#define INCLUDED_GEOMAP_HPP

//#ifndef INCLUDED_BASICS_HPP
//#include "basics.hpp"
//                   // Basic system definitions
//#endif

//#ifndef INCLUDED_COMMON_HPP
//#include "common.hpp"
//                   // Common macros and declarations
//#endif

// Data types
// ----------

typedef double t_Real;
typedef int t_Retc;

#define RC_DONE 2
#define RC_FAIL -1
#define RC_OKAY 0
#define RC_SKIP 1

// Geodetical position
typedef struct
{
    t_Real altitude;
    // Altitude, i. e., vertical distance between point
    // and the WGS-84 rotational ellipsoid along the
    // orthonormal from the point to this ellipsoid;
    // metres
    bool defined;
    // Geodetical position defined
    t_Real latitude;
    // Geodetical latitude; radians
    // By convention: positive means north half,
    // negative means south half of earth
    t_Real longitude;
    // Geodetical longitude; radians
    // By convention: positive means east of,
    // negative means west of zero meridian
} t_GPos;

// Cartesian position components
#define M_CPOS_NUM 3
// Number of components in Cartesian position
#define M_CPOS_X 0
// x component of Cartesian position
#define M_CPOS_Y 1
// y component of Cartesian position
#define M_CPOS_Z 2
// z component of Cartesian position

#define M_CPOS_U 0
// u component of stereographic position
#define M_CPOS_V 1
// v component of stereographic position
#define M_CPOS_H 2
// h component of stereographic position

// Cartesian position
typedef struct
{
    bool defined;
    // Cartesian position defined
    t_Real value[M_CPOS_NUM];
    // Cartesian position (x, y, z)
} t_CPos;

#define M_PI_HALF (0.5 * M_PI)
#define M_TWO_PI (2.0 * M_PI)

#define M_DEG2RAD (M_PI / 180.0)
// Conversion factor: from degrees to radians
#define M_EPSILON 1.0E-6
// A rather small value
#define M_FL2MTR 30.48
// Conversion factor: from FL to metres
#define M_FT2MTR 0.3048
// Conversion factor: from feet to metres
#define M_KILO 1024
// 2**10
#define M_MEGA 1048576
// 2**20
#define M_MPS2FPM (60.0 / M_FT2MTR)
// Conversion factor: from metres/second to feet/minute
#define M_MPS2KTS (3600.0 / 1852.0)
// Conversion factor: from metres/second to knots
#define M_NMI2MTR 1852.0
// Conversion factor: from nautical miles to metres
#define M_MTR2NMI (1.0 / 1852.0)
// Conversion factor: from metres to nautical miles
#define M_RAD2DEG (180.0 / M_PI)
// Conversion factor: from radians to degrees

// Geodetical mapping information
typedef struct
{
    t_Real a11, a12, a13, a21, a22, a23, a31, a32, a33;
    // Rotation matrix
    t_Real b1, b2, b3;
    // Translation vector
    t_Real best_radius;
    // Local "best" earth radius; metres
    bool defined;
    // Geodetical mapping information defined
    t_Real gn;
    // So-called "grande normale"; metres
    t_Real height;
    // WGS-84 height of radar or reference point
} t_Mapping_Info;

extern t_Real map_mch_to_hae(t_Real mch);
// Map SSR mode C height to WGS-84 height
extern void preset_cpos(t_CPos* item_ptr);
// Preset Cartesian position
extern void preset_gpos(t_GPos* item_ptr);
// Preset geodetical position

// Access functions:
// -----------------

extern t_Retc geo_calc_elv(t_Mapping_Info* info_ptr, t_Real rng, t_Real hgt, t_Real* elv_ptr);
// Calculate elevation from height
extern t_Retc geo_calc_info(t_GPos centre, t_Mapping_Info* info_ptr);
// Calculate geodetical mapping information
extern t_Retc geo_grs_to_lcl(t_Mapping_Info* info_ptr, t_CPos grs_pos, t_CPos* lcl_ptr);
// Map a GRS position to local coordinates
extern t_Retc geo_grs_to_llh(t_CPos grs_pos, t_GPos* gpos_ptr);
// Map GRS coordinates to geodetical point
extern t_Retc geo_lcl_to_grs(t_Mapping_Info* info_ptr, t_CPos lcl_pos, t_CPos* grs_ptr);
// Map local coordinates to GRS coordinates
extern t_Retc geo_llh_to_grs(t_GPos geo_pos, t_CPos* grs_ptr);
// Map geodetical point to GRS coordinates
extern t_Retc geo_lpc_to_lcl(t_Real range, t_Real azimuth, t_Real elevation, t_CPos* local_ptr);
// Map local polar coordinates to local coordinates
extern void preset_mapping_info(t_Mapping_Info* info_ptr);
// Preset geodetical mapping information

#endif  // INCLUDED_GEOMAP_HPP
// end-of-file
