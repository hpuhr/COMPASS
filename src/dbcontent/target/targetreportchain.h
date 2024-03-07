#ifndef DBCONTENT_TARGETREPORTCHAIN_H
#define DBCONTENT_TARGETREPORTCHAIN_H

#include "dbcontent/target/targetreportdefs.h"
#include "dbcontent/target/targetposition.h"
#include "dbcontent/target/targetpositionaccuracy.h"
#include "dbcontent/target/targetvelocity.h"
#include "projection/transformation.h"

#include "boost/date_time/posix_time/ptime.hpp"
#include <boost/optional.hpp>

#include <vector>
#include <map>
#include <memory>
#include <vector>
#include <set>
#include <string>


namespace dbContent {

class DBContentAccessor;

namespace TargetReport {

struct DataMappingTimes // mapping to respective tst data
{
    boost::posix_time::ptime timestamp_; // tod of test

    bool has_other1_ {false};
    boost::posix_time::ptime timestamp_other1_;
    DataID dataid_other1_;

    bool has_other2_ {false};
    boost::posix_time::ptime timestamp_other2_;
    DataID dataid_other2_;
};


struct DataMapping // mapping to respective ref data
{
    boost::posix_time::ptime timestamp_; // timestamp of original

    bool has_ref1_ {false};
    boost::posix_time::ptime timestamp_ref1_;
    DataID dataid_ref1_;

    bool has_ref2_ {false};
    boost::posix_time::ptime timestamp_ref2_;
    DataID dataid_ref2_;

    bool has_ref_pos_ {false};
    dbContent::TargetPosition pos_ref_;

    bool has_ref_spd_ {false};
    dbContent::TargetVelocity spd_ref_;
};

class Chain
{
public:
    typedef TargetReport::DataID                                  DataID;
    typedef std::multimap<boost::posix_time::ptime, Index>      IndexMap;

    Chain(std::shared_ptr<dbContent::DBContentAccessor> accessor, const std::string& dbcontent_name);
    virtual ~Chain();

    void addIndex (boost::posix_time::ptime timestamp, unsigned int index);

    bool hasData() const;

    void finalize () const;

    unsigned int size () const;
    unsigned int ignoredSize() const;

    boost::posix_time::ptime timeBegin() const;
    std::string timeBeginStr() const;
    boost::posix_time::ptime timeEnd() const;
    std::string timeEndStr() const;
    boost::posix_time::time_duration timeDuration() const;

    std::set<std::string> acids() const;
    std::string acidsStr() const;

    std::set<unsigned int> acads() const;
    std::string acadsStr() const;

    std::set<unsigned int> modeACodes() const;
    std::string modeACodesStr() const;

    bool hasModeC() const;
    float modeCMin() const;
    std::string modeCMinStr() const;
    float modeCMax() const;
    std::string modeCMaxStr() const;

    bool isPrimaryOnly () const;

    const IndexMap& timestampIndexes() const;

    double latitudeMin() const;
    double latitudeMax() const;
    double longitudeMin() const;
    double longitudeMax() const;

    bool hasPos() const;

    DataID dataID(const boost::posix_time::ptime& timestamp) const;

    unsigned int dsID(const DataID& id) const;

    TargetPosition pos(const DataID& id) const;
    boost::optional<TargetPosition> posOpt(const DataID& id) const;

    boost::optional<TargetPositionAccuracy> posAccuracy(const DataID& id) const;

    boost::optional<TargetVelocity> speed(const DataID& id) const;
    // estimate ref baro alt at tod,index TODO should be replaced by real altitude reconstructor

    boost::optional<TargetVelocityAccuracy> speedAccuracy(const DataID& id) const;

    boost::optional<std::string> acid(const DataID& id) const;

    boost::optional<unsigned int> modeA(
            const DataID& id, bool ignore_invalid = true, bool ignore_garbled = true) const; // only if set

    boost::optional<float> modeC(
            const DataID& id, bool ignore_invalid = true, bool ignore_garbled = true) const; // only if set

    boost::optional<unsigned int> acad(const DataID& id) const;

    boost::optional<bool> groundBit(const DataID& id) const; // gbs

    boost::optional<unsigned int> tstTrackNum(const DataID& id) const;

    boost::optional<float> groundSpeed(const DataID& id) const; // m/s

    boost::optional<float> trackAngle(const DataID& id) const; // deg

    Index indexFromDataID(const DataID& id) const;
    boost::posix_time::ptime timestampFromDataID(const DataID& id) const;

    DataMapping calculateDataMapping(boost::posix_time::ptime timestamp) const; // test tod
    void addPositionsSpeedsToMapping (DataMapping& mapping) const;

    DataMappingTimes findDataMappingTimes(boost::posix_time::ptime timestamp_ref) const; // ref tod

    void setIgnoredPositions(std::vector<bool> ignored_positions);
    bool ignorePosition(const DataID& id) const;

protected:
    std::shared_ptr<dbContent::DBContentAccessor> accessor_;
    std::string dbcontent_name_;

    std::multimap<boost::posix_time::ptime, Index> timestamp_index_lookup_; // timestamp -> index
    std::vector<unsigned int> indexes_;

    boost::optional<std::vector<bool>> ignored_positions_;

    mutable std::set<std::string> acids_;
    mutable std::set<unsigned int> acads_;
    mutable std::set<unsigned int> mode_a_codes_;

    mutable bool  has_mode_c_ {false};
    mutable float mode_c_min_ {0};
    mutable float mode_c_max_ {0};

    mutable bool   has_pos_      {false};
    mutable double latitude_min_ {0};
    mutable double latitude_max_ {0};

    mutable double longitude_min_ {0};
    mutable double longitude_max_ {0};

    mutable Transformation trafo_;

    void updateACIDs() const;
    void updateACADs() const;
    void updateModeACodes() const;
    void updateModeCMinMax() const;
    void updatePositionMinMax() const;

    std::pair<bool, float> estimateAltitude (
            const boost::posix_time::ptime& timestamp, unsigned int index_internal) const;

};

} // namespace TargetReport

} // namespace dbContent

#endif // DBCONTENT_TARGETREPORTCHAIN_H
