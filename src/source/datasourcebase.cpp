#include "source/datasourcebase.h"
#include "logger.h"
#include "number.h"


using namespace Utils;
using namespace std;
using namespace nlohmann;

namespace dbContent
{


DataSourceBase::DataSourceBase()
{

}

std::string DataSourceBase::dsType() const
{
    return ds_type_;
}

void DataSourceBase::dsType(const std::string& ds_type)
{
    ds_type_ = ds_type;
}

unsigned int DataSourceBase::sac() const
{
    return sac_;
}

void DataSourceBase::sac(unsigned int sac)
{
    sac_ = sac;
}

unsigned int DataSourceBase::sic() const
{
    return sic_;
}

void DataSourceBase::sic(unsigned int sic)
{
    sic_ = sic;
}

unsigned int DataSourceBase::id() const
{
    return Number::dsIdFrom(sac(), sic());
}

std::string DataSourceBase::name() const
{
    return name_;
}

void DataSourceBase::name(const std::string &name)
{
    name_ = name;
}

bool DataSourceBase::hasShortName() const { return has_short_name_; }

void DataSourceBase::removeShortName()
{
    has_short_name_ = false;
    short_name_ = "";
}

void DataSourceBase::shortName(const std::string& short_name)
{
    loginf << "DataSourceBase " << name_ << ": shortName: " << short_name;
    has_short_name_ = true;
    this->short_name_ = short_name;
}

const std::string& DataSourceBase::shortName() const
{
    assert(has_short_name_);
    return short_name_;
}

void DataSourceBase::info(const std::string& info)
{
    info_ = json::parse(info);
}

nlohmann::json& DataSourceBase::info()
{
    return info_;
}

std::string DataSourceBase::infoStr()
{
    return info_.dump();
}

bool DataSourceBase::hasPosition() const
{
    return info_.contains("position");
}

bool DataSourceBase::hasFullPosition() const
{
    return info_.contains("position")
            && info_.at("position").contains("latitude")
            && info_.at("position").contains("longitude")
            && info_.at("position").contains("altitude");
}

void DataSourceBase::latitude (double value)
{
    info_["position"]["latitude"] = value;
}
double DataSourceBase::latitude () const
{
    assert (hasPosition());

    if (!info_.at("position").contains("latitude"))
        return 0.0;
    else
        return info_.at("position").at("latitude");
}

void DataSourceBase::longitude (double value)
{
    info_["position"]["longitude"] = value;
}
double DataSourceBase::longitude () const
{
    assert (hasPosition());

    if (!info_.at("position").contains("longitude"))
        return 0.0;
    else
        return info_.at("position").at("longitude");
}

void DataSourceBase::altitude (double value)
{
    info_["position"]["altitude"] = value;
}

double DataSourceBase::altitude () const
{
    assert (hasPosition());

    if (!info_.at("position").contains("altitude"))
        return 0.0;
    else
        return info_.at("position").at("altitude");
}

bool DataSourceBase::hasRadarRanges() const
{
    return info_.contains("radar_range");
}

void DataSourceBase::addRadarRanges()
{
    assert (!hasRadarRanges());
    info_["radar_range"] = json::object();
}

std::map<std::string, double> DataSourceBase::radarRanges() const
{
    assert (hasRadarRanges());
    return info_.at("radar_range").get<std::map<std::string, double>>();
}

void DataSourceBase::radarRange (const std::string& key, const double range)
{
    info_["radar_range"][key] = range;
}

bool DataSourceBase::hasNetworkLines() const
{
    return info_.contains("network_lines");
}

void DataSourceBase::addNetworkLines()
{
    assert (!hasNetworkLines());
    info_["network_lines"] = json::object();
}

std::map<std::string, std::pair<std::string, unsigned int>> DataSourceBase::networkLines() const
{
    assert (hasNetworkLines());

    std::map<std::string, std::pair<std::string, unsigned int>> ret;
    set<string> existing_lines; // to check

    const json& network_lines = info_.at("network_lines");
    assert (network_lines.is_object());

    string ip;
    unsigned int port;

    for (auto& line_it : network_lines.get<json::object_t>())  // iterate over array
    {
        assert (line_it.first == "L1" || line_it.first == "L2" || line_it.first == "L3" || line_it.first == "L4");

        assert (line_it.second.is_string());

        if (line_it.second.size() == 0) // empty string
            continue;

        ip = String::ipFromString(line_it.second);
        port = String::portFromString(line_it.second);

        if (existing_lines.count(ip+":"+to_string(port)))
        {
            logwrn << "DataSourceBase: networkLines: source " << name_
                   << " line " << ip << ":" << port
                   << " already in use";
        }
        else
            ret[line_it.first] = {ip, port};
    }

    return ret;
}

void DataSourceBase::networkLine (const std::string& key, const std::string ip_port)
{
    assert (key == "L1" || key == "L2" || key == "L3" || key == "L4");

    info_["network_lines"][key] = ip_port;
}

void DataSourceBase::setFromJSONDeprecated (const nlohmann::json& j)
{
    info_.clear();

    //    j["dbcontent_name"] = dbcontent_name_;
    assert(j.contains("dbcontent_name"));
    ds_type_ = j.at("dbcontent_name");


    //    j["name"] = name_;
    assert(j.contains("name"));
    name_ = j.at("name");

    //    if (has_short_name_)
    //        j["short_name"] = short_name_;
    if (j.contains("short_name"))
    {
        has_short_name_ = true;
        short_name_ = j.at("short_name");
    }
    else
        has_short_name_ = false;

    //    if (has_sac_)
    //        j["sac"] = sac_;
    assert(j.contains("sac"));
    sac_ = j.at("sac");

    //    if (has_sic_)
    //        j["sic"] = sic_;
    assert(j.contains("sic"));
    sic_ = j.at("sic");

    //    if (has_latitude_)
    //        j["latitude"] = latitude_;
    if (j.contains("latitude"))
        info_["position"]["latitude"] = j.at("latitude");

    //    if (has_longitude_)
    //        j["longitude"] = longitude_;
    if (j.contains("longitude"))
        info_["position"]["longitude"] = j.at("longitude");

    //    if (has_altitude_)
    //        j["altitude"] = altitude_;
    if (j.contains("altitude"))
        info_["position"]["altitude"] = j.at("altitude");

    //    // psr
    //    if (has_primary_azimuth_stddev_)
    //        j["primary_azimuth_stddev"] = primary_azimuth_stddev_;
    if (j.contains("primary_azimuth_stddev"))
        info_["radar_accuracy"]["primary_azimuth_stddev"] = j.at("primary_azimuth_stddev");

    //    if (has_primary_range_stddev_)
    //        j["primary_range_stddev"] = primary_range_stddev_;
    if (j.contains("primary_range_stddev"))
        info_["radar_accuracy"]["primary_range_stddev"] = j.at("primary_range_stddev");

    //    if (has_primary_ir_min_)
    //        j["primary_ir_min"] = primary_ir_min_;
    if (j.contains("primary_ir_min"))
        info_["radar_range"]["primary_ir_min"] = j.at("primary_ir_min");

    //    if (has_primary_ir_max_)
    //        j["primary_ir_max"] = primary_ir_max_;
    if (j.contains("primary_ir_max"))
        info_["radar_range"]["primary_ir_max"] = j.at("primary_ir_max");

    //    // ssr
    //    if (has_secondary_azimuth_stddev_)
    //        j["secondary_azimuth_stddev"] = secondary_azimuth_stddev_;
    if (j.contains("secondary_azimuth_stddev"))
        info_["radar_accuracy"]["secondary_azimuth_stddev"] = j.at("secondary_azimuth_stddev");

    //    if (has_secondary_range_stddev_)
    //        j["secondary_range_stddev"] = secondary_range_stddev_;
    if (j.contains("secondary_range_stddev"))
        info_["radar_accuracy"]["secondary_range_stddev"] = j.at("secondary_range_stddev");


    //    if (has_secondary_ir_min_)
    //        j["secondary_ir_min"] = secondary_ir_min_;
    if (j.contains("secondary_ir_min"))
        info_["radar_range"]["secondary_ir_min"] = j.at("secondary_ir_min");

    //    if (has_secondary_ir_max_)
    //        j["secondary_ir_max"] = secondary_ir_max_;
    if (j.contains("secondary_ir_max"))
        info_["radar_range"]["secondary_ir_max"] = j.at("secondary_ir_max");

    //    // mode s
    //    if (has_mode_s_azimuth_stddev_)
    //        j["mode_s_azimuth_stddev"] = mode_s_azimuth_stddev_;
    if (j.contains("mode_s_azimuth_stddev"))
        info_["radar_accuracy"]["mode_s_azimuth_stddev"] = j.at("mode_s_azimuth_stddev");

    //    if (has_mode_s_range_stddev_)
    //        j["mode_s_range_stddev"] = mode_s_range_stddev_;
    if (j.contains("mode_s_range_stddev"))
        info_["radar_accuracy"]["mode_s_range_stddev"] = j.at("mode_s_range_stddev");

    //    if (has_mode_s_ir_min_)
    //        j["mode_s_ir_min"] = mode_s_ir_min_;
    if (j.contains("mode_s_ir_min"))
        info_["radar_range"]["mode_s_ir_min"] = j.at("mode_s_ir_min");

    //    if (has_mode_s_ir_max_)
    //        j["mode_s_ir_max"] = mode_s_ir_max_;
    if (j.contains("secondary_ir_min"))
        info_["radar_range"]["secondary_ir_min"] = j.at("secondary_ir_min");

}

void DataSourceBase::setFromJSON (const nlohmann::json& j)
{
    //    "ds_type": "Radar",
    assert(j.contains("ds_type"));
    ds_type_ = j.at("ds_type");

    //    "has_short_name": true,
    //    "short_name": "dsgsd",
    if (j.contains("short_name"))
    {
        short_name_ = j.at("short_name");
        has_short_name_ = true;
    }
    else
        has_short_name_ = false;

    //    "info": {
    //        "position": {
    //            "altitude": 323.0,
    //            "latitude": 44.23
    //            "longitude": 134.18
    //        }
    //    },
    if (j.contains("info"))
        info_ = j.at("info");
    else
        info_.clear();

    //    "name": "sdgsdf",
    assert(j.contains("name"));
    name_ = j.at("name");

    //    "sac": 50,
    assert(j.contains("sac"));
    sac_ = j.at("sac");

    //    "sic": 0
    assert(j.contains("sic"));
    sic_ = j.at("sic");
}

}

