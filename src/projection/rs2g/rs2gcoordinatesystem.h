#ifndef RS2GCOORDINATESYSTEM_H
#define RS2GCOORDINATESYSTEM_H

#include <Eigen/Dense>

class RS2GCoordinateSystem
{
public:
    RS2GCoordinateSystem(unsigned int id, double latitude_deg, double longitude_deg, double altitude_m);

    bool calculateRadSlt2Geocentric (double x, double y, double z, Eigen::Vector3d& geoc_pos);

    static void geodesic2Geocentric(Eigen::Vector3d& input);

    static bool geocentric2Geodesic(Eigen::Vector3d& input);

protected:
    static const double EE_A;
    static const double EE_F;
    static const double EE_E2;
    static const double ALMOST_ZERO;
    static const double PRECISION_GEODESIC;

    unsigned int id_ {0};
    double latitude_deg_ {0};
    double longitude_deg_ {0};
    double altitude_m_ {0};

    Eigen::Matrix3d rs2g_A_;

    Eigen::Matrix3d rs2g_T_Ai_; // transposed matrix (depends on radar)
    Eigen::Vector3d rs2g_bi_;  // vector (depends on radar)
    double rs2g_hi_; // height of selected radar
    double rs2g_Rti_; // earth radius of tangent sphere at the selected radar
    double rs2g_ho_; // height of COP
    double rs2g_Rto_; // earth radius of tangent sphere at the COP

    Eigen::Matrix3d rs2g_A_p0q0_;
    Eigen::Vector3d rs2g_b_p0q0_;

    void init ();

    double rs2gAzimuth(double x, double y);
    double rs2gElevation(double z, double rho);
    void radarSlant2LocalCart(Eigen::Vector3d& local);
    void sysCart2SysStereo(Eigen::Vector3d& b, double* x, double* y);
    void localCart2Geocentric(Eigen::Vector3d& input);

    static void rs2gGeodesic2Geocentric(Eigen::Vector3d& input);

    static void rs2gFillMat(Eigen::Matrix3d& A, double lat, double lon);

    static void rs2gFillVec(Eigen::Vector3d& b, double lat, double lon, double height);
};

#endif // RS2GCOORDINATESYSTEM_H
