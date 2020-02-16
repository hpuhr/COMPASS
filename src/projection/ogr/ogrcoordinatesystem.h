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

    /// @brief Projects geo-coordinate in WGS-84 to cartesian coordinate, returns false on error
    bool ogrGeo2Cart (double latitude, double longitude, double& x_pos, double& y_pos);
    /// @brief Projects cartesian coordinate to geo-coordinate in WGS-84, returns false on error
    bool ogrCart2Geo (double x_pos, double y_pos, double& latitude, double& longitude);

protected:
    unsigned int id_ {0};
    double latitude_deg_ {0};
    double longitude_deg_ {0};
    double altitude_m_ {0};

    OGRSpatialReference wgs84_;
    OGRSpatialReference local_;

    std::unique_ptr<OGRCoordinateTransformation> ogr_geo2cart_;
    std::unique_ptr<OGRCoordinateTransformation> ogr_cart2geo_;
};

#endif // OGRCOORDINATESYSTEM_H
