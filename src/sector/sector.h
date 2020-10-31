#ifndef SECTOR_H
#define SECTOR_H

#include "json.hpp"

#include "gdal_priv.h"

class DBInterface;
class EvaluationTargetPosition;

class Sector
{
public:
    // should be protected?
    Sector(unsigned int id, const std::string& name, const std::string& layer_name,
           std::vector<std::pair<double,double>> points);
    Sector(unsigned int id, const std::string& name, const std::string& layer_name,
           const std::string& json_str);

    unsigned int id() const;

    std::string name() const;
    void name(const std::string& name);

    std::string layerName() const;
    void layerName(const std::string& layer_name);

    nlohmann::json jsonData() const;
    std::string jsonDataStr() const;

    unsigned int size () { return points_.size(); }

    const std::vector<std::pair<double, double>>& points() const;

    bool hasMinimumAltitude();
    double minimumAltitude();
    void minimumAltitude(double value);
    void removeMinimumAltitude();

    bool hasMaximumAltitude();
    double maximumAltitude();
    void maximumAltitude(double value);
    void removeMaximumAltitude();

    std::string colorStr();
    void colorStr(std::string value);
    void removeColorStr();

    void save();

    bool isInside(const EvaluationTargetPosition& pos) const;

    std::pair<double, double> getMinMaxLatitude() const;
    std::pair<double, double> getMinMaxLongitude() const;

protected:
    unsigned int id_;
    std::string name_;
    std::string layer_name_;

    std::vector<std::pair<double,double>> points_;

    bool has_min_altitude_ {false};
    double min_altitude_{0.0};

    bool has_max_altitude_ {false};
    double max_altitude_{0.0};

    std::string color_str_;

    std::unique_ptr<OGRPolygon> ogr_polygon_;

    void createPolygon();
};

#endif // SECTOR_H
