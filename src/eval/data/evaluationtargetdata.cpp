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

#include "evaluationtargetdata.h"
#include "buffer.h"
#include "logger.h"
#include "stringconv.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/metavariable.h"
#include "dbcontent/target/target.h"
#include "compass.h"
#include "dbcontent/dbcontentmanager.h"
#include "evaluationmanager.h"
#include "util/number.h"
#include "util/timeconv.h"
#include "sector/airspace.h"

#include <boost/algorithm/string.hpp>

#include <cassert>
#include <algorithm>
#include <cmath>

#include <Eigen/Core>

using namespace std;
using namespace Utils;
using namespace boost::posix_time;
using namespace dbContent::TargetReport;

//const unsigned int debug_utn = 3275;

EvaluationTargetData::EvaluationTargetData(unsigned int utn, 
                                           EvaluationData& eval_data,
                                           std::shared_ptr<dbContent::Cache> cache,
                                           EvaluationManager& eval_man,
                                           DBContentManager& dbcont_man)
    :   utn_       (utn)
    ,   eval_data_ (eval_data)
    ,   cache_(cache)
    ,   eval_man_  (eval_man)
    ,   dbcont_man_(dbcont_man)
    ,   ref_chain_(cache_, eval_man_.dbContentNameRef())
    ,   tst_chain_(cache_, eval_man_.dbContentNameTst())
{
}

EvaluationTargetData::~EvaluationTargetData() = default;

//EvaluationTargetData::DataID EvaluationTargetData::dataID(const boost::posix_time::ptime& timestamp,
//                                                          DataType dtype) const
//{
//    const IndexMap& index_map = (dtype == DataType::Reference) ? ref_data_ : tst_data_;

//    auto range = index_map.equal_range(timestamp);
//#if 0
//    if (range.first == index_map.end())
//        return DataID();
//#else
//    assert(range.first != index_map.end());
//#endif

//    assert(range.first->second.idx_internal < index_map.size());

//    return DataID(timestamp).addIndex(range.first->second);
//}

//EvaluationTargetData::Index EvaluationTargetData::indexFromDataID(const DataID& id, DataType dtype) const
//{
//    assert(id.valid());

//    if (!id.hasIndex())
//    {
//        auto id_ret = dataID(id.timestamp(), dtype);
//        assert(id_ret.valid());

//        return id_ret.index();
//    }

//    return id.index();
//}

//boost::posix_time::ptime EvaluationTargetData::timestampFromDataID(const DataID& id) const
//{
//    assert(id.valid());
//    return id.timestamp();
//}

void EvaluationTargetData::addRefIndex (boost::posix_time::ptime timestamp, unsigned int index)
{
    ref_chain_.addIndex(timestamp, index);

    //    unsigned int idx_int = (unsigned int)ref_indices_.size();

    //    ref_chain_.insert({timestamp, Index(index, idx_int)});
    //    ref_indices_.push_back(index);
}

void EvaluationTargetData::addTstIndex (boost::posix_time::ptime timestamp, unsigned int index)
{
    tst_chain_.addIndex(timestamp, index);

    //    unsigned int idx_int = (unsigned int)tst_indices_.size();

    //    tst_chain_.insert({timestamp, Index(index, idx_int)});
    //    tst_indices_.push_back(index);
}

bool EvaluationTargetData::hasData() const
{
    return (hasRefData() || hasTstData());
}

bool EvaluationTargetData::hasRefData () const
{
    //return !ref_chain_.empty();
    return ref_chain_.hasData();
}

bool EvaluationTargetData::hasTstData () const
{
    //return !tst_chain_.empty();
    return tst_chain_.hasData();
}

void EvaluationTargetData::finalize () const
{
    //    loginf << "EvaluationTargetData: finalize: utn " << utn_
    //           << " ref " << hasRefData() << " up " << ref_rec_nums_.size()
    //           << " tst " << hasTstData() << " up " << tst_rec_nums_.size();

    ref_chain_.finalize();
    tst_chain_.finalize();

    updateACIDs();
    updateACADs();
    updateModeACodes();
    updateModeCMinMax();
    updatePositionMinMax();

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    if (dbcont_man.hasTargetsInfo() && dbcont_man.existsTarget(utn_)
            && dbcont_man.target(utn_).hasAdsbMOPSVersions())
    {
        has_adsb_info_ = true;
        has_mops_versions_ = true;
        mops_versions_ = dbcont_man.target(utn_).adsbMOPSVersions();
    }

    //    std::set<unsigned int> mops_version;
    //    std::tuple<bool, unsigned int, unsigned int> nucp_info;
    //    std::tuple<bool, unsigned int, unsigned int> nacp_info;

    //    if (eval_man_->hasADSBInfo() && target_addresses_.size())
    //    {
    //        for (auto ta_it : target_addresses_)
    //        {
    //            if (eval_man_->hasADSBInfo(ta_it))
    //            {
    //                tie(mops_version, nucp_info, nacp_info) = eval_man_->adsbInfo(ta_it);
    //                mops_versions_.insert(mops_version.begin(), mops_version.end());

    //                if (get<0>(nucp_info))
    //                {
    //                    if (has_nucp_nic_)
    //                    {
    //                        min_nucp_nic_ = min (min_nucp_nic_, get<1>(nucp_info));
    //                        max_nucp_nic_ = max (max_nucp_nic_, get<2>(nucp_info));
    //                    }
    //                    else
    //                    {
    //                        min_nucp_nic_ = get<1>(nucp_info);
    //                        max_nucp_nic_ = get<2>(nucp_info);
    //                        has_nucp_nic_ = true;
    //                    }
    //                }

    //                if (get<0>(nacp_info))
    //                {
    //                    if (has_nacp)
    //                    {
    //                        min_nacp_ = min (min_nacp_, get<1>(nacp_info));
    //                        max_nacp_ = max (max_nacp_, get<2>(nacp_info));
    //                    }
    //                    else
    //                    {
    //                        min_nacp_ = get<1>(nacp_info);
    //                        max_nacp_ = get<2>(nacp_info);
    //                        has_nacp = true;
    //                    }
    //                }
    //            }
    //        }

    //        has_adsb_info_ = mops_versions_.size();
    //        //        loginf << "UGA utn " << utn_ << " mops " << mopsVersionsStr()
    //        //               << " nucp_nic " <<  (has_nucp_nic_ ? nucpNicStr() : " none ")
    //        //               << " nacp "<<  (has_nacp ? nacpStr() : " none ");
    //    }

    calculateTestDataMappings();
    computeSectorInsideInfo();
}

unsigned int EvaluationTargetData::numUpdates () const
{
    return ref_chain_.size() + tst_chain_.size();
}

unsigned int EvaluationTargetData::numRefUpdates () const
{
    return ref_chain_.size() ;
}

unsigned int EvaluationTargetData::numTstUpdates () const
{
    return tst_chain_.size() ;
}

ptime EvaluationTargetData::timeBegin() const
{
    if (ref_chain_.size() && tst_chain_.size())
        return min(ref_chain_.timeBegin(), tst_chain_.timeBegin());
    else if (ref_chain_.size())
        return ref_chain_.timeBegin();
    else if (tst_chain_.size())
        return tst_chain_.timeBegin();
    else
        throw std::runtime_error("EvaluationTargetData: timeBegin: no data");
}

std::string EvaluationTargetData::timeBeginStr() const
{
    if (hasData())
        return Time::toString(timeBegin());
    else
        return "";
}

ptime EvaluationTargetData::timeEnd() const
{
    if (ref_chain_.size() && tst_chain_.size())
        return max(ref_chain_.timeEnd(), tst_chain_.timeEnd());
    else if (ref_chain_.size())
        return ref_chain_.timeEnd();
    else if (tst_chain_.size())
        return tst_chain_.timeEnd();
    else
        throw std::runtime_error("EvaluationTargetData: timeEnd: no data");
}

std::string EvaluationTargetData::timeEndStr() const
{
    if (hasData())
        return Time::toString(timeEnd());
    else
        return "";
}

time_duration EvaluationTargetData::timeDuration() const
{
    if (hasData())
        return timeEnd() - timeBegin();
    else
        return {};
}

std::set<unsigned int> EvaluationTargetData::modeACodes() const
{
    logdbg << "EvaluationTargetData: modeACodes: utn " << utn_ << " num codes " << mode_a_codes_.size();
    return mode_a_codes_;
}

std::string EvaluationTargetData::modeACodesStr() const
{
    std::ostringstream out;

    unsigned int cnt=0;
    for (auto& ma_it : mode_a_codes_)
    {
        if (cnt != 0)
            out << ", ";

        out << String::octStringFromInt(ma_it, 4, '0');

        ++cnt;
    }

    return out.str();
}

bool EvaluationTargetData::hasModeC() const
{
    return has_mode_c_;
}

float EvaluationTargetData::modeCMin() const
{
    assert (has_mode_c_);
    return mode_c_min_;
}

std::string EvaluationTargetData::modeCMinStr() const
{
    if (has_mode_c_)
        return to_string((int)mode_c_min_);
    else
        return "";
}

float EvaluationTargetData::modeCMax() const
{
    assert (has_mode_c_);
    return mode_c_max_;
}

std::string EvaluationTargetData::modeCMaxStr() const
{
    if (has_mode_c_)
        return to_string((int)mode_c_max_);
    else
        return "";
}

bool EvaluationTargetData::isPrimaryOnly () const
{
    return !acids_.size() && !acads_.size() && !mode_a_codes_.size() && !has_mode_c_;
}

bool EvaluationTargetData::use() const
{
    return dbcont_man_.utnUseEval(utn_);
}

const dbContent::TargetReport::Chain& EvaluationTargetData::refChain() const
{
    return ref_chain_;
}

const dbContent::TargetReport::Chain& EvaluationTargetData::tstChain() const
{
    return tst_chain_;
}

//const EvaluationTargetData::IndexMap& EvaluationTargetData::refData() const
//{
//    return ref_data_;
//}

//const EvaluationTargetData::IndexMap& EvaluationTargetData::tstData() const
//{
//    return tst_data_;
//}

bool EvaluationTargetData::hasMappedRefData(const DataID& tst_id,
                                            time_duration d_max) const
{
    //auto timestamp = tst_chain_.timestampFromDataID(tst_id);
    auto timestamp = tst_id.timestamp();
    auto index     = tst_chain_.indexFromDataID(tst_id);

    const DataMapping& mapping = tst_data_mappings_.at(index.idx_internal);

    if (!mapping.has_ref1_ && !mapping.has_ref2_) // no ref data
        return false;

    if (mapping.has_ref1_ && mapping.has_ref2_) // interpolated
    {
        assert (mapping.timestamp_ref1_ <= timestamp);
        assert (mapping.timestamp_ref2_ >= timestamp);

        if (timestamp - mapping.timestamp_ref1_ > d_max) // lower to far
            return false;

        if (mapping.timestamp_ref2_ - timestamp > d_max) // upper to far
            return false;

        return true;
    }

    return false;
}

std::pair<ptime, ptime> EvaluationTargetData::mappedRefTimes(const DataID& tst_id,
                                                             time_duration d_max)  const
{
    auto timestamp = tst_id.timestamp();
    auto index     = tst_chain_.indexFromDataID(tst_id);

    const DataMapping& mapping = tst_data_mappings_.at(index.idx_internal);

    if (!mapping.has_ref1_ && !mapping.has_ref2_) // no ref data
        return {{}, {}};

    if (mapping.has_ref1_ && mapping.has_ref2_) // interpolated
    {
        assert (mapping.timestamp_ref1_ <= timestamp);
        assert (mapping.timestamp_ref2_ >= timestamp);

        if (timestamp - mapping.timestamp_ref1_ > d_max) // lower to far
            return {{}, {}};

        if (mapping.timestamp_ref2_ - timestamp > d_max) // upper to far
            return {{}, {}};

        return {mapping.timestamp_ref1_, mapping.timestamp_ref2_};
    }

    return {{}, {}};
}

