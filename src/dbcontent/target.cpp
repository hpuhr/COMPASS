#include "target.h"

namespace dbContent {


Target::Target(unsigned int utn, nlohmann::json info)
    : utn_(utn), info_(info)
{
}

std::set<unsigned int> Target::tas()
{
    return info_.at("ACADs").get<std::set<unsigned int>>();
}
void Target::tas(const std::set<unsigned int>& tas)
{
    info_["ACADs"] = tas;
}

std::set<unsigned int> Target::mas()
{
    return info_.at("M3As").get<std::set<unsigned int>>();
}
void Target::mas(const std::set<unsigned int>& mas)
{
    info_["M3As"] = mas;
}

}
