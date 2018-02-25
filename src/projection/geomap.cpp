/*----------------------------------------------------------------------------*/
/* Project:        Analysis of Surveillance Systems (Alyss)                   */
/* File:           src/geomap.cpp                                             */
/* Contents:       Geodetical mapping functions                               */
/* Author(s):      kb                                                         */
/* Last change:    2016-12-13                                                 */
/*----------------------------------------------------------------------------*/
/* Copyright (c) 2012 - 2016 by Helmut Kobelbauer, Sinabelkirchen (Austria).  */
/* All rights reserved. Do not copy or distribute without expressed prior     */
/* written permission by the owner of this software.                          */
/*----------------------------------------------------------------------------*/

//#include "basics.hpp"
//                   // Basic system definitions
//#include "common.hpp"
//                   // Common declarations and definitions

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <cassert>

//#include "areas.hpp"
//                   // Aeronautical area definitions
//#include "chkass.hpp"
//                   // Checking assertions
//#if DBGAID
//#include "dbgaid.hpp"
//                   // Debugging aid services
//#endif
#include "geomap.h"
//                   // Geodetical mapping function definitions
//#include "utlfnc.hpp"
//                   // Utility function definitions

//                   // Local constants
//                   // ---------------

//static const char *this_file = "./src/geomap.cpp";
// Source file identification

static const t_Real earth_a = 6378137.0;
// Semi major axis of WGS-84 earth; metres
static const t_Real earth_b = 6356752.3142;
// Semi minor axis of WGS-84 earth; metres
static const t_Real earth_e1sq = 0.0066943800;
// Square of first excentricity of WGS-84 earth

/*----------------------------------------------------------------------------*/
/* utl_azimuth     -- Compute azimuth (in degrees)                            */
/*----------------------------------------------------------------------------*/

t_Real utl_azimuth (t_Real xpos, t_Real ypos)
{
    t_Real azf;

    // Suppress very small values to inhibit
    // arithmetic overflow/underflow
    if (std::fabs (xpos) <= 1.0e-6)
    {
        xpos = 0.0;
    }
    if (std::fabs (ypos) <= 1.0e-6)
    {
        ypos = 0.0;
    }

    // Prevent abnormal behaviour of atan2()
    if (xpos == 0.0 && ypos == 0.0)
    {
        azf = 0.0;
        goto done;
    }

    // Calculate azimuth
    azf = std::atan2 (xpos, ypos);

    // Convert from radians to degrees
    // and map to [0.0 .. 360.0[ domain
    azf = M_RAD2DEG * azf;
    if (azf < 0.0)
    {
        azf += 360.0;
    }

done:          // We are done
    return azf;
}

/*----------------------------------------------------------------------------*/
/* utl_azm_diff    -- Compute difference in azimuth (in degrees)              */
/*----------------------------------------------------------------------------*/

t_Real utl_azm_diff (t_Real azm1, t_Real azm2)
{
    t_Real da;     // Difference in azimuth; degrees
    t_Real ret_value;
    // Return value

    // Preset the return value
    ret_value = 0.0;

    // Check parameters
    assert (0.0 <= azm1 && azm1 < 360.0); //, "Invalid parameter");
    assert (0.0 <= azm2 && azm2 < 360.0); //, "Invalid parameter");

    // Compute difference in azimuth
    da = azm1 - azm2;

    // And normalize
    if (da <= -180.0)
    {
        da += 360.0;
    }
    else if (da > +180.0)
    {
        da -= 360.0;
    }

    // Check
    assert (-180.0 <= da && da < +180.0); //, "Invalid difference");

    // Set the return value
    ret_value = da;

    // We are done
    return ret_value;
}

/*----------------------------------------------------------------------------*/
/* utl_between     -- Determine if azimuth lies between given boundaries      */
/*----------------------------------------------------------------------------*/