boost::optional<dbContent::TargetPosition> EvaluationTargetData::mappedRefPos(const DataID& tst_id) const
{
    auto index = tst_chain_.indexFromDataID(tst_id);

    const auto& tdm = tst_data_mappings_.at(index.idx_internal);
    if (!tdm.has_ref_pos_)
        return {};

    return tdm.pos_ref_;
}

std::pair<dbContent::TargetPosition, bool> EvaluationTargetData::mappedRefPos(const DataID& tst_id,
                                                                              time_duration d_max) const
{
    auto timestamp = tst_id.timestamp();
    auto index     = tst_chain_.indexFromDataID(tst_id);

    const DataMapping& mapping = tst_data_mappings_.at(index.idx_internal);

    if (!mapping.has_ref1_ && !mapping.has_ref2_) // no ref data
        return {{}, false};

    if (mapping.has_ref1_ && mapping.has_ref2_) // interpolated
    {
        assert (mapping.timestamp_ref1_ <= timestamp);
        assert (mapping.timestamp_ref2_ >= timestamp);

        if (timestamp - mapping.timestamp_ref1_ > d_max) // lower to far
        {
            //            if (utn_ == debug_utn)
            //                loginf << "EvaluationTargetData: interpolatedRefPosForTime: lower too far";

            return {{}, false};
        }

        if (mapping.timestamp_ref2_ - timestamp > d_max) // upper to far
        {
            //            if (utn_ == debug_utn)
            //                loginf << "EvaluationTargetData: interpolatedRefPosForTime: upper too far";

            return {{}, false};
        }

        if (!mapping.has_ref_pos_)
        {
            //            if (utn_ == debug_utn)
            //                loginf << "EvaluationTargetData: interpolatedRefPosForTime: no ref pos";

            return {{}, false};
        }

        //        if (utn_ == debug_utn)
        //            loginf << "EvaluationTargetData: interpolatedRefPosForTime: 2pos tod " << String::timeStringFromDouble(tod)
        //                   << " has_alt " << mapping.pos_ref_.has_altitude_
        //                   << " alt_calc " << mapping.pos_ref_.altitude_calculated_
        //                   << " alt " << mapping.pos_ref_.altitude_;

        return {mapping.pos_ref_, true};
    }

    return {{}, false};
}

std::pair<dbContent::TargetVelocity, bool> EvaluationTargetData::mappedRefSpeed(const DataID& tst_id,
                                                                                time_duration d_max) const
{
    auto timestamp = tst_id.timestamp();
    auto index     = tst_chain_.indexFromDataID(tst_id);

    const DataMapping& mapping = tst_data_mappings_.at(index.idx_internal);

    if (!mapping.has_ref1_ && !mapping.has_ref2_) // no ref data
        return {{}, false};

    if (mapping.has_ref1_ && mapping.has_ref2_) // interpolated
    {
        assert (mapping.timestamp_ref1_ <= timestamp);
        assert (mapping.timestamp_ref2_ >= timestamp);

        if (timestamp - mapping.timestamp_ref1_ > d_max) // lower to far
        {
            //            if (utn_ == debug_utn)
            //                loginf << "EvaluationTargetData: interpolatedRefPosForTime: lower too far";

            return {{}, false};
        }

        if (mapping.timestamp_ref2_ - timestamp > d_max) // upper to far
        {
            //            if (utn_ == debug_utn)
            //                loginf << "EvaluationTargetData: interpolatedRefPosForTime: upper too far";

            return {{}, false};
        }

        if (!mapping.has_ref_spd_)
        {
            //            if (utn_ == debug_utn)
            //                loginf << "EvaluationTargetData: interpolatedRefPosForTime: no ref pos";

            return {{}, false};
        }

        //        if (utn_ == debug_utn)
        //            loginf << "EvaluationTargetData: interpolatedRefPosForTime: 2pos tod " << String::timeStringFromDouble(tod)
        //                   << " has_alt " << mapping.pos_ref_.has_altitude_
        //                   << " alt_calc " << mapping.pos_ref_.altitude_calculated_
        //                   << " alt " << mapping.pos_ref_.altitude_;

        return {mapping.spd_ref_, true};
    }

    return {{}, false};
}

std::pair<bool,bool> EvaluationTargetData::mappedRefGroundBit(const DataID& tst_id,
                                                              time_duration d_max) const
// has gbs, gbs true
{
    bool has_gbs = false;
    bool gbs = false;

    auto timestamp = tst_id.timestamp();
    auto index     = tst_chain_.indexFromDataID(tst_id);

    const DataMapping& mapping = tst_data_mappings_.at(index.idx_internal);

    if (!mapping.has_ref1_ && !mapping.has_ref2_) // no ref data
        return {has_gbs, gbs};

    if (mapping.has_ref1_ && mapping.has_ref2_) // interpolated
    {
        assert (mapping.timestamp_ref1_ <= timestamp);
        assert (mapping.timestamp_ref2_ >= timestamp);

        if (timestamp - mapping.timestamp_ref1_ > d_max) // lower to far
            return {has_gbs, gbs};

        if (mapping.timestamp_ref2_ - timestamp > d_max) // upper to far
            return {has_gbs, gbs};

        tie (has_gbs, gbs) = ref_chain_.groundBit(mapping.timestamp_ref1_);

        if (!gbs)
            tie (has_gbs, gbs) = ref_chain_.groundBit(mapping.timestamp_ref2_);
    }

    return {has_gbs, gbs};
}

//bool EvaluationTargetData::hasRefPos(const DataID& id) const
//{
//    auto timestamp = timestampFromDataID(id);
//    return ref_chain_.count(timestamp);
//}

//dbContent::TargetPosition EvaluationTargetData::refPos(const DataID& id) const
//{
//    auto timestamp = timestampFromDataID(id);
//    auto index     = indexFromDataID(id, DataType::Reference);

//    unsigned int index_ext = index.idx_external;

//    dbContent::TargetPosition pos;

//    NullableVector<double>& latitude_vec  = eval_data_.ref_buffer_->get<double>(eval_data_.ref_latitude_name_);
//    NullableVector<double>& longitude_vec = eval_data_.ref_buffer_->get<double>(eval_data_.ref_longitude_name_);
//    NullableVector<float>& altitude_vec   = eval_data_.ref_buffer_->get<float>(eval_data_.ref_modec_name_);

//    NullableVector<float>* altitude_trusted_vec {nullptr};

//    if (eval_data_.ref_modec_trusted_name_.size())
//    {
//        assert (eval_data_.ref_buffer_->has<float>(eval_data_.ref_modec_trusted_name_));
//        altitude_trusted_vec = &eval_data_.ref_buffer_->get<float>(eval_data_.ref_modec_trusted_name_);
//    }

//    assert (!latitude_vec.isNull(index_ext));
//    assert (!longitude_vec.isNull(index_ext));

//    pos.latitude_  = latitude_vec.get(index_ext);
//    pos.longitude_ = longitude_vec.get(index_ext);

//    if (altitude_trusted_vec && !altitude_trusted_vec->isNull(index_ext))
//    {
//        pos.has_altitude_        = true;
//        pos.altitude_calculated_ = false;
//        pos.altitude_            = altitude_trusted_vec->get(index_ext);
//    }
//    else if (!altitude_vec.isNull(index_ext))
//    {
//        pos.has_altitude_        = true;
//        pos.altitude_calculated_ = false;
//        pos.altitude_            = altitude_vec.get(index_ext);
//    }
//    else if (eval_data_.has_ref_altitude_secondary_
//             && !eval_data_.ref_buffer_->get<float>(eval_data_.ref_altitude_secondary_name_).isNull(index_ext))
//    {
//        pos.has_altitude_        = true;
//        pos.altitude_calculated_ = true;
//        pos.altitude_            = eval_data_.ref_buffer_->get<float>(eval_data_.ref_altitude_secondary_name_).get(index_ext);
//    }
//    else // calculate
//    {
//        bool found;
//        float alt_calc;

//        tie(found,alt_calc) = estimateRefAltitude(timestamp, index.idx_internal);

//        if (found)
//        {
//            pos.has_altitude_        = true;
//            pos.altitude_calculated_ = true;
//            pos.altitude_            = alt_calc;
//        }
//    }

//    //    if (utn_ == debug_utn)
//    //        loginf << "EvaluationTargetData: refPosForTime: tod " << String::timeStringFromDouble(tod)
//    //               << " has_alt " << pos.has_altitude_ << " alt_calc " << pos.altitude_calculated_
//    //               << " alt " << pos.altitude_;

//    return pos;
//}

//std::pair<bool, float> EvaluationTargetData::estimateRefAltitude (const boost::posix_time::ptime& timestamp,
//                                                                  unsigned int index_internal) const
//{
//    assert(index_internal < ref_indices_.size());

//    NullableVector<float>& altitude_vec = eval_data_.ref_buffer_->get<float>(eval_data_.ref_modec_name_);
//    NullableVector<ptime>& ts_vec = eval_data_.ref_buffer_->get<ptime>(eval_data_.ref_timestamp_name_);

//    NullableVector<float>* altitude_trusted_vec {nullptr};

//    if (eval_data_.ref_modec_trusted_name_.size())
//    {
//        assert (eval_data_.ref_buffer_->has<float>(eval_data_.ref_modec_trusted_name_));
//        altitude_trusted_vec = &eval_data_.ref_buffer_->get<float>(eval_data_.ref_modec_trusted_name_);
//    }

//    bool found_prev {false};
//    float altitude_prev {0.0};
//    bool found_after {false};
//    float altitude_after {0.0};

//    // search for prev index
//    ptime timestamp_prev;
//    auto prev_it  = ref_indices_.begin() + index_internal;
//    auto after_it = prev_it;

//    const time_duration max_tdiff = seconds(120);

//    while (prev_it != ref_indices_.end() && timestamp - ts_vec.get(*prev_it) < max_tdiff)
//    {
//        if (altitude_trusted_vec && !altitude_trusted_vec->isNull(*prev_it))
//        {
//            found_prev = true;
//            altitude_prev = altitude_trusted_vec->get(*prev_it);
//            timestamp_prev = ts_vec.get(*prev_it);

//            break;
//        }
//        else if (!altitude_vec.isNull(*prev_it))
//        {
//            found_prev = true;
//            altitude_prev = altitude_vec.get(*prev_it);
//            timestamp_prev = ts_vec.get(*prev_it);

//            //            if (utn_ == debug_utn)
//            //                loginf << "EvaluationTargetData: refPosForTime: found prev at tod "
//            //                       << String::timeStringFromDouble(tod_prev);

//            break;
//        }

//        if (prev_it == ref_indices_.begin()) // undefined decrement
//            break;

//        --prev_it;
//    }

//    //    if (utn_ == debug_utn)
//    //    {
//    //        if (prev_it != ref_indices_.end())
//    //            loginf << "EvaluationTargetData: refPosForTime: checking prev found end";
//    //        else if (prev_it != ref_indices_.begin())
//    //            loginf << "EvaluationTargetData: refPosForTime: checking prev found begin";
//    //        else
//    //            loginf << "EvaluationTargetData: refPosForTime: finished prev tod "
//    //                   << String::timeStringFromDouble(tods.get(*prev_it)) << " found " << found_prev;
//    //    }

//    // search after index
//    ptime timestamp_after;

//    while (after_it != ref_indices_.end() && ts_vec.get(*after_it) - timestamp < max_tdiff)
//    {
//        //        if (utn_ == debug_utn)
//        //            loginf << "EvaluationTargetData: refPosForTime: checking after tod "
//        //                   << String::timeStringFromDouble(tods.get(*after_it));

