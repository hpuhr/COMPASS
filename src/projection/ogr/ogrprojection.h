#ifndef OGRPROJECTION_H
#define OGRPROJECTION_H

#include "projection.h"

class ProjectionManager;
class OGRCoordinateSystem;

class OGRProjection : public Projection
{
  public:
    OGRProjection(const std::string& class_id, const std::string& instance_id,
                  ProjectionManager& proj_manager);
    virtual ~OGRProjection();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    virtual bool hasCoordinateSystem(unsigned int id);
    virtual void addCoordinateSystem(unsigned int id, double latitude_deg, double longitude_deg,
                                     double altitude_m);
    virtual bool polarToWGS84(unsigned int id, double azimuth_rad, double slant_range_m,
                              bool has_baro_altitude, double baro_altitude_ft, double& latitude,
                              double& longitude);

    //    std::string getWorldPROJ4Info ();
    //    void setNewCartesianEPSG (unsigned int epsg_value);
    //    std::string getCartesianPROJ4Info ();
    //    unsigned int getEPSG () { return epsg_value_; }

  protected:
    std::map<unsigned int, std::unique_ptr<OGRCoordinateSystem>> coordinate_systems_;

    // unsigned int epsg_value_;

    virtual void checkSubConfigurables();
};

#endif  // OGRPROJECTION_H