bool utl_between (t_Real azm, t_Real lower, t_Real upper)
{
    bool ret;      // Return value

    // Preset return value
    ret = false;

    if (lower <= upper)
        // Simple case
    {
        if (lower <= azm && azm <= upper)
        {
            ret = true;
        }
    }
    else
        // Slightly more complex case
    {
        if (lower <= azm || azm <= upper)
        {
            ret = true;
        }
    }

    return ret;
}

/*----------------------------------------------------------------------------*/
/* utl_distance    -- Compute Euclidian distance                              */
/*----------------------------------------------------------------------------*/

t_Real utl_distance (t_Real dx, t_Real dy)
{
    return std::sqrt (dx * dx + dy * dy);
}

/*----------------------------------------------------------------------------*/
/* utl_distance    -- Compute Euclidian distance                              */
/*----------------------------------------------------------------------------*/

t_Real utl_distance (t_Real dx, t_Real dy, t_Real dz)
{
    return std::sqrt (dx * dx + dy * dy + dz * dz);
}

/*----------------------------------------------------------------------------*/
/* geo_calc_elv    -- Calculate elevation from height                         */
/*----------------------------------------------------------------------------*/

t_Retc geo_calc_elv
(t_Mapping_Info *info_ptr, t_Real rng, t_Real hgt, t_Real *elv_ptr)
{
    t_Real elv;    // Elevation of plot above radar plane; radians
    t_Real er;     // Local "best" earth radius; metres
    t_Real f;      // Auxiliary
    t_Real f1;     // Auxiliary
    t_Real f2;     // Auxiliary
    t_Real hs;     // Height of sensor; metres above WGS-84 ellipsoid
    t_Real ht;     // Height of target; metres above WGS-84 ellipsoid
    t_Retc ret;    // Return code

    // Preset the return code
    ret = RC_FAIL;

    // Check parameters
    assert (info_ptr != NULL); //, "Invalid parameter");
    assert (rng > 0.0); //, "Invalid parameter");
    assert (elv_ptr != NULL); //, "Invalid parameter");

    // Is the geodetical mapping information already defined ?
    assert (info_ptr->defined); //, "Mapping information not defined");

    // Set the "best" local earth radius
    er = info_ptr->best_radius;

    // Set the sensor's height
    hs = info_ptr->height;

    // Set the target (or whatever) height
    ht = hgt;

    // Compute auxiliaries
    f1 = (2.0 * er + hs + ht) * (ht - hs) - rng * rng;
    f2 = 2.0 * (er + hs) * rng;

    if (f2 <= 0.0)
    {
        goto done;
    }

    f = f1 / f2;

    // Check against allowable range [-1.0, +1.0]
    if (std::fabs (f) > 1.0)
    {
        goto done;
    }

    // Compute elevation
    elv = std::asin (f);

    // Store result value (in degrees)
    *elv_ptr = (180.0 / M_PI) * elv;

    // Set the return code
    ret = RC_OKAY;

done:          // We are done
    return ret;
}

/*----------------------------------------------------------------------------*/
/* geo_calc_info   -- Calculate geodetical mapping information                */
/*----------------------------------------------------------------------------*/

