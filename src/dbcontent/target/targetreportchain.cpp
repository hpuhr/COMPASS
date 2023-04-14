#include "targetreportchain.h"
#include "targetposition.h"
#include "targetvelocity.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/metavariable.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent.h"
#include "dbcontentcache.h"
#include "util/number.h"
#include "util/timeconv.h"

#include <boost/algorithm/string.hpp>

using namespace std;
using namespace Utils;
using namespace boost::posix_time;

namespace dbContent {

namespace TargetReport {

Chain::Chain(std::shared_ptr<dbContent::Cache> cache, const std::string& dbcontent_name)
    : cache_(cache), dbcontent_name_(dbcontent_name)
{

}

Chain::~Chain()
{

}

void Chain::addIndex (boost::posix_time::ptime timestamp, unsigned int index)
{
    unsigned int idx_int = (unsigned int)ref_indices_.size();

    ref_data_.insert({timestamp, Index(index, idx_int)});
    ref_indices_.push_back(index);
}

bool Chain::hasData() const
{
    return !ref_data_.empty();
}

void Chain::finalize () const
{
    updateCallsigns();
    updateTargetAddresses();
    updateModeACodes();
    updateModeCMinMax();
    updatePositionMinMax();
}

unsigned int Chain::numUpdates () const
{
    return ref_data_.size();
}

ptime Chain::timeBegin() const
{
    if (ref_data_.size())
        return ref_data_.begin()->first;
    else
        throw std::runtime_error("Chain: timeBegin: no data");
}

std::string Chain::timeBeginStr() const
{
    if (hasData())
        return Time::toString(timeBegin());
    else
        return "";
}

ptime Chain::timeEnd() const
{
    if (ref_data_.size())
        return ref_data_.rbegin()->first;
    else
        throw std::runtime_error("Chain: timeEnd: no data");
}

std::string Chain::timeEndStr() const
{
    if (hasData())
        return Time::toString(timeEnd());
    else
        return "";
}

time_duration Chain::timeDuration() const
{
    if (hasData())
        return timeEnd() - timeBegin();
    else
        return {};
}

std::set<unsigned int> Chain::modeACodes() const
{
    return mode_a_codes_;
}

std::string Chain::modeACodesStr() const
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

bool Chain::hasModeC() const
{
    return has_mode_c_;
}

float Chain::modeCMin() const
{
    assert (has_mode_c_);
    return mode_c_min_;
}

std::string Chain::modeCMinStr() const
{
    if (has_mode_c_)
        return to_string((int)mode_c_min_);
    else
        return "";
}

float Chain::modeCMax() const
{
    assert (has_mode_c_);
    return mode_c_max_;
}

std::string Chain::modeCMaxStr() const
{
    if (has_mode_c_)
        return to_string((int)mode_c_max_);
    else
        return "";
}

bool Chain::isPrimaryOnly () const
{
    return !callsigns_.size() && !target_addresses_.size() && !mode_a_codes_.size() && !has_mode_c_;
}

const Chain::IndexMap& Chain::data() const
{
    return ref_data_;
}

double Chain::latitudeMin() const
{
    assert (has_pos_);
    return latitude_min_;
}

double Chain::latitudeMax() const
{
    assert (has_pos_);
    return latitude_max_;
}

double Chain::longitudeMin() const
{
    assert (has_pos_);
    return longitude_min_;
}

double Chain::longitudeMax() const
{
    assert (has_pos_);
    return longitude_max_;
}

bool Chain::hasPos() const
{
    return has_pos_;
}

Chain::DataID Chain::dataID(const boost::posix_time::ptime& timestamp) const
{
    const IndexMap& index_map = ref_data_;

    auto range = index_map.equal_range(timestamp);

    assert(range.first != index_map.end());

    assert(range.first->second.idx_internal < index_map.size());

    return DataID(timestamp).addIndex(range.first->second);
}

bool Chain::hasRefPos(const DataID& id) const
{
    auto timestamp = timestampFromDataID(id);
    return ref_data_.count(timestamp);
}

dbContent::TargetPosition Chain::refPos(const DataID& id) const
{
    auto timestamp = timestampFromDataID(id);
    auto index     = indexFromDataID(id);

    unsigned int index_ext = index.idx_external;

    dbContent::TargetPosition pos;

    NullableVector<double>& latitude_vec  = cache_->getMetaVar<double>(dbcontent_name_, DBContent::meta_var_latitude_);
    NullableVector<double>& longitude_vec = cache_->getMetaVar<double>(dbcontent_name_, DBContent::meta_var_longitude_);
    NullableVector<float>& altitude_vec   = cache_->getMetaVar<float>(dbcontent_name_, DBContent::meta_var_mc_);

    NullableVector<float>* altitude_trusted_vec {nullptr};
    NullableVector<float>* altitude_secondary_vec {nullptr};

    if (dbcontent_name_ == "CAT062")
    {
        altitude_trusted_vec = &cache_->getVar<float>(dbcontent_name_, DBContent::var_cat062_fl_measured_);
        altitude_secondary_vec = &cache_->getVar<float>(dbcontent_name_, DBContent::var_cat062_baro_alt_);
    }

    assert (!latitude_vec.isNull(index_ext));
    assert (!longitude_vec.isNull(index_ext));

    pos.latitude_  = latitude_vec.get(index_ext);
    pos.longitude_ = longitude_vec.get(index_ext);

    if (altitude_trusted_vec && !altitude_trusted_vec->isNull(index_ext))
    {
        pos.has_altitude_        = true;
        pos.altitude_calculated_ = false;
        pos.altitude_            = altitude_trusted_vec->get(index_ext);
    }
    else if (!altitude_vec.isNull(index_ext))
    {
        pos.has_altitude_        = true;
        pos.altitude_calculated_ = false;
        pos.altitude_            = altitude_vec.get(index_ext);
    }
    else if (altitude_secondary_vec && !altitude_secondary_vec->isNull(index_ext))
    {
        pos.has_altitude_        = true;
        pos.altitude_calculated_ = true;
        pos.altitude_            = altitude_secondary_vec->get(index_ext);
    }
    else // calculate
    {
        bool found;
        float alt_calc;

        tie(found,alt_calc) = estimateRefAltitude(timestamp, index.idx_internal);

        if (found)
        {
            pos.has_altitude_        = true;
            pos.altitude_calculated_ = true;
            pos.altitude_            = alt_calc;
        }
    }

    return pos;
}

bool Chain::hasRefSpeed(const DataID& id) const
{
    auto index = indexFromDataID(id);

    unsigned int index_ext = index.idx_external;

    NullableVector<double>& speed_vec = cache_->getMetaVar<double>(
                dbcontent_name_, DBContent::meta_var_ground_speed_);
    NullableVector<double>& track_angle_vec = cache_->getMetaVar<double>(
                dbcontent_name_, DBContent::meta_var_track_angle_);

    return !speed_vec.isNull(index_ext) && !track_angle_vec.isNull(index_ext);
}

dbContent::TargetVelocity Chain::refSpeed(const DataID& id) const
{
    auto index = indexFromDataID(id);

    unsigned int index_ext = index.idx_external;

    NullableVector<double>& speed_vec = cache_->getMetaVar<double>(
                dbcontent_name_, DBContent::meta_var_ground_speed_);
    NullableVector<double>& track_angle_vec = cache_->getMetaVar<double>(
                dbcontent_name_, DBContent::meta_var_track_angle_);

    assert (!speed_vec.isNull(index_ext));
    assert (!track_angle_vec.isNull(index_ext));

    dbContent::TargetVelocity spd;

    spd.speed_ = speed_vec.get(index_ext) * KNOTS2M_S; // true north to mathematical
    spd.track_angle_ = track_angle_vec.get(index_ext);

    return spd;
}

bool Chain::hasRefCallsign(const DataID& id) const
{
    auto index = indexFromDataID(id);

    unsigned int index_ext = index.idx_external;

    NullableVector<string>& callsign_vec = cache_->getMetaVar<string>(dbcontent_name_, DBContent::meta_var_ti_);

    return !callsign_vec.isNull(index_ext);
}

std::string Chain::refCallsign(const DataID& id) const
{
    assert (hasRefCallsign(id));

    auto index = indexFromDataID(id);

    unsigned int index_ext = index.idx_external;

    NullableVector<string>& callsign_vec = cache_->getMetaVar<string>(dbcontent_name_, DBContent::meta_var_ti_);
    assert (!callsign_vec.isNull(index_ext));

    return boost::trim_copy(callsign_vec.get(index_ext)); // remove spaces
}

bool Chain::hasRefModeA(const DataID& id) const
{
    auto index = indexFromDataID(id);

    unsigned int index_ext = index.idx_external;

    if (cache_->getMetaVar<unsigned int>(dbcontent_name_, DBContent::meta_var_m3a_).isNull(index_ext))
        return false;

    if (cache_->hasMetaVar<bool>(dbcontent_name_, DBContent::meta_var_m3a_v_)
            && !cache_->getMetaVar<bool>(dbcontent_name_, DBContent::meta_var_m3a_v_).isNull(index_ext)
            && !cache_->getMetaVar<bool>(dbcontent_name_, DBContent::meta_var_m3a_v_).get(index_ext)) // not valid
        return false;

    if (cache_->hasMetaVar<bool>(dbcontent_name_, DBContent::meta_var_m3a_g_)
            && !cache_->getMetaVar<bool>(dbcontent_name_, DBContent::meta_var_m3a_g_).isNull(index_ext)
            && cache_->getMetaVar<bool>(dbcontent_name_, DBContent::meta_var_m3a_g_).get(index_ext)) // garbled
        return false;

    return true;
}

unsigned int Chain::refModeA(const DataID& id) const
{
    assert (hasRefModeA(id));

    auto index = indexFromDataID(id);

    unsigned int index_ext = index.idx_external;

    NullableVector<unsigned int>& modea_vec = cache_->getMetaVar<unsigned int>(
                dbcontent_name_, DBContent::meta_var_m3a_);
    assert (!modea_vec.isNull(index_ext));

    return modea_vec.get(index_ext);
}

bool Chain::hasRefModeC(const DataID& id) const
{
    auto index = indexFromDataID(id);

    unsigned int index_ext = index.idx_external;

    NullableVector<float>* altitude_trusted_vec {nullptr};
    //NullableVector<float>* altitude_secondary_vec {nullptr}; only used in position?

    if (dbcontent_name_ == "CAT062")
    {
        altitude_trusted_vec = &cache_->getVar<float>(dbcontent_name_, DBContent::var_cat062_fl_measured_);
        //altitude_secondary_vec = &cache_->getVar<float>(dbcontent_name_, DBContent::var_cat062_baro_alt_);
    }

    if (altitude_trusted_vec && !altitude_trusted_vec->isNull(index_ext))
        return true;

    if (cache_->getMetaVar<float>(dbcontent_name_, DBContent::meta_var_mc_).isNull(index_ext))
        return false;

    if (cache_->hasMetaVar<bool>(dbcontent_name_, DBContent::meta_var_mc_v_)
            && !cache_->getMetaVar<bool>(dbcontent_name_, DBContent::meta_var_mc_v_).isNull(index_ext)
            && !cache_->getMetaVar<bool>(dbcontent_name_, DBContent::meta_var_mc_v_).get(index_ext)) // not valid
        return false;

    if (cache_->hasMetaVar<bool>(dbcontent_name_, DBContent::meta_var_mc_g_)
            && !cache_->getMetaVar<bool>(dbcontent_name_, DBContent::meta_var_mc_g_).isNull(index_ext)
            && cache_->getMetaVar<bool>(dbcontent_name_, DBContent::meta_var_mc_g_).get(index_ext)) // garbled
        return false;

    return true;
}

float Chain::refModeC(const DataID& id) const
{
    assert (hasRefModeC(id));

    auto index = indexFromDataID(id);

    unsigned int index_ext = index.idx_external;

    NullableVector<float>* altitude_trusted_vec {nullptr};

    if (dbcontent_name_ == "CAT062")
        altitude_trusted_vec = &cache_->getVar<float>(dbcontent_name_, DBContent::var_cat062_fl_measured_);

    if (altitude_trusted_vec && !altitude_trusted_vec->isNull(index_ext))
        return altitude_trusted_vec->get(index_ext);

    assert (!cache_->getMetaVar<float>(dbcontent_name_, DBContent::meta_var_mc_).isNull(index_ext));

    return cache_->getMetaVar<float>(dbcontent_name_, DBContent::meta_var_mc_).get(index_ext);
}

bool Chain::hasRefTA(const DataID& id) const
{
    auto index = indexFromDataID(id);

    unsigned int index_ext = index.idx_external;

    if (cache_->hasMetaVar<unsigned int>(dbcontent_name_, DBContent::meta_var_ta_)
            && !cache_->getMetaVar<unsigned int>(dbcontent_name_, DBContent::meta_var_ta_).isNull(index_ext))
        return true;

    return false;
}

unsigned int Chain::refTA(const DataID& id) const
{
    assert (hasRefTA(id));

    auto index = indexFromDataID(id);

    unsigned int index_ext = index.idx_external;

    assert (!cache_->getMetaVar<unsigned int>(dbcontent_name_, DBContent::meta_var_ta_).isNull(index_ext));

    return cache_->getMetaVar<unsigned int>(dbcontent_name_, DBContent::meta_var_ta_).get(index_ext);
}

std::pair<bool,bool> Chain::refGroundBit(const DataID& id) const // has gbs, gbs true
{
    auto index = indexFromDataID(id);

    unsigned int index_ext = index.idx_external;

    if (cache_->hasMetaVar<bool>(dbcontent_name_, DBContent::meta_var_ground_bit_)
            && !cache_->getMetaVar<bool>(dbcontent_name_, DBContent::meta_var_ground_bit_).isNull(index_ext))
    {
        return {true, cache_->getMetaVar<bool>(dbcontent_name_, DBContent::meta_var_ground_bit_).get(index_ext)};
    }
    else
        return {false, false};
}

std::pair<bool, float> Chain::estimateRefAltitude (const boost::posix_time::ptime& timestamp,
                                                   unsigned int index_internal) const
{
    assert(index_internal < ref_indices_.size());

    NullableVector<float>& altitude_vec = cache_->getMetaVar<float>(dbcontent_name_, DBContent::meta_var_mc_);
    NullableVector<ptime>& ts_vec = cache_->getMetaVar<ptime>(dbcontent_name_, DBContent::meta_var_timestamp_);

    NullableVector<float>* altitude_trusted_vec {nullptr};

    if (dbcontent_name_ == "CAT062")
        altitude_trusted_vec = &cache_->getVar<float>(dbcontent_name_, DBContent::var_cat062_fl_measured_);

    bool found_prev {false};
    float altitude_prev {0.0};
    bool found_after {false};
    float altitude_after {0.0};

    // search for prev index
    ptime timestamp_prev;
    auto prev_it  = ref_indices_.begin() + index_internal;
    auto after_it = prev_it;

    const time_duration max_tdiff = seconds(120);

    while (prev_it != ref_indices_.end() && timestamp - ts_vec.get(*prev_it) < max_tdiff)
    {
        if (altitude_trusted_vec && !altitude_trusted_vec->isNull(*prev_it))
        {
            found_prev = true;
            altitude_prev = altitude_trusted_vec->get(*prev_it);
            timestamp_prev = ts_vec.get(*prev_it);

            break;
        }
        else if (!altitude_vec.isNull(*prev_it))
        {
            found_prev = true;
            altitude_prev = altitude_vec.get(*prev_it);
            timestamp_prev = ts_vec.get(*prev_it);

            break;
        }

        if (prev_it == ref_indices_.begin()) // undefined decrement
            break;

        --prev_it;
    }

    // search after index
    ptime timestamp_after;

    while (after_it != ref_indices_.end() && ts_vec.get(*after_it) - timestamp < max_tdiff)
    {
        if (altitude_trusted_vec && !altitude_trusted_vec->isNull(*after_it))
        {
            found_after = true;
            altitude_after = altitude_trusted_vec->get(*after_it);
            timestamp_after = ts_vec.get(*after_it);

            break;
        }
        else if (!altitude_vec.isNull(*after_it))
        {
            found_after = true;
            altitude_after = altitude_vec.get(*after_it);
            timestamp_after = ts_vec.get(*after_it);

            break;
        }
        ++after_it;
    }

    if (found_prev && found_after)
    {
        if (timestamp_after <= timestamp_prev || timestamp_prev >= timestamp)
        {
            logerr << "Chain: estimateRefAltitude: ts_prev " << Time::toString(timestamp_prev)
                   << " ts " << Time::toString(timestamp) << " ts_after " << Time::toString(timestamp_after);

            return {false, 0}; // should never happen
        }

        float d_alt_ft = altitude_after - altitude_prev;
        float d_t = Time::partialSeconds(timestamp_after - timestamp_prev);

        float alt_spd_ft_s = d_alt_ft/d_t;

        float d_t2 = Time::partialSeconds(timestamp - timestamp_prev);

        float alt_calc = altitude_prev + alt_spd_ft_s*d_t2;

        return {true, alt_calc};
    }
    else if (found_prev && timestamp - timestamp_prev < max_tdiff)
        return {true, altitude_prev};
    else if (found_after && timestamp_after - timestamp < max_tdiff)
        return {true, altitude_after};
    else
    {
        return {false, 0}; // none found
    }
}

Index Chain::indexFromDataID(const DataID& id) const
{
    assert(id.valid());

    if (!id.hasIndex())
    {
        auto id_ret = dataID(id.timestamp());
        assert(id_ret.valid());

        return id_ret.index();
    }

    return id.index();
}

boost::posix_time::ptime Chain::timestampFromDataID(const DataID& id) const
{
    assert(id.valid());
    return id.timestamp();
}

std::set<string> Chain::callsigns() const
{
    return callsigns_;
}

string Chain::callsignsStr() const
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

std::set<unsigned int> Chain::targetAddresses() const
{
    return target_addresses_;
}

std::string Chain::targetAddressesStr() const
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

void Chain::updateCallsigns() const
{
    callsigns_.clear();

    if (ref_data_.size())
    {
        NullableVector<string>& value_vec = cache_->getMetaVar<string>(dbcontent_name_, DBContent::meta_var_ti_);
        map<string, vector<unsigned int>> distinct_values = value_vec.distinctValuesWithIndexes(ref_indices_);

        for (auto& val_it : distinct_values)
        {
            if (!callsigns_.count(val_it.first))
                callsigns_.insert(val_it.first);
        }
    }

}

void Chain::updateTargetAddresses() const
{
    target_addresses_.clear();

    if (ref_data_.size())
    {
        NullableVector<unsigned int>& value_vec = cache_->getMetaVar<unsigned int>(
                    dbcontent_name_, DBContent::meta_var_ta_);
        map<unsigned int, vector<unsigned int>> distinct_values = value_vec.distinctValuesWithIndexes(ref_indices_);

        for (auto& val_it : distinct_values)
        {
            if (!target_addresses_.count(val_it.first))
                target_addresses_.insert(val_it.first);
        }
    }

}

void Chain::updateModeACodes() const
{
    logdbg << "Chain: updateModeACodes";

    mode_a_codes_.clear();

    if (ref_data_.size())
    {
        NullableVector<unsigned int>& mode_a_codes = cache_->getMetaVar<unsigned int>(
                    dbcontent_name_, DBContent::meta_var_m3a_);
        map<unsigned int, vector<unsigned int>> distinct_codes = mode_a_codes.distinctValuesWithIndexes(ref_indices_);
        //unsigned int null_cnt = mode_a_codes.nullValueIndexes(ref_rec_nums_).size();

        for (auto& ma_it : distinct_codes)
        {
            if (!mode_a_codes_.count(ma_it.first))
            {
                logdbg << "Chain: updateModeACodes: new ref m3a "
                       << String::octStringFromInt(ma_it.first, 4, '0');
                mode_a_codes_.insert(ma_it.first);
            }
        }
    }


    logdbg << "Chain: updateModeACodes: num codes " << mode_a_codes_.size();
}

void Chain::updateModeCMinMax() const
{
    logdbg << "Chain: updateModeC";

    // garbled, valid flags?

    has_mode_c_ = false;
    float mode_c_value;

    if (ref_data_.size())
    {
        NullableVector<float>& modec_codes_ft = cache_->getMetaVar<float>(dbcontent_name_, DBContent::meta_var_mc_);

        NullableVector<float>* altitude_trusted_vec {nullptr};

        if (dbcontent_name_ == "CAT062")
            altitude_trusted_vec = &cache_->getVar<float>(dbcontent_name_, DBContent::var_cat062_fl_measured_);

        for (auto ind_it : ref_indices_)
        {
            if (altitude_trusted_vec && !altitude_trusted_vec->isNull(ind_it))
                mode_c_value = altitude_trusted_vec->get(ind_it);
            else if (!modec_codes_ft.isNull(ind_it))
                mode_c_value = modec_codes_ft.get(ind_it);
            else
                continue;

            // mode_c_value is set

            if (!has_mode_c_)
            {
                has_mode_c_ = true;
                mode_c_min_ = mode_c_value;
                mode_c_max_ = mode_c_value;
            }
            else
            {
                mode_c_min_ = min(mode_c_min_, mode_c_value);
                mode_c_max_ = max(mode_c_max_, mode_c_value);
            }
        }
    }


}

void Chain::updatePositionMinMax() const
{
    has_pos_ = false;

    if (ref_data_.size())
    {
        NullableVector<double>& lats = cache_->getMetaVar<double>(dbcontent_name_, DBContent::meta_var_latitude_);
        NullableVector<double>& longs = cache_->getMetaVar<double>(dbcontent_name_, DBContent::meta_var_longitude_);

        for (auto ind_it : ref_indices_)
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


} // namespace TargetReport

} // namespace dbContent