//        if (altitude_trusted_vec && !altitude_trusted_vec->isNull(*after_it))
//        {
//            found_after = true;
//            altitude_after = altitude_trusted_vec->get(*after_it);
//            timestamp_after = ts_vec.get(*after_it);

//            break;
//        }
//        else if (!altitude_vec.isNull(*after_it))
//        {
//            found_after = true;
//            altitude_after = altitude_vec.get(*after_it);
//            timestamp_after = ts_vec.get(*after_it);

//            //            if (utn_ == debug_utn)
//            //                loginf << "EvaluationTargetData: refPosForTime: found after at tod "
//            //                       << String::timeStringFromDouble(tod_after);

//            break;
//        }
//        ++after_it;
//    }

//    //    if (utn_ == debug_utn)
//    //    {
//    //        if (after_it != ref_indices_.end())
//    //            loginf << "EvaluationTargetData: refPosForTime: checking after found end";
//    //        else
//    //            loginf << "EvaluationTargetData: refPosForTime: finished after tod "
//    //                   << String::timeStringFromDouble(tods.get(*after_it)) << " found " << found_after;
//    //    }

//    if (found_prev && found_after)
//    {
//        if (timestamp_after <= timestamp_prev || timestamp_prev >= timestamp)
//        {
//            logerr << "EvaluationTargetData::estimateRefAltitude ts_prev " << Time::toString(timestamp_prev)
//                   << " ts " << Time::toString(timestamp)
//                   << " ts_after " << Time::toString(timestamp_after);

//            return {false, 0}; // should never happen
//        }

//        float d_alt_ft = altitude_after - altitude_prev;
//        float d_t = Time::partialSeconds(timestamp_after - timestamp_prev);

//        float alt_spd_ft_s = d_alt_ft/d_t;

//        float d_t2 = Time::partialSeconds(timestamp - timestamp_prev);

//        float alt_calc = altitude_prev + alt_spd_ft_s*d_t2;

//        //loginf << "UGA " << alt_calc;

//        //        if (utn_ == debug_utn)
//        //            loginf << "EvaluationTargetData: refPosForTime: returning alt " << alt_calc;

//        return {true, alt_calc};
//    }
//    else if (found_prev && timestamp - timestamp_prev < max_tdiff)
//        return {true, altitude_prev};
//    else if (found_after && timestamp_after - timestamp < max_tdiff)
//        return {true, altitude_after};
//    else
//    {
//        //        if (utn_ == debug_utn)
//        //            loginf << "EvaluationTargetData: refPosForTime: tod " << String::timeStringFromDouble(tod)
//        //                   << " none found";

//        return {false, 0}; // none found
//    }
//}

//bool EvaluationTargetData::hasRefSpeed(const DataID& id) const
//{
//    auto index = indexFromDataID(id, DataType::Reference);

//    unsigned int index_ext = index.idx_external;

//    NullableVector<double>& speed_vec = eval_data_.ref_buffer_->get<double>(
//                eval_data_.ref_spd_ground_speed_kts_name_);
//    NullableVector<double>& track_angle_vec = eval_data_.ref_buffer_->get<double>(
//                eval_data_.ref_spd_track_angle_deg_name_);

//    return !speed_vec.isNull(index_ext) && !track_angle_vec.isNull(index_ext);
//}

//dbContent::TargetVelocity EvaluationTargetData::refSpeed(const DataID& id) const
//{
//    auto index = indexFromDataID(id, DataType::Reference);

//    unsigned int index_ext = index.idx_external;

//    NullableVector<double>& speed_vec = eval_data_.ref_buffer_->get<double>(
//                eval_data_.ref_spd_ground_speed_kts_name_);
//    NullableVector<double>& track_angle_vec = eval_data_.ref_buffer_->get<double>(
//                eval_data_.ref_spd_track_angle_deg_name_);

//    assert (!speed_vec.isNull(index_ext));
//    assert (!track_angle_vec.isNull(index_ext));

//    dbContent::TargetVelocity spd;

//    spd.speed_ = speed_vec.get(index_ext) * KNOTS2M_S; // true north to mathematical
//    spd.track_angle_ = track_angle_vec.get(index_ext);

//    return spd;
//}

//bool EvaluationTargetData::hasRefCallsign(const DataID& id) const
//{
//    auto index = indexFromDataID(id, DataType::Reference);

//    unsigned int index_ext = index.idx_external;

//    NullableVector<string>& callsign_vec = eval_data_.ref_buffer_->get<string>(eval_data_.ref_callsign_name_);

//    return !callsign_vec.isNull(index_ext);
//}

//std::string EvaluationTargetData::refCallsign(const DataID& id) const
//{
//    assert (hasRefCallsign(id));

//    auto index = indexFromDataID(id, DataType::Reference);

//    unsigned int index_ext = index.idx_external;

//    NullableVector<string>& callsign_vec = eval_data_.ref_buffer_->get<string>(eval_data_.ref_callsign_name_);
//    assert (!callsign_vec.isNull(index_ext));

//    return boost::trim_copy(callsign_vec.get(index_ext)); // remove spaces
//}

//bool EvaluationTargetData::hasRefModeA(const DataID& id) const
//{
//    auto index = indexFromDataID(id, DataType::Reference);

//    unsigned int index_ext = index.idx_external;

//    if (eval_data_.ref_buffer_->get<unsigned int>(eval_data_.ref_modea_name_).isNull(index_ext))
//        return false;

//    if (eval_data_.ref_modea_v_name_.size()
//            && !eval_data_.ref_buffer_->get<bool>(eval_data_.ref_modea_v_name_).isNull(index_ext)
//            && !eval_data_.ref_buffer_->get<bool>(eval_data_.ref_modea_v_name_).get(index_ext)) // not valid
//        return false;

//    if (eval_data_.ref_modea_g_name_.size()
//            && !eval_data_.ref_buffer_->get<bool>(eval_data_.ref_modea_g_name_).isNull(index_ext)
//            && eval_data_.ref_buffer_->get<bool>(eval_data_.ref_modea_g_name_).get(index_ext)) // garbled
//        return false;

//    return true;
//}

//unsigned int EvaluationTargetData::refModeA(const DataID& id) const
//{
//    assert (hasRefModeA(id));

//    auto index = indexFromDataID(id, DataType::Reference);

//    unsigned int index_ext = index.idx_external;

//    NullableVector<unsigned int>& modea_vec = eval_data_.ref_buffer_->get<unsigned int>(eval_data_.ref_modea_name_);
//    assert (!modea_vec.isNull(index_ext));

//    return modea_vec.get(index_ext);
//}

//bool EvaluationTargetData::hasRefModeC(const DataID& id) const
//{
//    auto index = indexFromDataID(id, DataType::Reference);

//    unsigned int index_ext = index.idx_external;

//    if (eval_data_.ref_modec_trusted_name_.size() &&
//            !eval_data_.ref_buffer_->get<float>(eval_data_.ref_modec_trusted_name_).isNull(index_ext))
//        return true;

//    if (eval_data_.ref_buffer_->get<float>(eval_data_.ref_modec_name_).isNull(index_ext))
//        return false;

//    if (eval_data_.ref_modec_v_name_.size()
//            && !eval_data_.ref_buffer_->get<bool>(eval_data_.ref_modec_v_name_).isNull(index_ext)
//            && !eval_data_.ref_buffer_->get<bool>(eval_data_.ref_modec_v_name_).get(index_ext)) // not valid
//        return false;

//    if (eval_data_.ref_modec_g_name_.size()
//            && !eval_data_.ref_buffer_->get<bool>(eval_data_.ref_modec_g_name_).isNull(index_ext)
//            && eval_data_.ref_buffer_->get<bool>(eval_data_.ref_modec_g_name_).get(index_ext)) // garbled
//        return false;

//    return true;
//}

//float EvaluationTargetData::refModeC(const DataID& id) const
//{
//    assert (hasRefModeC(id));

//    auto index = indexFromDataID(id, DataType::Reference);

//    unsigned int index_ext = index.idx_external;

//    if (eval_data_.ref_modec_trusted_name_.size() &&
//            !eval_data_.ref_buffer_->get<float>(eval_data_.ref_modec_trusted_name_).isNull(index_ext))
//        return eval_data_.ref_buffer_->get<float>(eval_data_.ref_modec_trusted_name_).get(index_ext);

//    NullableVector<float>& modec_vec = eval_data_.ref_buffer_->get<float>(eval_data_.ref_modec_name_);
//    assert (!modec_vec.isNull(index_ext));

//    return modec_vec.get(index_ext);
//}

//bool EvaluationTargetData::hasRefTA(const DataID& id) const
//{
//    auto index = indexFromDataID(id, DataType::Reference);

//    unsigned int index_ext = index.idx_external;

//    if (eval_data_.ref_target_address_name_.size()
//            && !eval_data_.ref_buffer_->get<unsigned int>(eval_data_.ref_target_address_name_).isNull(index_ext))
//        return true;

//    return false;
//}

//unsigned int EvaluationTargetData::refTA(const DataID& id) const
//{
//    assert (hasRefTA(id));

//    auto index = indexFromDataID(id, DataType::Reference);

//    unsigned int index_ext = index.idx_external;

//    assert (!eval_data_.ref_buffer_->get<unsigned int>(eval_data_.ref_target_address_name_).isNull(index_ext));

//    return eval_data_.ref_buffer_->get<unsigned int>(eval_data_.ref_target_address_name_).get(index_ext);
//}

//std::pair<bool,bool> EvaluationTargetData::refGroundBit(const DataID& id) const // has gbs, gbs true
//{
//    auto index = indexFromDataID(id, DataType::Reference);

//    unsigned int index_ext = index.idx_external;

//    if (eval_data_.ref_ground_bit_name_.size()
//            && !eval_data_.ref_buffer_->get<bool>(eval_data_.ref_ground_bit_name_).isNull(index_ext))
//    {
//        return {true, eval_data_.ref_buffer_->get<bool>(eval_data_.ref_ground_bit_name_).get(index_ext)};
//    }
//    else
//        return {false, false};
//}

//bool EvaluationTargetData::hasTstPos(const DataID& id) const
//{
//    auto timestamp = timestampFromDataID(id);
//    return tst_chain_.count(timestamp);
//}

//dbContent::TargetPosition EvaluationTargetData::tstPos(const DataID& id) const
//{
//    assert (hasTstPos(id));

//    auto timestamp = timestampFromDataID(id);
//    auto index     = indexFromDataID(id, DataType::Test);

//    auto index_ext = index.idx_external;

//    dbContent::TargetPosition pos;

//    NullableVector<double>& latitude_vec = eval_data_.tst_buffer_->get<double>(eval_data_.tst_latitude_name_);
//    NullableVector<double>& longitude_vec = eval_data_.tst_buffer_->get<double>(eval_data_.tst_longitude_name_);
//    NullableVector<float>& altitude_vec = eval_data_.tst_buffer_->get<float>(eval_data_.tst_modec_name_);

//    NullableVector<float>* altitude_trusted_vec {nullptr};

//    if (eval_data_.tst_modec_trusted_name_.size())
//    {
//        assert (eval_data_.tst_buffer_->has<float>(eval_data_.tst_modec_trusted_name_));
//        altitude_trusted_vec = &eval_data_.tst_buffer_->get<float>(eval_data_.tst_modec_trusted_name_);
//    }

//    assert (!latitude_vec.isNull(index_ext));
//    assert (!longitude_vec.isNull(index_ext));

//    pos.latitude_ = latitude_vec.get(index_ext);
//    pos.longitude_ = longitude_vec.get(index_ext);