t_Retc geo_calc_info (t_GPos centre, t_Mapping_Info *info_ptr)
{
    t_Real alt;    // WGS-84 altitude of reference point; metres
    t_Real cos_lat;
    // Cosine of latitude
    t_Real cos_lon;
    // Cosine of longitude
    t_Real f;      // Auxiliary
    t_Real gn;     // The so-called "Grande Normale"; metres
    t_Real lat;    // Geodetical latitude of reference point; radians
    t_Real lon;    // Geodetical longitude of reference point; radians
    t_Retc ret;    // Return code
    t_Real sin_lat;
    // Sine of latitude
    t_Real sin_lon;
    // Sine of longitude

    // Preset the return code
    ret = RC_FAIL;

    // Check parameters
    assert (info_ptr != NULL); //, "Invalid parameter");

    // Centre point must be defined
    assert (centre.defined); //, "Centre point not defined");

    // Is the geodetical mapping information already defined ?
    assert (!(info_ptr->defined)); //, "Mapping information already defined");

    // Extract geodetical coordinates of centre point
    alt = centre.altitude;
    lat = centre.latitude;
    lon = centre.longitude;

    // Check geodetical latitude and longitude
    assert (-M_PI_HALF <= lat && lat <= +M_PI_HALF); //, "Bad latitude value");
    assert (-M_PI <= lon && lon <= +M_PI); //, "Bad longitude value");

    // Compute auxiliaries
    cos_lat = std::cos (lat);
    cos_lon = std::cos (lon);
    sin_lat = std::sin (lat);
    sin_lon = std::sin (lon);

    // Set the coefficients of the rotation matrix
    info_ptr->a11 = - sin_lon;
    info_ptr->a12 = - sin_lat * cos_lon;
    info_ptr->a13 = cos_lat * cos_lon;
    info_ptr->a21 = cos_lon;
    info_ptr->a22 = - sin_lat * sin_lon;
    info_ptr->a23 = cos_lat * sin_lon;
    info_ptr->a31 = 0.0;
    info_ptr->a32 = cos_lat;
    info_ptr->a33 = sin_lat;

    // Compute the so-called "Grande Normale"
    f = 1.0 - earth_e1sq * sin_lat * sin_lat;
    assert (f > 0.0); //, "Bad numerical value");
    gn = earth_a / std::sqrt (f);

    // Set the elements of the translation vector
    info_ptr->b1 = (gn + alt) * cos_lat * cos_lon;
    info_ptr->b2 = (gn + alt) * cos_lat * sin_lon;
    info_ptr->b3 = ((1.0 - earth_e1sq) * gn + alt) * sin_lat;

    // Compute the local "best" earth radius
    info_ptr->best_radius =
            earth_a * (1.0 - 0.5 * earth_e1sq * std::cos (2.0 * lat));

    // Store the so-called "Grande Normale"
    info_ptr->gn = gn;

    // Remember WGS-84 height of centre point
    info_ptr->height = alt;

    // The geodetical mapping information is defined
    info_ptr->defined = true;

    // Set the return code
    ret = RC_OKAY;

    // We are done
    return ret;
}

/*----------------------------------------------------------------------------*/
/* geo_grs_to_lcl  -- Map a GRS point to local coordinates                    */
/*----------------------------------------------------------------------------*/

t_Retc geo_grs_to_lcl
(t_Mapping_Info *info_ptr, t_CPos grs_pos, t_CPos *lcl_ptr)
{
    t_Real gx, gy, gz;
    // Auxiliaries
    t_Real lx, ly, lz;
    // Auxiliaries
    t_Retc ret;    // Return code

    // Preset the return code
    ret = RC_FAIL;

    // Check parameters
    assert (info_ptr != NULL); //, "Invalid parameter");
    assert (lcl_ptr != NULL); //, "Invalid parameter");

    // The mapping information must be defined
    assert (info_ptr->defined); //, "Mapping information not defined");

    // The GRS point must be defined
    assert (grs_pos.defined); //, "GRS point not defined");

    // Get GRS coordinates
    gx = grs_pos.value[M_CPOS_X];
    gy = grs_pos.value[M_CPOS_Y];
    gz = grs_pos.value[M_CPOS_Z];

    // Subtract translation vector
    gx -= info_ptr->b1;
    gy -= info_ptr->b2;
    gz -= info_ptr->b3;

    // Multiply with inverse of transformation matrix
    lx = info_ptr->a11 * gx + info_ptr->a21 * gy + info_ptr->a31 * gz;
    ly = info_ptr->a12 * gx + info_ptr->a22 * gy + info_ptr->a32 * gz;
    lz = info_ptr->a13 * gx + info_ptr->a23 * gy + info_ptr->a33 * gz;

    // Set return value
    lcl_ptr->defined = true;
    lcl_ptr->value[M_CPOS_X] = lx;
    lcl_ptr->value[M_CPOS_Y] = ly;
    lcl_ptr->value[M_CPOS_Z] = lz;

    // Set the return code
    ret = RC_OKAY;

    // We are done
    return ret;
}

