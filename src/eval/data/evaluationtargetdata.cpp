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
#include "dbcontent/target.h"
#include "compass.h"
#include "dbcontent/dbcontentmanager.h"
#include "evaluationmanager.h"

//#include <ogr_spatialref.h>

#include <boost/algorithm/string.hpp>

#include <cassert>
#include <algorithm>
#include <cmath>

using namespace std;
using namespace Utils;
using namespace boost::posix_time;

//const unsigned int debug_utn = 3275;

EvaluationTargetData::EvaluationTargetData()
{

}

EvaluationTargetData::EvaluationTargetData(unsigned int utn, EvaluationData& eval_data, EvaluationManager& eval_man)
    : utn_(utn), eval_data_(&eval_data), eval_man_(&eval_man)
    //wgs84_{new OGRSpatialReference()}, local_{new OGRSpatialReference()}
{
    use_ = eval_man_->useUTN(utn_);

    //wgs84_->SetWellKnownGeogCS("WGS84");
}

EvaluationTargetData::~EvaluationTargetData()
{

}

void EvaluationTargetData::addRefIndex (boost::posix_time::ptime timestamp, unsigned int index)
{
    ref_data_.insert({timestamp, index});
}

void EvaluationTargetData::addTstIndex (boost::posix_time::ptime timestamp, unsigned int index)
{
    tst_data_.insert({timestamp, index});
}

bool EvaluationTargetData::hasData() const
{
    return ref_data_.size() || tst_data_.size();
}

bool EvaluationTargetData::hasRefData () const
{
    return ref_data_.size();
}

bool EvaluationTargetData::hasTstData () const
{
    return tst_data_.size();
}

