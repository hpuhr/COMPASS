#pragma once

//#include "dbcontent/target/targetposition.h"
#include "dbcontent/target/targetreportdefs.h"
#include "projection/transformation.h"

#include "boost/date_time/posix_time/ptime.hpp"
#include "boost/optional.hpp"

#include <vector>
#include <string>
#include <map>
#include <set>


class SimpleReconstructor;

namespace dbContent {

enum class ComparisonResult
{
    UNKNOWN,
    SAME,
    DIFFERENT
};

class ReconstructorTarget
{
  public:
    typedef std::pair<dbContent::targetReport::ReconstructorInfo*,
                      dbContent::targetReport::ReconstructorInfo*> ReconstructorInfoPair; // both can be nullptr

    ReconstructorTarget(SimpleReconstructor& reconstructor);
    virtual ~ReconstructorTarget();

    SimpleReconstructor& reconstructor_; // to get the real target reports

    boost::optional<unsigned int> utn_;

    // target report aggregation & search structures, by record numbers
    std::vector<unsigned long> target_reports_;
    std::multimap<boost::posix_time::ptime, unsigned long> tr_timestamps_;
    // all sources sorted by time, ts -> record_num
    std::map<unsigned int, std::map<unsigned int, std::multimap<boost::posix_time::ptime, unsigned long>>> tr_ds_timestamps_;
    // dbcontent id -> ds_id -> ts -> record_num

    std::set<unsigned int> tas_;
    std::set<std::string> ids_;
    std::set<unsigned int> mas_;
    //std::set<unsigned int> mops_versions_;

    boost::posix_time::ptime timestamp_min_;
    boost::posix_time::ptime timestamp_max_;

    boost::optional<float> mode_c_min_;
    boost::optional<float> mode_c_max_;

//    boost::optional<double> speed_min_;
//    boost::optional<double> speed_avg_;
//    boost::optional<double> speed_max_;

    std::set <unsigned int> ds_ids_;
    //std::set <std::pair<unsigned int, unsigned int>> track_nums_; // ds_id, tn

    mutable Transformation trafo_;

    void addTargetReport (unsigned long rec_num);
    void addTargetReports (std::vector<unsigned long> rec_nums);
    unsigned int numAssociated() const;
    unsigned long lastAssociated() const;

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

    ReconstructorInfoPair dataFor (boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const;

//    std::pair<boost::posix_time::ptime, boost::posix_time::ptime> timesFor (
//        boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const;
    // lower/upper times, -1 if not existing

    std::pair<dbContent::targetReport::Position, bool> interpolatedPosForTime (
        boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const;
    std::pair<dbContent::targetReport::Position, bool> interpolatedPosForTimeFast (
        boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const;

//    bool hasDataForExactTime (boost::posix_time::ptime timestamp) const;
//    unsigned long dataForExactTime (boost::posix_time::ptime timestamp) const; // TargetReport& ???
//    dbContent::targetReport::Position posForExactTime (boost::posix_time::ptime timestamp) const;

    bool hasDataFor (unsigned long rec_num) const;
    dbContent::targetReport::ReconstructorInfo& dataFor (unsigned long rec_num) const;

    bool hasPositionFor (unsigned long rec_num) const;
    dbContent::targetReport::Position positionFor (unsigned long rec_num) const;

    float duration () const;
    bool timeOverlaps (const ReconstructorTarget& other) const;
    float probTimeOverlaps (const ReconstructorTarget& other) const; // ratio of overlap, measured by shortest target

    std::tuple<std::vector<unsigned long>,
               std::vector<unsigned long>,
               std::vector<unsigned long>> compareModeACodes (
        const ReconstructorTarget& other, boost::posix_time::time_duration max_time_diff) const;
    // unknown, same, different
    ComparisonResult compareModeACode (
        dbContent::targetReport::ReconstructorInfo& tr, boost::posix_time::time_duration max_time_diff) const;

    std::tuple<std::vector<unsigned long>,
               std::vector<unsigned long>,
               std::vector<unsigned long>> compareModeCCodes (
        const ReconstructorTarget& other, const std::vector<unsigned long>& rec_nums,
        boost::posix_time::time_duration max_time_diff, float max_alt_diff, bool debug) const;

    ComparisonResult compareModeCCode (dbContent::targetReport::ReconstructorInfo& tr,
                                   boost::posix_time::time_duration max_time_diff, float max_alt_diff,
                                   bool debug) const;
    // unknown, same, different timestamps from this

    //void calculateSpeeds();
    //void removeNonModeSTRs();

    std::map <std::string, unsigned int> getDBContentCounts();

//    bool hasADSBMOPSVersion();
//    std::set<unsigned int> getADSBMOPSVersions();
};

} // namespace dbContent

