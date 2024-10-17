#pragma once

class ProjectionCoordinateSystemBase
{
public:
    ProjectionCoordinateSystemBase(unsigned int id,
                                   double latitude_deg, double longitude_deg,
                                   double altitude_m)
        : id_(id), latitude_deg_(latitude_deg), longitude_deg_(longitude_deg), altitude_m_(altitude_m)
    {}

protected:

    unsigned int id_{0};
    double latitude_deg_{0};
    double longitude_deg_{0};
    double altitude_m_{0};
};
