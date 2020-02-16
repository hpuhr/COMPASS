#ifndef RS2GCOORDINATESYSTEM_H
#define RS2GCOORDINATESYSTEM_H

#include "rs2g.h"

class RS2GCoordinateSystem
{
public:
    RS2GCoordinateSystem(unsigned int id, double latitude_deg, double longitude_deg, double altitude_m);

    bool calculateRadSlt2Geocentric (double x, double y, double z, Eigen::Vector3d& geoc_pos);

protected:
    unsigned int id_ {0};
    double latitude_deg_ {0};
    double longitude_deg_ {0};
    double altitude_m_ {0};

    //bool finalized_ {false};

    MatA rs2g_A_;

    MatA rs2g_T_Ai_; // transposed matrix (depends on radar)
    VecB rs2g_bi_;  // vector (depends on radar)
    double rs2g_hi_; // height of selected radar
    double rs2g_Rti_; // earth radius of tangent sphere at the selected radar
    double rs2g_ho_; // height of COP
    double rs2g_Rto_; // earth radius of tangent sphere at the COP

    MatA rs2g_A_p0q0_;
    VecB rs2g_b_p0q0_;

    void init ();

    double rs2gAzimuth(double x, double y);
    double rs2gElevation(double z, double rho);
    void radarSlant2LocalCart(VecB& local);
    void sysCart2SysStereo(VecB& b, double* x, double* y);
    void localCart2Geocentric(VecB& input);
};

#endif // RS2GCOORDINATESYSTEM_H
