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
    unsigned int idx_int = (unsigned int)indexes_.size();

    timestamp_index_lookup_.insert({timestamp, Index(index, idx_int)});
    indexes_.push_back(index);
}

bool Chain::hasData() const
{
    return !timestamp_index_lookup_.empty();
}

void Chain::finalize () const
{
    updateCallsigns();
    updateTargetAddresses();
    updateModeACodes();
    updateModeCMinMax();
    updatePositionMinMax();
}

unsigned int Chain::size () const
{
    return timestamp_index_lookup_.size();
}

ptime Chain::timeBegin() const
{
    if (timestamp_index_lookup_.size())
        return timestamp_index_lookup_.begin()->first;
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
    if (timestamp_index_lookup_.size())
        return timestamp_index_lookup_.rbegin()->first;
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
    return !acids_.size() && !acads_.size() && !mode_a_codes_.size() && !has_mode_c_;
}

const Chain::IndexMap& Chain::timestampIndexes() const
{
    return timestamp_index_lookup_;
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
    const IndexMap& index_map = timestamp_index_lookup_;

    auto range = index_map.equal_range(timestamp);

    assert(range.first != index_map.end());

    assert(range.first->second.idx_internal < index_map.size());

    return DataID(timestamp).addIndex(range.first->second);
}

bool Chain::hasPos(const DataID& id) const
{
    auto timestamp = timestampFromDataID(id);
    return timestamp_index_lookup_.count(timestamp);
}

dbContent::TargetPosition Chain::pos(const DataID& id) const
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

        tie(found,alt_calc) = estimateAltitude(timestamp, index.idx_internal);

        if (found)
        {
            pos.has_altitude_        = true;
            pos.altitude_calculated_ = true;
            pos.altitude_            = alt_calc;
        }
    }

    return pos;
}

bool Chain::hasSpeed(const DataID& id) const
{
    auto index = indexFromDataID(id);

    unsigned int index_ext = index.idx_external;

    NullableVector<double>& speed_vec = cache_->getMetaVar<double>(
                dbcontent_name_, DBContent::meta_var_ground_speed_);
    NullableVector<double>& track_angle_vec = cache_->getMetaVar<double>(
                dbcontent_name_, DBContent::meta_var_track_angle_);

    return !speed_vec.isNull(index_ext) && !track_angle_vec.isNull(index_ext);
}

dbContent::TargetVelocity Chain::speed(const DataID& id) const
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

bool Chain::hasACID(const DataID& id) const
{
    auto index = indexFromDataID(id);

    unsigned int index_ext = index.idx_external;

    NullableVector<string>& callsign_vec = cache_->getMetaVar<string>(dbcontent_name_, DBContent::meta_var_ti_);

    return !callsign_vec.isNull(index_ext);
}

std::string Chain::acid(const DataID& id) const
{
    assert (hasACID(id));

    auto index = indexFromDataID(id);

    unsigned int index_ext = index.idx_external;

    NullableVector<string>& callsign_vec = cache_->getMetaVar<string>(dbcontent_name_, DBContent::meta_var_ti_);
    assert (!callsign_vec.isNull(index_ext));

    return boost::trim_copy(callsign_vec.get(index_ext)); // remove spaces
}

bool Chain::hasModeA(const DataID& id) const
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

unsigned int Chain::modeA(const DataID& id) const
{
    assert (hasModeA(id));

    auto index = indexFromDataID(id);

    unsigned int index_ext = index.idx_external;

    NullableVector<unsigned int>& modea_vec = cache_->getMetaVar<unsigned int>(
                dbcontent_name_, DBContent::meta_var_m3a_);
    assert (!modea_vec.isNull(index_ext));

    return modea_vec.get(index_ext);
}

bool Chain::hasModeC(const DataID& id) const
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

