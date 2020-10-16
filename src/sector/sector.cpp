#include "sector.h"
#include "atsdb.h"
#include "evaluationmanager.h"
#include "evaluationtargetposition.h"

#include <cassert>

using namespace nlohmann;
using namespace std;

const string default_color{"#AAAAAA"};

Sector::Sector(unsigned int id, const std::string& name, const std::string& layer_name,
               std::vector<std::pair<double,double>> points)
    : id_(id), name_(name), layer_name_(layer_name), points_(points)
{
    color_str_ = default_color;

    createPolygon();
}

Sector::Sector(unsigned int id, const std::string& name, const std::string& layer_name,
               const std::string& json_str)
    : id_(id), name_(name), layer_name_(layer_name)
{
    json j = json::parse(json_str);

    assert (j.contains("id"));
    assert (j.at("id") == id_);

    assert (j.contains("name"));
    assert (j.at("name") == name_);

    assert (j.contains("layer_name"));
    assert (j.at("layer_name") == layer_name_);

    assert (j.contains("points"));
    json& points = j.at("points");
    assert (points.is_array());

    for (json& point_it : points.get<json::array_t>())
    {
        assert (point_it.is_array());
        assert (point_it.size() == 2);
        points_.push_back({point_it[0], point_it[1]});
    }

    has_min_altitude_ = j.contains("min_altitude");
    if (has_min_altitude_)
        min_altitude_ = j.at("min_altitude");

    has_max_altitude_ = j.contains("max_altitude");
    if (has_max_altitude_)
        max_altitude_ = j.at("max_altitude");

    if (!j.contains("color_str"))
        color_str_ = default_color;
    else
        color_str_ = j.at("color_str");

    createPolygon();
}

std::string Sector::name() const
{
    return name_;
}

std::string Sector::layerName() const
{
    return layer_name_;
}

nlohmann::json Sector::jsonData () const
{
    json j = json::object();

    j["id"] = id_;
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

    if (has_min_altitude_)
        j["min_altitude"] = min_altitude_;

    if (has_max_altitude_)
        j["max_altitude"] = max_altitude_;

    j["color_str"] = color_str_;

    return j;
}

std::string Sector::jsonDataStr() const
{
    return jsonData().dump();
}

const std::vector<std::pair<double, double>>& Sector::points() const
{
    return points_;
}

bool Sector::hasMinimumAltitude()
{
    return has_min_altitude_;
}

double Sector::minimumAltitude()
{
    assert (has_min_altitude_);
    return min_altitude_;
}
void Sector::minimumAltitude(double value)
{
    loginf << "Sector: minimumAltitude: " << value << "";

    has_min_altitude_ = true;
    min_altitude_ = value;

    save();
}

void Sector::removeMinimumAltitude()
{
    loginf << "Sector: removeMinimumAltitude";

    has_min_altitude_ = false;
    min_altitude_ = 0;

    save();
}

bool Sector::hasMaximumAltitude()
{
    return has_max_altitude_;
}
double Sector::maximumAltitude()
{
    assert (has_max_altitude_);
    return max_altitude_;
}
void Sector::maximumAltitude(double value)
{
    loginf << "Sector: maximumAltitude: " << value << "";

    has_max_altitude_ = true;
    max_altitude_ = value;

    save();
}

void Sector::removeMaximumAltitude()
{
    loginf << "Sector: removeMaximumAltitude";

    has_max_altitude_ = false;
    max_altitude_ = 0;

    save();
}

std::string Sector::colorStr()
{
    return color_str_;
}
void Sector::colorStr(std::string value)
{
    loginf << "Sector: colorStr: '" << value << "'";

    color_str_ = value;

    save();
}

void Sector::removeColorStr()
{
    color_str_ = default_color;

    save();
}

unsigned int Sector::id() const
{
    return id_;
}

void Sector::name(const std::string& name)
{
    loginf << "Sector: name: '" << name << "'";

    name_ = name;

    save();
}

void Sector::layerName(const std::string& layer_name)
{
    loginf << "Sector: layerName: '" << layer_name << "'";

    EvaluationManager& eval_man = ATSDB::instance().evaluationManager();

    string old_layer_name = layer_name_;
    layer_name_ = layer_name;

    eval_man.moveSector(id_, old_layer_name, layer_name); // moves and saves
}

void Sector::save()
{
    EvaluationManager& eval_man = ATSDB::instance().evaluationManager();

    eval_man.saveSector(id_);
}

bool Sector::isInside(const EvaluationTargetPosition& pos) const
{
    if (pos.has_altitude_)
    {
        if (has_min_altitude_ && pos.altitude_ < min_altitude_)
            return false;
        else if (has_max_altitude_ && pos.altitude_ > max_altitude_)
            return false;
    }

    OGRPoint ogr_pos (pos.latitude_, pos.longitude_);
    return ogr_polygon_->Contains(&ogr_pos);
}

void Sector::createPolygon()
{
    ogr_polygon_.reset(new OGRPolygon());
    //ogr_polygon_->addRingDirectly()
    OGRLinearRing* ring = new OGRLinearRing();

    for (auto& point_it : points_)
        ring->addPoint(point_it.first, point_it.second);

    ogr_polygon_->addRingDirectly(ring);
}

