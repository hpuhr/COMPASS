#ifndef SECTOR_H
#define SECTOR_H

#include "json.hpp"

class DBInterface;

class Sector
{
public:
    Sector(unsigned int id, const std::string& name, const std::string& layer_name,
           std::vector<std::pair<double,double>> points);
    Sector(unsigned int id, const std::string& name, const std::string& layer_name,
           const std::string& json_str);

    unsigned int id() const;

    std::string name() const;
    void name(const std::string& name);

    std::string layerName() const;
    void layerName(const std::string& layer_name);

    std::string jsonData() const;

    unsigned int size () { return points_.size(); }

    const std::vector<std::pair<double, double>>& points() const;

    bool hasMinimumAltitude();
    double minimumAltitude();
    void minimumAltitude(double value);

    bool hasMaximumAltitude();
    double maximumAltitude();
    void maximumAltitude(double value);

    bool hasColorStr();
    std::string colorStr();
    void colorStr(std::string value);


protected:
    unsigned int id_;
    std::string name_;
    std::string layer_name_;

    std::vector<std::pair<double,double>> points_;

    bool has_min_altitude_ {false};
    double min_altitude_{0.0};

    bool has_max_altitude_ {false};
    double max_altitude_{0.0};

    bool has_color_str_ {false};
    std::string color_str_;

    friend class DBInterface;

};

#endif // SECTOR_H