/*----------------------------------------------------------------------------*/
/* geo_grs_to_llh  -- Map GRS coordinates to geodetical point                 */
/*----------------------------------------------------------------------------*/

t_Retc geo_grs_to_llh (t_CPos grs_pos, t_GPos *gpos_ptr)
{
    t_Real alt;    // Geographical altitude; metres
    t_Real cos_phi;
    // Auxiliary
    t_Real ex, ey, ez;
    // Auxiliaries
    t_Real f, f1, f2, f3;
    // Auxiliaries
    t_Real gn;     // So-called "grande normale"; metres
    t_Real lat;    // Geodetical latitude; degrees
    t_Real lon;    // Geodetical longitude; degrees
    t_Real phi;    // Auxiliary
    t_Real rho;    // Auxiliary
    t_Retc ret;    // Return code
    t_Real sin_lat;
    // Auxiliary
    t_Real sin_phi;
    // Auxiliary

    // Preset the return code
    ret = RC_FAIL;

    // Check parameters
    assert (gpos_ptr != NULL); //, "Invalid parameter");

    // The GRS point must be defined
    assert (grs_pos.defined); //, "GRS point not defined");

    // Get coordinate values
    ex = grs_pos.value[M_CPOS_X];
    ey = grs_pos.value[M_CPOS_Y];
    ez = grs_pos.value[M_CPOS_Z];

    // We are using Bowring's approximation

    rho = utl_distance (ex, ey);

    phi = utl_azimuth (ez * earth_a, rho * earth_b);

    sin_phi = std::sin (phi);
    cos_phi = std::cos (phi);

    f3 = earth_a / earth_b;
    f3 = f3 * f3 * earth_e1sq;

    f1 = ez + earth_b * f3 * sin_phi * sin_phi * sin_phi;
    f2 = rho - earth_a * earth_e1sq * cos_phi * cos_phi * cos_phi;

    lat = utl_azimuth (f1, f2);
    lon = utl_azimuth (ey, ex);

    if (lat > +90.0)
    {
        lat -= 360.0;
    }

    if (lon > +180.0)
    {
        lon -= 360.0;
    }

    sin_lat = std::sin (M_DEG2RAD * lat);
    f = 1.0 - earth_e1sq * sin_lat * sin_lat;
    assert (f > 0.0); //, "Domain violation");
    gn = earth_a / std::sqrt (f);

    alt = rho / std::cos (M_DEG2RAD * lat) - gn;

    // Set the return value
    gpos_ptr->altitude = alt;
    gpos_ptr->defined = true;
    gpos_ptr->latitude = M_DEG2RAD * lat;
    gpos_ptr->longitude = M_DEG2RAD * lon;

    // Set the return code
    ret = RC_OKAY;

    // We are done
    return ret;
}

/*----------------------------------------------------------------------------*/
/* geo_lcl_to_grs  -- Map local coordinates to GRS coordinates                */
/*----------------------------------------------------------------------------*/

