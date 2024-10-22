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

#include "rs2gcoordinatesystem.h"

#include "global.h"
#include "logger.h"


RS2GCoordinateSystem::RS2GCoordinateSystem(unsigned int id, double latitude_deg,
                                           double longitude_deg, double altitude_m)
    : ProjectionCoordinateSystemBase(id, latitude_deg, longitude_deg, altitude_m)
{
    logdbg << "RS2GCoordinateSystem: init: id " << id_ << " lat " << latitude_deg_ << " long "
           << longitude_deg_ << " altitude " << altitude_m_;

    double lat_rad = latitude_deg_ * DEG2RAD;
    double long_rad = longitude_deg_ * DEG2RAD;

    rs2gFillMat(rs2g_A_, lat_rad, long_rad);

    rs2g_T_Ai_ = rs2g_A_.transpose();

    Eigen::Matrix3d A_p0(3, 3), A_q0(3, 3);
    Eigen::Vector3d b_p0(3), b_q0(3);

    rs2gFillMat(A_p0, lat_rad, long_rad);
    // mult(A_q0, trans(A_p0), *A_p0q0_); // A_p0q0 = A_q0 * Transpose(A_p0) in doxygen ...
    rs2g_A_p0q0_ = A_q0 * A_p0.transpose();
    //(*it).second.A_p0q0 = A_p0q0_;

    // #if defined(DEBUG_ARTAS_TRF)
    //       printf(" - radar matrix final values\n");
    //       print_all_matrix(*A_p0q0_);
    // #endif

    rs2gFillVec(b_p0, lat_rad, long_rad, altitude_m_);

    // add(scaled(b_q0, -1.0), b_p0, b_p0); // b_p0 = b_p0 - b_q0 in doxygen ...
    b_p0 = b_p0 - b_q0;

    // mult(A_q0, b_p0, *b_p0q0);    // b_p0q0 = A_q0 * (b_p0 - b_q0) in doxygen ...
    rs2g_b_p0q0_ = A_q0 * b_p0;

    // #if defined(DEBUG_ARTAS_TRF)
    //       printf(" - radar vector final values\n");
    //       print_vector(*b_p0q0);
    // #endif
    //    }

    // from setradar
    rs2gFillVec(rs2g_bi_, lat_rad, long_rad, altitude_m_);

    h_r_ = altitude_m_;
}

double RS2GCoordinateSystem::rs2gAzimuth(double x_m, double y_m)
{
    double azimuth_rad = 0.0;

    if (x_m == 0.0 && y_m == 0.0)
        return azimuth_rad;

    //    this is the implementation in ARTAS/COMSOFT
    //    IRS document Appendix A2 v6.5 2002/06/28
    //
    //    if (x != 0.0 || y != 0.0) {
    //       if (y == 0.0) {
    //          if (x > 0.0)
    //             azimuth = M_PI / 2.0;
    //          else
    //             azimuth = 3.0 * M_PI / 2.0;
    //       }
    //       else {
    //          azimuth = atan(x/y);
    //          if (y < 0.0)
    //             azimuth += M_PI;
    //          else {
    //             if (x < 0.0)
    //                azimuth += 2.0 * M_PI;
    //          }
    //       }
    //    }

    // this is an equivalent implementation to the above
    // and was taken from the TRANSLIB library function
    // 'azimuth' in file 'artas_trans.c'
    if (fabs(y_m) < ALMOST_ZERO)
        azimuth_rad = (x_m / fabs(x_m)) * M_PI / 2.0;
    else
        azimuth_rad = atan2(x_m, y_m);

    if (azimuth_rad < 0.0)
        azimuth_rad += 2.0 * M_PI;

    return azimuth_rad;
}

void RS2GCoordinateSystem::radarSlant2LocalCart(double azimuth_rad, double rho_m,
                                                bool has_altitude, double altitude_m,
                                                double& local_x, double& local_y, double& local_z)
{
    // logdbg << "radarSlant2LocalCart: in x: " << local[0] << " y: " << local[1]
    //        << " z: " << local[2];

    double elevation_m {0};

    if (has_altitude)
        elevation_m = altitude_m;
    else
        elevation_m = h_r_;  // the Z value has not been filled so use at least the radar height

    double elev_angle_rad = rs2gElevation(elevation_m, rho_m);

    //    if (rho < 50000)
    //    {
    //        loginf << "radarSlant2LocalCart: in x: " << local[0] << " y: " << local[1] << " z: "
    //        << local[2]; loginf << "radarSlant2LocalCart: rho: " << rho << " elevation: " <<
    //        elevation << " azimuth: " << azimuth;
    //    }

    local_x = rho_m * cos(elev_angle_rad) * sin(azimuth_rad);
    local_y = rho_m * cos(elev_angle_rad) * cos(azimuth_rad);
    local_z = rho_m * sin(elev_angle_rad);

    logdbg << "radarSlant2LocalCart: out x: " << local_x << " y: " << local_y
           << " z: " << local_z;
}

// void RS2GCoordinateSystem::sysCart2SysStereo(Eigen::Vector3d& b, double* x, double* y)
// {
//     double H = sqrt(pow(b[0], 2) + pow(b[1], 2) + pow(b[2] + rs2g_ho_ + rs2g_Rto_, 2)) - rs2g_Rto_;
//     double k = 2 * rs2g_Rto_ / (2 * rs2g_Rto_ + rs2g_ho_ + b[2] + H);

//     *x = k * b[0];
//     *y = k * b[1];
// }

