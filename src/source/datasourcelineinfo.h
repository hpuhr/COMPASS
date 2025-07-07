#ifndef DATASOURCELINEINFO_H
#define DATASOURCELINEINFO_H

#include "json_fwd.hpp"

class DataSourceLineInfo
{
public:
    DataSourceLineInfo(const std::string& key, nlohmann::json& config);


    bool hasListenIP() const;
    const std::string listenIP() const;
    void listenIP(const std::string& value);

    const std::string mcastIP() const;
    void mcastIP(const std::string& value);

    unsigned int mcastPort() const;
    void mcastPort(unsigned int value);

    bool hasSenderIP() const;
    const std::string senderIP() const;
    void senderIP(const std::string& value);

    std::string asString() const;

private:
    const std::string key_; // L1,L2,L3,L4
    nlohmann::json& config_;

//    listen: "123.324.21.24",           # optional, kann auch "0.0.0.0" oder "any" sein
    //    mcast: "239.192.21.24:8600", # notwendig
    //    sender: "139.192.21.24" # optional, wird nur gefiltert wenn gesetzt
};

#endif // DATASOURCELINEINFO_H