t_Retc geo_lcl_to_grs
(t_Mapping_Info *info_ptr, t_CPos lcl_pos, t_CPos *grs_ptr)
{
    t_Real gx;     // GRS x coordinate; metres
    t_Real gy;     // GRS y coordinate; metres
    t_Real gz;     // GRS z coordinate; metres
    t_Real lx;     // Local x coordinate; metres
    t_Real ly;     // Local y coordinate; metres
    t_Real lz;     // Local z coordinate; metres
    t_Retc ret;    // Return code

    // Preset the return code
    ret = RC_FAIL;

    // Check parameters
    assert (info_ptr != NULL); //, "Invalid parameter");
    assert (grs_ptr != NULL); //, "Invalid parameter");

    // The local point must be defined
    assert (lcl_pos.defined); //, "Local point not defined");

    // Extract the local coordinates
    lx = lcl_pos.value[M_CPOS_X];
    ly = lcl_pos.value[M_CPOS_Y];
    lz = lcl_pos.value[M_CPOS_Z];

    // The mapping information must be defined
    assert (info_ptr->defined); //, "Mapping information not defined");

    // Multiply with rotation matrix
    gx = info_ptr->a11 * lx + info_ptr->a12 * ly + info_ptr->a13 * lz;
    gy = info_ptr->a21 * lx + info_ptr->a22 * ly + info_ptr->a23 * lz;
    gz = info_ptr->a31 * lx + info_ptr->a32 * ly + info_ptr->a33 * lz;

    // Add translation vector
    gx = gx + info_ptr->b1;
    gy = gy + info_ptr->b2;
    gz = gz + info_ptr->b3;

    // Store GRS coordinates
    grs_ptr->defined = true;
    grs_ptr->value[M_CPOS_X] = gx;
    grs_ptr->value[M_CPOS_Y] = gy;
    grs_ptr->value[M_CPOS_Z] = gz;

    // Set the return code
    ret = RC_OKAY;

    // We are done
    return ret;
}

/*----------------------------------------------------------------------------*/
/* geo_llh_to_grs  -- Map geodetical point to GRS coordinates                 */
/*----------------------------------------------------------------------------*/

t_Retc geo_llh_to_grs (t_GPos geo_pos, t_CPos *grs_ptr)
{
    t_Real alt;    // Geographical altitude; metres
    t_Real cos_lat;
    // Cosine of latitude
    t_Real cos_lon;
    // Cosine of longitude
    t_Real f;      // Auxiliary
    t_Real gn;     // So-called "grande normale"; metres
    t_Real gx;     // GRS x coordinate; metres
    t_Real gy;     // GRS y coordinate; metres
    t_Real gz;     // GRS z coordinate; metres
    t_Retc ret;    // Return code
    t_Real sin_lat;
    // Sine of latitude
    t_Real sin_lon;
    // Sine of longitude

    // Preset the return code
    ret = RC_FAIL;

    // Check geodetical point
    assert (geo_pos.defined); //, "Geodetical point not defined");
    assert (-M_PI_HALF <= geo_pos.latitude && geo_pos.latitude <= +M_PI_HALF); //, "Invalid parameter");
    assert (-M_PI <= geo_pos.longitude && geo_pos.longitude <= +M_PI); //,"Invalid parameter");
    assert (grs_ptr != NULL); //, "Invalid parameter");

    // Remember WGS-84 height
    alt = geo_pos.altitude;

    // Set auxiliaries
    cos_lat = std::cos (geo_pos.latitude);
    cos_lon = std::cos (geo_pos.longitude);
    sin_lat = std::sin (geo_pos.latitude);
    sin_lon = std::sin (geo_pos.longitude);

    // Compute the so-called "Grande Normale"
    f = 1.0 - earth_e1sq * sin_lat * sin_lat;
    assert (f > 0.0); //, "Domain violation");
    gn = earth_a / std::sqrt (f);

    // Compute GRS coordinates
    gx = (gn + alt) * cos_lat * cos_lon;
    gy = (gn + alt) * cos_lat * sin_lon;
    gz = ((1.0 - earth_e1sq) * gn + alt) * sin_lat;

    // Set the return value
    grs_ptr->defined = true;
    grs_ptr->value[M_CPOS_X] = gx;
    grs_ptr->value[M_CPOS_Y] = gy;
    grs_ptr->value[M_CPOS_Z] = gz;

    // Set the return code
    ret = RC_OKAY;

    // We are done
    return ret;
}

/*----------------------------------------------------------------------------*/
/* geo_lpc_to_lcl  -- Map local polar coordinates to local coordinates        */
/*----------------------------------------------------------------------------*/