//    if (altitude_trusted_vec && !altitude_trusted_vec->isNull(index_ext))
//    {
//        pos.has_altitude_ = true;
//        pos.altitude_calculated_ = false;
//        pos.altitude_ = altitude_trusted_vec->get(index_ext);
//    }
//    else if (!altitude_vec.isNull(index_ext))
//    {
//        pos.has_altitude_ = true;
//        pos.altitude_ = altitude_vec.get(index_ext);
//        pos.altitude_calculated_ = false;
//    }
//    else // calculate
//    {
//        bool found;
//        float alt_calc;

//        tie(found,alt_calc) = estimateTstAltitude(timestamp, index.idx_internal);

//        if (found)
//        {
//            pos.has_altitude_ = true;
//            pos.altitude_calculated_ = true;
//            pos.altitude_ = alt_calc;
//        }
//    }

//    return pos;
//}

//std::pair<bool, float> EvaluationTargetData::estimateTstAltitude(const boost::posix_time::ptime& timestamp,
//                                                                 unsigned int index_internal) const
//{
//    assert(index_internal < tst_indices_.size());

//    NullableVector<float>& altitude_vec = eval_data_.tst_buffer_->get<float>(eval_data_.tst_modec_name_);
//    NullableVector<ptime>& ts_vec = eval_data_.tst_buffer_->get<ptime>(eval_data_.tst_timestamp_name_);

//    NullableVector<float>* altitude_trusted_vec {nullptr};

//    if (eval_data_.tst_modec_trusted_name_.size())
//    {
//        assert (eval_data_.tst_buffer_->has<float>(eval_data_.tst_modec_trusted_name_));
//        altitude_trusted_vec = &eval_data_.tst_buffer_->get<float>(eval_data_.tst_modec_trusted_name_);
//    }

//    bool found_prev {false};
//    float altitude_prev {0.0};
//    bool found_after {false};
//    float altitude_after {0.0};

//    // search for prev index
//    ptime timestamp_prev;
//    auto prev_it  = tst_indices_.begin() + index_internal;
//    auto after_it = prev_it;

//    const time_duration max_tdiff = seconds(120);

//    while (prev_it != tst_indices_.end() && timestamp - ts_vec.get(*prev_it) < max_tdiff)
//    {
//        if (altitude_trusted_vec && !altitude_trusted_vec->isNull(*prev_it))
//        {
//            found_prev = true;
//            altitude_prev = altitude_trusted_vec->get(*prev_it);
//            timestamp_prev = ts_vec.get(*prev_it);

//            break;
//        }
//        else if (!altitude_vec.isNull(*prev_it))
//        {
//            found_prev = true;
//            altitude_prev = altitude_vec.get(*prev_it);
//            timestamp_prev = ts_vec.get(*prev_it);

//            break;
//        }

//        if (prev_it == tst_indices_.begin()) // undefined decrement
//            break;

//        --prev_it;
//    }

//    // search after index
//    ptime timestamp_after;

//    while (after_it != tst_indices_.end() && ts_vec.get(*after_it) - timestamp < max_tdiff)
//    {
//        if (altitude_trusted_vec && !altitude_trusted_vec->isNull(*after_it))
//        {
//            found_after = true;
//            altitude_after = altitude_trusted_vec->get(*after_it);
//            timestamp_after = ts_vec.get(*after_it);

//            break;
//        }
//        else if (!altitude_vec.isNull(*after_it))
//        {
//            found_after = true;
//            altitude_after = altitude_vec.get(*after_it);
//            timestamp_after = ts_vec.get(*after_it);

//            break;
//        }
//        ++after_it;
//    }

//    if (found_prev && found_after)
//    {
//        if (timestamp_after <= timestamp_prev || timestamp_prev >= timestamp)
//        {
//            logerr << "EvaluationTargetData: estimateTstAltitude ts_prev " << Time::toString(timestamp_prev)
//                   << " ts " << Time::toString(timestamp)
//                   << " ts_after " << Time::toString(timestamp_after);
//            return {false, 0}; // should never happen
//        }

//        float d_alt_ft = altitude_after - altitude_prev;
//        float d_t = Time::partialSeconds(timestamp_after - timestamp_prev);

//        float alt_spd_ft_s = d_alt_ft/d_t;

//        float d_t2 = Time::partialSeconds(timestamp - timestamp_prev);

//        float alt_calc = altitude_prev + alt_spd_ft_s*d_t2;

//        return {true, alt_calc};
//    }
//    else if (found_prev && timestamp - timestamp_prev < max_tdiff)
//        return {true, altitude_prev};
//    else if (found_after && timestamp_after - timestamp < max_tdiff)
//        return {true, altitude_after};
//    else
//    {
//        return {false, 0}; // none found
//    }
//}

//bool EvaluationTargetData::hasTstCallsign(const DataID& id) const
//{
//    auto index = indexFromDataID(id, DataType::Test);

//    auto index_ext = index.idx_external;

//    NullableVector<string>& callsign_vec = eval_data_.tst_buffer_->get<string>(eval_data_.tst_callsign_name_);
//    return !callsign_vec.isNull(index_ext);
//}

//std::string EvaluationTargetData::tstCallsign(const DataID& id) const
//{
//    assert (hasTstCallsign(id));

//    auto index = indexFromDataID(id, DataType::Test);

//    auto index_ext = index.idx_external;

//    NullableVector<string>& callsign_vec = eval_data_.tst_buffer_->get<string>(eval_data_.tst_callsign_name_);
//    assert (!callsign_vec.isNull(index_ext));

//    return boost::trim_copy(callsign_vec.get(index_ext)); // remove spaces
//}

//bool EvaluationTargetData::hasTstModeA(const DataID& id) const
//{
//    auto index = indexFromDataID(id, DataType::Test);

//    auto index_ext = index.idx_external;

//    if (eval_data_.tst_buffer_->get<unsigned int>(eval_data_.tst_modea_name_).isNull(index_ext))
//        return false;

//    if (eval_data_.tst_modea_v_name_.size()
//            && !eval_data_.tst_buffer_->get<bool>(eval_data_.tst_modea_v_name_).isNull(index_ext)
//            && !eval_data_.tst_buffer_->get<bool>(eval_data_.tst_modea_v_name_).get(index_ext)) // not valid
//        return false;

//    if (eval_data_.tst_modea_g_name_.size()
//            && !eval_data_.tst_buffer_->get<bool>(eval_data_.tst_modea_g_name_).isNull(index_ext)
//            && eval_data_.tst_buffer_->get<bool>(eval_data_.tst_modea_g_name_).get(index_ext)) // garbled
//        return false;

//    return true;
//}

//unsigned int EvaluationTargetData::tstModeA(const DataID& id) const
//{
//    assert (hasTstModeA(id));

//    auto index = indexFromDataID(id, DataType::Test);

//    auto index_ext = index.idx_external;

//    NullableVector<unsigned int>& modea_vec = eval_data_.tst_buffer_->get<unsigned int>(eval_data_.tst_modea_name_);
//    assert (!modea_vec.isNull(index_ext));

//    return modea_vec.get(index_ext);
//}

//bool EvaluationTargetData::hasTstModeC(const DataID& id) const
//{
//    auto index = indexFromDataID(id, DataType::Test);

//    auto index_ext = index.idx_external;

//    if (eval_data_.tst_modec_trusted_name_.size() &&
//            !eval_data_.tst_buffer_->get<float>(eval_data_.tst_modec_trusted_name_).isNull(index_ext))
//        return true;

//    if (eval_data_.tst_buffer_->get<float>(eval_data_.tst_modec_name_).isNull(index_ext))
//        return false;

//    if (eval_data_.tst_modec_v_name_.size()
//            && !eval_data_.tst_buffer_->get<bool>(eval_data_.tst_modec_v_name_).isNull(index_ext)
//            && !eval_data_.tst_buffer_->get<bool>(eval_data_.tst_modec_v_name_).get(index_ext)) // not valid
//        return false;

//    if (eval_data_.tst_modec_g_name_.size()
//            && !eval_data_.tst_buffer_->get<bool>(eval_data_.tst_modec_g_name_).isNull(index_ext)
//            && eval_data_.tst_buffer_->get<bool>(eval_data_.tst_modec_g_name_).get(index_ext)) // garbled
//        return false;

//    return true;
//}

//bool EvaluationTargetData::hasTstGroundBit(const DataID& id) const // only if set
//{
//    auto index = indexFromDataID(id, DataType::Test);

//    auto index_ext = index.idx_external;

//    if (eval_data_.tst_ground_bit_name_.size()
//            && !eval_data_.tst_buffer_->get<bool>(eval_data_.tst_ground_bit_name_).isNull(index_ext))
//        return true;

//    return false;
//}

//bool EvaluationTargetData::tstGroundBit(const DataID& id) const // true is on ground
//{
//    assert (hasTstGroundBit(id));

//    auto index = indexFromDataID(id, DataType::Test);

//    auto index_ext = index.idx_external;

//    if (eval_data_.tst_ground_bit_name_.size()
//            && !eval_data_.tst_buffer_->get<bool>(eval_data_.tst_ground_bit_name_).isNull(index_ext)
//            && eval_data_.tst_buffer_->get<bool>(eval_data_.tst_ground_bit_name_).get(index_ext))
//        return true;

//    return false;
//}

//bool EvaluationTargetData::hasTstTA(const DataID& id) const
//{
//    auto index = indexFromDataID(id, DataType::Test);

//    auto index_ext = index.idx_external;

//    if (eval_data_.tst_target_address_name_.size()
//            && !eval_data_.tst_buffer_->get<unsigned int>(eval_data_.tst_target_address_name_).isNull(index_ext))
//        return true;

//    return false;
//}

//unsigned int EvaluationTargetData::tstTA(const DataID& id) const
//{
//    assert (hasTstTA(id));

//    auto index = indexFromDataID(id, DataType::Test);

//    auto index_ext = index.idx_external;

//    assert (!eval_data_.tst_buffer_->get<unsigned int>(eval_data_.tst_target_address_name_).isNull(index_ext));

//    return eval_data_.tst_buffer_->get<unsigned int>(eval_data_.tst_target_address_name_).get(index_ext);
//}

//// has gbs, gbs true
pair<bool,bool> EvaluationTargetData::tstGroundBitInterpolated(const DataID& ref_id) const // true is on ground
{
//    if (!eval_data_.tst_ground_bit_name_.size())
//        return pair<bool,bool>(false, false);

    auto ref_timestamp = ref_chain_.timestampFromDataID(ref_id);

    DataMappingTimes times = tst_chain_.findDataMappingTimes(ref_timestamp);

    if (times.has_other1_ && (ref_timestamp - times.timestamp_other1_).abs() < seconds(15)
            && tst_chain_.hasGroundBit(times.dataid_other1_)
            && get<1>(tst_chain_.groundBit(times.dataid_other1_)))
        return pair<bool,bool> (true, true);

    if (times.has_other2_ && (ref_timestamp - times.timestamp_other2_).abs() < seconds(15)
            && tst_chain_.hasGroundBit(times.timestamp_other2_))
        return pair<bool,bool> (true, get<1>(tst_chain_.groundBit(times.timestamp_other2_)));

    return pair<bool,bool>(false, false);
}

//bool EvaluationTargetData::hasTstTrackNum(const DataID& id) const
//{
//    if (!eval_data_.tst_track_num_name_.size())
//        return false;

//    auto index = indexFromDataID(id, DataType::Test);

//    auto index_ext = index.idx_external;

//    return !eval_data_.tst_buffer_->get<unsigned int>(eval_data_.tst_track_num_name_).isNull(index_ext);
//}

//unsigned int EvaluationTargetData::tstTrackNum(const DataID& id) const
//{
//    assert (hasTstTrackNum(id));

//    auto index = indexFromDataID(id, DataType::Test);