void EvaluationTargetData::finalize () const
{
    //    loginf << "EvaluationTargetData: finalize: utn " << utn_
    //           << " ref " << hasRefData() << " up " << ref_rec_nums_.size()
    //           << " tst " << hasTstData() << " up " << tst_rec_nums_.size();

    for (auto& ref_it : ref_data_)
        ref_indexes_.push_back(ref_it.second);

    for (auto& tst_it : tst_data_)
        tst_indexes_.push_back(tst_it.second);

    updateCallsigns();
    updateTargetAddresses();
    updateModeACodes();
    updateModeCMinMax();
    updatePositionMinMax();

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    if (dbcont_man.hasTargetsInfo() && dbcont_man.existsTarget(utn_) && dbcont_man.target(utn_)->hasAdsbMOPSVersion())
    {
        has_adsb_info_ = true;
        has_mops_version_ = true;
        mops_version_ = dbcont_man.target(utn_)->adsbMOPSVersion();
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
}

unsigned int EvaluationTargetData::numUpdates () const
{
    return ref_data_.size() + tst_data_.size();
}

unsigned int EvaluationTargetData::numRefUpdates () const
{
    return ref_data_.size();
}
unsigned int EvaluationTargetData::numTstUpdates () const
{
    return tst_data_.size();
}

ptime EvaluationTargetData::timeBegin() const
{
    if (ref_data_.size() && tst_data_.size())
        return min(ref_data_.begin()->first, tst_data_.begin()->first);
    else if (ref_data_.size())
        return ref_data_.begin()->first;
    else if (tst_data_.size())
        return tst_data_.begin()->first;
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
    if (ref_data_.size() && tst_data_.size())
        return max(ref_data_.rbegin()->first, tst_data_.rbegin()->first);
    else if (ref_data_.size())
        return ref_data_.rbegin()->first;
    else if (tst_data_.size())
        return tst_data_.rbegin()->first;
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
    return !callsigns_.size() && !target_addresses_.size() && !mode_a_codes_.size() && !has_mode_c_;
}

bool EvaluationTargetData::use() const
{
    return use_;
}

void EvaluationTargetData::use(bool use)
{
    loginf << "EvaluationTargetData: use: utn " << utn_ << " use " << use;

    use_ = use;
}

const std::multimap<ptime, unsigned int>& EvaluationTargetData::refData() const
{
    return ref_data_;
}


const std::multimap<ptime, unsigned int>& EvaluationTargetData::tstData() const
{
    return tst_data_;
}

bool EvaluationTargetData::hasRefDataForTime (ptime timestamp, time_duration d_max) const
{
    assert (test_data_mappings_.count(timestamp));
    TstDataMapping& mapping = test_data_mappings_.at(timestamp);

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

std::pair<ptime, ptime> EvaluationTargetData::refTimesFor (
        boost::posix_time::ptime timestamp, time_duration d_max)  const
{
    assert (test_data_mappings_.count(timestamp));
    TstDataMapping& mapping = test_data_mappings_.at(timestamp);

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

std::pair<EvaluationTargetPosition, bool>  EvaluationTargetData::interpolatedRefPosForTime (
        ptime timestamp, time_duration d_max) const
{
    assert (test_data_mappings_.count(timestamp));
    TstDataMapping& mapping = test_data_mappings_.at(timestamp);

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

std::pair<EvaluationTargetVelocity, bool>  EvaluationTargetData::interpolatedRefPosBasedSpdForTime (
        ptime timestamp, time_duration d_max) const
{
    assert (test_data_mappings_.count(timestamp));
    TstDataMapping& mapping = test_data_mappings_.at(timestamp);

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

        return {mapping.posbased_spd_ref_, true};
    }

    return {{}, false};
}

bool EvaluationTargetData::hasRefPosForTime (ptime timestamp) const
{
    return ref_data_.count(timestamp);
}

EvaluationTargetPosition EvaluationTargetData::refPosForTime (ptime timestamp) const
{
    assert (hasRefPosForTime(timestamp));

    auto it_pair = ref_data_.equal_range(timestamp);

    assert (it_pair.first != ref_data_.end());

    unsigned int index = it_pair.first->second;

    EvaluationTargetPosition pos;

    NullableVector<double>& latitude_vec = eval_data_->ref_buffer_->get<double>(eval_data_->ref_latitude_name_);
    NullableVector<double>& longitude_vec = eval_data_->ref_buffer_->get<double>(eval_data_->ref_longitude_name_);
    NullableVector<float>& altitude_vec = eval_data_->ref_buffer_->get<float>(eval_data_->ref_modec_name_);

    assert (!latitude_vec.isNull(index));
    assert (!longitude_vec.isNull(index));

    pos.latitude_ = latitude_vec.get(index);
    pos.longitude_ = longitude_vec.get(index);

    if (!altitude_vec.isNull(index))
    {
        pos.has_altitude_ = true;
        pos.altitude_calculated_ = false;
        pos.altitude_ = altitude_vec.get(index);
    }
    else if (eval_data_->has_ref_altitude_secondary_
             && !eval_data_->ref_buffer_->get<float>(eval_data_->ref_altitude_secondary_name_).isNull(index))
    {
        pos.has_altitude_ = true;
        pos.altitude_calculated_ = true;
        pos.altitude_ = eval_data_->ref_buffer_->get<float>(eval_data_->ref_altitude_secondary_name_).get(index);
    }
    else // calculate
    {
        bool found;
        float alt_calc;

        tie(found,alt_calc) = estimateRefAltitude(timestamp, index);

        if (found)
        {
            pos.has_altitude_ = true;
            pos.altitude_calculated_ = true;
            pos.altitude_ = alt_calc;
        }
    }

    //    if (utn_ == debug_utn)
    //        loginf << "EvaluationTargetData: refPosForTime: tod " << String::timeStringFromDouble(tod)
    //               << " has_alt " << pos.has_altitude_ << " alt_calc " << pos.altitude_calculated_
    //               << " alt " << pos.altitude_;

    return pos;
}

std::pair<bool, float> EvaluationTargetData::estimateRefAltitude (ptime timestamp, unsigned int index) const
{
    NullableVector<float>& altitude_vec = eval_data_->ref_buffer_->get<float>(eval_data_->ref_modec_name_);
    NullableVector<ptime>& ts_vec = eval_data_->ref_buffer_->get<ptime>(eval_data_->ref_timestamp_name_);

    bool found_prev {false};
    bool found_after {false};

    // search for prev index
    ptime timestamp_prev;
    auto prev_it = find(ref_indexes_.begin(), ref_indexes_.end(), index);
    assert (prev_it != ref_indexes_.end());

    auto after_it = prev_it;

    const time_duration max_tdiff = seconds(120);

    while (prev_it != ref_indexes_.end() && timestamp - ts_vec.get(*prev_it) < max_tdiff)
    {
        if (!altitude_vec.isNull(*prev_it))
        {
            found_prev = true;
            timestamp_prev = ts_vec.get(*prev_it);

            //            if (utn_ == debug_utn)
            //                loginf << "EvaluationTargetData: refPosForTime: found prev at tod "
            //                       << String::timeStringFromDouble(tod_prev);

            break;
        }

        if (prev_it == ref_indexes_.begin()) // undefined decrement
            break;

        --prev_it;
    }

    //    if (utn_ == debug_utn)
    //    {
    //        if (prev_it != ref_indexes_.end())
    //            loginf << "EvaluationTargetData: refPosForTime: checking prev found end";
    //        else if (prev_it != ref_indexes_.begin())
    //            loginf << "EvaluationTargetData: refPosForTime: checking prev found begin";
    //        else
    //            loginf << "EvaluationTargetData: refPosForTime: finished prev tod "
    //                   << String::timeStringFromDouble(tods.get(*prev_it)) << " found " << found_prev;
    //    }

    // search after index
    ptime timestamp_after;

    while (after_it != ref_indexes_.end() && ts_vec.get(*after_it) - timestamp < max_tdiff)
    {
        //        if (utn_ == debug_utn)
        //            loginf << "EvaluationTargetData: refPosForTime: checking after tod "
        //                   << String::timeStringFromDouble(tods.get(*after_it));

        if (!altitude_vec.isNull(*after_it))
        {
            found_after = true;
            timestamp_after = ts_vec.get(*after_it);

            //            if (utn_ == debug_utn)
            //                loginf << "EvaluationTargetData: refPosForTime: found after at tod "
            //                       << String::timeStringFromDouble(tod_after);

            break;
        }
        ++after_it;
    }

    //    if (utn_ == debug_utn)
    //    {
    //        if (after_it != ref_indexes_.end())
    //            loginf << "EvaluationTargetData: refPosForTime: checking after found end";
    //        else
    //            loginf << "EvaluationTargetData: refPosForTime: finished after tod "
    //                   << String::timeStringFromDouble(tods.get(*after_it)) << " found " << found_after;
    //    }

    if (found_prev && found_after)
    {
        float alt_prev = altitude_vec.get(*prev_it);
        float alt_after = altitude_vec.get(*after_it);

        if (timestamp_after <= timestamp_prev || timestamp_prev >= timestamp)
        {
            logerr << "EvaluationTargetData::estimateRefAltitude ts_prev " << Time::toString(timestamp_prev)
                   << " ts " << Time::toString(timestamp)
                   << " ts_after " << Time::toString(timestamp_after);

            return {false, 0}; // should never happen
        }

        float d_alt_ft = alt_after - alt_prev;
        float d_t = Time::partialSeconds(timestamp_after - timestamp_prev);

        float alt_spd_ft_s = d_alt_ft/d_t;

        float d_t2 = Time::partialSeconds(timestamp - timestamp_prev);

        float alt_calc = alt_prev + alt_spd_ft_s*d_t2;

        //loginf << "UGA " << alt_calc;

        //        if (utn_ == debug_utn)
        //            loginf << "EvaluationTargetData: refPosForTime: returning alt " << alt_calc;

        return {true, alt_calc};
    }
    else if (found_prev && timestamp - timestamp_prev < max_tdiff)
        return {true, altitude_vec.get(*prev_it)};
    else if (found_after && timestamp_after - timestamp < max_tdiff)
        return {true, altitude_vec.get(*after_it)};
    else
    {
        //        if (utn_ == debug_utn)
        //            loginf << "EvaluationTargetData: refPosForTime: tod " << String::timeStringFromDouble(tod)
        //                   << " none found";

        return {false, 0}; // none found
    }
}

bool EvaluationTargetData::hasRefCallsignForTime (ptime timestamp) const
{
    if (!ref_data_.count(timestamp))
        return false;

    auto it_pair = ref_data_.equal_range(timestamp);

    assert (it_pair.first != ref_data_.end());

    unsigned int index = it_pair.first->second;

    NullableVector<string>& callsign_vec = eval_data_->ref_buffer_->get<string>(eval_data_->ref_callsign_name_);

    return !callsign_vec.isNull(index);
}

std::string EvaluationTargetData::refCallsignForTime (ptime timestamp) const
{
    assert (hasRefCallsignForTime(timestamp));

    auto it_pair = ref_data_.equal_range(timestamp);

    assert (it_pair.first != ref_data_.end());

    unsigned int index = it_pair.first->second;

    NullableVector<string>& callsign_vec = eval_data_->ref_buffer_->get<string>(eval_data_->ref_callsign_name_);
    assert (!callsign_vec.isNull(index));

    return boost::trim_copy(callsign_vec.get(index)); // remove spaces
}

bool EvaluationTargetData::hasRefModeAForTime (ptime timestamp) const
{
    if (!ref_data_.count(timestamp))
        return false;

    auto it_pair = ref_data_.equal_range(timestamp);

    assert (it_pair.first != ref_data_.end());

    unsigned int index = it_pair.first->second;

    if (eval_data_->ref_buffer_->get<unsigned int>(eval_data_->ref_modea_name_).isNull(index))
        return false;

    if (eval_data_->ref_modea_v_name_.size()
            && !eval_data_->ref_buffer_->get<bool>(eval_data_->ref_modea_v_name_).isNull(index)
            && !eval_data_->ref_buffer_->get<bool>(eval_data_->ref_modea_v_name_).get(index)) // not valid
        return false;

    if (eval_data_->ref_modea_g_name_.size()
            && !eval_data_->ref_buffer_->get<bool>(eval_data_->ref_modea_g_name_).isNull(index)
            && eval_data_->ref_buffer_->get<bool>(eval_data_->ref_modea_g_name_).get(index)) // garbled
        return false;

    return true;
}

unsigned int EvaluationTargetData::refModeAForTime (ptime timestamp) const
{
    assert (hasRefModeAForTime(timestamp));

    auto it_pair = ref_data_.equal_range(timestamp);

    assert (it_pair.first != ref_data_.end());

    unsigned int index = it_pair.first->second;

    NullableVector<unsigned int>& modea_vec = eval_data_->ref_buffer_->get<unsigned int>(eval_data_->ref_modea_name_);
    assert (!modea_vec.isNull(index));

    return modea_vec.get(index);
}

bool EvaluationTargetData::hasRefModeCForTime (ptime timestamp) const
{
    if (!ref_data_.count(timestamp))
        return false;

    auto it_pair = ref_data_.equal_range(timestamp);

    assert (it_pair.first != ref_data_.end());

    unsigned int index = it_pair.first->second;

    if (eval_data_->ref_buffer_->get<float>(eval_data_->ref_modec_name_).isNull(index))
        return false;

    if (eval_data_->ref_modec_v_name_.size()
            && !eval_data_->ref_buffer_->get<bool>(eval_data_->ref_modec_v_name_).isNull(index)
            && !eval_data_->ref_buffer_->get<bool>(eval_data_->ref_modec_v_name_).get(index)) // not valid
        return false;

    if (eval_data_->ref_modec_g_name_.size()
            && !eval_data_->ref_buffer_->get<bool>(eval_data_->ref_modec_g_name_).isNull(index)
            && eval_data_->ref_buffer_->get<bool>(eval_data_->ref_modec_g_name_).get(index)) // garbled
        return false;

    return true;
}

float EvaluationTargetData::refModeCForTime (ptime timestamp) const
{
    assert (hasRefModeCForTime(timestamp));

    auto it_pair = ref_data_.equal_range(timestamp);

    assert (it_pair.first != ref_data_.end());

    unsigned int index = it_pair.first->second;

    NullableVector<float>& modec_vec = eval_data_->ref_buffer_->get<float>(eval_data_->ref_modec_name_);
    assert (!modec_vec.isNull(index));

    return modec_vec.get(index);
}

bool EvaluationTargetData::hasRefTAForTime (ptime timestamp) const
{
    if (!ref_data_.count(timestamp))
        return false;

    auto it_pair = ref_data_.equal_range(timestamp);

    assert (it_pair.first != ref_data_.end());

    unsigned int index = it_pair.first->second;

    if (eval_data_->ref_target_address_name_.size()
            && !eval_data_->ref_buffer_->get<unsigned int>(eval_data_->ref_target_address_name_).isNull(index))
        return true;

    return false;
}

unsigned int EvaluationTargetData::refTAForTime (ptime timestamp) const
{
    assert (hasRefTAForTime(timestamp));

    auto it_pair = ref_data_.equal_range(timestamp);

    assert (it_pair.first != ref_data_.end());

    unsigned int index = it_pair.first->second;

    assert (!eval_data_->ref_buffer_->get<unsigned int>(eval_data_->ref_target_address_name_).isNull(index));

    return eval_data_->ref_buffer_->get<unsigned int>(eval_data_->ref_target_address_name_).get(index);
}

std::pair<bool,bool> EvaluationTargetData::refGroundBitForTime (ptime timestamp) const // has gbs, gbs true
{
    if (!ref_data_.count(timestamp))
        return {false, false};

    auto it_pair = ref_data_.equal_range(timestamp);

    assert (it_pair.first != ref_data_.end());

    unsigned int index = it_pair.first->second;

    if (eval_data_->ref_ground_bit_name_.size()
            && !eval_data_->ref_buffer_->get<bool>(eval_data_->ref_ground_bit_name_).isNull(index))
    {
        return {true, eval_data_->ref_buffer_->get<bool>(eval_data_->ref_ground_bit_name_).get(index)};
    }
    else
        return {false, false};
}

std::pair<bool,bool> EvaluationTargetData::interpolatedRefGroundBitForTime (ptime timestamp, time_duration d_max) const
// has gbs, gbs true
{
    assert (test_data_mappings_.count(timestamp));

    bool has_gbs = false;
    bool gbs = false;

    TstDataMapping& mapping = test_data_mappings_.at(timestamp);

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

        tie (has_gbs, gbs) = refGroundBitForTime(mapping.timestamp_ref1_);

        if (!gbs)
            tie (has_gbs, gbs) = refGroundBitForTime(mapping.timestamp_ref2_);
    }

    return {has_gbs, gbs};
}

bool EvaluationTargetData::hasTstPosForTime (ptime timestamp) const
{
    return tst_data_.count(timestamp);
}

EvaluationTargetPosition EvaluationTargetData::tstPosForTime (ptime timestamp) const
{
    assert (hasTstPosForTime(timestamp));

    auto it_pair = tst_data_.equal_range(timestamp);

    assert (it_pair.first != tst_data_.end());

    unsigned int index = it_pair.first->second;

    EvaluationTargetPosition pos;

    NullableVector<double>& latitude_vec = eval_data_->tst_buffer_->get<double>(eval_data_->tst_latitude_name_);
    NullableVector<double>& longitude_vec = eval_data_->tst_buffer_->get<double>(eval_data_->tst_longitude_name_);
    NullableVector<float>& altitude_vec = eval_data_->tst_buffer_->get<float>(eval_data_->tst_modec_name_);

    assert (!latitude_vec.isNull(index));
    assert (!longitude_vec.isNull(index));

    pos.latitude_ = latitude_vec.get(index);
    pos.longitude_ = longitude_vec.get(index);

    if (!altitude_vec.isNull(index))
    {
        pos.has_altitude_ = true;
        pos.altitude_ = altitude_vec.get(index);
        pos.altitude_calculated_ = false;
    }
    else // calculate
    {
        bool found;
        float alt_calc;

        tie(found,alt_calc) = estimateTstAltitude(timestamp, index);

        if (found)
        {
            pos.has_altitude_ = true;
            pos.altitude_calculated_ = true;
            pos.altitude_ = alt_calc;
        }
    }

    return pos;
}

std::pair<bool, float> EvaluationTargetData::estimateTstAltitude (ptime timestamp, unsigned int index) const
{
    NullableVector<float>& altitude_vec = eval_data_->tst_buffer_->get<float>(eval_data_->tst_modec_name_);
    NullableVector<ptime>& ts_vec = eval_data_->tst_buffer_->get<ptime>(eval_data_->tst_timestamp_name_);

    bool found_prev {false};
    bool found_after {false};

    // search for prev index
    ptime timestamp_prev;
    auto prev_it = find(tst_indexes_.begin(), tst_indexes_.end(), index);
    assert (prev_it != tst_indexes_.end());

    auto after_it = prev_it;

    const time_duration max_tdiff = seconds(120);

    while (prev_it != tst_indexes_.end() && timestamp - ts_vec.get(*prev_it) < max_tdiff)
    {
        if (!altitude_vec.isNull(*prev_it))
        {
            found_prev = true;
            timestamp_prev = ts_vec.get(*prev_it);

            break;
        }

        if (prev_it == tst_indexes_.begin()) // undefined decrement
            break;

        --prev_it;
    }

    // search after index
    ptime timestamp_after;

    while (after_it != tst_indexes_.end() && ts_vec.get(*after_it) - timestamp < max_tdiff)
    {
        if (!altitude_vec.isNull(*after_it))
        {
            found_after = true;
            timestamp_after = ts_vec.get(*after_it);

            break;
        }
        ++after_it;
    }

    if (found_prev && found_after)
    {
        float alt_prev = altitude_vec.get(*prev_it);
        float alt_after = altitude_vec.get(*after_it);

        if (timestamp_after <= timestamp_prev || timestamp_prev >= timestamp)
        {
            logerr << "EvaluationTargetData: estimateTstAltitude ts_prev " << Time::toString(timestamp_prev)
                   << " ts " << Time::toString(timestamp)
                   << " ts_after " << Time::toString(timestamp_after);
            return {false, 0}; // should never happen
        }

        float d_alt_ft = alt_after - alt_prev;
        float d_t = Time::partialSeconds(timestamp_after - timestamp_prev);

        float alt_spd_ft_s = d_alt_ft/d_t;

        float d_t2 = Time::partialSeconds(timestamp - timestamp_prev);

        float alt_calc = alt_prev + alt_spd_ft_s*d_t2;

        return {true, alt_calc};
    }
    else if (found_prev && timestamp - timestamp_prev < max_tdiff)
        return {true, altitude_vec.get(*prev_it)};
    else if (found_after && timestamp_after - timestamp < max_tdiff)
        return {true, altitude_vec.get(*after_it)};
    else
    {
        return {false, 0}; // none found
    }
}

bool EvaluationTargetData::hasTstCallsignForTime (ptime timestamp) const
{
    if (!tst_data_.count(timestamp))
        return false;

    auto it_pair = tst_data_.equal_range(timestamp);

    assert (it_pair.first != tst_data_.end());

    unsigned int index = it_pair.first->second;

    NullableVector<string>& callsign_vec = eval_data_->tst_buffer_->get<string>(eval_data_->tst_callsign_name_);
    return !callsign_vec.isNull(index);
}

std::string EvaluationTargetData::tstCallsignForTime (ptime timestamp) const
{
    assert (hasTstCallsignForTime(timestamp));

    auto it_pair = tst_data_.equal_range(timestamp);

    assert (it_pair.first != tst_data_.end());

    unsigned int index = it_pair.first->second;

    NullableVector<string>& callsign_vec = eval_data_->tst_buffer_->get<string>(eval_data_->tst_callsign_name_);
    assert (!callsign_vec.isNull(index));

    return boost::trim_copy(callsign_vec.get(index)); // remove spaces
}

bool EvaluationTargetData::hasTstModeAForTime (ptime timestamp) const
{
    if (!tst_data_.count(timestamp))
        return false;

    auto it_pair = tst_data_.equal_range(timestamp);

    assert (it_pair.first != tst_data_.end());

    unsigned int index = it_pair.first->second;

    if (eval_data_->tst_buffer_->get<unsigned int>(eval_data_->tst_modea_name_).isNull(index))
        return false;

    if (eval_data_->tst_modea_v_name_.size()
            && !eval_data_->tst_buffer_->get<bool>(eval_data_->tst_modea_v_name_).isNull(index)
            && !eval_data_->tst_buffer_->get<bool>(eval_data_->tst_modea_v_name_).get(index)) // not valid
        return false;

    if (eval_data_->tst_modea_g_name_.size()
            && !eval_data_->tst_buffer_->get<bool>(eval_data_->tst_modea_g_name_).isNull(index)
            && eval_data_->tst_buffer_->get<bool>(eval_data_->tst_modea_g_name_).get(index)) // garbled
        return false;

    return true;
}

unsigned int EvaluationTargetData::tstModeAForTime (ptime timestamp) const
{
    assert (hasTstModeAForTime(timestamp));

    auto it_pair = tst_data_.equal_range(timestamp);

    assert (it_pair.first != tst_data_.end());

    unsigned int index = it_pair.first->second;

    NullableVector<unsigned int>& modea_vec = eval_data_->tst_buffer_->get<unsigned int>(eval_data_->tst_modea_name_);
    assert (!modea_vec.isNull(index));

    return modea_vec.get(index);
}

bool EvaluationTargetData::hasTstModeCForTime (ptime timestamp) const
{
    if (!tst_data_.count(timestamp))
        return false;

    auto it_pair = tst_data_.equal_range(timestamp);

    assert (it_pair.first != tst_data_.end());

    unsigned int index = it_pair.first->second;

    if (eval_data_->tst_buffer_->get<float>(eval_data_->tst_modec_name_).isNull(index))
        return false;

    if (eval_data_->tst_modec_v_name_.size()
            && !eval_data_->tst_buffer_->get<bool>(eval_data_->tst_modec_v_name_).isNull(index)
            && !eval_data_->tst_buffer_->get<bool>(eval_data_->tst_modec_v_name_).get(index)) // not valid
        return false;

    if (eval_data_->tst_modec_g_name_.size()
            && !eval_data_->tst_buffer_->get<bool>(eval_data_->tst_modec_g_name_).isNull(index)
            && eval_data_->tst_buffer_->get<bool>(eval_data_->tst_modec_g_name_).get(index)) // garbled
        return false;

    return true;
}

bool EvaluationTargetData::hasTstGroundBitForTime (ptime timestamp) const // only if set
{
    if (!tst_data_.count(timestamp))
        return false;

    auto it_pair = tst_data_.equal_range(timestamp);

    assert (it_pair.first != tst_data_.end());

    unsigned int index = it_pair.first->second;

    if (eval_data_->tst_ground_bit_name_.size()
            && !eval_data_->tst_buffer_->get<bool>(eval_data_->tst_ground_bit_name_).isNull(index))
        return true;

    return false;
}

bool EvaluationTargetData::tstGroundBitForTime (ptime timestamp) const // true is on ground
{
    assert (hasTstGroundBitForTime(timestamp));

    auto it_pair = tst_data_.equal_range(timestamp);

    assert (it_pair.first != tst_data_.end());

    unsigned int index = it_pair.first->second;

    if (eval_data_->tst_ground_bit_name_.size()
            && !eval_data_->tst_buffer_->get<bool>(eval_data_->tst_ground_bit_name_).isNull(index)
            && eval_data_->tst_buffer_->get<bool>(eval_data_->tst_ground_bit_name_).get(index))
        return true;

    return false;
}

bool EvaluationTargetData::hasTstTAForTime (ptime timestamp) const
{
    if (!tst_data_.count(timestamp))
        return false;

    auto it_pair = tst_data_.equal_range(timestamp);

    assert (it_pair.first != tst_data_.end());

    unsigned int index = it_pair.first->second;

    if (eval_data_->tst_target_address_name_.size()
            && !eval_data_->tst_buffer_->get<unsigned int>(eval_data_->tst_target_address_name_).isNull(index))
        return true;

    return false;
}

unsigned int EvaluationTargetData::tstTAForTime (ptime timestamp) const
{
    assert (hasTstTAForTime(timestamp));

    auto it_pair = tst_data_.equal_range(timestamp);

    assert (it_pair.first != tst_data_.end());

    unsigned int index = it_pair.first->second;

    assert (!eval_data_->tst_buffer_->get<unsigned int>(eval_data_->tst_target_address_name_).isNull(index));

    return eval_data_->tst_buffer_->get<unsigned int>(eval_data_->tst_target_address_name_).get(index);
}

// has gbs, gbs true
pair<bool,bool> EvaluationTargetData::tstGroundBitForTimeInterpolated (ptime timestamp) const // true is on ground
{
    if (!eval_data_->tst_ground_bit_name_.size())
        return pair<bool,bool>(false, false);

    DataMappingTimes times = findTstTimes(timestamp);

    if (times.has_other1_ && (timestamp - times.timestamp_other1_).abs() < seconds(15)
            && hasTstGroundBitForTime(times.timestamp_other1_)
            && tstGroundBitForTime(times.timestamp_other1_))
        return pair<bool,bool> (true, true);

    if (times.has_other2_ && (timestamp - times.timestamp_other2_).abs() < seconds(15)
            && hasTstGroundBitForTime(times.timestamp_other2_))
        return pair<bool,bool> (true, tstGroundBitForTime(times.timestamp_other2_));

    return pair<bool,bool>(false, false);
}

bool EvaluationTargetData::hasTstTrackNumForTime (ptime timestamp) const
{
    if (!eval_data_->tst_track_num_name_.size())
        return false;

    auto it_pair = tst_data_.equal_range(timestamp);

    assert (it_pair.first != tst_data_.end());

    unsigned int index = it_pair.first->second;

    return !eval_data_->tst_buffer_->get<unsigned int>(eval_data_->tst_track_num_name_).isNull(index);
}

unsigned int EvaluationTargetData::tstTrackNumForTime (ptime timestamp) const
{
    assert (hasTstTrackNumForTime(timestamp));

    auto it_pair = tst_data_.equal_range(timestamp);

    assert (it_pair.first != tst_data_.end());

    unsigned int index = it_pair.first->second;

    return eval_data_->tst_buffer_->get<unsigned int>(eval_data_->tst_track_num_name_).get(index);
}

bool EvaluationTargetData::hasTstMeasuredSpeedForTime (ptime timestamp) const
{
    if (!eval_data_->tst_spd_ground_speed_kts_name_.size()
            && (!eval_data_->tst_spd_x_ms_name_.size() || !eval_data_->tst_spd_y_ms_name_.size()))
        return false;

    auto it_pair = tst_data_.equal_range(timestamp);

    assert (it_pair.first != tst_data_.end());

    unsigned int index = it_pair.first->second;

    if (eval_data_->tst_spd_ground_speed_kts_name_.size())
        return !eval_data_->tst_buffer_->get<double>(eval_data_->tst_spd_ground_speed_kts_name_).isNull(index);
    else
        return !eval_data_->tst_buffer_->get<double>(eval_data_->tst_spd_x_ms_name_).isNull(index)
                && !eval_data_->tst_buffer_->get<double>(eval_data_->tst_spd_y_ms_name_).isNull(index);
}

float EvaluationTargetData::tstMeasuredSpeedForTime (ptime timestamp) const // m/s
{
    assert (hasTstMeasuredSpeedForTime(timestamp));

    auto it_pair = tst_data_.equal_range(timestamp);

    assert (it_pair.first != tst_data_.end());

    unsigned int index = it_pair.first->second;

    if (eval_data_->tst_spd_ground_speed_kts_name_.size())
    {
        return eval_data_->tst_buffer_->get<double>(eval_data_->tst_spd_ground_speed_kts_name_).get(index) * KNOTS2M_S;
    }
    else
    {
        double v_x_ms = eval_data_->tst_buffer_->get<double>(eval_data_->tst_spd_x_ms_name_).get(index);
        double v_y_ms = eval_data_->tst_buffer_->get<double>(eval_data_->tst_spd_y_ms_name_).get(index);

        double speed = sqrt(pow(v_x_ms, 2) + pow(v_y_ms, 2));

        assert (!isnan(speed));
        assert (!isinf(speed));

        return speed;
    }
}

bool EvaluationTargetData::hasTstMeasuredTrackAngleForTime (ptime timestamp) const
{
    if (!eval_data_->tst_spd_track_angle_deg_name_.size()
            && (!eval_data_->tst_spd_x_ms_name_.size() || !eval_data_->tst_spd_y_ms_name_.size()))
        return false;

    auto it_pair = tst_data_.equal_range(timestamp);

    assert (it_pair.first != tst_data_.end());

    unsigned int index = it_pair.first->second;

    if (eval_data_->tst_spd_track_angle_deg_name_.size())
        return !eval_data_->tst_buffer_->get<double>(eval_data_->tst_spd_track_angle_deg_name_).isNull(index);
    else
        return !eval_data_->tst_buffer_->get<double>(eval_data_->tst_spd_x_ms_name_).isNull(index)
                && !eval_data_->tst_buffer_->get<double>(eval_data_->tst_spd_y_ms_name_).isNull(index);
}

float EvaluationTargetData::tstMeasuredTrackAngleForTime (ptime timestamp) const // deg
{
    assert (hasTstMeasuredTrackAngleForTime(timestamp));

    auto it_pair = tst_data_.equal_range(timestamp);

    assert (it_pair.first != tst_data_.end());

    unsigned int index = it_pair.first->second;

    if (eval_data_->tst_spd_track_angle_deg_name_.size())
    {
        return eval_data_->tst_buffer_->get<double>(eval_data_->tst_spd_track_angle_deg_name_).get(index);
    }
    else
    {
        double v_x_ms = eval_data_->tst_buffer_->get<double>(eval_data_->tst_spd_x_ms_name_).get(index);
        double v_y_ms = eval_data_->tst_buffer_->get<double>(eval_data_->tst_spd_y_ms_name_).get(index);

        double angle = atan2(v_y_ms,v_x_ms);

        assert (!isnan(angle));
        assert (!isinf(angle));

        return angle;
    }
}

bool EvaluationTargetData::canCheckTstMultipleSources() const
{
    if (!eval_data_->tst_multiple_srcs_name_.size())
        return false;

    if (!eval_data_->tst_buffer_->has<bool>(eval_data_->tst_multiple_srcs_name_))
        return false;

    NullableVector<bool>& tst_multiple_srcs_vec =
            eval_data_->tst_buffer_->get<bool>(eval_data_->tst_multiple_srcs_name_);

    for (auto tst_index : tst_indexes_)
    {
        if (!tst_multiple_srcs_vec.isNull(tst_index))
            return true;
    }

    return false;
}

bool EvaluationTargetData::hasTstMultipleSources() const
{
    assert (canCheckTstMultipleSources());

    NullableVector<bool>& tst_multiple_srcs_vec =
            eval_data_->tst_buffer_->get<bool>(eval_data_->tst_multiple_srcs_name_);

    for (auto tst_index : tst_indexes_) // one must be not null according to canCheckTstMultipleSources
    {
        if (!tst_multiple_srcs_vec.isNull(tst_index) && tst_multiple_srcs_vec.get(tst_index))
            return true;
    }

    return false;
}

bool EvaluationTargetData::canCheckTrackLUDSID() const
{
    if (!eval_data_->tst_track_lu_ds_id_name_.size())
        return false;

    if (!eval_data_->tst_buffer_->has<unsigned int>(eval_data_->tst_track_lu_ds_id_name_))
        return false;

    NullableVector<unsigned int> tst_ls_ds_id_vec =
            eval_data_->tst_buffer_->get<unsigned int>(eval_data_->tst_track_lu_ds_id_name_);

    for (auto tst_index : tst_indexes_)
    {
        if (!tst_ls_ds_id_vec.isNull(tst_index))
            return true;
    }

    return false;
}

bool EvaluationTargetData::hasSingleLUDSID() const
{
    assert (canCheckTrackLUDSID());

    // check if only single source updates
    assert (canCheckTstMultipleSources());
    assert (!hasTstMultipleSources());

    bool lu_ds_id_found = false;
    unsigned int lu_ds_id;

    NullableVector<unsigned int> tst_ls_ds_id_vec =
            eval_data_->tst_buffer_->get<unsigned int>(eval_data_->tst_track_lu_ds_id_name_);

    for (auto tst_index : tst_indexes_)
    {
        if (!tst_ls_ds_id_vec.isNull(tst_index))
        {
            if (!lu_ds_id_found)
            {
                lu_ds_id_found = true;
                lu_ds_id = tst_ls_ds_id_vec.get(tst_index);

                continue;
            }

            if (lu_ds_id_found && tst_ls_ds_id_vec.get(tst_index) != lu_ds_id)
                return false;
        }
    }

    return true;
}

unsigned int EvaluationTargetData::singleTrackLUDSID() const
{
    assert (hasSingleLUDSID());

    NullableVector<unsigned int> tst_ls_ds_id_vec =
            eval_data_->tst_buffer_->get<unsigned int>(eval_data_->tst_track_lu_ds_id_name_);

    for (auto tst_index : tst_indexes_)
    {
        if (!tst_ls_ds_id_vec.isNull(tst_index))
            return tst_ls_ds_id_vec.get(tst_index);
    }

    assert (false); // can not be reached
}


float EvaluationTargetData::tstModeCForTime (ptime timestamp) const
{
    assert (hasTstModeCForTime(timestamp));

    auto it_pair = tst_data_.equal_range(timestamp);

    assert (it_pair.first != tst_data_.end());

    int index = it_pair.first->second;

    NullableVector<float>& modec_vec = eval_data_->tst_buffer_->get<float>(eval_data_->tst_modec_name_);
    assert (!modec_vec.isNull(index));

    return modec_vec.get(index);
}

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
    return has_mops_version_;
}

unsigned int EvaluationTargetData::mopsVersion() const
{
    assert (has_mops_version_);
    return mops_version_;
}

std::string EvaluationTargetData::mopsVersionStr() const
{
    if (hasMOPSVersion())
        return to_string(mopsVersion());
    else
        return "?";
}

std::string EvaluationTargetData::nucpNicStr() const
{
    if (hasNucpNic())
    {
        if (min_nucp_nic_ == max_nucp_nic_)
            return to_string(min_nucp_nic_);
        else
            return to_string(min_nucp_nic_)+"-"+to_string(max_nucp_nic_);
    }
    else
        return "?";
}

std::string EvaluationTargetData::nacpStr() const
{
    if (hasNacp())
    {
        if (min_nacp_ == max_nacp_)
            return to_string(min_nacp_);
        else
            return to_string(min_nacp_)+"-"+to_string(max_nacp_);
    }
    else
        return "?";
}

bool EvaluationTargetData::hasNucpNic() const
{
    return has_nucp_nic_;
}

bool EvaluationTargetData::hasNacp() const
{
    return has_nacp;
}

//std::set<unsigned int> EvaluationTargetData::NACPs() const
//{
//    return nacps_;
//}

//std::set<unsigned int> EvaluationTargetData::NUCpNICs() const
//{
//    return nucp_nics_;
//}

//std::set<unsigned int> EvaluationTargetData::SILs() const
//{
//    return sils_;
//}

//std::shared_ptr<Buffer> EvaluationTargetData::refBuffer() const
//{
//    return ref_buffer_;
//}

//std::shared_ptr<Buffer> EvaluationTargetData::tstBuffer() const
//{
//    return tst_buffer_;
//}

std::set<string> EvaluationTargetData::callsigns() const
{
    return callsigns_;
}

string EvaluationTargetData::callsignsStr() const
{
    std::ostringstream out;

    unsigned int cnt=0;
    for (auto& cs_it : callsigns_)
    {
        if (cnt != 0)
            out << ", ";

        out << cs_it;
        ++cnt;
    }

    return out.str().c_str();
}

std::set<unsigned int> EvaluationTargetData::targetAddresses() const
{
    return target_addresses_;
}

std::string EvaluationTargetData::targetAddressesStr() const
{
    std::ostringstream out;

    unsigned int cnt=0;
    for (auto& ta_it : target_addresses_)
    {
        if (cnt != 0)
            out << ", ";

        out << String::hexStringFromInt(ta_it, 6, '0');
        ++cnt;
    }

    return out.str().c_str();
}

void EvaluationTargetData::updateCallsigns() const
{
    callsigns_.clear();

    if (ref_data_.size())
    {
        NullableVector<string>& value_vec = eval_data_->ref_buffer_->get<string>(eval_data_->ref_callsign_name_);
        map<string, vector<unsigned int>> distinct_values = value_vec.distinctValuesWithIndexes(ref_indexes_);

        for (auto& val_it : distinct_values)
        {
            if (!callsigns_.count(val_it.first))
                callsigns_.insert(val_it.first);
        }
    }

    if (tst_data_.size())
    {
        NullableVector<string>& value_vec = eval_data_->tst_buffer_->get<string>(eval_data_->tst_callsign_name_);
        map<string, vector<unsigned int>> distinct_values = value_vec.distinctValuesWithIndexes(tst_indexes_);

        for (auto& val_it : distinct_values)
        {
            if (!callsigns_.count(val_it.first))
                callsigns_.insert(val_it.first);
        }
    }
}

void EvaluationTargetData::updateTargetAddresses() const
{
    target_addresses_.clear();

    if (ref_data_.size())
    {
        NullableVector<unsigned int>& value_vec = eval_data_->ref_buffer_->get<unsigned int>(
                    eval_data_->ref_target_address_name_);
        map<unsigned int, vector<unsigned int>> distinct_values = value_vec.distinctValuesWithIndexes(ref_indexes_);

        for (auto& val_it : distinct_values)
        {
            if (!target_addresses_.count(val_it.first))
                target_addresses_.insert(val_it.first);
        }
    }

    if (tst_data_.size())
    {
        NullableVector<unsigned int>& value_vec = eval_data_->tst_buffer_->get<unsigned int>(
                    eval_data_->tst_target_address_name_);
        map<unsigned int, vector<unsigned int>> distinct_values = value_vec.distinctValuesWithIndexes(tst_indexes_);

        for (auto& val_it : distinct_values)
        {
            if (!target_addresses_.count(val_it.first))
                target_addresses_.insert(val_it.first);
        }
    }
}

void EvaluationTargetData::updateModeACodes() const
{
    logdbg << "EvaluationTargetData: updateModeACodes: utn " << utn_;

    mode_a_codes_.clear();

    if (ref_data_.size())
    {
        NullableVector<unsigned int>& mode_a_codes = eval_data_->ref_buffer_->get<unsigned int>(
                    eval_data_->ref_modea_name_);
        map<unsigned int, vector<unsigned int>> distinct_codes = mode_a_codes.distinctValuesWithIndexes(ref_indexes_);
        //unsigned int null_cnt = mode_a_codes.nullValueIndexes(ref_rec_nums_).size();

        for (auto& ma_it : distinct_codes)
        {
            if (!mode_a_codes_.count(ma_it.first))
            {
                logdbg << "EvaluationTargetData: updateModeACodes: utn " << utn_ << " new ref m3a "
                       << String::octStringFromInt(ma_it.first, 4, '0');
                mode_a_codes_.insert(ma_it.first);
            }
        }
    }

    if (tst_data_.size())
    {
        NullableVector<unsigned int>& mode_a_codes = eval_data_->tst_buffer_->get<unsigned int>(
                    eval_data_->tst_modea_name_);
        map<unsigned int, vector<unsigned int>> distinct_codes = mode_a_codes.distinctValuesWithIndexes(tst_indexes_);

        for (auto& ma_it : distinct_codes)
        {
            if (!mode_a_codes_.count(ma_it.first))
            {
                logdbg << "EvaluationTargetData: updateModeACodes: utn " << utn_ << " new tst m3a "
                       << String::octStringFromInt(ma_it.first, 4, '0');
                mode_a_codes_.insert(ma_it.first);
            }
        }
    }

    logdbg << "EvaluationTargetData: updateModeACodes: utn " << utn_ << " num codes " << mode_a_codes_.size();
}

void EvaluationTargetData::updateModeCMinMax() const
{
    logdbg << "EvaluationTargetData: updateModeC: utn " << utn_;

    // garbled, valid flags?

    has_mode_c_ = false;

    if (ref_data_.size())
    {
        assert (eval_data_->ref_buffer_->has<float>(eval_data_->ref_modec_name_));
        NullableVector<float>& modec_codes_ft = eval_data_->ref_buffer_->get<float>(eval_data_->ref_modec_name_);

        for (auto ind_it : ref_indexes_)
        {
            if (!modec_codes_ft.isNull(ind_it))
            {
                assert (ind_it < modec_codes_ft.size());

                if (!has_mode_c_)
                {
                    has_mode_c_ = true;
                    mode_c_min_ = modec_codes_ft.get(ind_it);
                    mode_c_max_ = modec_codes_ft.get(ind_it);
                }
                else
                {
                    mode_c_min_ = min(mode_c_min_, modec_codes_ft.get(ind_it));
                    mode_c_max_ = max(mode_c_max_, modec_codes_ft.get(ind_it));
                }
            }
        }
    }

    if (tst_data_.size())
    {
        assert (eval_data_->tst_buffer_->has<float>(eval_data_->tst_modec_name_));
        NullableVector<float>& modec_codes_ft = eval_data_->tst_buffer_->get<float>(eval_data_->tst_modec_name_);

        for (auto ind_it : tst_indexes_)
        {
            if (!modec_codes_ft.isNull(ind_it))
            {
                assert (ind_it < modec_codes_ft.size());

                if (!has_mode_c_)
                {
                    has_mode_c_ = true;
                    mode_c_min_ = modec_codes_ft.get(ind_it);
                    mode_c_max_ = modec_codes_ft.get(ind_it);
                }
                else
                {
                    mode_c_min_ = min(mode_c_min_, modec_codes_ft.get(ind_it));
                    mode_c_max_ = max(mode_c_max_, modec_codes_ft.get(ind_it));
                }
            }
        }
    }
}

void EvaluationTargetData::updatePositionMinMax() const
{
    has_pos_ = false;

    if (ref_data_.size())
    {
        assert (eval_data_->ref_buffer_->has<double>(eval_data_->ref_latitude_name_));
        assert (eval_data_->ref_buffer_->has<double>(eval_data_->ref_longitude_name_));

        NullableVector<double>& lats = eval_data_->ref_buffer_->get<double>(eval_data_->ref_latitude_name_);
        NullableVector<double>& longs = eval_data_->ref_buffer_->get<double>(eval_data_->ref_longitude_name_);

        for (auto ind_it : ref_indexes_)
        {
            assert (!lats.isNull(ind_it));
            assert (!longs.isNull(ind_it));

            if (!has_pos_)
            {
                has_pos_ = true;
                latitude_min_ = lats.get(ind_it);
                latitude_max_ = lats.get(ind_it);
                longitude_min_ = longs.get(ind_it);
                longitude_max_ = longs.get(ind_it);
            }
            else
            {
                latitude_min_ = min(latitude_min_, lats.get(ind_it));
                latitude_max_ = max(latitude_max_, lats.get(ind_it));
                longitude_min_ = min(longitude_min_, longs.get(ind_it));
                longitude_max_ = max(longitude_max_, longs.get(ind_it));
            }
        }
    }

    if (tst_data_.size())
    {
        assert (eval_data_->tst_buffer_->has<double>(eval_data_->tst_latitude_name_));
        assert (eval_data_->tst_buffer_->has<double>(eval_data_->tst_longitude_name_));

        NullableVector<double>& lats = eval_data_->tst_buffer_->get<double>(eval_data_->tst_latitude_name_);
        NullableVector<double>& longs = eval_data_->tst_buffer_->get<double>(eval_data_->tst_longitude_name_);

        for (auto ind_it : tst_indexes_)
        {
            assert (!lats.isNull(ind_it));
            assert (!longs.isNull(ind_it));

            if (!has_pos_)
            {
                has_pos_ = true;
                latitude_min_ = lats.get(ind_it);
                latitude_max_ = lats.get(ind_it);
                longitude_min_ = longs.get(ind_it);
                longitude_max_ = longs.get(ind_it);
            }
            else
            {
                latitude_min_ = min(latitude_min_, lats.get(ind_it));
                latitude_max_ = max(latitude_max_, lats.get(ind_it));
                longitude_min_ = min(longitude_min_, longs.get(ind_it));
                longitude_max_ = max(longitude_max_, longs.get(ind_it));
            }
        }
    }
}

//void EvaluationTargetData::updateADSBInfo() const
//{
//    has_adsb_info_ = false;

//    string mops_name {"mops_version"};
//    string nacp_name {"nac_p"};
//    string nucp_nic_name {"nucp_nic"};
//    string sil_name {"sil"};

//    if (ref_data_.size() && ref_buffer_->dbContentName() == "ADSB")
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

//    if (tst_data_.size() && tst_buffer_->dbContentName() == "ADSB")
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

    assert (!test_data_mappings_.size());

    for (auto& tst_it : tst_data_)
    {
        test_data_mappings_[tst_it.first] = calculateTestDataMapping(tst_it.first);
    }

    unsigned int cnt=0;

    for (auto& tst_map_it : test_data_mappings_)
    {
        if (tst_map_it.second.has_ref_pos_)
            ++cnt;
    }

    loginf << "EvaluationTargetData: calculateTestDataMappings: utn " << utn_ << " done, num map "
           << test_data_mappings_.size() << " ref pos " << cnt;
}

