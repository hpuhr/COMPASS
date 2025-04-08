#pragma once


#include "propertylist.h"

#include "json.hpp"

#include "boost/date_time/posix_time/ptime.hpp"
#include "boost/optional.hpp"

#include <set>

namespace dbContent {

class Target
{
public:

    enum class EmitterCategory // slightly related to ADS-B ECAT
    {
        Unknown = 0,

        // Light general aviation aircraft (e.g., Cessna 152, Piper Cub)
        LightAircraft = 1,

        // Small high-performance piston or turboprop aircraft (e.g., Beechcraft Bonanza, Piper Malibu)
        SmallAircraft = 2,

        // Regional jets and smaller airliners (e.g., Embraer E175, Bombardier CRJ700)
        MediumAircraft = 3,

        // Large wake-generating aircraft (e.g., Airbus A310, Boeing 757)
        HighVortexLargeAircraft = 4,

        // Large commercial airliners (e.g., Boeing 777, Airbus A380)
        HeavyAircraft = 5,

        // High-speed fighter or interceptor jets (e.g., F-16, Eurofighter Typhoon)
        HighSpeedManoeuvrable = 6,

        // Helicopters and VTOL aircraft (e.g., Bell 412, Eurocopter EC135)
        Rotocraft = 10,

        // Gliders, UAVs, ultralights, lighter-than-air, etc. (e.g., DJI Matrice 300, ASH 31 Mi, Paramotor)
        OtherAirborne = 11,

        // Ground vehicles
        Vehicle = 20,
        Obstacle = 22 // obstruction or obstacles
    };

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

    bool hasPositionBounds() const;
    void setPositionBounds (double latitude_min, double latitude_max,
                           double longitude_min, double longitude_max);
    double latitudeMin() const;
    double latitudeMax() const;
    double longitudeMin() const;
    double longitudeMax() const;

    void emitterCategory(EmitterCategory ecat);
    EmitterCategory emitterCategory() const;
    std::string emitterCategoryStr() const;

    static const Property     DBColumnID;
    static const Property     DBColumnInfo;
    static const PropertyList DBPropertyList;

    // Convert ADS-B ECAT (optional) to EmitterCategory enum
    static EmitterCategory fromECAT(boost::optional<unsigned int> ecat) {
        if (!ecat) return EmitterCategory::Unknown;

        switch (*ecat) {
        case 1: return EmitterCategory::LightAircraft;
        case 2: return EmitterCategory::SmallAircraft;
        case 3: return EmitterCategory::MediumAircraft;
        case 4: return EmitterCategory::HighVortexLargeAircraft;
        case 5: return EmitterCategory::HeavyAircraft;
        case 6: return EmitterCategory::HighSpeedManoeuvrable;
        case 10: return EmitterCategory::Rotocraft;
        case 11: case 12: case 13: case 14: case 15: case 16:
            return EmitterCategory::OtherAirborne;
        case 20: case 21:
            return EmitterCategory::Vehicle;
        case 22: case 23: case 24:
            return EmitterCategory::Obstacle;
        default: return EmitterCategory::Unknown;
        }
    }

    // String representation of EmitterCategory
    static std::string toString(EmitterCategory ecat) {
        switch (ecat) {
        case EmitterCategory::LightAircraft: return "LightAircraft";
        case EmitterCategory::SmallAircraft: return "SmallAircraft";
        case EmitterCategory::MediumAircraft: return "MediumAircraft";
        case EmitterCategory::HighVortexLargeAircraft: return "HighVortexLargeAircraft";
        case EmitterCategory::HeavyAircraft: return "HeavyAircraft";
        case EmitterCategory::HighSpeedManoeuvrable: return "HighSpeedManoeuvrable";
        case EmitterCategory::Rotocraft: return "Rotocraft";
        case EmitterCategory::OtherAirborne: return "OtherAirborne";
        case EmitterCategory::Vehicle: return "Vehicle";
        case EmitterCategory::Obstacle: return "Obstacle";
        default: return "Unknown";
        }
    }

    static EmitterCategory FromString(const std::string& name) {
        static const std::unordered_map<std::string, EmitterCategory> str_to_category = {
            { "LightAircraft", EmitterCategory::LightAircraft },
            { "SmallAircraft", EmitterCategory::SmallAircraft },
            { "MediumAircraft", EmitterCategory::MediumAircraft },
            { "HighVortexLargeAircraft", EmitterCategory::HighVortexLargeAircraft },
            { "HeavyAircraft", EmitterCategory::HeavyAircraft },
            { "HighSpeedManoeuvrable", EmitterCategory::HighSpeedManoeuvrable },
            { "Rotocraft", EmitterCategory::Rotocraft },
            { "OtherAirborne", EmitterCategory::OtherAirborne },
            { "Vehicle", EmitterCategory::Vehicle },
            { "Obstacle", EmitterCategory::Obstacle },
            { "Unknown", EmitterCategory::Unknown }
        };

        auto it = str_to_category.find(name);
        if (it != str_to_category.end()) {
            return it->second;
        }
        return EmitterCategory::Unknown;
    }

    // Accessor helpers
    static double getAvgSize(EmitterCategory ecat) {
        assert (emitter_specs_.count(toString(ecat)));
        return emitter_specs_.at(toString(ecat)).at("avg_size_m");
    }

    static double getMaxSpeedKnots(EmitterCategory ecat) {
        assert (emitter_specs_.count(toString(ecat)));
        return emitter_specs_.at(toString(ecat)).at("max_speed_knots");
    }

    static double getMaxAccel(EmitterCategory ecat) {
        assert (emitter_specs_.count(toString(ecat)));
        return emitter_specs_.at(toString(ecat)).at("max_accel_mps2");
    }

    static bool isGroundOnly(EmitterCategory ecat) {
        assert (emitter_specs_.count(toString(ecat)));
        return emitter_specs_.at(toString(ecat)).at("ground_only");
    }

protected:
    nlohmann::json info_;
    mutable std::string time_duration_str_;

    static const nlohmann::json emitter_specs_;
};

}