//    auto index_ext = index.idx_external;

//    return eval_data_.tst_buffer_->get<unsigned int>(eval_data_.tst_track_num_name_).get(index_ext);
//}

//bool EvaluationTargetData::hasTstMeasuredSpeed(const DataID& id) const
//{
//    assert (eval_data_.tst_spd_ground_speed_kts_name_.size());

//    auto index = indexFromDataID(id, DataType::Test);

//    auto index_ext = index.idx_external;

//    return !eval_data_.tst_buffer_->get<double>(eval_data_.tst_spd_ground_speed_kts_name_).isNull(index_ext);
//}

//float EvaluationTargetData::tstMeasuredSpeed(const DataID& id) const // m/s
//{
//    assert (hasTstMeasuredSpeed(id));

//    auto index = indexFromDataID(id, DataType::Test);

//    auto index_ext = index.idx_external;

//    assert(eval_data_.tst_spd_ground_speed_kts_name_.size());
//    return eval_data_.tst_buffer_->get<double>(eval_data_.tst_spd_ground_speed_kts_name_).get(index_ext) * KNOTS2M_S;
//}

//bool EvaluationTargetData::hasTstMeasuredTrackAngle(const DataID& id) const
//{
//    assert (eval_data_.tst_spd_track_angle_deg_name_.size());

//    auto index = indexFromDataID(id, DataType::Test);

//    auto index_ext = index.idx_external;

//    return !eval_data_.tst_buffer_->get<double>(eval_data_.tst_spd_track_angle_deg_name_).isNull(index_ext);
//}

//float EvaluationTargetData::tstMeasuredTrackAngle(const DataID& id) const // deg
//{
//    assert (hasTstMeasuredTrackAngle(id));

//    auto index = indexFromDataID(id, DataType::Test);

//    auto index_ext = index.idx_external;

//    assert (eval_data_.tst_spd_track_angle_deg_name_.size());
//    return eval_data_.tst_buffer_->get<double>(eval_data_.tst_spd_track_angle_deg_name_).get(index_ext);
//}

//float EvaluationTargetData::tstModeC(const DataID& id) const
//{
//    assert (hasTstModeC(id));

//    auto index = indexFromDataID(id, DataType::Test);

//    auto index_ext = index.idx_external;

//    if (eval_data_.tst_modec_trusted_name_.size() &&
//            !eval_data_.tst_buffer_->get<float>(eval_data_.tst_modec_trusted_name_).isNull(index_ext))
//        return eval_data_.tst_buffer_->get<float>(eval_data_.tst_modec_trusted_name_).get(index_ext);

//    NullableVector<float>& modec_vec = eval_data_.tst_buffer_->get<float>(eval_data_.tst_modec_name_);
//    assert (!modec_vec.isNull(index_ext));

//    return modec_vec.get(index_ext);
//}

//bool EvaluationTargetData::canCheckTstMultipleSources() const
//{
//    if (!eval_data_.tst_multiple_srcs_name_.size())
//        return false;

//    if (!eval_data_.tst_buffer_->has<bool>(eval_data_.tst_multiple_srcs_name_))
//        return false;

//    NullableVector<bool>& tst_multiple_srcs_vec =
//            eval_data_.tst_buffer_->get<bool>(eval_data_.tst_multiple_srcs_name_);

//    for (auto tst_index : tst_indices_)
//    {
//        if (!tst_multiple_srcs_vec.isNull(tst_index))
//            return true;
//    }

//    return false;
//}

//bool EvaluationTargetData::hasTstMultipleSources() const
//{
//    assert (canCheckTstMultipleSources());

//    NullableVector<bool>& tst_multiple_srcs_vec =
//            eval_data_.tst_buffer_->get<bool>(eval_data_.tst_multiple_srcs_name_);

//    for (auto tst_index : tst_indices_) // one must be not null according to canCheckTstMultipleSources
//    {
//        if (!tst_multiple_srcs_vec.isNull(tst_index) && tst_multiple_srcs_vec.get(tst_index))
//            return true;
//    }

//    return false;
//}

//bool EvaluationTargetData::canCheckTrackLUDSID() const
//{
//    if (!eval_data_.tst_track_lu_ds_id_name_.size())
//        return false;

//    if (!eval_data_.tst_buffer_->has<unsigned int>(eval_data_.tst_track_lu_ds_id_name_))
//        return false;

//    NullableVector<unsigned int> tst_ls_ds_id_vec =
//            eval_data_.tst_buffer_->get<unsigned int>(eval_data_.tst_track_lu_ds_id_name_);

//    for (auto tst_index : tst_indices_)
//    {
//        if (!tst_ls_ds_id_vec.isNull(tst_index))
//            return true;
//    }

//    return false;
//}

//bool EvaluationTargetData::hasSingleLUDSID() const
//{
//    assert (canCheckTrackLUDSID());

//    // check if only single source updates
//    assert (canCheckTstMultipleSources());
//    assert (!hasTstMultipleSources());

//    bool lu_ds_id_found = false;
//    unsigned int lu_ds_id;

//    NullableVector<unsigned int> tst_ls_ds_id_vec =
//            eval_data_.tst_buffer_->get<unsigned int>(eval_data_.tst_track_lu_ds_id_name_);

//    for (auto tst_index : tst_indices_)
//    {
//        if (!tst_ls_ds_id_vec.isNull(tst_index))
//        {
//            if (!lu_ds_id_found)
//            {
//                lu_ds_id_found = true;
//                lu_ds_id = tst_ls_ds_id_vec.get(tst_index);

//                continue;
//            }

//            if (lu_ds_id_found && tst_ls_ds_id_vec.get(tst_index) != lu_ds_id)
//                return false;
//        }
//    }

//    return true;
//}

//unsigned int EvaluationTargetData::singleTrackLUDSID() const
//{
//    assert (hasSingleLUDSID());

//    NullableVector<unsigned int> tst_ls_ds_id_vec =
//            eval_data_.tst_buffer_->get<unsigned int>(eval_data_.tst_track_lu_ds_id_name_);

//    for (auto tst_index : tst_indices_)
//    {
//        if (!tst_ls_ds_id_vec.isNull(tst_index))
//            return tst_ls_ds_id_vec.get(tst_index);
//    }

//    assert (false); // can not be reached
//}

double EvaluationTargetData::latitudeMin() const
{
    assert (has_pos_);
    return latitude_min_;
}

double EvaluationTargetData::latitudeMax() const
{
    assert (has_pos_);
    return latitude_max_;
}

double EvaluationTargetData::longitudeMin() const
{
    assert (has_pos_);
    return longitude_min_;
}

double EvaluationTargetData::longitudeMax() const
{
    assert (has_pos_);
    return longitude_max_;
}

bool EvaluationTargetData::hasPos() const
{
    return has_pos_;
}

bool EvaluationTargetData::hasADSBInfo() const
{
    return has_adsb_info_;
}

bool EvaluationTargetData::hasMOPSVersion() const
{
    return has_mops_versions_;
}

std::set<unsigned int> EvaluationTargetData::mopsVersions() const
{
    assert (has_mops_versions_);
    return mops_versions_;
}

std::string EvaluationTargetData::mopsVersionStr() const
{
    if (hasMOPSVersion())
    {
        std::ostringstream out;

        unsigned int cnt=0;
        for (const auto it : mops_versions_)
        {
            if (cnt != 0)
                out << ", ";

            out << it;
            ++cnt;
        }

        return out.str().c_str();
    }
    else
        return "?";
}

//std::string EvaluationTargetData::nucpNicStr() const
//{
//    if (hasNucpNic())
//    {
//        if (min_nucp_nic_ == max_nucp_nic_)
//            return to_string(min_nucp_nic_);
//        else
//            return to_string(min_nucp_nic_)+"-"+to_string(max_nucp_nic_);
//    }
//    else
//        return "?";
//}

//std::string EvaluationTargetData::nacpStr() const
//{
//    if (hasNacp())
//    {
//        if (min_nacp_ == max_nacp_)
//            return to_string(min_nacp_);
//        else
//            return to_string(min_nacp_)+"-"+to_string(max_nacp_);
//    }
//    else
//        return "?";
//}

//bool EvaluationTargetData::hasNucpNic() const
//{
//    return has_nucp_nic_;
//}

//bool EvaluationTargetData::hasNacp() const
//{
//    return has_nacp;
//}

std::set<string> EvaluationTargetData::acids() const
{
    return acids_;
}

string EvaluationTargetData::acidsStr() const
{
    std::ostringstream out;

    unsigned int cnt=0;
    for (auto& cs_it : acids_)
    {
        if (cnt != 0)
            out << ", ";

        out << cs_it;
        ++cnt;
    }

    return out.str().c_str();
}

std::set<unsigned int> EvaluationTargetData::acads() const
{
    return acads_;
}

std::string EvaluationTargetData::acadsStr() const
{
    std::ostringstream out;

    unsigned int cnt=0;
    for (auto& ta_it : acads_)
    {
        if (cnt != 0)
            out << ", ";

        out << String::hexStringFromInt(ta_it, 6, '0');
        ++cnt;
    }

    return out.str().c_str();
}

void EvaluationTargetData::updateACIDs() const
{
    acids_.clear();

    if (ref_chain_.size())
    {
        auto acids = ref_chain_.acids();
        acids_.insert(acids.begin(), acids.end());

//        NullableVector<string>& value_vec = eval_data_.ref_buffer_->get<string>(eval_data_.ref_callsign_name_);
//        map<string, vector<unsigned int>> distinct_values = value_vec.distinctValuesWithIndexes(ref_indices_);

//        for (auto& val_it : distinct_values)
//        {
//            if (!callsigns_.count(val_it.first))
//                callsigns_.insert(val_it.first);
//        }
    }

    if (tst_chain_.size())
    {
        auto acids = tst_chain_.acids();
        acids_.insert(acids.begin(), acids.end());

//        NullableVector<string>& value_vec = eval_data_.tst_buffer_->get<string>(eval_data_.tst_callsign_name_);
//        map<string, vector<unsigned int>> distinct_values = value_vec.distinctValuesWithIndexes(tst_indices_);

//        for (auto& val_it : distinct_values)
//        {
//            if (!acids_.count(val_it.first))
//                acids_.insert(val_it.first);
//        }
    }
}

void EvaluationTargetData::updateACADs() const
{
    acads_.clear();

    if (ref_chain_.size())
    {
        auto acads = ref_chain_.acads();
        acads_.insert(acads.begin(), acads.end());

//        NullableVector<unsigned int>& value_vec = eval_data_.ref_buffer_->get<unsigned int>(
//                    eval_data_.ref_target_address_name_);
//        map<unsigned int, vector<unsigned int>> distinct_values = value_vec.distinctValuesWithIndexes(ref_indices_);

//        for (auto& val_it : distinct_values)
//        {
//            if (!acads_.count(val_it.first))
//                acads_.insert(val_it.first);
//        }
    }

    if (tst_chain_.size())
    {
        auto acads = ref_chain_.acads();
        acads_.insert(acads.begin(), acads.end());

//        NullableVector<unsigned int>& value_vec = eval_data_.tst_buffer_->get<unsigned int>(
//                    eval_data_.tst_target_address_name_);
//        map<unsigned int, vector<unsigned int>> distinct_values = value_vec.distinctValuesWithIndexes(tst_indices_);

//        for (auto& val_it : distinct_values)
//        {
//            if (!acads_.count(val_it.first))
//                acads_.insert(val_it.first);
//        }
    }
}

