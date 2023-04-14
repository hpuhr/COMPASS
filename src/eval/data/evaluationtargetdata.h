/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EVALUATIONTARGETDATA_H
#define EVALUATIONTARGETDATA_H

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

#include <Eigen/Core>

class Buffer;
class EvaluationData;
class EvaluationManager;
class DBContentManager;
class SectorLayer;

struct TstDataMapping // mapping to respective ref data
{
    boost::posix_time::ptime timestamp_; // timestmap of test

    bool has_ref1_ {false};
    boost::posix_time::ptime timestamp_ref1_;

    bool has_ref2_ {false};
    boost::posix_time::ptime timestamp_ref2_;

    bool has_ref_pos_ {false};
    dbContent::TargetPosition pos_ref_;

    bool has_ref_spd_ {false};
    dbContent::TargetVelocity spd_ref_;
};

struct DataMappingTimes // mapping to respective tst data
{
    boost::posix_time::ptime timestamp_; // tod of test

    bool has_other1_ {false};
    boost::posix_time::ptime timestamp_other1_;

    bool has_other2_ {false};
    boost::posix_time::ptime timestamp_other2_;
};

namespace Evaluation
{
    enum class DataType
    {
        Reference = 0,
        Test
    };

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
                throw std::runtime_error("DataID::index(): No index stored");
            return index_.value(); 
        }

    private:
        boost::posix_time::ptime timestamp_;
        boost::optional<Index>   index_;
        bool                     valid_ = false;
    };
}

class EvaluationTargetData
{
public:
    typedef Evaluation::DataType                                DataType;
    typedef Evaluation::Index                                   Index;
    typedef Evaluation::DataID                                  DataID;
    typedef std::multimap<boost::posix_time::ptime, Index>      IndexMap;
    typedef Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic> InsideCheckMatrix;

    EvaluationTargetData(unsigned int utn, 
                         EvaluationData& eval_data,
                         EvaluationManager& eval_man, 
                         DBContentManager& dbcont_man);
    virtual ~EvaluationTargetData();

    void addRefIndex (boost::posix_time::ptime timestamp, unsigned int index);
    void addTstIndex (boost::posix_time::ptime timestamp, unsigned int index);

    bool hasData() const;
    bool hasRefData () const;
    bool hasTstData () const;

    void finalize () const;

    const unsigned int utn_{0};

    unsigned int numUpdates () const;
    unsigned int numRefUpdates () const;
    unsigned int numTstUpdates () const;

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

    bool use() const;

    const IndexMap& refData() const;
    const IndexMap& tstData() const;

    bool canCheckTstMultipleSources() const;
    bool hasTstMultipleSources() const;

    bool canCheckTrackLUDSID() const;
    bool hasSingleLUDSID() const;
    unsigned int singleTrackLUDSID() const;

    double latitudeMin() const;
    double latitudeMax() const;
    double longitudeMin() const;
    double longitudeMax() const;

    bool hasPos() const;

    bool hasADSBInfo() const;
    bool hasMOPSVersion() const;
    std::set<unsigned int> mopsVersions() const;
    std::string mopsVersionStr() const;

    bool hasNucpNic() const;
    std::string nucpNicStr() const;
    bool hasNacp() const;
    std::string nacpStr() const;

    DataID dataID(const boost::posix_time::ptime& timestamp, DataType dtype) const;

    // ref
    bool hasMappedRefData(const DataID& id, boost::posix_time::time_duration d_max) const;
    std::pair<boost::posix_time::ptime, boost::posix_time::ptime> mappedRefTimes(
            const DataID& id, boost::posix_time::time_duration d_max) const;
    // lower/upper times, {} if not existing

    boost::optional<dbContent::TargetPosition> mappedRefPos(const DataID& id) const;
    std::pair<dbContent::TargetPosition, bool> mappedRefPos(
            const DataID& id, boost::posix_time::time_duration d_max) const;
    // bool ok
    std::pair<dbContent::TargetVelocity, bool> mappedRefSpeed(
            const DataID& id, boost::posix_time::time_duration d_max) const;

    bool hasRefPos(const DataID& id) const;
    dbContent::TargetPosition refPos(const DataID& id) const;
    bool hasRefSpeed(const DataID& id) const;
    dbContent::TargetVelocity refSpeed(const DataID& id) const;
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
    std::pair<bool,bool> mappedRefGroundBit(
            const DataID& id, boost::posix_time::time_duration d_max) const; // has gbs, gbs true

    // test
    bool hasTstPos(const DataID& id) const;
    dbContent::TargetPosition tstPos(const DataID& id) const;

    bool hasTstCallsign(const DataID& id) const;
    std::string tstCallsign(const DataID& id) const;

    bool hasTstModeA(const DataID& id) const; // only if set, is v, not g
    unsigned int tstModeA(const DataID& id) const;

