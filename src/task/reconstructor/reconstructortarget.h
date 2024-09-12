#pragma once

//#include "dbcontent/target/targetposition.h"
#include "dbcontent/target/targetreportdefs.h"
#include "projection/transformation.h"
#include "reconstruction_defs.h"

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/optional.hpp>

#include <vector>
#include <string>
#include <map>
#include <set>

class Buffer;
class ReconstructorBase;

namespace reconstruction
{
class KalmanOnlineTracker;
class KalmanChain;
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
    struct GlobalStats
    {
        void reset()
        {
            num_chain_added                   = 0;
            num_chain_updates                 = 0;
            num_chain_updates_valid           = 0;
            num_chain_updates_failed          = 0;
            num_chain_updates_failed_numeric  = 0;
            num_chain_updates_failed_badstate = 0;
            num_chain_updates_failed_other    = 0;
            num_chain_updates_skipped         = 0;
            num_chain_updates_proj_changed    = 0; 

            num_chain_predictions                 = 0;
            num_chain_predictions_failed          = 0;
            num_chain_predictions_failed_numeric  = 0;
            num_chain_predictions_failed_badstate = 0;
            num_chain_predictions_failed_other    = 0;
            num_chain_predictions_fixed           = 0;

            num_rec_updates                 = 0;
            num_rec_updates_valid           = 0;
            num_rec_updates_failed          = 0;
            num_rec_updates_failed_numeric  = 0;
            num_rec_updates_failed_badstate = 0;
            num_rec_updates_failed_other    = 0;
            num_rec_updates_skipped         = 0;
            num_rec_smooth_steps_failed     = 0;
            num_rec_smooth_target_failed    = 0;
            num_rec_interp_failed           = 0;
        }

        size_t num_chain_added                   = 0;
        size_t num_chain_updates                 = 0;
        size_t num_chain_updates_valid           = 0;
        size_t num_chain_updates_failed          = 0;
        size_t num_chain_updates_failed_numeric  = 0;
        size_t num_chain_updates_failed_badstate = 0;
        size_t num_chain_updates_failed_other    = 0;
        size_t num_chain_updates_skipped         = 0;
        size_t num_chain_updates_proj_changed    = 0;

        size_t num_chain_predictions                 = 0;
        size_t num_chain_predictions_failed          = 0;
        size_t num_chain_predictions_failed_numeric  = 0;
        size_t num_chain_predictions_failed_badstate = 0;
        size_t num_chain_predictions_failed_other    = 0;
        size_t num_chain_predictions_fixed           = 0;
        size_t num_chain_predictions_proj_changed    = 0;

        size_t num_rec_updates                 = 0;
        size_t num_rec_updates_valid           = 0;
        size_t num_rec_updates_failed          = 0;
        size_t num_rec_updates_failed_numeric  = 0;
        size_t num_rec_updates_failed_badstate = 0;
        size_t num_rec_updates_failed_other    = 0;
        size_t num_rec_updates_skipped         = 0;
        size_t num_rec_smooth_steps_failed     = 0;
        size_t num_rec_smooth_target_failed    = 0;
        size_t num_rec_interp_failed           = 0;
    };

    struct InterpOptions
    {
        enum class InitMode
        {
            First = 0,
            Last,
            FirstValid,
            RecNum
        };

        InterpOptions() {}

        InterpOptions& initFirst() { init_mode_ = InitMode::First; return *this; }
        InterpOptions& initLast() { init_mode_ = InitMode::Last; return *this; }
        InterpOptions& initFirstValid() { init_mode_ = InitMode::FirstValid; return *this; }
        InterpOptions& initRecNum(unsigned long rec_num) { init_mode_ = InitMode::RecNum; init_rec_num_ = rec_num; return *this; }

        InterpOptions& enableDebug(bool ok) { debug_ = ok; return *this; }

        InitMode initMode() const { return init_mode_; }
        unsigned long initRecNum() const { return init_rec_num_; }

        bool debug() const { return debug_; }

    private:
        InitMode      init_mode_    = InitMode::FirstValid;
        unsigned long init_rec_num_ = 0;

        bool debug_ = false;
    };

    typedef std::pair<dbContent::targetReport::ReconstructorInfo*,
                      dbContent::targetReport::ReconstructorInfo*> ReconstructorInfoPair; // both can be nullptr

    typedef std::pair<const reconstruction::Reference*,
                      const reconstruction::Reference*> ReferencePair; // both can be nullptr

    typedef std::function<bool(const dbContent::targetReport::ReconstructorInfo& tr_info)> InfoValidFunc;

    ReconstructorTarget(ReconstructorBase& reconstructor, 
                        unsigned int utn, 
                        bool tmp_utn,
                        bool multithreaded_predictions,
                        bool dynamic_insertions);
    virtual ~ReconstructorTarget();

    ReconstructorBase& reconstructor_; // to get the real target reports

    unsigned int utn_;
    bool tmp_utn_ {false};

    bool created_in_current_slice_ {false};

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

    void addTargetReport (unsigned long rec_num,
                         bool add_to_tracker = true);
    void addTargetReports (const ReconstructorTarget& other,
                          bool add_to_tracker = true);

