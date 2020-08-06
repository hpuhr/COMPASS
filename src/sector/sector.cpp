#include "sector.h"

#include <cassert>

using namespace nlohmann;

Sector::Sector(const std::string& name, const std::string& layer_name, std::vector<std::pair<double,double>> points)
    : name_(name), layer_name_(layer_name), points_(points)
{
}

Sector::Sector(const std::string& name, const std::string& layer_name, const std::string& json_str)
: name_(name), layer_name_(layer_name)
{
    json j = json::parse(json_str);

    assert (j.contains("name"));
    assert (j.contains("layer_name"));
    assert (j.contains("points"));

    assert (j.at("name") == name_);
    assert (j.at("layer_name") == layer_name_);

    json& points = j.at("points");
    assert (points.is_array());

    for (json& point_it : points.get<json::array_t>())
    {
        assert (point_it.is_array());
        assert (point_it.size() == 2);
        points_.push_back({point_it[0], point_it[1]});
    }
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
    json j = json::object();

    j["name"] = name_;
    j["layer_name"] = layer_name_;

    j["points"] = json::array();

    unsigned int cnt = 0;
    for (auto& p_it : points_)
    {
        j["points"][cnt] = json::array();;
        j["points"][cnt][0] = p_it.first;
        j["points"][cnt][1] = p_it.second;
        ++cnt;
    }

    return j.dump();
}

const std::vector<std::pair<double, double>>& Sector::points() const
{
    return points_;
}

void Sector::name(const std::string& name)
{
    name_ = name;
}
