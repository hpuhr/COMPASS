#include "target.h"

namespace dbContent {


Target::Target(unsigned int utn, nlohmann::json info)
    : utn_(utn), info_(info)
{
    if (!info_.contains("used"))
        info_["used"] = true;
}

bool Target::use() const
{
    return info_.at("used");
}

void Target::use(bool value)
{
    info_["used"] = value;
}

std::string Target::comment() const
{
    if (!info_.contains("comment"))
        return "";

    return info_.at("comment");
}

void Target::comment (const std::string& value)
{
    info_["comment"] = value;
}

std::set<unsigned int> Target::tas()
{
    return info_.at("acads").get<std::set<unsigned int>>();
}
void Target::tas(const std::set<unsigned int>& tas)
{
    info_["acads"] = tas;
}

std::set<unsigned int> Target::mas()
{
    return info_.at("m3as").get<std::set<unsigned int>>();
}
void Target::mas(const std::set<unsigned int>& mas)
{
    info_["m3as"] = mas;
}

unsigned int Target::dbContentCount(const std::string& dbcontent_name)
{
    if (info_.contains("dbcontent_counts") && info_.at("dbcontent_counts").contains(dbcontent_name))
        return info_.at("dbcontent_counts").at(dbcontent_name);
    else
        return 0;
}

void Target::dbContentCount(const std::string& dbcontent_name, unsigned int value)
{
    info_["dbcontent_counts"][dbcontent_name] = value;
}

bool Target::hasAdsbMOPSVersion()
{
    return info_.contains("adsb_mops_version");
}

unsigned int Target::adsbMOPSVersion()
{
    assert (hasAdsbMOPSVersion());
    return info_.at("adsb_mops_version");
}

void Target::adsbMOPSVersion(unsigned int value)
{
    info_["adsb_mops_version"] = value;
}

}