    unsigned int numAssociated() const;
    unsigned long lastAssociated() const;

    bool hasACAD () const;
    bool hasACAD (unsigned int ta)  const;
    bool hasAllOfACADs (std::set<unsigned int> tas) const;
    bool hasAnyOfACADs (std::set<unsigned int> tas) const;

    bool hasACID () const;
    bool hasACID (const std::string& acid)  const;

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
    ReconstructorInfoPair dataFor (boost::posix_time::ptime timestamp,
                                  boost::posix_time::time_duration d_max,
                                  const InfoValidFunc& tr_valid_func = InfoValidFunc(),
                                  const InterpOptions& interp_options = InterpOptions()) const;
    ReferencePair refDataFor (boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const;

    //    std::pair<boost::posix_time::ptime, boost::posix_time::ptime> timesFor (
    //        boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const;
    // lower/upper times, -1 if not existing

    // TODO lambda for selective data without do not use pos
    std::pair<dbContent::targetReport::Position, bool> interpolatedPosForTime (
        boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const;
    std::pair<dbContent::targetReport::Position, bool> interpolatedPosForTimeFast (
        boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const;

    std::pair<boost::optional<dbContent::targetReport::Position>,
              boost::optional<dbContent::targetReport::PositionAccuracy>> interpolatedRefPosForTimeFast (
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

    boost::optional<float> modeCCodeAt (boost::posix_time::ptime timestamp,
                                       boost::posix_time::time_duration max_time_diff,
                                       const InterpOptions& interp_options = InterpOptions()) const;
    boost::optional<bool> groundBitAt (boost::posix_time::ptime timestamp,
                                      boost::posix_time::time_duration max_time_diff,
                                      const InterpOptions& interp_options = InterpOptions()) const;
    boost::optional<double> groundSpeedAt (boost::posix_time::ptime timestamp,
                                          boost::posix_time::time_duration max_time_diff,
                                          const InterpOptions& interp_options = InterpOptions()) const; // m/s

    ComparisonResult compareModeCCode (const dbContent::targetReport::ReconstructorInfo& tr,
                                      boost::posix_time::time_duration max_time_diff, float max_alt_diff,
                                      bool debug) const;
    // unknown, same, different timestamps from this

    //fl_unknown, fl_on_ground, fl_index
    std::tuple<bool, bool, unsigned int> getAltitudeState (
        const boost::posix_time::ptime& ts, boost::posix_time::time_duration max_time_diff,
        const InterpOptions& interp_options = InterpOptions()) const;

    void updateCounts();
    std::map <std::string, unsigned int> getDBContentCounts() const;

    std::shared_ptr<Buffer> getReferenceBuffer();

    void removeOutdatedTargetReports();
    void removeTargetReportsNewerThan(boost::posix_time::ptime ts);

    // online reconstructor
    size_t trackerCount() const;
    boost::posix_time::ptime trackerTime(size_t idx) const;
    bool canPredict(boost::posix_time::ptime ts) const;
    bool hasChainState(boost::posix_time::ptime ts) const;
    bool predictPositionClose(boost::posix_time::ptime ts, double lat, double lon) const;
    bool predict(reconstruction::Measurement& mm, 
                 const boost::posix_time::ptime& ts,
                 reconstruction::PredictionStats* stats = nullptr) const;
    bool predictMT(reconstruction::Measurement& mm, 
                   const boost::posix_time::ptime& ts,
                   unsigned int thread_id,
                   reconstruction::PredictionStats* stats = nullptr) const;
    bool getChainState(reconstruction::Measurement& mm, 
                       const boost::posix_time::ptime& ts,
                       reconstruction::PredictionStats* stats = nullptr) const;
    // hp: plz rework to tr -> posix timestamp, mm to targetreportdefs structs pos, posacc, maybe by return

    const reconstruction::KalmanChain& getChain() const;

    //    bool hasADSBMOPSVersion();
    //    std::set<unsigned int> getADSBMOPSVersions();

    static GlobalStats& globalStats() { return global_stats_; }
    static void addUpdateToGlobalStats(const reconstruction::UpdateStats& s);
    static void addPredictionToGlobalStats(const reconstruction::PredictionStats& s);

protected:
    enum class TargetReportSkipResult
    {
        Valid = 0,
        SkipReference,
        SkipFunc
    };

    static const double on_ground_max_alt_ft_;
    static const double on_ground_max_speed_ms_;

    bool hasTracker() const;
    void reinitTracker();
    void reinitChain();
    bool addToTracker(const dbContent::targetReport::ReconstructorInfo& tr, 
                      bool reestimate = true,
                      reconstruction::UpdateStats* stats = nullptr);

    bool addTargetReport (unsigned long rec_num,
                         bool add_to_tracker,
                         bool reestimate);

    TargetReportSkipResult skipTargetReport (const dbContent::targetReport::ReconstructorInfo& tr,
                                            const InfoValidFunc& tr_valid_func = InfoValidFunc()) const;

    std::unique_ptr<reconstruction::KalmanChain> chain_;

    bool multithreaded_predictions_ = true;
    bool dynamic_insertions_        = true;

    static GlobalStats global_stats_;
};

} // namespace dbContent