TstDataMapping EvaluationTargetData::calculateTestDataMapping(ptime timestamp) const
{
    TstDataMapping ret;

    ret.timestamp_ = timestamp;

    //    Return iterator to lower bound
    //    Returns an iterator pointing to the first element in the container whose key is not considered to go
    //    before k (i.e., either it is equivalent or goes after).

    auto lb_it = ref_data_.lower_bound(timestamp);

    //auto ub_it = ref_data_.upper_bound(tod);

    if (lb_it != ref_data_.end()) // upper tod found
    {
        assert (lb_it->first >= timestamp);

        // save upper value
        ret.has_ref2_ = true;
        ret.timestamp_ref2_ = lb_it->first;

        // search lower values by decrementing iterator
        while (lb_it != ref_data_.end() && (timestamp < lb_it->first || lb_it->first == ret.timestamp_ref2_))
        {
            if (lb_it == ref_data_.begin()) // exit condition on first value
            {
                if (timestamp < lb_it->first) // set as not found
                    lb_it = ref_data_.end();

                break;
            }

            lb_it--;
        }

        if (lb_it != ref_data_.end() && lb_it->first != ret.timestamp_ref2_) // lower tod found
        {
            assert (timestamp >= lb_it->first);

            // add lower value
            ret.has_ref1_ = true;
            ret.timestamp_ref1_ = lb_it->first;
        }
        else // not found, clear previous
        {
            ret.has_ref2_ = false;
            ret.timestamp_ref2_ = {};
        }
    }

    addRefPositiosToMapping(ret);

    return ret;
}

