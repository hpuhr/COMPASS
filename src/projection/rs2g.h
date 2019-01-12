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

#ifndef RS2G_H
#define RS2G_H

#include <Eigen/Dense>

const double EE_A = 6378137;  // earth ellipsoid major axis (m)

const double EE_F = 1.0 / 298.257223563;

const double EE_E2 = EE_F * (2 - EE_F); // earth ellipsoid excentricity

// conversion factors:
const double SEC2RAD = M_PI / (3600.0 * 180.0); // seconds to radians
//const double DEG2RAD = M_PI / 180.0; // degrees to radians
const double FL2M    = 30.48; // flight levels to metres
const double NM2M    = 1852; // NM to meters
//const double RAD2DEG = 180.0 / M_PI; // radians to degrees
const double M_S2KNOTS = 3600.0 / 1852.0; // metres per second to knots

const double FT2M = 0.3048;

const double ALMOST_ZERO = 1e-10;

const double PRECISION_GEODESIC = 1e-8;

typedef Eigen::Matrix3d MatA;
typedef Eigen::Vector3d VecB;

extern void rs2gGeodesic2Geocentric(VecB& input);

extern void rs2gFillMat(MatA& A, double lat, double lon);

extern void rs2gFillVec(VecB& b, double lat, double lon, double height);

extern void geodesic2Geocentric(VecB& input);

extern bool geocentric2Geodesic(VecB& input);

#endif // RS2G_H
