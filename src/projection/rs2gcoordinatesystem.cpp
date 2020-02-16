#include "rs2gcoordinatesystem.h"
#include "rs2g.h"
#include "logger.h"
#include "global.h"

RS2GCoordinateSystem::RS2GCoordinateSystem(unsigned int id, double latitude_deg, double longitude_deg,
                                           double altitude_m)
    : id_(id), latitude_deg_(latitude_deg), longitude_deg_(longitude_deg), altitude_m_(altitude_m)
{
    init ();
}

void RS2GCoordinateSystem::init()
{
    loginf << "RS2GCoordinateSystem: init: id " << id_ << " lat " << latitude_deg_ << " long " << longitude_deg_
           << " altitude " << altitude_m_;

    double lat_rad = latitude_deg_ * DEG2RAD;
    double long_rad = longitude_deg_ * DEG2RAD;

    rs2gFillMat(rs2g_A_, lat_rad, long_rad);

    rs2g_T_Ai_ = rs2g_A_.transpose();

    MatA A_p0(3,3), A_q0(3,3);
    VecB b_p0(3), b_q0(3);

    rs2g_Rti_ = EE_A * (1 - EE_E2) / sqrt(pow(1 - EE_E2 * pow(sin(lat_rad), 2), 3));
    //       printf(" - best earth radius:%g\n", (*it).second.rad->Rti());

    rs2gFillMat(A_p0, lat_rad, long_rad);
    //mult(A_q0, trans(A_p0), *A_p0q0_); // A_p0q0 = A_q0 * Transpose(A_p0) in doxygen ...
    rs2g_A_p0q0_ = A_q0 * A_p0.transpose();
    //(*it).second.A_p0q0 = A_p0q0_;

    // #if defined(DEBUG_ARTAS_TRF)
    //       printf(" - radar matrix final values\n");
    //       print_all_matrix(*A_p0q0_);
    // #endif

    rs2gFillVec(b_p0, lat_rad, long_rad, altitude_m_);

    //add(scaled(b_q0, -1.0), b_p0, b_p0); // b_p0 = b_p0 - b_q0 in doxygen ...
    b_p0 = b_p0 - b_q0;

    //mult(A_q0, b_p0, *b_p0q0);    // b_p0q0 = A_q0 * (b_p0 - b_q0) in doxygen ...
    rs2g_b_p0q0_ = A_q0 * b_p0;

    // #if defined(DEBUG_ARTAS_TRF)
    //       printf(" - radar vector final values\n");
    //       print_vector(*b_p0q0);
    // #endif
    //    }

    // from setradar
    rs2gFillVec(rs2g_bi_, lat_rad, long_rad, altitude_m_);

    rs2g_hi_ = altitude_m_;
}

double RS2GCoordinateSystem::rs2gAzimuth(double x, double y)
{
    double azimuth = 0.0;

    if (x == 0.0 && y == 0.0)
        return azimuth;

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
    if (fabs(y) < ALMOST_ZERO)
        azimuth = (x / fabs(x)) * M_PI / 2.0;
    else
        azimuth = atan2(x, y);

    if (azimuth < 0.0)
        azimuth += 2.0 * M_PI;

    return azimuth;
}

double RS2GCoordinateSystem::rs2gElevation(double z, double rho)
{
    double elevation = 0.0;

    if (rho >= ALMOST_ZERO) {
       double x = (2 * rs2g_Rti_ * (z - rs2g_hi_) + pow(z, 2) - pow(rs2g_hi_, 2) - pow(rho, 2)) /
               (2 * rho * (rs2g_Rti_ + rs2g_hi_));
       if (fabs(x) <= 1.0)
          elevation = asin(x);
    }

//    if (rho >= ALMOST_ZERO)
//    {
//        //        elevation = asin((2 * rs2g_Rti_ * (z - rs2g_hi_) + pow(z, 2) - pow(rs2g_hi_, 2) - pow(rho, 2)) /
//        //                         (2 * rho * (rs2g_Rti_ + rs2g_hi_)));
//        elevation = asin((z - rs2g_hi_)/rho);

//        //        if (rho < 50000)
//        //            loginf << "RS2GCoordinateSystem: rs2gElevation: z " << z << " rho " << rho << " elev " << elevation;
//    }

    return elevation;
}


void RS2GCoordinateSystem::radarSlant2LocalCart(VecB& local)
{
    logdbg << "radarSlant2LocalCart: in x: " << local[0] << " y: " << local[1] << " z: " << local[2];

    double z = local[2];
    if (z == -1000.0)
        z = rs2g_hi_; // the Z value has not been filled so use at least the radar height

    //double rho = sqrt(pow(local[0], 2) + pow(local[1], 2) + pow(z, 2));
    double rho = sqrt(pow(local[0], 2) + pow(local[1], 2));
    double elevation = rs2gElevation(z, rho);
    double azimuth = rs2gAzimuth(local[0], local[1]);

    //    if (rho < 50000)
    //    {
    //        loginf << "radarSlant2LocalCart: in x: " << local[0] << " y: " << local[1] << " z: " << local[2];
    //        loginf << "radarSlant2LocalCart: rho: " << rho << " elevation: " << elevation << " azimuth: " << azimuth;
    //    }

    local[0] = rho * cos(elevation) * sin(azimuth);
    local[1] = rho * cos(elevation) * cos(azimuth);
    local[2] = rho * sin(elevation);

    logdbg << "radarSlant2LocalCart: out x: " << local[0] << " y: " << local[1] << " z: " << local[2];
}

void RS2GCoordinateSystem::sysCart2SysStereo(VecB& b, double* x, double* y)
{
    double H = sqrt(pow(b[0], 2) + pow(b[1], 2) + pow(b[2] + rs2g_ho_ + rs2g_Rto_, 2)) - rs2g_Rto_;
    double k = 2 * rs2g_Rto_ / (2 * rs2g_Rto_ + rs2g_ho_ + b[2] + H);

    *x = k * b[0];
    *y = k * b[1];
}

void RS2GCoordinateSystem::localCart2Geocentric(VecB& input)
{
    logdbg << "localCart2Geocentric: in x: " << input[0] << " y:" << input[1] << " z:" << input[2];

    VecB Xinput(3);

    // local cartesian to geocentric
    //mult(T_Ai_, input, Xinput); // Xinput = Transposed(Ai_) * input
    Xinput = rs2g_T_Ai_ * input;

    //add(bi_, Xinput, input); // Xinput = Xinput + bi_
    input = Xinput + rs2g_bi_;

    logdbg << "localCart2Geocentric: out x: " << input[0] << " y:" << input[1] << " z:" << input[2];
}

bool RS2GCoordinateSystem::calculateRadSlt2Geocentric (double x, double y, double z, Eigen::Vector3d& geoc_pos)
{
    VecB Xlocal(3);  //, Xsystem(3);

    // the coordinates are in radar slant coordinates
    Xlocal[0] = x;
    Xlocal[1] = y;
    Xlocal[2] = z;

    // radar slant to local cartesian
    radarSlant2LocalCart(Xlocal);

    // local cartesian to geocentric
    localCart2Geocentric(Xlocal);

    geoc_pos = Xlocal;

    // geocentric to geodesic
    //Geocentric2Geodesic(Xlocal);
    // done later

    return true;
}
