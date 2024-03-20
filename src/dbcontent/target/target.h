#pragma once

#include "json.hpp"

#include "boost/date_time/posix_time/ptime.hpp"

#include <set>

namespace dbContent {

class Target
{
public:
    const unsigned int utn_ {0};

    Target(unsigned int utn, nlohmann::json info);

    unsigned int utn() { return utn_; }

    bool useInEval() const;
    void useInEval(bool value);

    std::string comment() const;
    void comment (const std::string& value);

    nlohmann::json info() const { return info_; }

    void timeBegin(boost::posix_time::ptime value);
    boost::posix_time::ptime timeBegin() const;
    std::string timeBeginStr() const;
    void timeEnd(boost::posix_time::ptime value);
    boost::posix_time::ptime timeEnd() const;
    std::string timeEndStr() const;
    boost::posix_time::time_duration timeDuration() const;
    std::string timeDurationStr() const;

    void aircraftIdentifications(const std::set<std::string>& ids);
    std::set<std::string> aircraftIdentifications() const;
    std::string aircraftIdentificationsStr() const;

    std::set<unsigned int> aircraftAddresses() const;
    void aircraftAddresses(const std::set<unsigned int>& tas);
    std::string aircraftAddressesStr() const;

    std::set<unsigned int> modeACodes() const;
    void modeACodes(const std::set<unsigned int>& mas);
    std::string modeACodesStr() const;

    bool hasModeC() const;
    void modeCMinMax(float min, float max);
    float modeCMin() const;
    std::string modeCMinStr() const;
    float modeCMax() const;
    std::string modeCMaxStr() const;

    bool isPrimaryOnly () const;
    bool isModeACOnly () const;

    unsigned int numUpdates () const;

    unsigned int dbContentCount(const std::string& dbcontent_name) const;
    void dbContentCount(const std::string& dbcontent_name, unsigned int value);
    void clearDBContentCount(const std::string& dbcontent_name);

    bool hasAdsbMOPSVersions() const;
    std::set<unsigned int> adsbMOPSVersions() const;
    void adsbMOPSVersions(std::set<unsigned int> values);
    std::string adsbMOPSVersionsStr() const;

protected:
    nlohmann::json info_;
    mutable std::string time_duration_str_;
};

}