void EvaluationTargetData::addRefPositiosToMapping (TstDataMapping& mapping) const
{
    if (mapping.has_ref1_ && hasRefPosForTime(mapping.timestamp_ref1_)
            && mapping.has_ref2_ && hasRefPosForTime(mapping.timestamp_ref2_)) // two positions which can be interpolated
    {
        ptime lower = mapping.timestamp_ref1_;
        ptime upper = mapping.timestamp_ref2_;

        EvaluationTargetPosition pos1 = refPosForTime(lower);
        EvaluationTargetPosition pos2 = refPosForTime(upper);
        float d_t = Time::partialSeconds(upper - lower);

        double speed,angle;

        logdbg << "EvaluationTargetData: addRefPositiosToMapping: d_t " << d_t;

        assert (d_t > 0);

        if (pos1.latitude_ == pos2.latitude_ && pos1.longitude_ == pos2.longitude_) // same pos
        {
            mapping.has_ref_pos_ = true;
            mapping.pos_ref_ = pos1;

            mapping.posbased_spd_ref_.x_ = NAN;
            mapping.posbased_spd_ref_.y_ = NAN;
            mapping.posbased_spd_ref_.track_angle_ = NAN;
            mapping.posbased_spd_ref_.speed_ = NAN;
        }
        else
        {

            if (lower == upper) // same time
            {
                logwrn << "EvaluationTargetData: addRefPositiosToMapping: ref has same time twice";
            }
            else
            {
                //                local_->SetStereographic(pos1.latitude_, pos1.longitude_, 1.0, 0.0, 0.0);

                //                unique_ptr<OGRCoordinateTransformation> ogr_geo2cart {
                //                    OGRCreateCoordinateTransformation(wgs84_.get(), local_.get())};
                //                assert (ogr_geo2cart);
                //                unique_ptr<OGRCoordinateTransformation> ogr_cart2geo{
                //                    OGRCreateCoordinateTransformation(local_.get(), wgs84_.get())};
                //                assert (ogr_cart2geo);

                logdbg << "EvaluationTargetData: addRefPositiosToMapping: pos1 "
                       << pos1.latitude_ << ", " << pos1.longitude_;
                logdbg << "EvaluationTargetData: addRefPositiosToMapping: pos2 "
                       << pos2.latitude_ << ", " << pos2.longitude_;

                bool ok;
                double x_pos, y_pos;

                //                if (in_appimage_) // inside appimage
                //                {
                //                    x_pos = pos2.longitude_;
                //                    y_pos = pos2.latitude_;
                //                }
                //                else
                //                {
                //                    x_pos = pos2.latitude_;
                //                    y_pos = pos2.longitude_;
                //                }

                tie(ok, x_pos, y_pos) = trafo_.distanceCart(
                            pos1.latitude_, pos1.longitude_, pos2.latitude_, pos2.longitude_);

                //                logdbg << "EvaluationTargetData: addRefPositiosToMapping: geo2cart";
                //                bool ret = ogr_geo2cart->Transform(1, &x_pos, &y_pos); // wgs84 to cartesian offsets
                if (!ok)
                {
                    logerr << "EvaluationTargetData: addRefPositiosToMapping: error with latitude " << pos2.latitude_
                           << " longitude " << pos2.longitude_;
                }
                else
                {

                    logdbg << "EvaluationTargetData: addRefPositiosToMapping: offsets x " << fixed << x_pos
                           << " y " << fixed << y_pos << " dist " << fixed << sqrt(pow(x_pos,2)+pow(y_pos,2));

                    double v_x = x_pos/d_t;
                    double v_y = y_pos/d_t;
                    logdbg << "EvaluationTargetData: addRefPositiosToMapping: v_x " << v_x << " v_y " << v_y;

                    float d_t2 = Time::partialSeconds(mapping.timestamp_ - lower);
                    logdbg << "EvaluationTargetData: addRefPositiosToMapping: d_t2 " << d_t2;

                    assert (d_t2 >= 0);

                    x_pos = v_x * d_t2;
                    y_pos = v_y * d_t2;

                    logdbg << "EvaluationTargetData: addRefPositiosToMapping: interpolated offsets x "
                           << x_pos << " y " << y_pos;

                    //ret = ogr_cart2geo->Transform(1, &x_pos, &y_pos);

                    tie (ok, x_pos, y_pos) = trafo_.wgsAddCartOffset(pos1.latitude_, pos1.longitude_, x_pos, y_pos);

                    //assert (ok); TODO?

                    // x_pos long, y_pos lat

                    logdbg << "EvaluationTargetData: addRefPositiosToMapping: interpolated lat "
                           << x_pos << " long " << y_pos;

                    // calculate altitude
                    bool has_altitude = false;
                    float altitude = 0.0;

                    if (pos1.has_altitude_ && !pos2.has_altitude_)
                    {
                        has_altitude = true;
                        altitude = pos1.altitude_;
                    }
                    else if (!pos1.has_altitude_ && pos2.has_altitude_)
                    {
                        has_altitude = true;
                        altitude = pos2.altitude_;
                    }
                    else if (pos1.has_altitude_ && pos2.has_altitude_)
                    {
                        float v_alt = (pos2.altitude_ - pos1.altitude_)/d_t;
                        has_altitude = true;
                        altitude = pos1.altitude_ + v_alt*d_t2;
                    }

                    logdbg << "EvaluationTargetData: addRefPositiosToMapping: pos1 has alt "
                           << pos1.has_altitude_ << " alt " << pos1.altitude_
                           << " pos2 has alt " << pos2.has_altitude_ << " alt " << pos2.altitude_
                           << " interpolated has alt " << has_altitude << " alt " << altitude;

                    mapping.has_ref_pos_ = true;

                    //                    if (in_appimage_) // inside appimage
                    //                        mapping.pos_ref_ = EvaluationTargetPosition(y_pos, x_pos, has_altitude, true, altitude);
                    //                    else
                    mapping.pos_ref_ = EvaluationTargetPosition(x_pos, y_pos, has_altitude, true, altitude);

                    angle = atan2(v_y,v_x);
                    speed = sqrt(pow(v_x, 2) + pow(v_y, 2));

                    if (!isnan(angle) && !isinf(angle) && !isnan(speed) && !isinf(speed))
                    {
                        mapping.has_ref_spd_ = true;
                        mapping.posbased_spd_ref_.x_ = v_x;
                        mapping.posbased_spd_ref_.y_ = v_y;
                        mapping.posbased_spd_ref_.track_angle_ = angle;
                        mapping.posbased_spd_ref_.speed_ = speed;
                    }
                    else
                        mapping.has_ref_spd_ = false;

                }
            }
        }
    }
    // else do nothing
}