void EvaluationTargetData::updateModeACodes() const
{
    logdbg << "EvaluationTargetData: updateModeACodes: utn " << utn_;

    mode_a_codes_.clear();

    if (ref_chain_.size())
    {
        auto values = ref_chain_.modeACodes();
        mode_a_codes_.insert(values.begin(), values.end());

//        NullableVector<unsigned int>& mode_a_codes = eval_data_.ref_buffer_->get<unsigned int>(
//                    eval_data_.ref_modea_name_);
//        map<unsigned int, vector<unsigned int>> distinct_codes = mode_a_codes.distinctValuesWithIndexes(ref_indices_);
//        //unsigned int null_cnt = mode_a_codes.nullValueIndexes(ref_rec_nums_).size();

//        for (auto& ma_it : distinct_codes)
//        {
//            if (!mode_a_codes_.count(ma_it.first))
//            {
//                logdbg << "EvaluationTargetData: updateModeACodes: utn " << utn_ << " new ref m3a "
//                       << String::octStringFromInt(ma_it.first, 4, '0');
//                mode_a_codes_.insert(ma_it.first);
//            }
//        }
    }

    if (tst_chain_.size())
    {
        auto values = tst_chain_.modeACodes();
        mode_a_codes_.insert(values.begin(), values.end());

//        NullableVector<unsigned int>& mode_a_codes = eval_data_.tst_buffer_->get<unsigned int>(
//                    eval_data_.tst_modea_name_);
//        map<unsigned int, vector<unsigned int>> distinct_codes = mode_a_codes.distinctValuesWithIndexes(tst_indices_);

//        for (auto& ma_it : distinct_codes)
//        {
//            if (!mode_a_codes_.count(ma_it.first))
//            {
//                logdbg << "EvaluationTargetData: updateModeACodes: utn " << utn_ << " new tst m3a "
//                       << String::octStringFromInt(ma_it.first, 4, '0');
//                mode_a_codes_.insert(ma_it.first);
//            }
//        }
    }

    logdbg << "EvaluationTargetData: updateModeACodes: utn " << utn_ << " num codes " << mode_a_codes_.size();
}

void EvaluationTargetData::updateModeCMinMax() const
{
    logdbg << "EvaluationTargetData: updateModeC: utn " << utn_;

    // garbled, valid flags?

    has_mode_c_ = false;
    //float mode_c_value;

    if (ref_chain_.size() && ref_chain_.hasModeC())
    {
        has_mode_c_ = true;
        mode_c_min_ = ref_chain_.modeCMin();
        mode_c_max_ = ref_chain_.modeCMax();

//        assert (eval_data_.ref_buffer_->has<float>(eval_data_.ref_modec_name_));
//        NullableVector<float>& modec_codes_ft = eval_data_.ref_buffer_->get<float>(eval_data_.ref_modec_name_);

//        NullableVector<float>* altitude_trusted_vec {nullptr};

//        if (eval_data_.ref_modec_trusted_name_.size())
//        {
//            assert (eval_data_.ref_buffer_->has<float>(eval_data_.ref_modec_trusted_name_));
//            altitude_trusted_vec = &eval_data_.ref_buffer_->get<float>(eval_data_.ref_modec_trusted_name_);
//        }

//        for (auto ind_it : ref_indices_)
//        {
//            if (altitude_trusted_vec && !altitude_trusted_vec->isNull(ind_it))
//                mode_c_value = altitude_trusted_vec->get(ind_it);
//            else if (!modec_codes_ft.isNull(ind_it))
//                mode_c_value = modec_codes_ft.get(ind_it);
//            else
//                continue;

//            // mode_c_value is set

//            if (!has_mode_c_)
//            {
//                has_mode_c_ = true;
//                mode_c_min_ = mode_c_value;
//                mode_c_max_ = mode_c_value;
//            }
//            else
//            {
//                mode_c_min_ = min(mode_c_min_, mode_c_value);
//                mode_c_max_ = max(mode_c_max_, mode_c_value);
//            }
//        }
    }

    if (tst_chain_.size() && tst_chain_.hasModeC())
    {
        if (!has_mode_c_)
        {
            has_mode_c_ = true;
            mode_c_min_ = tst_chain_.modeCMin();
            mode_c_max_ = tst_chain_.modeCMax();
        }
        else
        {
            mode_c_min_ = min(mode_c_min_, tst_chain_.modeCMin());
            mode_c_max_ = max(mode_c_max_, tst_chain_.modeCMax());
        }

//        assert (eval_data_.tst_buffer_->has<float>(eval_data_.tst_modec_name_));
//        NullableVector<float>& modec_codes_ft = eval_data_.tst_buffer_->get<float>(eval_data_.tst_modec_name_);

//        NullableVector<float>* altitude_trusted_vec {nullptr};

//        if (eval_data_.tst_modec_trusted_name_.size())
//        {
//            assert (eval_data_.tst_buffer_->has<float>(eval_data_.tst_modec_trusted_name_));
//            altitude_trusted_vec = &eval_data_.tst_buffer_->get<float>(eval_data_.tst_modec_trusted_name_);
//        }

//        for (auto ind_it : tst_indices_)
//        {
//            if (altitude_trusted_vec && !altitude_trusted_vec->isNull(ind_it))
//                mode_c_value = altitude_trusted_vec->get(ind_it);
//            else if (!modec_codes_ft.isNull(ind_it))
//                mode_c_value = modec_codes_ft.get(ind_it);
//            else
//                continue;

//            // mode_c_value is set

//            if (!has_mode_c_)
//            {
//                has_mode_c_ = true;
//                mode_c_min_ = mode_c_value;
//                mode_c_max_ = mode_c_value;
//            }
//            else
//            {
//                mode_c_min_ = min(mode_c_min_, mode_c_value);
//                mode_c_max_ = max(mode_c_max_, mode_c_value);
//            }
//        }
    }
}

void EvaluationTargetData::updatePositionMinMax() const
{
    has_pos_ = false;

    if (ref_chain_.size() && ref_chain_.hasPos())
    {
        has_pos_ = true;
        latitude_min_ = ref_chain_.latitudeMin();
        latitude_max_ = ref_chain_.latitudeMax();
        longitude_min_ = ref_chain_.longitudeMin();
        longitude_max_ = ref_chain_.longitudeMax();

//        assert (eval_data_.ref_buffer_->has<double>(eval_data_.ref_latitude_name_));
//        assert (eval_data_.ref_buffer_->has<double>(eval_data_.ref_longitude_name_));

//        NullableVector<double>& lats = eval_data_.ref_buffer_->get<double>(eval_data_.ref_latitude_name_);
//        NullableVector<double>& longs = eval_data_.ref_buffer_->get<double>(eval_data_.ref_longitude_name_);

//        for (auto ind_it : ref_indices_)
//        {
//            assert (!lats.isNull(ind_it));
//            assert (!longs.isNull(ind_it));

//            if (!has_pos_)
//            {
//                has_pos_ = true;
//                latitude_min_ = lats.get(ind_it);
//                latitude_max_ = lats.get(ind_it);
//                longitude_min_ = longs.get(ind_it);
//                longitude_max_ = longs.get(ind_it);
//            }
//            else
//            {
//                latitude_min_ = min(latitude_min_, lats.get(ind_it));
//                latitude_max_ = max(latitude_max_, lats.get(ind_it));
//                longitude_min_ = min(longitude_min_, longs.get(ind_it));
//                longitude_max_ = max(longitude_max_, longs.get(ind_it));
//            }
//        }
    }

    if (tst_chain_.size() && tst_chain_.hasPos())
    {
        if (!has_pos_)
        {
            has_pos_ = true;
            latitude_min_ = tst_chain_.latitudeMin();
            latitude_max_ = tst_chain_.latitudeMax();
            longitude_min_ = tst_chain_.longitudeMin();
            longitude_max_ = tst_chain_.longitudeMax();
        }
        else
        {
            latitude_min_ = min(latitude_min_, tst_chain_.latitudeMin());
            latitude_max_ = max(latitude_max_, tst_chain_.latitudeMax());
            longitude_min_ = min(longitude_min_, tst_chain_.longitudeMin());
            longitude_max_ = max(longitude_max_, tst_chain_.longitudeMax());
        }

//        assert (eval_data_.tst_buffer_->has<double>(eval_data_.tst_latitude_name_));
//        assert (eval_data_.tst_buffer_->has<double>(eval_data_.tst_longitude_name_));

//        NullableVector<double>& lats = eval_data_.tst_buffer_->get<double>(eval_data_.tst_latitude_name_);
//        NullableVector<double>& longs = eval_data_.tst_buffer_->get<double>(eval_data_.tst_longitude_name_);

//        for (auto ind_it : tst_indices_)
//        {
//            assert (!lats.isNull(ind_it));
//            assert (!longs.isNull(ind_it));

//            if (!has_pos_)
//            {
//                has_pos_ = true;
//                latitude_min_ = lats.get(ind_it);
//                latitude_max_ = lats.get(ind_it);
//                longitude_min_ = longs.get(ind_it);
//                longitude_max_ = longs.get(ind_it);
//            }
//            else
//            {
//                latitude_min_ = min(latitude_min_, lats.get(ind_it));
//                latitude_max_ = max(latitude_max_, lats.get(ind_it));
//                longitude_min_ = min(longitude_min_, longs.get(ind_it));
//                longitude_max_ = max(longitude_max_, longs.get(ind_it));
//            }
//        }
    }
}

//void EvaluationTargetData::updateADSBInfo() const
//{
//    has_adsb_info_ = false;

//    string mops_name {"mops_version"};
//    string nacp_name {"nac_p"};
//    string nucp_nic_name {"nucp_nic"};
//    string sil_name {"sil"};

//    if (ref_chain_.size() && ref_buffer_->dbContentName() == "ADSB")
//    {
//        assert (ref_buffer_->has<int>(mops_name));
//        assert (ref_buffer_->has<char>(nacp_name));
//        assert (ref_buffer_->has<char>(nucp_nic_name));
//        assert (ref_buffer_->has<char>(sil_name));

//        NullableVector<int>& mops = ref_buffer_->get<int>(mops_name);
//        NullableVector<char>& nacps = ref_buffer_->get<char>(nacp_name);
//        NullableVector<char>& nucp_nics = ref_buffer_->get<char>(nucp_nic_name);
//        NullableVector<char>& sils = ref_buffer_->get<char>(sil_name);

//        for (auto ind_it : ref_indexes_)
//        {
//            if (!mops.isNull(ind_it) && mops_versions_.count(mops.get(ind_it)))
//            {
//                mops_versions_.insert(mops.get(ind_it));
//                has_adsb_info_ = true;
//            }

//            if (!nacps.isNull(ind_it) && nacps_.count(nacps.get(ind_it)))
//            {
//                mops_versions_.insert(nacps.get(ind_it));
//                has_adsb_info_ = true;
//            }

//            if (!nucp_nics.isNull(ind_it) && nucp_nics_.count(nucp_nics.get(ind_it)))
//            {
//                nucp_nics_.insert(nucp_nics.get(ind_it));
//                has_adsb_info_ = true;
//            }

//            if (!sils.isNull(ind_it) && sils_.count(sils.get(ind_it)))
//            {
//                sils_.insert(sils.get(ind_it));
//                has_adsb_info_ = true;
//            }
//        }
//    }

//    if (tst_chain_.size() && tst_buffer_->dbContentName() == "ADSB")
//    {
//        assert (tst_buffer_->has<int>(mops_name));
//        assert (tst_buffer_->has<char>(nacp_name));
//        assert (tst_buffer_->has<char>(nucp_nic_name));
//        assert (tst_buffer_->has<char>(sil_name));

//        NullableVector<int>& mops = tst_buffer_->get<int>(mops_name);
//        NullableVector<char>& nacps = tst_buffer_->get<char>(nacp_name);
//        NullableVector<char>& nucp_nics = tst_buffer_->get<char>(nucp_nic_name);
//        NullableVector<char>& sils = tst_buffer_->get<char>(sil_name);

