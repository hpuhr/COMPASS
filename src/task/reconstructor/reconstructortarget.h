#pragma once

//#include "dbcontent/target/targetposition.h"
#include "dbcontent/target/targetreportdefs.h"
#include "projection/transformation.h"
#include "reconstructor_defs.h"

#include "boost/date_time/posix_time/ptime.hpp"
#include "boost/optional.hpp"

#include <vector>
#include <string>
#include <map>
#include <set>

class Buffer;
class ReconstructorBase;

namespace reconstruction
{
    class KalmanOnlineTracker;
}

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

    typedef std::pair<const reconstruction::Reference*,
                      const reconstruction::Reference*> ReferencePair; // both can be nullptr

    ReconstructorTarget(ReconstructorBase& reconstructor, unsigned int utn, bool tmp_utn);
    virtual ~ReconstructorTarget();

    ReconstructorBase& reconstructor_; // to get the real target reports

    unsigned int utn_;
    bool tmp_utn_ {false};
    bool associations_written_ {false}; // set after the utn was used in db at least once
    bool track_begin_ {true}; // unset after first target report written

    // target report aggregation & search structures, by record numbers
    std::vector<unsigned long> target_reports_;
    std::multimap<boost::posix_time::ptime, unsigned long> tr_timestamps_;
    // all sources sorted by time, ts -> record_num
    std::map<unsigned int, std::map<unsigned int,
                                    std::map<unsigned int,
                                             std::multimap<boost::posix_time::ptime, unsigned long>>>> tr_ds_timestamps_;
    // dbcontent id -> ds_id -> line_id -> ts -> record_num

    std::set<unsigned int> acads_;
    std::set<std::string> acids_;
    std::set<unsigned int> mode_as_;
    //std::set<unsigned int> mops_versions_;

    boost::posix_time::ptime timestamp_min_;
    boost::posix_time::ptime timestamp_max_;

    boost::optional<float> mode_c_min_;
    boost::optional<float> mode_c_max_;

    std::set <unsigned int> ds_ids_;
    //std::set <std::pair<unsigned int, unsigned int>> track_nums_; // ds_id, tn

    std::map <unsigned int, unsigned int> counts_; // dbcontent id -> count

    std::map<boost::posix_time::ptime, reconstruction::Reference> references_; // ts -> tr

    boost::posix_time::ptime ts_prev_;
    bool has_prev_v_ {false};
    double v_x_prev_, v_y_prev_;

    bool has_prev_baro_alt_ {false};
    float baro_alt_prev_;

    mutable Transformation trafo_;

    std::unique_ptr<reconstruction::KalmanOnlineTracker> tracker_;

    void addTargetReport (unsigned long rec_num, bool add_to_tracker = true);
    void addTargetReports (std::vector<unsigned long> rec_nums, bool add_to_tracker = true);

    unsigned int numAssociated() const;
    unsigned long lastAssociated() const;

    bool hasACAD () const;
    bool hasACAD (unsigned int ta)  const;
    bool hasAllOfACADs (std::set<unsigned int> tas) const;
    bool hasAnyOfACADs (std::set<unsigned int> tas) const;

    bool hasModeA () const;
    bool hasModeA (unsigned int code) const;

    bool hasModeC () const;

    std::string asStr() const;
    std::string timeStr() const;

    bool hasTimestamps() const;
    bool isTimeInside (boost::posix_time::ptime timestamp) const;
    bool isTimeInside (boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const;
    bool hasDataForTime (boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const;

    // TODO lambda for selective data
    ReconstructorInfoPair dataFor (boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const;
    ReferencePair refDataFor (boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const;

//    std::pair<boost::posix_time::ptime, boost::posix_time::ptime> timesFor (
//        boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const;
    // lower/upper times, -1 if not existing

    // TODO lambda for selective data without do not use pos
    std::pair<dbContent::targetReport::Position, bool> interpolatedPosForTime (
        boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const;
    std::pair<dbContent::targetReport::Position, bool> interpolatedPosForTimeFast (
        boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const;

    boost::optional<dbContent::targetReport::Position> interpolatedRefPosForTimeFast (
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
        const dbContent::targetReport::ReconstructorInfo& tr, boost::posix_time::time_duration max_time_diff) const;

    std::tuple<std::vector<unsigned long>,
               std::vector<unsigned long>,
               std::vector<unsigned long>> compareModeCCodes (
        const ReconstructorTarget& other, const std::vector<unsigned long>& rec_nums,
        boost::posix_time::time_duration max_time_diff, float max_alt_diff, bool debug) const;

    ComparisonResult compareModeCCode (const dbContent::targetReport::ReconstructorInfo& tr,
                                   boost::posix_time::time_duration max_time_diff, float max_alt_diff,
                                   bool debug) const;
    // unknown, same, different timestamps from this

    //void calculateSpeeds();
    //void removeNonModeSTRs();

    void updateCounts();
    std::map <std::string, unsigned int> getDBContentCounts();

    std::shared_ptr<Buffer> getReferenceBuffer();

    void removeOutdatedTargetReports();

    // online reconstructor
    void reinitTracker();
    void addToTracker(const dbContent::targetReport::ReconstructorInfo& tr);
    bool canPredict(boost::posix_time::ptime timestamp) const;
    bool predict(reconstruction::Measurement& mm, const dbContent::targetReport::ReconstructorInfo& tr) const;
    // hp: plz rework to tr -> posix timestamp, mm to targetreportdefs structs pos, posacc, maybe by return

//    bool hasADSBMOPSVersion();
//    std::set<unsigned int> getADSBMOPSVersions();
};

} // namespace dbContent