t_Retc geo_lpc_to_lcl
(t_Real range, t_Real azimuth, t_Real elevation, t_CPos *local_ptr)
{
    t_Real lx;     // Local x coordinate; metres
    t_Real ly;     // Local y coordinate; metres
    t_Real lz;     // Local z coordinate; metres
    t_Retc ret;    // Return code

    // Preset the return code
    ret = RC_FAIL;

    // Check parameters
    assert (range > 0.0); //, "Invalid parameter");
    assert (0.0 <= azimuth && azimuth < M_TWO_PI); //, "Invalid parameter");
    assert (-M_PI <= elevation && elevation <= M_PI); //, "Invalid parameter");
    assert (local_ptr != NULL); //, "Invalid parameter");

    // Compute local coordinates
    lx = range * std::sin (azimuth) * std::cos (elevation);
    ly = range * std::cos (azimuth) * std::cos (elevation);
    lz = range * std::sin (elevation);

    // Store local coordinates
    local_ptr->defined = true;
    local_ptr->value[M_CPOS_X] = lx;
    local_ptr->value[M_CPOS_Y] = ly;
    local_ptr->value[M_CPOS_Z] = lz;

    // Set the return code
    ret = RC_OKAY;

    // We are done
    return ret;
}

/*----------------------------------------------------------------------------*/
/* preset_mapping_info -- Preset geodetical mapping information               */
/*----------------------------------------------------------------------------*/

void preset_mapping_info (t_Mapping_Info *info_ptr)
{
    // Check parameters
    assert (info_ptr != NULL); //, "Invalid parameter");

    // Preset
    info_ptr->a11 = 0.0;
    info_ptr->a12 = 0.0;
    info_ptr->a13 = 0.0;
    info_ptr->a21 = 0.0;
    info_ptr->a22 = 0.0;
    info_ptr->a23 = 0.0;
    info_ptr->a31 = 0.0;
    info_ptr->a32 = 0.0;
    info_ptr->a33 = 0.0;
    info_ptr->b1 = 0.0;
    info_ptr->b2 = 0.0;
    info_ptr->b3 = 0.0;
    info_ptr->best_radius = 0.0;
    info_ptr->defined = false;
    info_ptr->gn = 0.0;
    info_ptr->height = 0.0;

    // We are done
    return;
}

/*----------------------------------------------------------------------------*/
/* map_mch_to_hae  -- Map SSR mode C height to WGS-84 height                  */
/*----------------------------------------------------------------------------*/

//t_Real map_mch_to_hae (t_Real mch)
//{
//    t_Real hae;    // WGS-84 height (above ellipsoid); metres

//    // Map SSR mode C height
//    hae = mode_c_offset + (1.0 + mode_c_factor) * mch;

//    // We are done
//    return hae;
//}

/*----------------------------------------------------------------------------*/
/* preset_cpos     -- Preset Cartesian position                               */
/*----------------------------------------------------------------------------*/

void preset_cpos (t_CPos *item_ptr)
{
    // Check parameters
    assert (item_ptr != NULL); //, "Invalid parameter");

    // Preset this Cartesian position
    item_ptr->defined = false;
    item_ptr->value[M_CPOS_X] = 0.0;
    item_ptr->value[M_CPOS_Y] = 0.0;
    item_ptr->value[M_CPOS_Z] = 0.0;

    // We are done
    return;
}

/*----------------------------------------------------------------------------*/
/* preset_gpos     -- Preset geodetical position                              */
/*----------------------------------------------------------------------------*/

 void preset_gpos (t_GPos *item_ptr)
{
                   // Check parameters
    assert (item_ptr != NULL); //, "Invalid parameter");

                   // Preset this geodetical position
    item_ptr->altitude = 0.0;
    item_ptr->defined = false;
    item_ptr->latitude = 0.0;
    item_ptr->longitude = 0.0;

                   // We are done
    return;
}
// end-of-file
