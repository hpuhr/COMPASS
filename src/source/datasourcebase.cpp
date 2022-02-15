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

bool DataSourceBase::hasPosition()
{
    return info_.contains("position");
}

bool DataSourceBase::hasFullPosition()
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
double DataSourceBase::latitude ()
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
double DataSourceBase::longitude ()
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

double DataSourceBase::altitude ()
{
    assert (hasPosition());

    if (!info_.at("position").contains("altitude"))
        return 0.0;
    else
        return info_.at("position").at("altitude");
}

bool DataSourceBase::hasNetworkLines()
{
    return info_.contains("network_lines");
}

std::map<std::string, std::pair<std::string, unsigned int>> DataSourceBase::networkLines()
{
    assert (hasNetworkLines());

    std::map<std::string, std::pair<std::string, unsigned int>> ret;
    set<string> existing_lines; // to check

    json& network_lines = info_.at("network_lines");
    assert (network_lines.is_object());

    string ip;
    unsigned int port;

    for (auto& line_it : network_lines.get<json::object_t>())  // iterate over array
    {
        assert (line_it.first == "L1" || line_it.first == "L2" || line_it.first == "L3" || line_it.first == "L4");

        assert (line_it.second.is_string());

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

}

