#pragma once

#include "global.h"

class ProjectionCoordinateSystemBase
{
public:
    ProjectionCoordinateSystemBase(unsigned int id,
                                   double latitude_deg, double longitude_deg,
                                   double altitude_m);


    double rs2gElevation(double H, double rho);

    void getGroundRange(double slant_range_m,
                          bool has_altitude, double altitude_m,
                          double& ground_range_m, double& adjusted_altitude_m, bool debug=false);

protected:
    // SSS D.2
    static const double EE_A; //  WGS−84 geoid in m
    static const double EE_F;
    static const double EE_E2;
    static const double ALMOST_ZERO;
    static const double PRECISION_GEODESIC;

    unsigned int id_{0};
    double latitude_deg_{0};
    double longitude_deg_{0};
    double altitude_m_{0};

    double h_r_;                 // height of selected radar
    double R_T_;                 // earth radius of tangent sphere at the selected radar

};
