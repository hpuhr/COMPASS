#include "datasourcelineinfo.h"
#include "number.h"

using namespace Utils;
using namespace std;
using namespace nlohmann;

const string listen_ip_key{"listen_ip"};
const string mcast_ip_key{"mcast_ip"};
const string mcast_port_key{"mcast_port"};
const string sender_ip_key{"sender_ip"};

DataSourceLineInfo::DataSourceLineInfo(const std::string& key, nlohmann::json& config)
    : key_(key), config_(config)
{
    assert (key_ == "L1" || key_ == "L2" || key_ == "L3" || key_ == "L4");

    if (config_.is_string()) // deprecated version
    {
        string value = config_;

        string ip = String::ipFromString(value);
        unsigned int port = String::portFromString(value);

        assert (ip.size());

        config_ = json::object();

        mcastIP(ip);
        mcastPort(port);

        logdbg << "DataSourceLineInfo: ctor: created info from deprecated " << asString()
               << " '" << config_.dump() << "'";
    }

    if (!config_.contains(mcast_ip_key))
        config_[mcast_ip_key] = "";

    if (!config_.contains(mcast_port_key))
        config_[mcast_port_key] = 0;
}

bool DataSourceLineInfo::hasListenIP() const
{
    return config_.contains(listen_ip_key);
}

const std::string DataSourceLineInfo::listenIP() const
{
    assert (hasListenIP());
    assert (config_.at(listen_ip_key).is_string());
    return config_.at(listen_ip_key);
}

void DataSourceLineInfo::listenIP(const std::string& value)
{
    config_[listen_ip_key] = value;
}

const std::string DataSourceLineInfo::mcastIP() const
{
    assert (config_.contains(mcast_ip_key));
    assert (config_.at(mcast_ip_key).is_string());
    return config_.at(mcast_ip_key);
}

void DataSourceLineInfo::mcastIP(const std::string& value)
{
    config_[mcast_ip_key] = value;
}

unsigned int DataSourceLineInfo::mcastPort() const
{
    assert (config_.contains(mcast_port_key));
    assert (config_.at(mcast_port_key).is_number());
    return config_.at(mcast_port_key);
}

void DataSourceLineInfo::mcastPort(unsigned int value)
{
    config_[mcast_port_key] = value;
}

bool DataSourceLineInfo::hasSenderIP() const
{
    return config_.contains(sender_ip_key);
}

const std::string DataSourceLineInfo::senderIP() const
{
    assert (hasSenderIP());
    assert (config_.at(sender_ip_key).is_string());
    return config_.at(sender_ip_key);
}

void DataSourceLineInfo::senderIP(const std::string& value)
{
    config_[sender_ip_key] = value;
}

std::string DataSourceLineInfo::asString() const
{
    std::stringstream ss;

    if (hasListenIP())
        ss << "listen " << listenIP() << ";";

    ss << "mcast " << mcastIP() << ":" << mcastPort();

    if (hasSenderIP())
        ss << ";sender " << senderIP();

    return ss.str();
}
