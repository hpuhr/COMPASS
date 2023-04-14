#ifndef DBCONTENT_TARGETREPORTCHAIN_H
#define DBCONTENT_TARGETREPORTCHAIN_H

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

    unsigned int numUpdates () const;

    boost::posix_time::ptime timeBegin() const;
    std::string timeBeginStr() const;
    boost::posix_time::ptime timeEnd() const;
    std::string timeEndStr() const;
    boost::posix_time::time_duration timeDuration() const;

    std::set<std::string> callsigns() const;
    std::string callsignsStr() const;

    std::set<unsigned int> targetAddresses() const;
    std::string targetAddressesStr() const;

    std::set<unsigned int> modeACodes() const;
    std::string modeACodesStr() const;

    bool hasModeC() const;
    float modeCMin() const;
    std::string modeCMinStr() const;
    float modeCMax() const;
    std::string modeCMaxStr() const;

    bool isPrimaryOnly () const;

    const IndexMap& data() const;

    double latitudeMin() const;
    double latitudeMax() const;
    double longitudeMin() const;
    double longitudeMax() const;

    bool hasPos() const;

    DataID dataID(const boost::posix_time::ptime& timestamp) const;

    bool hasRefPos(const DataID& id) const;
    TargetPosition refPos(const DataID& id) const;
    bool hasRefSpeed(const DataID& id) const;
    TargetVelocity refSpeed(const DataID& id) const;
    // estimate ref baro alt at tod,index TODO should be replaced by real altitude reconstructor

    bool hasRefCallsign(const DataID& id) const;
    std::string refCallsign(const DataID& id) const;

    bool hasRefModeA(const DataID& id) const; // only if set, is v, not g
    unsigned int refModeA(const DataID& id) const;

    bool hasRefModeC(const DataID& id) const; // only if set, is v, not g
    float refModeC(const DataID& id) const;

    bool hasRefTA(const DataID& id) const;
    unsigned int refTA(const DataID& id) const;

    std::pair<bool,bool> refGroundBit(const DataID& id) const; // has gbs, gbs true

protected:
    std::shared_ptr<dbContent::Cache> cache_;
    std::string dbcontent_name_;

    std::multimap<boost::posix_time::ptime, Index> ref_data_; // timestamp -> index
    std::vector<unsigned int> ref_indices_;

    mutable std::set<std::string> callsigns_;
    mutable std::set<unsigned int> target_addresses_;
    mutable std::set<unsigned int> mode_a_codes_;

    mutable bool  has_mode_c_ {false};
    mutable float mode_c_min_ {0};
    mutable float mode_c_max_ {0};

    mutable bool   has_pos_      {false};
    mutable double latitude_min_ {0};
    mutable double latitude_max_ {0};

    mutable double longitude_min_ {0};
    mutable double longitude_max_ {0};

    void updateCallsigns() const;
    void updateTargetAddresses() const;
    void updateModeACodes() const;
    void updateModeCMinMax() const;
    void updatePositionMinMax() const;

    std::pair<bool, float> estimateRefAltitude (
            const boost::posix_time::ptime& timestamp, unsigned int index_internal) const;

    Index indexFromDataID(const DataID& id) const;
    boost::posix_time::ptime timestampFromDataID(const DataID& id) const;
};

} // namespace TargetReport

} // namespace dbContent

#endif // DBCONTENT_TARGETREPORTCHAIN_H
