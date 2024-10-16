#pragma once

#include <GeographicLib/LocalCartesian.hpp>

#include <memory>

class GeoCoordinateSystem
{
public:
    GeoCoordinateSystem(unsigned int id, double latitude_deg, double longitude_deg,
                        double altitude_m);
    virtual ~GeoCoordinateSystem();

    bool polarSlantToCartesian(double azimuth_rad, double slant_range_m, bool has_altitude,
                               double altitude_baro_m, double& x_pos_m, double& y_pos_m);

    bool polarHorizontalToCartesian(double azimuth_rad, double horizontal_range_m, double& x_pos_m,
                                    double& y_pos_m);

    /// @brief Projects geo-coordinate in WGS-84 to cartesian coordinate, returns false on error
    bool wgs842Cartesian(double latitude_deg, double longitude_deg, double& x_pos_m,
                         double& y_pos_m);
    /// @brief Projects cartesian coordinate to geo-coordinate in WGS-84, returns false on error
    bool cartesian2WGS84(double x_pos_m, double y_pos_m, double& latitude_deg,
                         double& longitude_deg);

    // azimuth_deg as math angle
    bool wgs842PolarHorizontal(double latitude_deg, double longitude_deg, double& azimuth_deg, double& ground_range_m);

protected:
    unsigned int id_{0};
    double latitude_deg_{0};
    double longitude_deg_{0};
    double altitude_m_{0};

    mutable std::unique_ptr<GeographicLib::LocalCartesian> proj_;
};