void RS2GCoordinateSystem::localCart2Geocentric(double local_x, double local_y, double local_z,
                                                double& ecef_x, double& ecef_y, double& ecef_z)
{
    logdbg << "localCart2Geocentric: in x: " << local_x << " y:" << local_y << " z:" << local_z;

    Eigen::Vector3d local_pos(local_x, local_y, local_z);

    // local cartesian to geocentric
    // mult(T_Ai_, input, Xinput); // Xinput = Transposed(Ai_) * input
    Eigen::Vector3d Xinput = rs2g_T_Ai_ * local_pos;

    // add(bi_, Xinput, input); // Xinput = Xinput + bi_
    Xinput += rs2g_bi_;

    ecef_x = Xinput[0];
    ecef_y = Xinput[1];
    ecef_z = Xinput[2];

    logdbg << "localCart2Geocentric: out x: " << ecef_x << " y:" << ecef_y << " z:" << ecef_z;
}

bool RS2GCoordinateSystem::calculateRadSlt2Geocentric(double azimuth_rad, double slant_range_m,
                                                      bool has_altitude, double altitude_m,
                                                      double& ecef_x, double& ecef_y, double& ecef_z)
{
    Eigen::Vector3d local_pos(3);

    // the coordinates are in radar slant coordinates

    // radar slant to local cartesian
    double local_x, local_y, local_z;

    radarSlant2LocalCart(azimuth_rad, slant_range_m, has_altitude, altitude_m, local_x, local_y, local_z);

    // local cartesian to geocentric

    localCart2Geocentric(local_x, local_y, local_z, ecef_x, ecef_y, ecef_z);

    // geocentric to geodesic
    // Geocentric2Geodesic(pos);
    // done later

    return true;
}

void RS2GCoordinateSystem::rs2gGeodesic2Geocentric(double lat_rad, double lon_rad, double height_m,
                                                   double& ecef_x, double& ecef_y, double& ecef_z)
{
    double v = EE_A / sqrt(1 - EE_E2 * pow(sin(lat_rad), 2));

    ecef_x = (v + height_m) * cos(lat_rad) * cos(lon_rad);
    ecef_y = (v + height_m) * cos(lat_rad) * sin(lon_rad);
    ecef_z = (v * (1 - EE_E2) + height_m) * sin(lat_rad);
}

// as in R matrix
void RS2GCoordinateSystem::rs2gFillMat(double lat_rad, double lon_rad, Eigen::Matrix3d& A)
{
    A(0, 0) = -sin(lon_rad);
    A(0, 1) = cos(lon_rad);
    A(0, 2) = 0.0;
    A(1, 0) = -sin(lat_rad) * cos(lon_rad);
    A(1, 1) = -sin(lat_rad) * sin(lon_rad);
    A(1, 2) = cos(lat_rad);
    A(2, 0) = cos(lat_rad) * cos(lon_rad);
    A(2, 1) = cos(lat_rad) * sin(lon_rad);
    A(2, 2) = sin(lat_rad);
}

void RS2GCoordinateSystem::rs2gFillVec(double lat_rad, double lon_rad, double height_m, Eigen::Vector3d& b)
{
    rs2gGeodesic2Geocentric(lat_rad, lon_rad, height_m, b[0], b[1], b[2]);
}

bool RS2GCoordinateSystem::geocentric2Geodesic(double ecef_x, double ecef_y, double ecef_z,
                                               double& lat_deg, double& lon_deg, double& height_m)
{
    double d_xy = sqrt(pow(ecef_x, 2) + pow(ecef_y, 2));

    double G = atan2(ecef_y, ecef_x);

    double L = atan2(ecef_z, (d_xy * (1 - EE_A * EE_E2 / sqrt(pow(d_xy, 2) + pow(ecef_z, 2)))));

    double eta = EE_A / sqrt(1 - EE_E2 * pow(sin(L), 2));
    double H = d_xy / cos(L) - eta;

    double Li;
    if (L >= 0.0)
        Li = -0.1;
    else
        Li = 0.1;

    while (fabs(L - Li) > PRECISION_GEODESIC)
    {
        Li = L;
        L = atan2(ecef_z * (1 + H / eta), (d_xy * (1 - EE_E2 + H / eta)));

        eta = EE_A / sqrt(1 - EE_E2 * pow(sin(L), 2));
        H = d_xy / cos(L) - eta;
    }

    lat_deg = L * RAD2DEG;
    lon_deg = G * RAD2DEG;
    height_m = H;

    return !std::isnan(lat_deg) && !std::isnan(lon_deg);
}

// bool RS2GCoordinateSystem::geodesic2Geocentric(Eigen::Vector3d& input)
// {
    // double d_xy = sqrt(pow(input[0], 2) + pow(input[1], 2));

    // double G = atan2(input[1], input[0]);

    // double L = atan2(input[2], (d_xy * (1 - EE_A * EE_E2 / sqrt(pow(d_xy, 2) + pow(input[2], 2)))));

    // double eta = EE_A / sqrt(1 - EE_E2 * pow(sin(L), 2));
    // double H = d_xy / cos(L) - eta;

    // double Li;
    // if (L >= 0.0)
    //     Li = -0.1;
    // else
    //     Li = 0.1;

    // while (fabs(L - Li) > PRECISION_GEODESIC)
    // {
    //     Li = L;
    //     L = atan2(input[2] * (1 + H / eta), (d_xy * (1 - EE_E2 + H / eta)));

    //     eta = EE_A / sqrt(1 - EE_E2 * pow(sin(L), 2));
    //     H = d_xy / cos(L) - eta;
    // }

    // input[0] = L * RAD2DEG;
    // input[1] = G * RAD2DEG;
    // input[2] = H;

    // return !std::isnan(input[0]) && !std::isnan(input[1]);
//}