    bool hasTstModeC(const DataID& id) const; // only if set, is v, not g
    float tstModeC(const DataID& id) const;

    bool hasTstGroundBit(const DataID& id) const; // only if set
    bool tstGroundBit(const DataID& id) const; // true is on ground

    bool hasTstTA(const DataID& id) const;
    unsigned int tstTA(const DataID& id) const;

    std::pair<bool,bool> tstGroundBitInterpolated(const DataID& id) const; // has gbs, gbs true

    bool hasTstTrackNum(const DataID& id) const;
    unsigned int tstTrackNum(const DataID& id) const;

    // speed, track angle
    bool hasTstMeasuredSpeed(const DataID& id) const;
    float tstMeasuredSpeed(const DataID& id) const; // m/s

    bool hasTstMeasuredTrackAngle(const DataID& id) const;
    float tstMeasuredTrackAngle(const DataID& id) const; // deg

    // inside check
    boost::optional<bool> availableGroundBit(const DataID& id, 
                                             Evaluation::DataType dtype,
                                             const boost::posix_time::time_duration& d_max) const;
    bool refPosAbove(const DataID& id) const;
    bool refPosGroundBitAvailable(const DataID& id) const;
    bool refPosInside(const SectorLayer& layer, 
                      const DataID& id) const;
    bool tstPosAbove(const DataID& id) const;
    bool tstPosGroundBitAvailable(const DataID& id) const;
    bool tstPosInside(const SectorLayer& layer, 
                      const DataID& id) const;
    bool mappedRefPosAbove(const DataID& id) const;
    bool mappedRefPosGroundBitAvailable(const DataID& id) const;
    bool mappedRefPosInside(const SectorLayer& layer, 
                            const DataID& id) const;

    static const int InterpGroundBitMaxSeconds = 15;

protected:
    void updateCallsigns() const;
    void updateTargetAddresses() const;
    void updateModeACodes() const;
    void updateModeCMinMax() const;
    void updatePositionMinMax() const;
    //void updateADSBInfo() const;

    void calculateTestDataMappings() const;
    TstDataMapping calculateTestDataMapping(boost::posix_time::ptime timestamp) const; // test tod
    void addRefPositionsSpeedsToMapping (TstDataMapping& mapping) const;
    //void addRefPositiosToMappingFast (TstDataMapping& mapping) const;
    void computeSectorInsideInfo() const;
    void computeSectorInsideInfo(InsideCheckMatrix& mat, 
                                 const dbContent::TargetPosition& pos,
                                 unsigned int idx_internal,
                                 const boost::optional<bool>& ground_bit,
                                 const SectorLayer* min_height_filter = nullptr) const;
    bool checkAbove(const InsideCheckMatrix& mat,
                    const Index& index) const;
    bool checkGroundBit(const InsideCheckMatrix& mat,
                        const Index& index) const;
    bool checkInside(const SectorLayer& layer,
                     const InsideCheckMatrix& mat,
                     const Index& index) const;
    
    DataMappingTimes findTstTimes(boost::posix_time::ptime timestamp_ref) const; // ref tod

    std::pair<bool, float> estimateRefAltitude (const boost::posix_time::ptime& timestamp, unsigned int index_internal) const;
    std::pair<bool, float> estimateTstAltitude (const boost::posix_time::ptime& timestamp, unsigned int index_internal) const;

    Index indexFromDataID(const DataID& id, DataType dtype) const;
    boost::posix_time::ptime timestampFromDataID(const DataID& id) const;

    EvaluationData&    eval_data_;
    EvaluationManager& eval_man_;
    DBContentManager&  dbcont_man_;

    std::multimap<boost::posix_time::ptime, Index> ref_data_; // timestamp -> index
    std::vector<unsigned int> ref_indices_;

    std::multimap<boost::posix_time::ptime, Index> tst_data_; // timestamp -> index
    std::vector<unsigned int> tst_indices_;
    mutable std::vector<TstDataMapping> tst_data_mappings_;
    
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

    mutable bool has_adsb_info_ {false};
    mutable bool has_mops_versions_ {false};
    mutable std::set<unsigned int> mops_versions_;
    mutable bool has_nucp_nic_ {false};
    mutable unsigned int min_nucp_nic_, max_nucp_nic_;
    mutable bool has_nacp {false};
    mutable unsigned int min_nacp_, max_nacp_;

    mutable Transformation trafo_;

    mutable InsideCheckMatrix                    inside_ref_;
    mutable InsideCheckMatrix                    inside_tst_;
    mutable InsideCheckMatrix                    inside_map_;
    mutable std::map<const SectorLayer*, size_t> inside_sector_layers_;
};

#endif // EVALUATIONTARGETDATA_H
