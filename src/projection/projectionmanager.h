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

#ifndef PROJECTIONMANAGER_H_
#define PROJECTIONMANAGER_H_

#include <ogr_spatialref.h>
#include "geomap.h"
#include "rs2g.h"

//#include <Eigen/Dense>

#include "configurable.h"
#include "singleton.h"

class ProjectionManagerWidget;

//typedef mtl::matrix<double,
//               mtl::rectangle<3,3>,
//               mtl::dense<>,
//               mtl::row_major>::type MatA;

//typedef Eigen::Matrix3d MatA;

//typedef mtl::dense1D<double> VecB;

//typedef Eigen::Vector2d VecB;

/**
 * @brief Singleton for coordinate projection handling
 *
 * Currently handles projection from world coordinates to Cartesian coordinates using the WGS-84 method.
 */
class ProjectionManager : public Singleton, public Configurable
{
protected:
    /// @brief Constructor
    ProjectionManager();

public:
    /// @brief Desctructor
    virtual ~ProjectionManager();

    /// @brief Projects cartesian coordinate to geo-coordinate in WGS-84, returns false on error
    bool sdlGRS2Geo (t_CPos grs_pos, t_GPos& geo_pos);

    /// @brief Projects geo-coordinate in WGS-84 to cartesian coordinate, returns false on error
    bool ogrGeo2Cart (double latitude, double longitude, double& x_pos, double& y_pos);
    /// @brief Projects cartesian coordinate to geo-coordinate in WGS-84, returns false on error
    bool ogrCart2Geo (double x_pos, double y_pos, double& latitude, double& longitude);

    std::string getWorldPROJ4Info ();
    void setNewCartesianEPSG (unsigned int epsg_value);
    std::string getCartesianPROJ4Info ();

    void createOGRProjection ();

    unsigned int getEPSG () { return epsg_value_; }

    ProjectionManagerWidget* widget ();

    void shutdown ();

    /// @brief Returns static instance
    static ProjectionManager& instance()
    {
        static ProjectionManager instance;
        return instance;
    }
    float sdlSystemLatitude() const;
    void sdlSystemLatitude(float sdl_system_latitude);

    float sdlSystemLongitude() const;
    void sdlSystemLongitude(float sdl_system_longitude);

    bool useSDLProjection() const;
    void useSDLProjection(bool use_sdl_projection);

    bool useOGRProjection() const;
    void useOGRProjection(bool use_ogr_projection);

    bool useRS2GProjection() const;
    void useRS2GProjection(bool use_rs2g_projection);

protected:
    bool use_sdl_projection_ {false};
    bool use_ogr_projection_ {false};
    bool use_rs2g_projection_ {false};

    float sdl_system_latitude_;
    float sdl_system_longitude_;
    t_Mapping_Info sdl_mapping_info_;

    unsigned int epsg_value_;
    OGRSpatialReference ogr_geo_;
    OGRSpatialReference ogr_cart_;

    OGRCoordinateTransformation* ogr_geo2cart_ {nullptr};
    OGRCoordinateTransformation* ogr_cart2geo_ {nullptr};

//    float rs2g_system_latitude_;
//    float rs2g_system_longitude_;
//    float rs2g_system_height_;

    //MatA* rs2g_A_;  // matrix (depends on current radar source selected)
    //VecB* rs2g_b_;  // vector (depends on current radar source selected)
//    MatA rs2g_R1_;  // matrix (depends on COP)
//    VecB rs2g_T1_;  // vector (depends on COP)
//    MatA rs2g_T_Ai_; // transposed matrix (depends on radar)
//    VecB rs2g_bi_;  // vector (depends on radar)
//    double rs2g_hi_; // height of selected radar
//    double rs2g_Rti_; // earth radius of tangent sphere at the selected radar
//    double rs2g_ho_; // height of COP
//    double rs2g_Rto_; // earth radius of tangent sphere at the COP

    ProjectionManagerWidget* widget_ {nullptr};

//    void initRS2G ();

//    void rs2gFillMat(MatA& A); //, Radar& radar);
//    void rs2gFillVec(VecB& b); //, Radar& radar);

//    void RadSlant2SysXY(double* x, double* y, double* z);
//    double Azimuth(double x, double y);
//    void RadarSlant2LocalCart(VecB& input);
//    void SysCart2SysStereo(VecB& b, double* x, double* y);
//    void Geodesic2Geocentric(VecB& input);
//    void LocalCart2Geocentric(VecB& input);

};

#endif /* PROJECTIONMANAGER_H_ */
