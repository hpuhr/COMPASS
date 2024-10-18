/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "projectioncoordinatesystembase.h"

#include <Eigen/Dense>

class RS2GCoordinateSystem : public ProjectionCoordinateSystemBase
{
  public:
    RS2GCoordinateSystem(unsigned int id, double latitude_deg, double longitude_deg,
                         double altitude_m);

    bool calculateRadSlt2Geocentric(double azimuth_rad, double slant_range_m,
                                    bool has_altitude, double altitude_m, Eigen::Vector3d& geoc_pos);

    static bool geocentric2Geodesic(Eigen::Vector3d& input);

  protected:
    Eigen::Matrix3d rs2g_A_;

    Eigen::Matrix3d rs2g_T_Ai_;  // transposed matrix (depends on radar)
    Eigen::Vector3d rs2g_bi_;    // vector (depends on radar)

    double rs2g_ho_;             // height of COP
    double rs2g_Rto_;            // earth radius of tangent sphere at the COP

    Eigen::Matrix3d rs2g_A_p0q0_;
    Eigen::Vector3d rs2g_b_p0q0_;

    double rs2gAzimuth(double x_m, double y_m);
    // calculates elevation angle El using H (altitude of aircraft) and rho (slant range)

    void radarSlant2LocalCart(double azimuth_rad, double slant_range_m,
                              bool has_altitude, double altitude_m,
                              Eigen::Vector3d& local);
    void sysCart2SysStereo(Eigen::Vector3d& b, double* x, double* y);
    void localCart2Geocentric(Eigen::Vector3d& input);

    static void rs2gGeodesic2Geocentric(Eigen::Vector3d& input);

    static void rs2gFillMat(Eigen::Matrix3d& A, double lat, double lon);

    static void rs2gFillVec(Eigen::Vector3d& b, double lat, double lon, double height);
};


