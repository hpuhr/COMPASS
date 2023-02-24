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

#include "evaluationtargetposition.h"
#include "evaluationtargetvelocity.h"
#include "projection/transformation.h"

#include "boost/date_time/posix_time/ptime.hpp"

#include <map>
#include <memory>
#include <vector>
#include <set>
#include <string>

class Buffer;
class EvaluationData;
class EvaluationManager;
class DBContentManager;

class TstDataMapping // mapping to respective ref data
{
public:
    boost::posix_time::ptime timestamp_; // timestmap of test

    bool has_ref1_ {false};
    boost::posix_time::ptime timestamp_ref1_;

    bool has_ref2_ {false};
    boost::posix_time::ptime timestamp_ref2_;

    bool has_ref_pos_ {false};
    EvaluationTargetPosition pos_ref_;

    bool has_ref_spd_ {false};
    EvaluationTargetVelocity spd_ref_;
};

class DataMappingTimes // mapping to respective tst data
{
public:
    boost::posix_time::ptime timestamp_; // tod of test

    bool has_other1_ {false};
    boost::posix_time::ptime timestamp_other1_;

    bool has_other2_ {false};
    boost::posix_time::ptime timestamp_other2_;
};

class EvaluationTargetData
{
public:
    EvaluationTargetData(unsigned int utn, EvaluationData& eval_data,
                         EvaluationManager& eval_man, DBContentManager& dbcont_man);
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
//    void use(bool use);

    const std::multimap<boost::posix_time::ptime, unsigned int>& refData() const;
    const std::multimap<boost::posix_time::ptime, unsigned int>& tstData() const;

    // ref
    bool hasRefDataForTime (boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const;
    std::pair<boost::posix_time::ptime, boost::posix_time::ptime> refTimesFor (
            boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const;
    // lower/upper times, {} if not existing

    std::pair<EvaluationTargetPosition, bool> interpolatedRefPosForTime (
            boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const;
    // bool ok
    std::pair<EvaluationTargetVelocity, bool> interpolatedRefSpdForTime (
            boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const;

    bool hasRefPosForTime (boost::posix_time::ptime timestamp) const;
    EvaluationTargetPosition refPosForTime (boost::posix_time::ptime timestamp) const;
    bool hasRefSpeedForTime (boost::posix_time::ptime timestamp) const;
    EvaluationTargetVelocity refSpdForTime (boost::posix_time::ptime timestamp) const;
    std::pair<bool, float> estimateRefAltitude (boost::posix_time::ptime timestamp, unsigned int index) const;
    // estimate ref baro alt at tod,index TODO should be replaced by real altitude reconstructor

    bool hasRefCallsignForTime (boost::posix_time::ptime timestamp) const;
    std::string refCallsignForTime (boost::posix_time::ptime timestamp) const;

    bool hasRefModeAForTime (boost::posix_time::ptime timestamp) const; // only if set, is v, not g
    unsigned int refModeAForTime (boost::posix_time::ptime timestamp) const;

    bool hasRefModeCForTime (boost::posix_time::ptime timestamp) const; // only if set, is v, not g
    float refModeCForTime (boost::posix_time::ptime timestamp) const;

    bool hasRefTAForTime (boost::posix_time::ptime timestamp) const;
    unsigned int refTAForTime (boost::posix_time::ptime timestamp) const;

    std::pair<bool,bool> refGroundBitForTime (boost::posix_time::ptime timestamp) const; // has gbs, gbs true
    std::pair<bool,bool> interpolatedRefGroundBitForTime (
            boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const; // has gbs, gbs true

    // test
    bool hasTstPosForTime (boost::posix_time::ptime timestamp) const;
    EvaluationTargetPosition tstPosForTime (boost::posix_time::ptime timestamp) const;
    std::pair<bool, float> estimateTstAltitude (boost::posix_time::ptime timestamp, unsigned int index) const;

    bool hasTstCallsignForTime (boost::posix_time::ptime timestamp) const;
    std::string tstCallsignForTime (boost::posix_time::ptime timestamp) const;

    bool hasTstModeAForTime (boost::posix_time::ptime timestamp) const; // only if set, is v, not g
    unsigned int tstModeAForTime (boost::posix_time::ptime timestamp) const;

    bool hasTstModeCForTime (boost::posix_time::ptime timestamp) const; // only if set, is v, not g
    float tstModeCForTime (boost::posix_time::ptime timestamp) const;

    bool hasTstGroundBitForTime (boost::posix_time::ptime timestamp) const; // only if set
    bool tstGroundBitForTime (boost::posix_time::ptime timestamp) const; // true is on ground

    bool hasTstTAForTime (boost::posix_time::ptime timestamp) const;
    unsigned int tstTAForTime (boost::posix_time::ptime timestamp) const;

    std::pair<bool,bool> tstGroundBitForTimeInterpolated (boost::posix_time::ptime timestamp) const; // has gbs, gbs true

    bool hasTstTrackNumForTime (boost::posix_time::ptime timestamp) const;
    unsigned int tstTrackNumForTime (boost::posix_time::ptime timestamp) const;

    // speed, track angle
    bool hasTstMeasuredSpeedForTime (boost::posix_time::ptime timestamp) const;
    float tstMeasuredSpeedForTime (boost::posix_time::ptime timestamp) const; // m/s

    bool hasTstMeasuredTrackAngleForTime (boost::posix_time::ptime timestamp) const;
    float tstMeasuredTrackAngleForTime (boost::posix_time::ptime timestamp) const; // deg

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

protected:
    EvaluationData& eval_data_;
    EvaluationManager& eval_man_;
    DBContentManager& dbcont_man_;

    std::multimap<boost::posix_time::ptime, unsigned int> ref_data_; // timestamp -> index
    mutable std::vector<unsigned int> ref_indexes_;

    std::multimap<boost::posix_time::ptime, unsigned int> tst_data_; // timestamp -> index
    mutable std::vector<unsigned int> tst_indexes_;

    mutable std::set<std::string> callsigns_;
    mutable std::set<unsigned int> target_addresses_;
    mutable std::set<unsigned int> mode_a_codes_;

    mutable bool has_mode_c_ {false};
    mutable float mode_c_min_ {0};
    mutable float mode_c_max_ {0};

    mutable bool has_pos_ {false};
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

    mutable std::map<boost::posix_time::ptime, TstDataMapping> test_data_mappings_;

    mutable Transformation trafo_;

    void updateCallsigns() const;
    void updateTargetAddresses() const;
    void updateModeACodes() const;
    void updateModeCMinMax() const;
    void updatePositionMinMax() const;
    //void updateADSBInfo() const;

    void calculateTestDataMappings() const;
    TstDataMapping calculateTestDataMapping(boost::posix_time::ptime timestamp) const; // test tod
    void addRefPositionsSpeedsToMapping (TstDataMapping& mapping) const;
    void addRefPositiosToMappingFast (TstDataMapping& mapping) const;

    DataMappingTimes findTstTimes(boost::posix_time::ptime timestamp_ref) const; // ref tod
};

#endif // EVALUATIONTARGETDATA_H