void EvaluationTargetData::addRefPositiosToMappingFast (TstDataMapping& mapping) const
{
    if (mapping.has_ref1_ && hasRefPosForTime(mapping.timestamp_ref1_)
            && mapping.has_ref2_ && hasRefPosForTime(mapping.timestamp_ref2_)) // two positions which can be interpolated
    {
        ptime lower = mapping.timestamp_ref1_;
        ptime upper = mapping.timestamp_ref2_;

        EvaluationTargetPosition pos1 = refPosForTime(lower);
        EvaluationTargetPosition pos2 = refPosForTime(upper);
        float d_t = Time::partialSeconds(upper - lower);

        logdbg << "EvaluationTargetData: addRefPositiosToMappingFast: d_t " << d_t;

        assert (d_t >= 0);

        if (pos1.latitude_ == pos2.latitude_ && pos1.longitude_ == pos2.longitude_) // same pos
        {
            mapping.has_ref_pos_ = true;
            mapping.pos_ref_ = pos1;
        }
        else
        {

            if (lower == upper) // same time
            {
                logwrn << "EvaluationTargetData: addRefPositiosToMappingFast: ref has same time twice";
            }
            else
            {
                double v_lat = (pos2.latitude_ - pos1.latitude_)/d_t;
                double v_long = (pos2.longitude_ - pos1.longitude_)/d_t;

                logdbg << "EvaluationTargetData: interpolatedPosForTimeFast: v_x " << v_lat << " v_y " << v_long;

                float d_t2 = Time::partialSeconds(mapping.timestamp_  - lower);
                logdbg << "EvaluationTargetData: interpolatedPosForTimeFast: d_t2 " << d_t2;

                assert (d_t2 >= 0);

                double int_lat = pos1.latitude_ + v_lat * d_t2;
                double int_long = pos1.longitude_ + v_long * d_t2;

                logdbg << "EvaluationTargetData: interpolatedPosForTimeFast: interpolated lat " << int_lat
                       << " long " << int_long;

                // calculate altitude
                bool has_altitude = false;
                float altitude = 0.0;

                if (pos1.has_altitude_ && !pos2.has_altitude_)
                {
                    has_altitude = true;
                    altitude = pos1.altitude_;
                }
                else if (!pos1.has_altitude_ && pos2.has_altitude_)
                {
                    has_altitude = true;
                    altitude = pos2.altitude_;
                }
                else if (pos1.has_altitude_ && pos2.has_altitude_)
                {
                    float v_alt = (pos2.altitude_ - pos1.altitude_)/d_t;
                    has_altitude = true;
                    altitude = pos1.altitude_ + v_alt*d_t2;
                }

                logdbg << "EvaluationTargetData: interpolatedPosForTimeFast: pos1 has alt "
                       << pos1.has_altitude_ << " alt " << pos1.altitude_
                       << " pos2 has alt " << pos2.has_altitude_ << " alt " << pos2.altitude_
                       << " interpolated has alt " << has_altitude << " alt " << altitude;

                mapping.has_ref_pos_ = true;

                mapping.pos_ref_ = EvaluationTargetPosition(int_lat, int_long, has_altitude, true, altitude);
            }
        }
    }
    // else do nothing
}

