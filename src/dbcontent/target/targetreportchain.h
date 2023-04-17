#ifndef DBCONTENT_TARGETREPORTCHAIN_H
#define DBCONTENT_TARGETREPORTCHAIN_H

#include "dbcontent/target/targetposition.h"
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

class Cache;
class TargetPosition;
class TargetVelocity;

namespace TargetReport {

struct Index
{
    Index() = default;
    Index(unsigned int idx_ext,
          unsigned int idx_int) : idx_external(idx_ext), idx_internal(idx_int) {}

    unsigned int idx_external; //external index (usually index into buffer)
    unsigned int idx_internal; //internal index (index into internal data structures)
};

class DataID
{
public:
    typedef std::pair<const boost::posix_time::ptime, Index> IndexPair;

    DataID() = default;
    DataID(const boost::posix_time::ptime& timestamp) : timestamp_(timestamp), valid_(true) {}
    DataID(const boost::posix_time::ptime& timestamp, const Index& index) : timestamp_(timestamp), index_(index), valid_(true) {}
    DataID(const IndexPair& ipair) : timestamp_(ipair.first), index_(ipair.second), valid_(true) {}
    virtual ~DataID() = default;

    bool valid() const { return valid_; }
    const boost::posix_time::ptime& timestamp() const { return timestamp_; }
    bool hasIndex() const { return index_.has_value(); }

    DataID& addIndex(const Index& index)
    {
        index_ = index;
        return *this;
    }

    const Index& index() const
    {
        if (!hasIndex())
            throw std::runtime_error("DataID: index: No index stored");
        return index_.value();
    }

private:
    boost::posix_time::ptime timestamp_;
    boost::optional<Index>   index_;
    bool                     valid_ = false;
};

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

    Chain(std::shared_ptr<dbContent::Cache> cache, const std::string& dbcontent_name);
    virtual ~Chain();

    void addIndex (boost::posix_time::ptime timestamp, unsigned int index);

    bool hasData() const;

    void finalize () const;

    unsigned int size () const;

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

    bool hasPos(const DataID& id) const;
    TargetPosition pos(const DataID& id) const;
    bool hasSpeed(const DataID& id) const;
    TargetVelocity speed(const DataID& id) const;
    // estimate ref baro alt at tod,index TODO should be replaced by real altitude reconstructor

    bool hasACID(const DataID& id) const;
    std::string acid(const DataID& id) const;

    bool hasModeA(const DataID& id) const; // only if set, is v, not g
    unsigned int modeA(const DataID& id) const;

    bool hasModeC(const DataID& id) const; // only if set, is v, not g
    float modeC(const DataID& id) const;

    bool hasACAD(const DataID& id) const;
    unsigned int acad(const DataID& id) const;

    bool hasGroundBit(const DataID& id) const; // only if set
    std::pair<bool,bool> groundBit(const DataID& id) const; // has gbs, gbs true

    bool hasTstTrackNum(const DataID& id) const;
    unsigned int tstTrackNum(const DataID& id) const;

    // speed, track angle
    bool hasTstMeasuredSpeed(const DataID& id) const;
    float tstMeasuredSpeed(const DataID& id) const; // m/s

    bool hasTstMeasuredTrackAngle(const DataID& id) const;
    float tstMeasuredTrackAngle(const DataID& id) const; // deg

    Index indexFromDataID(const DataID& id) const;
    boost::posix_time::ptime timestampFromDataID(const DataID& id) const;

    DataMapping calculateDataMapping(boost::posix_time::ptime timestamp) const; // test tod
    void addPositionsSpeedsToMapping (DataMapping& mapping) const;

    DataMappingTimes findDataMappingTimes(boost::posix_time::ptime timestamp_ref) const; // ref tod

protected:
    std::shared_ptr<dbContent::Cache> cache_;
    std::string dbcontent_name_;

    std::multimap<boost::posix_time::ptime, Index> timestamp_index_lookup_; // timestamp -> index
    std::vector<unsigned int> indexes_;

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

    void updateCallsigns() const;
    void updateTargetAddresses() const;
    void updateModeACodes() const;
    void updateModeCMinMax() const;
    void updatePositionMinMax() const;

    std::pair<bool, float> estimateAltitude (
            const boost::posix_time::ptime& timestamp, unsigned int index_internal) const;

};

} // namespace TargetReport

} // namespace dbContent

#endif // DBCONTENT_TARGETREPORTCHAIN_H