//        for (auto ind_it : tst_indexes_)
//        {
//            if (!mops.isNull(ind_it) && mops_versions_.count(mops.get(ind_it)))
//            {
//                mops_versions_.insert(mops.get(ind_it));
//                has_adsb_info_ = true;
//            }

//            if (!nacps.isNull(ind_it) && nacps_.count(nacps.get(ind_it)))
//            {
//                mops_versions_.insert(nacps.get(ind_it));
//                has_adsb_info_ = true;
//            }

//            if (!nucp_nics.isNull(ind_it) && nucp_nics_.count(nucp_nics.get(ind_it)))
//            {
//                nucp_nics_.insert(nucp_nics.get(ind_it));
//                has_adsb_info_ = true;
//            }

//            if (!sils.isNull(ind_it) && sils_.count(sils.get(ind_it)))
//            {
//                sils_.insert(sils.get(ind_it));
//                has_adsb_info_ = true;
//            }
//        }
//    }
//}

void EvaluationTargetData::calculateTestDataMappings() const
{
    loginf << "EvaluationTargetData: calculateTestDataMappings: utn " << utn_;

    assert (!tst_data_mappings_.size());

    tst_data_mappings_.resize(tst_chain_.size());

    for (auto& tst_it : tst_chain_.timestampIndexes())
    {
        tst_data_mappings_[tst_it.second.idx_internal] = ref_chain_.calculateDataMapping(tst_it.first);
    }

    unsigned int cnt = 0;

    for (const auto& tst_map_it : tst_data_mappings_)
    {
        if (tst_map_it.has_ref_pos_)
            ++cnt;
    }

    loginf << "EvaluationTargetData: calculateTestDataMappings: utn " << utn_ << " done, num map "
           << tst_data_mappings_.size() << " ref pos " << cnt;
}

//DataMapping EvaluationTargetData::calculateTestDataMapping(ptime timestamp) const
//{
//    DataMapping ret;

//    ret.timestamp_ = timestamp;

//    //    Return iterator to lower bound
//    //    Returns an iterator pointing to the first element in the container whose key is not considered to go
//    //    before k (i.e., either it is equivalent or goes after).

//    auto lb_it = ref_chain_.lower_bound(timestamp);

//    //auto ub_it = ref_chain_.upper_bound(tod);

//    if (lb_it != ref_chain_.end()) // upper tod found
//    {
//        assert (lb_it->first >= timestamp);

//        // save upper value
//        ret.has_ref2_ = true;
//        ret.timestamp_ref2_ = lb_it->first;

//        // search lower values by decrementing iterator
//        while (lb_it != ref_chain_.end() && (timestamp < lb_it->first || lb_it->first == ret.timestamp_ref2_))
//        {
//            if (lb_it == ref_chain_.begin()) // exit condition on first value
//            {
//                if (timestamp < lb_it->first) // set as not found
//                    lb_it = ref_chain_.end();

//                break;
//            }

//            lb_it--;
//        }

//        if (lb_it != ref_chain_.end() && lb_it->first != ret.timestamp_ref2_) // lower tod found
//        {
//            assert (timestamp >= lb_it->first);

//            // add lower value
//            ret.has_ref1_ = true;
//            ret.timestamp_ref1_ = lb_it->first;
//        }
//        else // not found, clear previous
//        {
//            ret.has_ref2_ = false;
//            ret.timestamp_ref2_ = {};
//        }
//    }

//    addRefPositionsSpeedsToMapping(ret);

//    return ret;
//}

//void EvaluationTargetData::addRefPositionsSpeedsToMapping (DataMapping& mapping) const
//{
//    if (mapping.has_ref1_ && hasRefPos(mapping.timestamp_ref1_)
//            && mapping.has_ref2_ && hasRefPos(mapping.timestamp_ref2_)) // two positions which can be interpolated
//    {
//        ptime lower = mapping.timestamp_ref1_;
//        ptime upper = mapping.timestamp_ref2_;

//        dbContent::TargetPosition pos1 = refPos(lower);
//        dbContent::TargetPosition pos2 = refPos(upper);
//        float d_t = Time::partialSeconds(upper - lower);

//        dbContent::TargetVelocity spd1;
//        dbContent::TargetVelocity spd2;

//        double acceleration;
//        double speed, angle;

//        logdbg << "EvaluationTargetData: addRefPositiosToMapping: d_t " << d_t;

//        assert (d_t > 0);

//        if (pos1.latitude_ == pos2.latitude_ && pos1.longitude_ == pos2.longitude_) // same pos
//        {
//            mapping.has_ref_pos_ = true;
//            mapping.pos_ref_ = pos1;

//            mapping.spd_ref_.track_angle_ = NAN;
//            mapping.spd_ref_.speed_       = NAN;
//        }
//        else
//        {
//            if (lower == upper) // same time
//            {
//                logwrn << "EvaluationTargetData: addRefPositiosToMapping: ref has same time twice";
//            }
//            else
//            {
//                logdbg << "EvaluationTargetData: addRefPositiosToMapping: pos1 "
//                       << pos1.latitude_ << ", " << pos1.longitude_;
//                logdbg << "EvaluationTargetData: addRefPositiosToMapping: pos2 "
//                       << pos2.latitude_ << ", " << pos2.longitude_;

//                bool ok;
//                double x_pos, y_pos;

//                tie(ok, x_pos, y_pos) = trafo_.distanceCart(
//                            pos1.latitude_, pos1.longitude_, pos2.latitude_, pos2.longitude_);

//                //                logdbg << "EvaluationTargetData: addRefPositiosToMapping: geo2cart";
//                //                bool ret = ogr_geo2cart->Transform(1, &x_pos, &y_pos); // wgs84 to cartesian offsets
//                if (!ok)
//                {
//                    logerr << "EvaluationTargetData: addRefPositiosToMapping: error with latitude " << pos2.latitude_
//                           << " longitude " << pos2.longitude_;
//                }
//                else // calculate interpolated position
//                {
//                    logdbg << "EvaluationTargetData: addRefPositiosToMapping: offsets x " << fixed << x_pos
//                           << " y " << fixed << y_pos << " dist " << fixed << sqrt(pow(x_pos,2)+pow(y_pos,2));

//                    double x_pos_orig = x_pos;
//                    double y_pos_orig = y_pos;

//                    double v_x = x_pos/d_t;
//                    double v_y = y_pos/d_t;
//                    logdbg << "EvaluationTargetData: addRefPositiosToMapping: v_x " << v_x << " v_y " << v_y;

//                    float d_t2 = Time::partialSeconds(mapping.timestamp_ - lower);
//                    logdbg << "EvaluationTargetData: addRefPositiosToMapping: d_t2 " << d_t2;

//                    assert (d_t2 >= 0);

//                    x_pos = v_x * d_t2;
//                    y_pos = v_y * d_t2;

//                    logdbg << "EvaluationTargetData: addRefPositiosToMapping: interpolated offsets x "
//                           << x_pos << " y " << y_pos;

//                    tie (ok, x_pos, y_pos) = trafo_.wgsAddCartOffset(pos1.latitude_, pos1.longitude_, x_pos, y_pos);

//                    // x_pos long, y_pos lat

//                    logdbg << "EvaluationTargetData: addRefPositiosToMapping: interpolated lat "
//                           << x_pos << " long " << y_pos;

//                    // calculate altitude
//                    bool has_altitude = false;
//                    float altitude = 0.0;

//                    if (pos1.has_altitude_ && !pos2.has_altitude_)
//                    {
//                        has_altitude = true;
//                        altitude = pos1.altitude_;
//                    }
//                    else if (!pos1.has_altitude_ && pos2.has_altitude_)
//                    {
//                        has_altitude = true;
//                        altitude = pos2.altitude_;
//                    }
//                    else if (pos1.has_altitude_ && pos2.has_altitude_)
//                    {
//                        float v_alt = (pos2.altitude_ - pos1.altitude_)/d_t;
//                        has_altitude = true;
//                        altitude = pos1.altitude_ + v_alt*d_t2;
//                    }

//                    logdbg << "EvaluationTargetData: addRefPositiosToMapping: pos1 has alt "
//                           << pos1.has_altitude_ << " alt " << pos1.altitude_
//                           << " pos2 has alt " << pos2.has_altitude_ << " alt " << pos2.altitude_
//                           << " interpolated has alt " << has_altitude << " alt " << altitude;

//                    mapping.has_ref_pos_ = true;

//                    mapping.pos_ref_ = dbContent::TargetPosition(x_pos, y_pos, has_altitude, true, altitude);

//                    // calulcate interpolated speed / track angle

//                    mapping.has_ref_spd_ = false;

//                    if (hasRefSpeed(mapping.timestamp_ref1_) && hasRefSpeed(mapping.timestamp_ref2_))
//                    {
//                        spd1 = refSpeed(mapping.timestamp_ref1_);
//                        spd2 = refSpeed(mapping.timestamp_ref2_);

//                        acceleration = (spd2.speed_ - spd1.speed_)/d_t;
//                        speed = spd1.speed_ + acceleration * d_t2;

//                        //loginf << "UGA spd1 " << spd1.speed_ << " 2 " << spd2.speed_ << " ipld " << speed;

//#if 0
//                        double angle_diff = Number::calculateMinAngleDifference(spd2.track_angle_, spd1.track_angle_);
//                        double turnrate   = angle_diff / d_t;

//                        angle = spd1.track_angle_ + turnrate * d_t2;
//#else
//                        angle = Number::interpolateBearing(0, 0, x_pos_orig, y_pos_orig, spd1.track_angle_, spd2.track_angle_, d_t2 / d_t);
//#endif

//                        //                        loginf << "UGA ang1 " << spd1.track_angle_ << " 2 " << spd2.track_angle_
//                        //                               << " angle_diff " << angle_diff << " turnrate " << turnrate << " ipld " << angle;

//                        mapping.has_ref_spd_          = true;
//                        mapping.spd_ref_.speed_       = speed;
//                        mapping.spd_ref_.track_angle_ = angle;
//                    }
//                }
//            }
//        }
//    }
//    // else do nothing
//}

//void EvaluationTargetData::addRefPositiosToMappingFast (DataMapping& mapping) const
//{
//    if (mapping.has_ref1_ && hasRefPosForTime(mapping.timestamp_ref1_)
//            && mapping.has_ref2_ && hasRefPosForTime(mapping.timestamp_ref2_)) // two positions which can be interpolated
//    {
//        ptime lower = mapping.timestamp_ref1_;
//        ptime upper = mapping.timestamp_ref2_;

//        dbContent::TargetPosition pos1 = refPosForTime(lower);
//        dbContent::TargetPosition pos2 = refPosForTime(upper);
//        float d_t = Time::partialSeconds(upper - lower);

//        logdbg << "EvaluationTargetData: addRefPositiosToMappingFast: d_t " << d_t;

//        assert (d_t >= 0);

//        if (pos1.latitude_ == pos2.latitude_ && pos1.longitude_ == pos2.longitude_) // same pos
//        {
//            mapping.has_ref_pos_ = true;
//            mapping.pos_ref_ = pos1;
//        }
//        else
//        {

//            if (lower == upper) // same time
//            {
//                logwrn << "EvaluationTargetData: addRefPositiosToMappingFast: ref has same time twice";
//            }
//            else
//            {
//                double v_lat = (pos2.latitude_ - pos1.latitude_)/d_t;
//                double v_long = (pos2.longitude_ - pos1.longitude_)/d_t;

//                logdbg << "EvaluationTargetData: interpolatedPosForTimeFast: v_x " << v_lat << " v_y " << v_long;

//                float d_t2 = Time::partialSeconds(mapping.timestamp_  - lower);
//                logdbg << "EvaluationTargetData: interpolatedPosForTimeFast: d_t2 " << d_t2;

