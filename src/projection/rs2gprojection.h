#ifndef RS2GPROJECTION_H
#define RS2GPROJECTION_H

#include "projection.h"

class ProjectionManager;
class RS2GCoordinateSystem;

class RS2GProjection : public Projection
{
public:
    RS2GProjection(const std::string& class_id, const std::string& instance_id, ProjectionManager& proj_manager);
    virtual ~RS2GProjection();

    virtual void generateSubConfigurable (const std::string& class_id, const std::string& instance_id);

    virtual bool hasCoordinateSystem (unsigned int id);
    virtual void addCoordinateSystem (unsigned int id, double latitude_deg, double longitude_deg, double altitude_m);
    virtual bool polarToWGS84 (unsigned int id, double azimuth_rad, double slant_range_m, bool has_baro_altitude,
                               double baro_altitude_ft, double& latitude, double& longitude);

protected:

    std::map <unsigned int, std::unique_ptr<RS2GCoordinateSystem>> coordinate_systems_;

    virtual void checkSubConfigurables ();
};

#endif // RS2GPROJECTION_H
