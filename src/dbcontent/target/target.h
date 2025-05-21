#pragma once

#include "targetbase.h"
#include "propertylist.h"
#include "util/timewindow.h"

#include "json.hpp"

#include "boost/date_time/posix_time/ptime.hpp"

#include <set>

namespace dbContent {

class Target : public TargetBase
{
public:
    const unsigned int utn_ {0};

    Target(unsigned int utn, nlohmann::json info);

    unsigned int utn() { return utn_; }

    bool useInEval() const;
    void useInEval(bool value);

    Utils::TimeWindowCollection& evalExcludedTimeWindows();
    std::set<std::string>& evalExcludedRequirements();

    const Utils::TimeWindowCollection& evalExcludedTimeWindows() const;
    const std::set<std::string>& evalExcludedRequirements() const;

    void storeEvalutionInfo(); // save efficient variables in json

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

    bool hasPositionBounds() const;
    void setPositionBounds (double latitude_min, double latitude_max,
                           double longitude_min, double longitude_max);
    double latitudeMin() const;
    double latitudeMax() const;
    double longitudeMin() const;
    double longitudeMax() const;

    static const std::string KEY_USED;
    static const std::string KEY_COMMENT;
    static const std::string KEY_TIME_BEGIN;
    static const std::string KEY_TIME_END;
    static const std::string KEY_ACAD;
    static const std::string KEY_ACID;
    static const std::string KEY_MODE_3A;
    static const std::string KEY_MODE_C_MIN;
    static const std::string KEY_MODE_C_MAX;
    static const std::string KEY_COUNTS;
    static const std::string KEY_ADSD_MOPS_VERSION;
    static const std::string KEY_LATITUDE_MIN;
    static const std::string KEY_LATITUDE_MAX;
    static const std::string KEY_LONGITUDE_MIN;
    static const std::string KEY_LONGITUDE_MAX;
    static const std::string KEY_ECAT;

    static const Property     DBColumnID;
    static const Property     DBColumnInfo;
    static const PropertyList DBPropertyList;

    virtual void targetCategory(Category ecat);
    virtual Category targetCategory() const;

protected:
    nlohmann::json info_;

    bool use_in_eval_;
    Utils::TimeWindowCollection excluded_time_windows_;
    std::set<std::string> excluded_requirements_;

    mutable std::string time_duration_str_;

};

}
