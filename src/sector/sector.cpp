#include "sector.h"

Sector::Sector(const std::string& name, const std::string& layer_name, std::vector<std::pair<double,double>> points)
    : name_(name), layer_name_(layer_name), points_(points)
{
}

Sector::Sector(const std::string& name, const std::string& layer_name, const std::string& json_str)
: name_(name), layer_name_(layer_name)
{
    // TODO points
}

std::string Sector::name() const
{
    return name_;
}

std::string Sector::layerName() const
{
    return layer_name_;
}

std::string Sector::jsonData () const
{
    // TODO
}