//                assert (d_t2 >= 0);

//                double int_lat = pos1.latitude_ + v_lat * d_t2;
//                double int_long = pos1.longitude_ + v_long * d_t2;

//                logdbg << "EvaluationTargetData: interpolatedPosForTimeFast: interpolated lat " << int_lat
//                       << " long " << int_long;

//                // calculate altitude
//                bool has_altitude = false;
//                float altitude = 0.0;

//                if (pos1.has_altitude_ && !pos2.has_altitude_)
//                {
//                    has_altitude = true;
//                    altitude = pos1.altitude_;
//                }
//                else if (!pos1.has_altitude_ && pos2.has_altitude_)
//                {
//                    has_altitude = true;
//                    altitude = pos2.altitude_;
//                }
//                else if (pos1.has_altitude_ && pos2.has_altitude_)
//                {
//                    float v_alt = (pos2.altitude_ - pos1.altitude_)/d_t;
//                    has_altitude = true;
//                    altitude = pos1.altitude_ + v_alt*d_t2;
//                }

//                logdbg << "EvaluationTargetData: interpolatedPosForTimeFast: pos1 has alt "
//                       << pos1.has_altitude_ << " alt " << pos1.altitude_
//                       << " pos2 has alt " << pos2.has_altitude_ << " alt " << pos2.altitude_
//                       << " interpolated has alt " << has_altitude << " alt " << altitude;

//                mapping.has_ref_pos_ = true;

//                mapping.pos_ref_ = dbContent::TargetPosition(int_lat, int_long, has_altitude, true, altitude);
//            }
//        }
//    }
//    // else do nothing
//}



/**
*/
void EvaluationTargetData::computeSectorInsideInfo() const
{
    inside_ref_           = {};
    inside_tst_           = {};
    inside_map_           = {};
    inside_sector_layers_ = {};

    auto sector_layers = eval_man_.sectorsLayers();

    //store sector layers
    {
        size_t cnt = 0;
        for (const auto& sl : sector_layers)
            inside_sector_layers_[ sl.get() ] = cnt++;
    }

    const SectorLayer* min_height_layer_ptr = nullptr;

    if (eval_man_.filterMinimumHeight())
        min_height_layer_ptr = eval_man_.minHeightFilterLayer().get();

    size_t num_sector_layers = sector_layers.size();
    size_t num_ref           = ref_chain_.size();
    size_t num_tst           = tst_chain_.size();
    size_t num_map           = tst_data_mappings_.size();

    size_t num_extra         = 2; //ground bit available / above min height filter

    size_t num_cols          = num_sector_layers + num_extra;

    assert(num_map == num_tst);

    auto gb_max_sec = boost::posix_time::seconds(InterpGroundBitMaxSeconds);

    if (num_ref)
    {
        //check ref data for inside
        inside_ref_.resize(num_ref, num_cols);
        inside_ref_.setZero();

        for (const auto& elem : ref_chain_.timestampIndexes())
        {
            DataID id(elem.first, elem.second);

            auto pos = ref_chain_.pos(id);
            auto gb  = availableRefGroundBit(id, gb_max_sec);

            computeSectorInsideInfo(inside_ref_, pos, elem.second.idx_internal, gb, min_height_layer_ptr);
        }
    }
    if (num_tst)
    {
        //check test data for inside
        inside_tst_.resize(num_tst, num_cols);
        inside_tst_.setZero();

        for (const auto& elem : tst_chain_.timestampIndexes())
        {
            DataID id(elem.first, elem.second);

            auto pos = tst_chain_.pos(id);
            auto gb  = availableTstGroundBit(id, gb_max_sec);

            computeSectorInsideInfo(inside_tst_, pos, elem.second.idx_internal, gb, min_height_layer_ptr);
        }
    }
    if (num_map)
    {
        //check mapped positions for inside
        inside_map_.resize(num_tst, num_cols);
        inside_map_.setZero();

        for (const auto& elem : tst_chain_.timestampIndexes())
        {
            DataID id(elem.first, elem.second);

            auto pos = mappedRefPos(id);
            if (pos.has_value())
            {
                auto gb = availableTstGroundBit(id, gb_max_sec);
                computeSectorInsideInfo(inside_map_, pos.value(), elem.second.idx_internal, gb, min_height_layer_ptr);
            }
        }
    }
}

boost::optional<bool> EvaluationTargetData::availableRefGroundBit(
        const Chain::DataID& id, const boost::posix_time::time_duration& d_max) const
{
    auto index = ref_chain_.indexFromDataID(id);
    auto ts    = ref_chain_.timestampFromDataID(id);

    DataID indexed_id = DataID(ts, index);

    auto ref_gb = ref_chain_.groundBit(indexed_id);

    bool has_ground_bit = ref_gb.first;
    bool ground_bit_set = ref_gb.second;

    if (!ground_bit_set)
        tie(has_ground_bit, ground_bit_set) = tstGroundBitInterpolated(ts);

    if (!has_ground_bit)
        return {};

    return ground_bit_set;
}

boost::optional<bool> EvaluationTargetData::availableTstGroundBit(
        const Chain::DataID& id, const boost::posix_time::time_duration& d_max) const
{
    auto index = tst_chain_.indexFromDataID(id);
    auto ts    = tst_chain_.timestampFromDataID(id);

    DataID indexed_id = DataID(ts, index);

    bool has_ground_bit = tst_chain_.hasGroundBit(indexed_id);
    bool ground_bit_set = false;

    if (has_ground_bit)
        ground_bit_set = get<1>(tst_chain_.groundBit(indexed_id));
    else
        ground_bit_set = false;

    if (!ground_bit_set)
        tie(has_ground_bit, ground_bit_set) = mappedRefGroundBit(indexed_id, d_max);

    if (!has_ground_bit)
        return {};

    return ground_bit_set;
}

/**
*/
//boost::optional<bool> EvaluationTargetData::availableGroundBit(const DataID& id,
//                                                               DataType dtype,
//                                                               const boost::posix_time::time_duration& d_max) const
//{
//    auto index = indexFromDataID(id, dtype);
//    auto ts    = timestampFromDataID(id);

//    DataID indexed_id = DataID(ts, index);

//    bool has_ground_bit = false;
//    bool ground_bit_set = false;

//    if (dtype == DataType::Test)
//    {
//        bool has_ground_bit = hasTstGroundBit(indexed_id);
//        bool ground_bit_set = false;

//        if (has_ground_bit)
//            ground_bit_set = tstGroundBit(indexed_id);
//        else
//            ground_bit_set = false;

//        if (!ground_bit_set)
//            tie(has_ground_bit, ground_bit_set) = mappedRefGroundBit(indexed_id, d_max);
//    }
//    else if (dtype == DataType::Reference)
//    {
//        auto ref_gb = refGroundBit(indexed_id);

//        bool has_ground_bit = ref_gb.first;
//        bool ground_bit_set = ref_gb.second;

//        if (!ground_bit_set)
//            tie(has_ground_bit, ground_bit_set) = tstGroundBitInterpolated(ts);
//    }

//    if (!has_ground_bit)
//        return {};

//    return ground_bit_set;
//}

/**
*/
void EvaluationTargetData::computeSectorInsideInfo(InsideCheckMatrix& mat, 
                                                   const dbContent::TargetPosition& pos,
                                                   unsigned int idx_internal,
                                                   const boost::optional<bool>& ground_bit,
                                                   const SectorLayer* min_height_filter) const
{
    assert(idx_internal < mat.rows());

    size_t num_sector_layers = inside_sector_layers_.size();
    size_t extra_offset      = num_sector_layers;

    bool has_gb = ground_bit.has_value();
    bool gb_set = ground_bit.has_value() ? ground_bit.value() : false;

    //check airspace if enabled
    bool above_ok = true;
    if (min_height_filter)
    {
        auto res = AirSpace::isAbove(min_height_filter,
                                     pos,
                                     has_gb,
                                     gb_set);

        if (res == AirSpace::AboveCheckResult::Below)
            above_ok = false;
    }

    for (const auto& sl : inside_sector_layers_)
    {
        auto layer = sl.first;
        assert(layer);

        auto lidx = sl.second;

        bool inside = above_ok && layer->isInside(pos, has_gb, gb_set);

        //check pos against layer and write to mat
        mat(idx_internal, lidx) = inside;
    }

    mat(idx_internal, extra_offset     ) = has_gb;
    mat(idx_internal, extra_offset + 1 ) = above_ok;
}

/**
*/
bool EvaluationTargetData::refPosAbove(const DataID& id) const
{
    auto index = ref_chain_.indexFromDataID(id);

    return checkAbove(inside_ref_, index);
}

/**
*/
bool EvaluationTargetData::refPosGroundBitAvailable(const DataID& id) const
{
    auto index = ref_chain_.indexFromDataID(id);

    return checkGroundBit(inside_ref_, index);
}

/**
*/
bool EvaluationTargetData::refPosInside(const SectorLayer& layer, 
                                        const DataID& id) const
{
    auto index = ref_chain_.indexFromDataID(id);

    return checkInside(layer, inside_ref_, index);
}

/**
*/
bool EvaluationTargetData::tstPosAbove(const DataID& id) const
{
    auto index = tst_chain_.indexFromDataID(id);

    return checkAbove(inside_tst_, index);
}

/**
*/
bool EvaluationTargetData::tstPosGroundBitAvailable(const DataID& id) const
{
    auto index = tst_chain_.indexFromDataID(id);

    return checkGroundBit(inside_tst_, index);
}

/**
*/
bool EvaluationTargetData::tstPosInside(const SectorLayer& layer, 
                                        const DataID& id) const
{
    auto index = tst_chain_.indexFromDataID(id);

    return checkInside(layer, inside_tst_, index);
}

/**
*/
bool EvaluationTargetData::mappedRefPosAbove(const DataID& id) const
{
    auto index = tst_chain_.indexFromDataID(id); // really tst

    return checkAbove(inside_map_, index);
}

/**
*/
bool EvaluationTargetData::mappedRefPosGroundBitAvailable(const DataID& id) const
{
    auto index = tst_chain_.indexFromDataID(id);

    return checkGroundBit(inside_map_, index);
}

/**
*/
bool EvaluationTargetData::mappedRefPosInside(const SectorLayer& layer, 
                                              const DataID& id) const
{
    auto index = tst_chain_.indexFromDataID(id);

    return checkInside(layer, inside_map_, index);
}

/**
*/
bool EvaluationTargetData::checkGroundBit(const InsideCheckMatrix& mat,
                                          const Index& index) const
{
    //check cached inside info
    auto idx_internal = index.idx_internal;
    assert(idx_internal < mat.rows());

    auto extra_offset = (int)inside_sector_layers_.size();
    assert(extra_offset < mat.cols());

    return mat(idx_internal, extra_offset);
}

/**
*/
bool EvaluationTargetData::checkAbove(const InsideCheckMatrix& mat,
                                      const Index& index) const
{
    //check cached inside info
    auto idx_internal = index.idx_internal;
    assert(idx_internal < mat.rows());

    auto extra_offset = (int)inside_sector_layers_.size();
    assert(extra_offset + 1 < mat.cols());

    return mat(idx_internal, extra_offset + 1);
}

/**
*/
bool EvaluationTargetData::checkInside(const SectorLayer& layer,
                                       const InsideCheckMatrix& mat,
                                       const Index& index) const
{
    auto lit = inside_sector_layers_.find(&layer);
    assert(lit != inside_sector_layers_.end());
    
    //check cached inside info
    auto idx_internal = index.idx_internal;
    assert(idx_internal < mat.rows());

    auto lidx = (int)lit->second;
    assert(lidx < mat.cols());

    return mat(idx_internal, lidx);
}
