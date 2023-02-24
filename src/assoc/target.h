#ifndef ASSOCIATIONTARGET_H
#define ASSOCIATIONTARGET_H

#include "evaluationtargetposition.h"
#include "projection/transformation.h"

#include "boost/date_time/posix_time/ptime.hpp"
#include "boost/date_time/time_duration.hpp"

#include <vector>
#include <string>
#include <map>
#include <set>

namespace Association
{
    using namespace std;

    class TargetReport;

    enum class CompareResult
    {
        UNKNOWN,
        SAME,
        DIFFERENT
    };

    class Target
    {
    public:
        Target(unsigned int utn, bool tmp);
        ~Target();

        static bool in_appimage_;

        unsigned int utn_{0};
        bool tmp_ {false};

        bool use_in_eval_ {true};
        std::string comment_;

        std::set<unsigned int> tas_;
        std::set<std::string> ids_;
        std::set<unsigned int> mas_;
        std::set<unsigned int> mops_versions_;

        bool has_timestamps_ {false};
        boost::posix_time::ptime timestamp_min_;
        boost::posix_time::ptime timestamp_max_;

        bool has_mode_c_ {false};
        float mode_c_min_;
        float mode_c_max_;

        bool has_speed_ {false};
        double speed_min_ {0};
        double speed_avg_ {0};
        double speed_max_ {0};

        vector<TargetReport*> assoc_trs_;
        std::map<boost::posix_time::ptime, unsigned int> timed_indexes_;
        std::set <unsigned int> ds_ids_;
        std::set <std::pair<unsigned int, unsigned int>> track_nums_; // ds_it, tn

        mutable Transformation trafo_;

        void addAssociated (TargetReport* tr);
        void addAssociated (vector<TargetReport*> trs);
        unsigned int numAssociated() const;
        const TargetReport& lastAssociated() const;

        bool hasTA () const;
        bool hasTA (unsigned int ta)  const;
        bool hasAllOfTAs (std::set<unsigned int> tas) const;
        bool hasAnyOfTAs (std::set<unsigned int> tas) const;

        bool hasMA () const;
        bool hasMA (unsigned int ma) const;

        std::string asStr() const;
        std::string timeStr() const;

        bool isTimeInside (boost::posix_time::ptime timestamp) const;
        bool hasDataForTime (boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const;
        std::pair<boost::posix_time::ptime, boost::posix_time::ptime> timesFor (
                boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const;
        // lower/upper times, -1 if not existing

        std::pair<EvaluationTargetPosition, bool> interpolatedPosForTime (
                boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const;
        std::pair<EvaluationTargetPosition, bool> interpolatedPosForTimeFast (
                boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const;

        bool hasDataForExactTime (boost::posix_time::ptime timestamp) const;
        TargetReport& dataForExactTime (boost::posix_time::ptime timestamp) const;
        EvaluationTargetPosition posForExactTime (boost::posix_time::ptime timestamp) const;

        float duration () const;
        bool timeOverlaps (const Target& other) const;
        float probTimeOverlaps (const Target& other) const; // ratio of overlap, measured by shortest target

        std::tuple<std::vector<boost::posix_time::ptime>,
                   std::vector<boost::posix_time::ptime>,
                   std::vector<boost::posix_time::ptime>> compareModeACodes (
                const Target& other, boost::posix_time::time_duration max_time_diff) const;
        // unknown, same, different
        CompareResult compareModeACode (
                bool has_ma, unsigned int ma, boost::posix_time::ptime timestamp,
                boost::posix_time::time_duration max_time_diff) const;

        std::tuple<std::vector<boost::posix_time::ptime>,
                   std::vector<boost::posix_time::ptime>,
                   std::vector<boost::posix_time::ptime>> compareModeCCodes (
                const Target& other, const std::vector<boost::posix_time::ptime>& timestamps,
                boost::posix_time::time_duration max_time_diff, float max_alt_diff, bool debug) const;
        CompareResult compareModeCCode (bool has_mc, float mc, boost::posix_time::ptime timestamp,
                                        boost::posix_time::time_duration max_time_diff, float max_alt_diff,
                                        bool debug) const;
        // unknown, same, different timestamps from this

        void calculateSpeeds();
        void removeNonModeSTRs();

        std::map <std::string, unsigned int> getDBContentCounts();

        bool hasADSBMOPSVersion();
        std::set<unsigned int> getADSBMOPSVersions();
    };

}

#endif // ASSOCIATIONTARGET_H
