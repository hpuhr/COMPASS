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

    boost::optional<bool> mappedRefGroundBit(
            const dbContent::TargetReport::Chain::DataID& tst_id, boost::posix_time::time_duration d_max) const; // gbs

    // test
    boost::optional<bool> tstGroundBitInterpolated(const dbContent::TargetReport::Chain::DataID& ref_id) const; // gds

    // TODO d_max not used
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
    
    EvaluationData&    eval_data_;
    std::shared_ptr<dbContent::Cache> cache_;
    EvaluationManager& eval_man_;
    DBContentManager&  dbcont_man_;

    dbContent::TargetReport::Chain ref_chain_;
    dbContent::TargetReport::Chain tst_chain_;

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

    mutable InsideCheckMatrix                    inside_ref_;
    mutable InsideCheckMatrix                    inside_tst_;
    mutable InsideCheckMatrix                    inside_map_;
    mutable std::map<const SectorLayer*, size_t> inside_sector_layers_;
};

#endif // EVALUATIONTARGETDATA_H