DataMappingTimes EvaluationTargetData::findTstTimes(ptime timestamp_ref) const // ref tod
{
    DataMappingTimes ret;

    ret.timestamp_ = timestamp_ref;

    //    Return iterator to lower bound
    //    Returns an iterator pointing to the first element in the container whose key is not considered to go
    //    before k (i.e., either it is equivalent or goes after).

    auto lb_it = tst_data_.lower_bound(timestamp_ref);

    if (lb_it != tst_data_.end()) // upper tod found
    {
        assert (lb_it->first >= timestamp_ref);

        // save upper value
        ret.has_other2_ = true;
        ret.timestamp_other2_ = lb_it->first;

        // search lower values by decrementing iterator
        while (lb_it != tst_data_.end() && (timestamp_ref < lb_it->first || lb_it->first == ret.timestamp_other2_))
        {
            if (lb_it == tst_data_.begin()) // exit condition on first value
            {
                if (timestamp_ref < lb_it->first) // set as not found
                    lb_it = tst_data_.end();

                break;
            }

            lb_it--;
        }

        if (lb_it != tst_data_.end() && lb_it->first != ret.timestamp_other2_) // lower tod found
        {
            assert (timestamp_ref >= lb_it->first);

            // add lower value
            ret.has_other1_ = true;
            ret.timestamp_other1_ = lb_it->first;
        }
        else // not found, clear previous
        {
            ret.has_other2_ = false;
            ret.timestamp_other2_ = {};
        }
    }

    return ret;
}