float Chain::modeC(const DataID& id) const
{
    assert (hasModeC(id));

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

bool Chain::hasACAD(const DataID& id) const
{
    auto index = indexFromDataID(id);

    unsigned int index_ext = index.idx_external;

    if (cache_->hasMetaVar<unsigned int>(dbcontent_name_, DBContent::meta_var_ta_)
            && !cache_->getMetaVar<unsigned int>(dbcontent_name_, DBContent::meta_var_ta_).isNull(index_ext))
        return true;

    return false;
}

unsigned int Chain::acad(const DataID& id) const
{
    assert (hasACAD(id));

    auto index = indexFromDataID(id);

    unsigned int index_ext = index.idx_external;

    assert (!cache_->getMetaVar<unsigned int>(dbcontent_name_, DBContent::meta_var_ta_).isNull(index_ext));

    return cache_->getMetaVar<unsigned int>(dbcontent_name_, DBContent::meta_var_ta_).get(index_ext);
}

bool Chain::hasGroundBit(const DataID& id) const // only if set
{
    auto index = indexFromDataID(id);

    unsigned int index_ext = index.idx_external;

    return cache_->hasMetaVar<bool>(dbcontent_name_, DBContent::meta_var_ground_bit_)
            && !cache_->getMetaVar<bool>(dbcontent_name_, DBContent::meta_var_ground_bit_).isNull(index_ext);
}

std::pair<bool,bool> Chain::groundBit(const DataID& id) const // has gbs, gbs true
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

bool Chain::hasTstTrackNum(const DataID& id) const
{
    auto index = indexFromDataID(id);

    auto index_ext = index.idx_external;

    return cache_->hasMetaVar<unsigned int>(dbcontent_name_, DBContent::meta_var_track_num_)
            && !cache_->getMetaVar<unsigned int>(dbcontent_name_, DBContent::meta_var_track_num_).isNull(index_ext);
}

unsigned int Chain::tstTrackNum(const DataID& id) const
{
    assert (hasTstTrackNum(id));

    auto index = indexFromDataID(id);

    auto index_ext = index.idx_external;

    return cache_->getMetaVar<unsigned int>(dbcontent_name_, DBContent::meta_var_track_num_).get(index_ext);
}

bool Chain::hasTstMeasuredSpeed(const DataID& id) const
{
    auto index = indexFromDataID(id);

    auto index_ext = index.idx_external;

    return cache_->hasMetaVar<double>(dbcontent_name_, DBContent::meta_var_ground_speed_)
            && !cache_->getMetaVar<double>(dbcontent_name_, DBContent::meta_var_ground_speed_).isNull(index_ext);
}

float Chain::tstMeasuredSpeed(const DataID& id) const // m/s
{
    assert (hasTstMeasuredSpeed(id));

    auto index = indexFromDataID(id);

    auto index_ext = index.idx_external;

    return cache_->getMetaVar<double>(dbcontent_name_, DBContent::meta_var_ground_speed_).get(index_ext) * KNOTS2M_S;
}

bool Chain::hasTstMeasuredTrackAngle(const DataID& id) const
{
    auto index = indexFromDataID(id);

    auto index_ext = index.idx_external;

    return cache_->hasMetaVar<double>(dbcontent_name_, DBContent::meta_var_track_angle_)
            && !cache_->getMetaVar<double>(dbcontent_name_, DBContent::meta_var_track_angle_).isNull(index_ext);

}

float Chain::tstMeasuredTrackAngle(const DataID& id) const // deg
{
    assert (hasTstMeasuredTrackAngle(id));

    auto index = indexFromDataID(id);

    auto index_ext = index.idx_external;

    return cache_->getMetaVar<double>(dbcontent_name_, DBContent::meta_var_track_angle_).get(index_ext);
}

std::pair<bool, float> Chain::estimateAltitude (const boost::posix_time::ptime& timestamp,
                                                unsigned int index_internal) const
{
    assert(index_internal < indexes_.size());

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
    auto prev_it  = indexes_.begin() + index_internal;
    auto after_it = prev_it;

    const time_duration max_tdiff = seconds(120);

    while (prev_it != indexes_.end() && timestamp - ts_vec.get(*prev_it) < max_tdiff)
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

        if (prev_it == indexes_.begin()) // undefined decrement
            break;

        --prev_it;
    }

    // search after index
    ptime timestamp_after;

    while (after_it != indexes_.end() && ts_vec.get(*after_it) - timestamp < max_tdiff)
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

DataMapping Chain::calculateDataMapping(ptime timestamp) const
{
    DataMapping ret;

    ret.timestamp_ = timestamp;

    //    Return iterator to lower bound
    //    Returns an iterator pointing to the first element in the container whose key is not considered to go
    //    before k (i.e., either it is equivalent or goes after).

    auto lb_it = timestamp_index_lookup_.lower_bound(timestamp);

    //auto ub_it = ref_chain_->upper_bound(tod);

    if (lb_it != timestamp_index_lookup_.end()) // upper tod found
    {
        assert (lb_it->first >= timestamp);

        // save upper value
        ret.has_ref2_ = true;
        ret.timestamp_ref2_ = lb_it->first;
        ret.dataid_ref2_ = dataID(lb_it->first);

        // search lower values by decrementing iterator
        while (lb_it != timestamp_index_lookup_.end()
               && (timestamp < lb_it->first || lb_it->first == ret.timestamp_ref2_))
        {
            if (lb_it == timestamp_index_lookup_.begin()) // exit condition on first value
            {
                if (timestamp < lb_it->first) // set as not found
                    lb_it = timestamp_index_lookup_.end();

                break;
            }

            lb_it--;
        }

        if (lb_it != timestamp_index_lookup_.end() && lb_it->first != ret.timestamp_ref2_) // lower tod found
        {
            assert (timestamp >= lb_it->first);

            // add lower value
            ret.has_ref1_ = true;
            ret.timestamp_ref1_ = lb_it->first;
            ret.dataid_ref1_ = dataID(lb_it->first);
        }
        else // not found, clear previous
        {
            ret.has_ref2_ = false;
            ret.timestamp_ref2_ = {};
            ret.dataid_ref1_ = {};
        }
    }

    addPositionsSpeedsToMapping(ret);

    return ret;
}

void Chain::addPositionsSpeedsToMapping (DataMapping& mapping) const
{
    if (mapping.has_ref1_ && hasPos(mapping.timestamp_ref1_)
            && mapping.has_ref2_ && hasPos(mapping.timestamp_ref2_)) // two positions which can be interpolated
    {
        ptime lower_ts = mapping.timestamp_ref1_;
        ptime upper_ts = mapping.timestamp_ref2_;

        dbContent::TargetPosition pos1 = pos(mapping.dataid_ref1_);
        dbContent::TargetPosition pos2 = pos(mapping.dataid_ref2_);
        float d_t = Time::partialSeconds(upper_ts - lower_ts);

        dbContent::TargetVelocity spd1;
        dbContent::TargetVelocity spd2;

        double acceleration_ms2;
        double speed_ms, angle_deg;

        logdbg << "Chain: addPositionsSpeedsToMapping: d_t " << d_t;

        assert (d_t > 0);

        if (pos1.latitude_ == pos2.latitude_ && pos1.longitude_ == pos2.longitude_) // same pos
        {
            mapping.has_ref_pos_ = true;
            mapping.pos_ref_ = pos1;

            mapping.spd_ref_.track_angle_ = NAN;
            mapping.spd_ref_.speed_       = NAN;
        }
        else
        {
            if (lower_ts == upper_ts) // same time
            {
                logwrn << "Chain: addPositionsSpeedsToMapping: ref has same time twice";
            }
            else
            {
                logdbg << "Chain: addPositionsSpeedsToMapping: pos1 "
                       << pos1.latitude_ << ", " << pos1.longitude_;
                logdbg << "Chain: addPositionsSpeedsToMapping: pos2 "
                       << pos2.latitude_ << ", " << pos2.longitude_;

                bool ok;
                double x_pos, y_pos;

                tie(ok, x_pos, y_pos) = trafo_.distanceCart(
                            pos1.latitude_, pos1.longitude_, pos2.latitude_, pos2.longitude_);

                //                logdbg << "Chain: addRefPositiosToMapping: geo2cart";
                //                bool ret = ogr_geo2cart->Transform(1, &x_pos, &y_pos); // wgs84 to cartesian offsets
                if (!ok)
                {
                    logerr << "Chain: addPositionsSpeedsToMapping: error with latitude " << pos2.latitude_
                           << " longitude " << pos2.longitude_;
                }
                else // calculate interpolated position
                {
                    logdbg << "Chain: addPositionsSpeedsToMapping: offsets x " << fixed << x_pos
                           << " y " << fixed << y_pos << " dist " << fixed << sqrt(pow(x_pos,2)+pow(y_pos,2));

                    double x_pos_orig = x_pos;
                    double y_pos_orig = y_pos;

                    double v_x = x_pos/d_t;
                    double v_y = y_pos/d_t;
                    logdbg << "Chain: addPositionsSpeedsToMapping: v_x " << v_x << " v_y " << v_y;

                    float d_t2 = Time::partialSeconds(mapping.timestamp_ - lower_ts);
                    logdbg << "Chain: addPositionsSpeedsToMapping: d_t2 " << d_t2;

                    assert (d_t2 >= 0);

                    x_pos = v_x * d_t2;
                    y_pos = v_y * d_t2;

                    logdbg << "Chain: addPositionsSpeedsToMapping: interpolated offsets x "
                           << x_pos << " y " << y_pos;

                    tie (ok, x_pos, y_pos) = trafo_.wgsAddCartOffset(pos1.latitude_, pos1.longitude_, x_pos, y_pos);

                    // x_pos long, y_pos lat

                    logdbg << "Chain: addPositionsSpeedsToMapping: interpolated lat "
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

                    logdbg << "Chain: addPositionsSpeedsToMapping: pos1 has alt "
                           << pos1.has_altitude_ << " alt " << pos1.altitude_
                           << " pos2 has alt " << pos2.has_altitude_ << " alt " << pos2.altitude_
                           << " interpolated has alt " << has_altitude << " alt " << altitude;

                    mapping.has_ref_pos_ = true;

                    mapping.pos_ref_ = dbContent::TargetPosition(x_pos, y_pos, has_altitude, true, altitude);

                    // calulcate interpolated speed / track angle

                    mapping.has_ref_spd_ = false;

                    if (hasSpeed(mapping.dataid_ref1_) && hasSpeed(mapping.dataid_ref2_))
                    {
                        spd1 = speed(mapping.dataid_ref1_);
                        spd2 = speed(mapping.dataid_ref2_);

                        acceleration_ms2 = (spd2.speed_ - spd1.speed_)/d_t;
                        speed_ms = spd1.speed_ + acceleration_ms2 * d_t2;

                        //loginf << "UGA spd1 " << spd1.speed_ << " 2 " << spd2.speed_ << " ipld " << speed;

#if 0
                        double angle_diff = Number::calculateMinAngleDifference(spd2.track_angle_, spd1.track_angle_);
                        double turnrate   = angle_diff / d_t;

                        angle = spd1.track_angle_ + turnrate * d_t2;
#else
                        angle_deg = Number::interpolateBearing(0, 0, x_pos_orig, y_pos_orig, spd1.track_angle_, spd2.track_angle_, d_t2 / d_t);
#endif

                        //                        loginf << "UGA ang1 " << spd1.track_angle_ << " 2 " << spd2.track_angle_
                        //                               << " angle_diff " << angle_diff << " turnrate " << turnrate << " ipld " << angle;

                        mapping.has_ref_spd_          = true;
                        mapping.spd_ref_.speed_       = speed_ms;
                        mapping.spd_ref_.track_angle_ = angle_deg;
                    }
                }
            }
        }
    }
    // else do nothing
}

DataMappingTimes Chain::findDataMappingTimes(ptime timestamp_ref) const // ref tod
{
    DataMappingTimes ret;

    ret.timestamp_ = timestamp_ref;

    //    Return iterator to lower bound
    //    Returns an iterator pointing to the first element in the container whose key is not considered to go
    //    before k (i.e., either it is equivalent or goes after).

    auto lb_it = timestamp_index_lookup_.lower_bound(timestamp_ref);

    if (lb_it != timestamp_index_lookup_.end()) // upper tod found
    {
        assert (lb_it->first >= timestamp_ref);

        // save upper value
        ret.has_other2_ = true;
        ret.timestamp_other2_ = lb_it->first;
        ret.dataid_other2_ = dataID(lb_it->first);

        // search lower values by decrementing iterator
        while (lb_it != timestamp_index_lookup_.end() && (timestamp_ref < lb_it->first || lb_it->first == ret.timestamp_other2_))
        {
            if (lb_it == timestamp_index_lookup_.begin()) // exit condition on first value
            {
                if (timestamp_ref < lb_it->first) // set as not found
                    lb_it = timestamp_index_lookup_.end();

                break;
            }

            lb_it--;
        }

        if (lb_it != timestamp_index_lookup_.end() && lb_it->first != ret.timestamp_other2_) // lower tod found
        {
            assert (timestamp_ref >= lb_it->first);

            // add lower value
            ret.has_other1_ = true;
            ret.timestamp_other1_ = lb_it->first;
            ret.dataid_other1_ = dataID(lb_it->first);
        }
        else // not found, clear previous
        {
            ret.has_other2_ = false;
            ret.timestamp_other2_ = {};
            ret.dataid_other2_ = {};
        }
    }

    return ret;
}

std::set<string> Chain::acids() const
{
    return acids_;
}

string Chain::acidsStr() const
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

std::set<unsigned int> Chain::acads() const
{
    return acads_;
}

std::string Chain::acadsStr() const
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

void Chain::updateCallsigns() const
{
    acids_.clear();

    if (timestamp_index_lookup_.size())
    {
        NullableVector<string>& value_vec = cache_->getMetaVar<string>(dbcontent_name_, DBContent::meta_var_ti_);
        map<string, vector<unsigned int>> distinct_values = value_vec.distinctValuesWithIndexes(indexes_);

        for (auto& val_it : distinct_values)
        {
            if (!acids_.count(val_it.first))
                acids_.insert(val_it.first);
        }
    }
}

void Chain::updateTargetAddresses() const
{
    acads_.clear();

    if (timestamp_index_lookup_.size())
    {
        NullableVector<unsigned int>& value_vec = cache_->getMetaVar<unsigned int>(
                    dbcontent_name_, DBContent::meta_var_ta_);
        map<unsigned int, vector<unsigned int>> distinct_values = value_vec.distinctValuesWithIndexes(indexes_);

        for (auto& val_it : distinct_values)
        {
            if (!acads_.count(val_it.first))
                acads_.insert(val_it.first);
        }
    }

}

void Chain::updateModeACodes() const
{
    logdbg << "Chain: updateModeACodes";

    mode_a_codes_.clear();

    if (timestamp_index_lookup_.size())
    {
        NullableVector<unsigned int>& mode_a_codes = cache_->getMetaVar<unsigned int>(
                    dbcontent_name_, DBContent::meta_var_m3a_);
        map<unsigned int, vector<unsigned int>> distinct_codes = mode_a_codes.distinctValuesWithIndexes(indexes_);
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

    if (timestamp_index_lookup_.size())
    {
        NullableVector<float>& modec_codes_ft = cache_->getMetaVar<float>(dbcontent_name_, DBContent::meta_var_mc_);

        NullableVector<float>* altitude_trusted_vec {nullptr};

        if (dbcontent_name_ == "CAT062")
            altitude_trusted_vec = &cache_->getVar<float>(dbcontent_name_, DBContent::var_cat062_fl_measured_);

        for (auto ind_it : indexes_)
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

    if (timestamp_index_lookup_.size())
    {
        NullableVector<double>& lats = cache_->getMetaVar<double>(dbcontent_name_, DBContent::meta_var_latitude_);
        NullableVector<double>& longs = cache_->getMetaVar<double>(dbcontent_name_, DBContent::meta_var_longitude_);

        for (auto ind_it : indexes_)
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
