#ifndef OGRCOORDINATESYSTEM_H
#define OGRCOORDINATESYSTEM_H

#include <ogr_spatialref.h>

#include <memory>

//class OGRProjection;

class OGRCoordinateSystem
{
public:
    OGRCoordinateSystem(unsigned int id, double latitude_deg, double longitude_deg, double altitude_m);
    virtual ~OGRCoordinateSystem();

    bool polarSlantToCartesian (double azimuth_rad, double slant_range_m, bool has_altitude, double altitude_baro_m,
                                double& x_pos_m, double& y_pos_m);

    bool polarHorizontalToCartesian (double azimuth_rad, double horizontal_range_m, double& x_pos_m, double& y_pos_m);

    /// @brief Projects geo-coordinate in WGS-84 to cartesian coordinate, returns false on error
    bool wgs842Cartesian (double latitude_deg, double longitude_deg, double& x_pos_m, double& y_pos_m);
    /// @brief Projects cartesian coordinate to geo-coordinate in WGS-84, returns false on error
    bool cartesian2WGS84 (double x_pos_m, double y_pos_m, double& latitude_deg, double& longitude_deg);

protected:
    unsigned int id_ {0};
    double latitude_deg_ {0};
    double longitude_deg_ {0};
    double altitude_m_ {0};

    OGRSpatialReference wgs84_;
    double wgs84_ellispoid_semi_major_ {0};
    double wgs84_ellispoid_semi_minor_ {0};

    OGRSpatialReference local_;

    std::unique_ptr<OGRCoordinateTransformation> ogr_geo2cart_;
    std::unique_ptr<OGRCoordinateTransformation> ogr_cart2geo_;

    // returns in m
    double getRadiusAt (double latitude_rad);
};

#endif // OGRCOORDINATESYSTEM_H
