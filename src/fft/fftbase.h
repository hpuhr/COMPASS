#pragma once

#include <json.hpp>

#include <string>

class FFTBase
{
public:
    FFTBase();

    std::string name() const;
    void name(const std::string &name);

    nlohmann::json& info();
    void info(const nlohmann::json& info);
    std::string infoStr();

    bool hasModeSAddress();
    unsigned int modeSAddress();
    void modeSAddress(unsigned int value);
    void clearModeSAddress();

    bool hasMode3ACode();
    unsigned int mode3ACode();
    void mode3ACode(unsigned int value);
    void clearMode3ACode();

    bool hasModeCCode();
    float modeCCode(); // ft
    void modeCCode(float value);
    void clearModeCCode();

    bool hasPosition() const;

    void latitude (double value);
    double latitude () const;

    void longitude (double value);
    double longitude () const;

    bool hasAltitude() const;
    void altitude (double value);
    double altitude () const;

    void setFromJSON (const nlohmann::json& j);

    virtual nlohmann::json getAsJSON() const;

protected:
    std::string name_;

    nlohmann::json info_;

};



