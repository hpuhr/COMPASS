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

#include "dbcontent/target/targetreportchain.h"
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

class EvaluationTargetData
{
public:
    typedef Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic> InsideCheckMatrix;

    EvaluationTargetData(unsigned int utn, 
                         EvaluationData& eval_data,
                         std::shared_ptr<dbContent::Cache> cache,
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

    bool use() const;

    const dbContent::TargetReport::Chain& refChain() const;
    const dbContent::TargetReport::Chain& tstChain() const;

//    const dbContent::TargetReport::Chain::IndexMap& refData() const;
//    const dbContent::TargetReport::Chain::IndexMap& tstData() const;

//    bool canCheckTstMultipleSources() const;
//    bool hasTstMultipleSources() const;

//    bool canCheckTrackLUDSID() const;
//    bool hasSingleLUDSID() const;
//    unsigned int singleTrackLUDSID() const;

    double latitudeMin() const;
    double latitudeMax() const;
    double longitudeMin() const;
    double longitudeMax() const;

    bool hasPos() const;

    bool hasADSBInfo() const;
    bool hasMOPSVersion() const;
    std::set<unsigned int> mopsVersions() const;
    std::string mopsVersionStr() const;

//    bool hasNucpNic() const;
//    std::string nucpNicStr() const;
//    bool hasNacp() const;
//    std::string nacpStr() const;

    //DataID dataID(const boost::posix_time::ptime& timestamp, DataType dtype) const;

    // ref
    bool hasMappedRefData(const dbContent::TargetReport::Chain::DataID& tst_id,
                          boost::posix_time::time_duration d_max) const;
    std::pair<boost::posix_time::ptime, boost::posix_time::ptime> mappedRefTimes(
            const dbContent::TargetReport::Chain::DataID& tst_id,
            boost::posix_time::time_duration d_max) const;
    // lower/upper times, {} if not existing

    boost::optional<dbContent::TargetPosition> mappedRefPos(
            const dbContent::TargetReport::Chain::DataID& tst_id) const;
    std::pair<dbContent::TargetPosition, bool> mappedRefPos(
            const dbContent::TargetReport::Chain::DataID& tst_id, boost::posix_time::time_duration d_max) const;
    // bool ok
    std::pair<dbContent::TargetVelocity, bool> mappedRefSpeed(
            const dbContent::TargetReport::Chain::DataID& tst_id, boost::posix_time::time_duration d_max) const;

//    bool hasRefPos(const DataID& id) const;
//    dbContent::TargetPosition refPos(const DataID& id) const;
//    bool hasRefSpeed(const DataID& id) const;
//    dbContent::TargetVelocity refSpeed(const DataID& id) const;
//    // estimate ref baro alt at tod,index TODO should be replaced by real altitude reconstructor

//    bool hasRefCallsign(const DataID& id) const;
//    std::string refCallsign(const DataID& id) const;

//    bool hasRefModeA(const DataID& id) const; // only if set, is v, not g
//    unsigned int refModeA(const DataID& id) const;

//    bool hasRefModeC(const DataID& id) const; // only if set, is v, not g
//    float refModeC(const DataID& id) const;

//    bool hasRefTA(const DataID& id) const;
//    unsigned int refTA(const DataID& id) const;

//    std::pair<bool,bool> refGroundBit(const DataID& id) const; // has gbs, gbs true
    std::pair<bool,bool> mappedRefGroundBit(
            const dbContent::TargetReport::Chain::DataID& tst_id, boost::posix_time::time_duration d_max) const; // has gbs, gbs true

    // test
//    bool hasTstPos(const DataID& id) const;
//    dbContent::TargetPosition tstPos(const DataID& id) const;

//    bool hasTstCallsign(const DataID& id) const;
//    std::string tstCallsign(const DataID& id) const;

//    bool hasTstModeA(const DataID& id) const; // only if set, is v, not g
//    unsigned int tstModeA(const DataID& id) const;

//    bool hasTstModeC(const DataID& id) const; // only if set, is v, not g
//    float tstModeC(const DataID& id) const;

//    bool hasTstGroundBit(const DataID& id) const; // only if set
//    bool tstGroundBit(const DataID& id) const; // true is on ground

//    bool hasTstTA(const DataID& id) const;
//    unsigned int tstTA(const DataID& id) const;

    std::pair<bool,bool> tstGroundBitInterpolated(const dbContent::TargetReport::Chain::DataID& ref_id) const; // has gbs, gbs true

//    bool hasTstTrackNum(const DataID& id) const;
//    unsigned int tstTrackNum(const DataID& id) const;

//    // speed, track angle
//    bool hasTstMeasuredSpeed(const DataID& id) const;
//    float tstMeasuredSpeed(const DataID& id) const; // m/s

//    bool hasTstMeasuredTrackAngle(const DataID& id) const;
//    float tstMeasuredTrackAngle(const DataID& id) const; // deg

//    // inside check
//    boost::optional<bool> availableGroundBit(const DataID& id,
//                                             Evaluation::DataType dtype,
//                                             const boost::posix_time::time_duration& d_max) const;
    boost::optional<bool> availableRefGroundBit(const dbContent::TargetReport::Chain::DataID& id,
                                                const boost::posix_time::time_duration& d_max) const;
    boost::optional<bool> availableTstGroundBit(const dbContent::TargetReport::Chain::DataID& id,
                                                const boost::posix_time::time_duration& d_max) const;

    bool refPosAbove(const dbContent::TargetReport::Chain::DataID& id) const;
    bool refPosGroundBitAvailable(const dbContent::TargetReport::Chain::DataID& id) const;
    bool refPosInside(const SectorLayer& layer,
                      const dbContent::TargetReport::Chain::DataID& id) const;
    bool tstPosAbove(const dbContent::TargetReport::Chain::DataID& id) const;
    bool tstPosGroundBitAvailable(const dbContent::TargetReport::Chain::DataID& id) const;
    bool tstPosInside(const SectorLayer& layer,
                      const dbContent::TargetReport::Chain::DataID& id) const;
    bool mappedRefPosAbove(const dbContent::TargetReport::Chain::DataID& id) const;
    bool mappedRefPosGroundBitAvailable(const dbContent::TargetReport::Chain::DataID& id) const;
    bool mappedRefPosInside(const SectorLayer& layer, 
                            const dbContent::TargetReport::Chain::DataID& id) const;

    static const int InterpGroundBitMaxSeconds = 15;

protected:
    void updateACIDs() const;
    void updateACADs() const;
    void updateModeACodes() const;
    void updateModeCMinMax() const;
    void updatePositionMinMax() const;
//    //void updateADSBInfo() const;

    void calculateTestDataMappings() const;
//    TstDataMapping calculateTestDataMapping(boost::posix_time::ptime timestamp) const; // test tod
//    void addRefPositionsSpeedsToMapping (TstDataMapping& mapping) const;
    //void addRefPositiosToMappingFast (TstDataMapping& mapping) const;
    void computeSectorInsideInfo() const;
    void computeSectorInsideInfo(InsideCheckMatrix& mat, 
                                 const dbContent::TargetPosition& pos,
                                 unsigned int idx_internal,
                                 const boost::optional<bool>& ground_bit,
                                 const SectorLayer* min_height_filter = nullptr) const;
    bool checkAbove(const InsideCheckMatrix& mat,
                    const dbContent::TargetReport::Index& index) const;
    bool checkGroundBit(const InsideCheckMatrix& mat,
                        const dbContent::TargetReport::Index& index) const;
    bool checkInside(const SectorLayer& layer,
                     const InsideCheckMatrix& mat,
                     const dbContent::TargetReport::Index& index) const;
    
//    DataMappingTimes findTstTimes(boost::posix_time::ptime timestamp_ref) const; // ref tod

//    std::pair<bool, float> estimateRefAltitude (const boost::posix_time::ptime& timestamp, unsigned int index_internal) const;
//    std::pair<bool, float> estimateTstAltitude (const boost::posix_time::ptime& timestamp, unsigned int index_internal) const;

//    Index indexFromDataID(const DataID& id, DataType dtype) const;
//    boost::posix_time::ptime timestampFromDataID(const DataID& id) const;

    EvaluationData&    eval_data_;
    std::shared_ptr<dbContent::Cache> cache_;
    EvaluationManager& eval_man_;
    DBContentManager&  dbcont_man_;

//    std::unique_ptr<dbContent::TargetReport::Chain> ref_chain_;
//    std::unique_ptr<dbContent::TargetReport::Chain> tst_chain_;

    dbContent::TargetReport::Chain ref_chain_;
    dbContent::TargetReport::Chain tst_chain_;

//    std::multimap<boost::posix_time::ptime, const dbContent::TargetReport::Index> ref_data_; // timestamp -> index
//    std::vector<unsigned int> ref_indices_;

//    std::multimap<boost::posix_time::ptime, const dbContent::TargetReport::Index> tst_data_; // timestamp -> index
//    std::vector<unsigned int> tst_indices_;
    mutable std::vector<dbContent::TargetReport::DataMapping> tst_data_mappings_;
    
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

    mutable bool has_adsb_info_ {false};
    mutable bool has_mops_versions_ {false};
    mutable std::set<unsigned int> mops_versions_;
    //    mutable bool has_nucp_nic_ {false};
    //    mutable unsigned int min_nucp_nic_, max_nucp_nic_;
    //    mutable bool has_nacp {false};
    //    mutable unsigned int min_nacp_, max_nacp_;

    //mutable Transformation trafo_;

    mutable InsideCheckMatrix                    inside_ref_;
    mutable InsideCheckMatrix                    inside_tst_;
    mutable InsideCheckMatrix                    inside_map_;
    mutable std::map<const SectorLayer*, size_t> inside_sector_layers_;
};

#endif // EVALUATIONTARGETDATA_H
