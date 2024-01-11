#include "fft/fftbase.h"
#include "logger.h"
#include "number.h"

using namespace Utils;
using namespace std;
using namespace nlohmann;

const string latitude_key = "latitude";
const string longitude_key = "longitude";
const string altitude_key = "altitude";
const string mode_s_address_key = "mode_s_address";
const string mode_3a_code_key = "mode_3a_code";

FFTBase::FFTBase()
{
}

std::string FFTBase::name() const
{
    return name_;
}

void FFTBase::name(const std::string &name)
{
    name_ = name;
}

nlohmann::json& FFTBase::info()
{
    return info_;
}

void FFTBase::info(const nlohmann::json& info)
{
    info_ = info;
}

std::string FFTBase::infoStr()
{
    return info_.dump();
}

bool FFTBase::hasModeSAddress()
{
    return info_.contains(mode_s_address_key);
}
unsigned int FFTBase::modeSAddress()
{
    assert (hasModeSAddress());
    return info_.at(mode_s_address_key);
}
void FFTBase::modeSAddress(unsigned int value)
{
    info_[mode_s_address_key] = value;
}

bool FFTBase::hasMode3Code()
{
    return info_.contains(mode_3a_code_key);
}
unsigned int FFTBase::mode3ACode()
{
    assert (hasMode3Code());
    return info_.at(mode_3a_code_key);
}

void FFTBase::mode3ACode(unsigned int value)
{
    info_[mode_3a_code_key] = value;
}

bool FFTBase::hasPosition() const
{
    return info_.contains(latitude_key)
            && info_.contains(longitude_key)
            && info_.contains(altitude_key);
}

void FFTBase::latitude (double value)
{
    info_[latitude_key] = value;
}
double FFTBase::latitude () const
{
    if (!info_.contains(latitude_key))
        return 0.0;
    else
        return info_.at(latitude_key);
}

void FFTBase::longitude (double value)
{
    info_[longitude_key] = value;
}
double FFTBase::longitude () const
{
    if (!info_.contains(longitude_key))
        return 0.0;
    else
        return info_.at(longitude_key);
}

void FFTBase::altitude (double value)
{
    info_[altitude_key] = value;
}

double FFTBase::altitude () const
{
    if (!info_.contains(altitude_key))
        return 0.0;
    else
        return info_.at(altitude_key);
}



void FFTBase::setFromJSON (const nlohmann::json& j)
{
    assert(j.contains("name"));
    name_ = j.at("name");

    assert(j.contains("info"));
    info_ = j.at("info");
}

json FFTBase::getAsJSON() const
{
    json j;

    j["name"] = name_;

    j["info"] = info_;

    return j;
}





