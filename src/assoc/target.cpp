#include "assoc/target.h"
#include "assoc/targetreport.h"
#include "logger.h"
#include "stringconv.h"
#include "util/timeconv.h"
#include "global.h"

#include <cassert>
#include <sstream>

//#include <ogr_spatialref.h>

using namespace std;
using namespace Utils;
using namespace boost::posix_time;

namespace Association
{
//    double Target::max_time_diff_ {15.0};
//    double Target::max_altitude_diff_ {300.0};

Target::Target(unsigned int utn, bool tmp)
    : utn_(utn), tmp_(tmp)
{
}

Target::~Target()
{
    if (!tmp_)
    {
        for (auto& tr_it : assoc_trs_)
            tr_it->removeAssociated(this);
    }
}

void Target::addAssociated (TargetReport* tr)
{
    assert (tr);

    // update min/max
    if (!assoc_trs_.size())
    {
        timestamp_min_ = tr->timestamp_;
        timestamp_max_ = tr->timestamp_;
    }
    else
    {
        timestamp_min_ = min(timestamp_min_, tr->timestamp_);
        timestamp_max_ = max(timestamp_max_, tr->timestamp_);
    }
    has_timestamps_ = true;


    if (tr->has_mc_)
    {
        if (has_mode_c_)
        {
            mode_c_min_ = min(mode_c_min_, tr->mc_);
            mode_c_max_ = max(mode_c_max_, tr->mc_);
        }
        else
        {
            mode_c_min_ = tr->mc_;
            mode_c_max_ = tr->mc_;
            has_mode_c_ = true;
        }
    }

    if (!ds_ids_.count(tr->ds_id_))
        ds_ids_.insert(tr->ds_id_);

    if (tr->has_tn_ && !track_nums_.count({tr->ds_id_, tr->tn_}))
        track_nums_.insert({tr->ds_id_, tr->tn_});

    if (tr->has_ma_ && !mas_.count(tr->ma_))
        mas_.insert(tr->ma_);

    timed_indexes_[tr->timestamp_] = assoc_trs_.size();
    assoc_trs_.push_back(tr);

    if (tr->has_ta_)
    {
        if (tas_.size() && !tas_.count(tr->ta_))
        {
            logwrn << "Target: addAssociated: ta mismatch, target " << asStr()
                   << " tr " << tr->asStr();
        }

        if (!tas_.count(tr->ta_))
            tas_.insert(tr->ta_);
    }

    if (tr->has_ti_)
    {
        if (!ids_.count(tr->ti_))
            ids_.insert(tr->ti_);
    }

    if (tr->has_adsb_info_ && tr->has_mops_version_)
    {
        if (!mops_versions_.count(tr->mops_version_))
            mops_versions_.insert(tr->mops_version_);
    }

    if (!tmp_)
        tr->addAssociated(this);
}

void Target::addAssociated (vector<TargetReport*> trs)
{
    for (auto& tr_it : trs)
        addAssociated(tr_it);
}

unsigned int Target::numAssociated() const
{
    return assoc_trs_.size();
}

const TargetReport& Target::lastAssociated() const
{
    assert (assoc_trs_.size());
    return *assoc_trs_.back();
}

bool Target::hasTA () const
{
    return tas_.size();
}

bool Target::hasTA (unsigned int ta) const
{
    return tas_.count(ta);
}

bool Target::hasAllOfTAs (std::set<unsigned int> tas) const
{
    assert (hasTA() && tas.size());

    for (auto other_ta : tas)
    {
        if (!tas_.count(other_ta))
            return false;
    }

    return true;
}

bool Target::hasAnyOfTAs (std::set<unsigned int> tas) const
{
    assert (hasTA() && tas.size());

    for (auto other_ta : tas)
    {
        if (tas_.count(other_ta))
            return true;
    }
    return false;
}

bool Target::hasMA () const
{
    return mas_.size();
}

bool Target::hasMA (unsigned int ma)  const
{
    return mas_.count(ma);
}

std::string Target::asStr() const
{
    stringstream ss;

    ss << "utn " << utn_ << " tmp " << tmp_;

    if (tas_.size())
    {
        ss << " tas '";

        bool first {true};
        for (auto ta_it : tas_)
        {
            if (first)
                ss << String::hexStringFromInt(ta_it, 6, '0');
            else
                ss << ", " << String::hexStringFromInt(ta_it, 6, '0');

            first = false;
        }

        ss << "'";
    }

    if (track_nums_.size())
    {
        ss << " tns ";

        bool first {true};
        for (auto tn_it : track_nums_)
        {
            if (first)
                ss << "(" << tn_it.first << "," << tn_it.second << ")";
            else
                ss << ", " << "(" << tn_it.first << "," << tn_it.second << ")";

            first = false;
        }
    }

    return ss.str();
}

std::string Target::timeStr() const
{
    return "["+Time::toString(timestamp_min_)+" - "+Time::toString(timestamp_max_)+"]";
}

bool Target::isTimeInside (ptime timestamp) const
{
    if (!has_timestamps_)
        return false;

    return timestamp >= timestamp_min_ && timestamp <= timestamp_max_;
}

bool Target::hasDataForTime (ptime timestamp, time_duration d_max) const
{
    if (!isTimeInside(timestamp))
        return false;

    if (timed_indexes_.count(timestamp))
        return true; // contains exact value

    //    Return iterator to lower bound
    //    Returns an iterator pointing to the first element in the container whose key is not considered to go
    //    before k (i.e., either it is equivalent or goes after).

    auto lb_it = timed_indexes_.lower_bound(timestamp);

    if (lb_it == timed_indexes_.end())
        return false;

    assert (lb_it->first >= timestamp);

    if (lb_it->first - timestamp > d_max)
        return false; // too much time difference

    // save value
    ptime upper = lb_it->first;

    // TODO lb_it--; ?
    while (lb_it != timed_indexes_.end() && timestamp < lb_it->first)
    {
        if (lb_it == timed_indexes_.begin()) // exit condition on first value
        {
            if (timestamp < lb_it->first) // set as not found
                lb_it = timed_indexes_.end();

            break;
        }

        lb_it--;
    }

    if (lb_it == timed_indexes_.end())
        return false;

    assert (timestamp >= lb_it->first);

    if (timestamp - lb_it->first > d_max)
        return false; // too much time difference

    ptime lower = lb_it->first;

    logdbg << "Target " << utn_ << ": hasDataForTime: found " << Time::toString(lower)
           << " <= " << Time::toString(timestamp)
           << " <= " << Time::toString(upper);

    return true;
}

std::pair<ptime, ptime> Target::timesFor (
        ptime timestamp, time_duration  d_max) const
// lower/upper times, invalid ts if not existing
{
    if (timed_indexes_.count(timestamp))
        return {timestamp, {}}; // contains exact value

    //    Return iterator to lower bound
    //    Returns an iterator pointing to the first element in the container whose key is not considered to go
    //    before k (i.e., either it is equivalent or goes after).

    auto lb_it = timed_indexes_.lower_bound(timestamp);

    if (lb_it == timed_indexes_.end())
        return {{}, {}};

    assert (lb_it->first >= timestamp);

    if (lb_it->first - timestamp > d_max)
        return {{}, {}}; // too much time difference

    // save value
    ptime upper = lb_it->first;

    // TODO lb_it--; ?
    while (lb_it != timed_indexes_.end() && timestamp < lb_it->first)
    {
        if (lb_it == timed_indexes_.begin()) // exit condition on first value
        {
            if (timestamp < lb_it->first) // set as not found
                lb_it = timed_indexes_.end();

            break;
        }

        lb_it--;
    }

    if (lb_it == timed_indexes_.end())
        return {{}, upper};

    assert (timestamp >= lb_it->first);

    if (timestamp - lb_it->first > d_max)
        return {{}, upper}; // too much time difference

    ptime lower = lb_it->first;

    return {lower, upper};
}

std::pair<dbContent::TargetPosition, bool> Target::interpolatedPosForTime (ptime timestamp, time_duration d_max) const
{
    ptime lower, upper;

    tie(lower, upper) = timesFor(timestamp, d_max);

    if (!lower.is_not_a_date_time() && upper.is_not_a_date_time()) // exact time
        return {posForExactTime(lower), true};

    if (lower.is_not_a_date_time())
        return {{}, false};

    dbContent::TargetPosition pos1 = posForExactTime(lower);
    dbContent::TargetPosition pos2 = posForExactTime(upper);
    float d_t = Time::partialSeconds(upper - lower);

    logdbg << "Target: interpolatedPosForTime: d_t " << d_t;

    assert (d_t >= 0);

    if (pos1.latitude_ == pos2.latitude_
            && pos1.longitude_ == pos2.longitude_) // same pos
        return {pos1, true};

    if (lower == upper) // same time
    {
        logwrn << "Target: interpolatedPosForTime: ref has same time twice";
        return {{}, false};
    }

    logdbg << "Target: interpolatedPosForTime: pos1 " << pos1.latitude_ << ", " << pos1.longitude_;
    logdbg << "Target: interpolatedPosForTime: pos2 " << pos2.latitude_ << ", " << pos2.longitude_;

    bool ok;
    double x_pos, y_pos;

    logdbg << "Target: interpolatedPosForTime: geo2cart";

    tie(ok, x_pos, y_pos) = trafo_.distanceCart(
                pos1.latitude_, pos1.longitude_, pos2.latitude_, pos2.longitude_);

    if (!ok)
    {
        logerr << "Target: interpolatedPosForTime: error with latitude " << pos2.latitude_
               << " longitude " << pos2.longitude_;
        return {{}, false};
    }

    logdbg << "Target: interpolatedPosForTime: offsets x " << fixed << x_pos
           << " y " << fixed << y_pos << " dist " << fixed << sqrt(pow(x_pos,2)+pow(y_pos,2));

    double v_x = x_pos/d_t;
    double v_y = y_pos/d_t;
    logdbg << "Target: interpolatedPosForTime: v_x " << v_x << " v_y " << v_y;

    float d_t2 = Time::partialSeconds(timestamp - lower);
    logdbg << "Target: interpolatedPosForTime: d_t2 " << d_t2;

    assert (d_t2 >= 0);

    x_pos = v_x * d_t2;
    y_pos = v_y * d_t2;

    logdbg << "Target: interpolatedPosForTime: interpolated offsets x " << x_pos << " y " << y_pos;

    tie (ok, x_pos, y_pos) = trafo_.wgsAddCartOffset(pos1.latitude_, pos1.longitude_, x_pos, y_pos);

    //ret = ogr_cart2geo->Transform(1, &x_pos, &y_pos);

    // x_pos long, y_pos lat

    logdbg << "Target: interpolatedPosForTime: interpolated lat " << x_pos << " long " << y_pos;

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

    logdbg << "Target: interpolatedPosForTime: pos1 has alt "
           << pos1.has_altitude_ << " alt " << pos1.altitude_
           << " pos2 has alt " << pos2.has_altitude_ << " alt " << pos2.altitude_
           << " interpolated has alt " << has_altitude << " alt " << altitude;

    //        if (in_appimage_) // inside appimage
    //            return {{y_pos, x_pos, has_altitude, true, altitude}, true};
    //        else
    return {{x_pos, y_pos, has_altitude, true, altitude}, true};
}

std::pair<dbContent::TargetPosition, bool> Target::interpolatedPosForTimeFast (
        ptime timestamp, time_duration d_max) const
{
    ptime lower, upper;

    tie(lower, upper) = timesFor(timestamp, d_max);

    if (!lower.is_not_a_date_time() && upper.is_not_a_date_time()) // exact time
        return {posForExactTime(lower), true};

    if (lower.is_not_a_date_time())
        return {{}, false};

    dbContent::TargetPosition pos1 = posForExactTime(lower);
    dbContent::TargetPosition pos2 = posForExactTime(upper);
    float d_t = Time::partialSeconds(upper - lower);

    logdbg << "Target: interpolatedPosForTimeFast: d_t " << d_t;

    assert (d_t >= 0);

    if (pos1.latitude_ == pos2.latitude_
            && pos1.longitude_ == pos2.longitude_) // same pos
        return {pos1, true};

    if (lower == upper) // same time
    {
        logwrn << "Target: interpolatedPosForTimeFast: ref has same time twice";
        return {{}, false};
    }

    double v_lat = (pos2.latitude_ - pos1.latitude_)/d_t;
    double v_long = (pos2.longitude_ - pos1.longitude_)/d_t;
    logdbg << "Target: interpolatedPosForTimeFast: v_x " << v_lat << " v_y " << v_long;

    float d_t2 = Time::partialSeconds(timestamp - lower);
    logdbg << "Target: interpolatedPosForTimeFast: d_t2 " << d_t2;

    assert (d_t2 >= 0);

    double int_lat = pos1.latitude_ + v_lat * d_t2;
    double int_long = pos1.longitude_ + v_long * d_t2;

    logdbg << "Target: interpolatedPosForTimeFast: interpolated lat " << int_lat << " long " << int_long;

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

    logdbg << "Target: interpolatedPosForTimeFast: pos1 has alt "
           << pos1.has_altitude_ << " alt " << pos1.altitude_
           << " pos2 has alt " << pos2.has_altitude_ << " alt " << pos2.altitude_
           << " interpolated has alt " << has_altitude << " alt " << altitude;

    return {{int_lat, int_long, has_altitude, true, altitude}, true};
}

bool Target::hasDataForExactTime (ptime timestamp) const
{
    return timed_indexes_.count(timestamp);
}

TargetReport& Target::dataForExactTime (ptime timestamp) const
{
    assert (hasDataForExactTime(timestamp));;
    assert (assoc_trs_.size() > timed_indexes_.at(timestamp));
    return *assoc_trs_.at(timed_indexes_.at(timestamp));
}

dbContent::TargetPosition Target::posForExactTime (ptime timestamp) const
{
    assert (hasDataForExactTime(timestamp));

    TargetReport& tr = dataForExactTime(timestamp);

    dbContent::TargetPosition pos;

    pos.latitude_ = tr.latitude_;
    pos.longitude_ = tr.longitude_;
    pos.has_altitude_ = tr.has_mc_;
    pos.altitude_ = tr.mc_;

    return pos;
}

float Target::duration () const
{
    if (!has_timestamps_)
        return 0;

    return Time::partialSeconds(timestamp_max_ - timestamp_min_);
}

bool Target::timeOverlaps (const Target& other) const
{
    if (!assoc_trs_.size() || !other.assoc_trs_.size())
        return false;

    assert (has_timestamps_);

    //a.start < b.end && b.start < a.end;
    return timestamp_min_ < other.timestamp_max_ && other.timestamp_min_ < timestamp_max_;
}

float Target::probTimeOverlaps (const Target& other) const
{
    if (!has_timestamps_)
        return 0.0;

    ptime overlap_begin = max(timestamp_min_, other.timestamp_min_);
    ptime overlap_end = min(timestamp_max_, other.timestamp_max_);

    if (overlap_begin >= overlap_end)
        return 0.0;

    float overlap_duration = Time::partialSeconds(overlap_end - overlap_begin);

    float targets_min_duration = min(duration(), other.duration());

    assert (overlap_duration <= targets_min_duration);

    if (targets_min_duration == 0)
        return 0.0;

    return overlap_duration / targets_min_duration;
}

std::tuple<vector<ptime>, vector<ptime>, vector<ptime>> Target::compareModeACodes (
        const Target& other, boost::posix_time::time_duration max_time_diff) const
{
    vector<ptime> unknown;
    vector<ptime> same;
    vector<ptime> different;

    ptime timestamp;
    CompareResult cmp_res;

    for (auto tr_it : assoc_trs_)
    {
        timestamp = tr_it->timestamp_;

        assert (hasDataForExactTime(timestamp));

        cmp_res = other.compareModeACode(tr_it->has_ma_, tr_it->ma_, timestamp, max_time_diff);

        if (cmp_res == CompareResult::UNKNOWN)
            unknown.push_back(timestamp);
        else if (cmp_res == CompareResult::SAME)
            same.push_back(timestamp);
        else if (cmp_res == CompareResult::DIFFERENT)
            different.push_back(timestamp);
    }

    assert (assoc_trs_.size() == unknown.size()+same.size()+different.size());

    return std::tuple<vector<ptime>, vector<ptime>, vector<ptime>>(unknown, same, different);
}

CompareResult Target::compareModeACode (
        bool has_ma, unsigned int ma, ptime timestamp, time_duration max_time_diff) const
{
    if (!hasDataForTime(timestamp, max_time_diff))
        return CompareResult::UNKNOWN;

    ptime lower, upper;

    tie(lower, upper) = timesFor(timestamp, max_time_diff);

    if (lower.is_not_a_date_time() && upper.is_not_a_date_time())
        return CompareResult::UNKNOWN;

    if ((lower.is_not_a_date_time() && !upper.is_not_a_date_time())
            || (!lower.is_not_a_date_time() && upper.is_not_a_date_time())) // only 1
    {
        TargetReport& ref1 = (!lower.is_not_a_date_time()) ? dataForExactTime(lower)
                                           : dataForExactTime(upper);

        if (!has_ma)
        {
            if (!ref1.has_ma_ ) // both have no mode a
                return CompareResult::SAME;
            else
                return CompareResult::DIFFERENT;
        }

        // mode a exists
        if (!ref1.has_ma_)
            return CompareResult::DIFFERENT;  // mode a here, but none in other

        if ((ref1.has_ma_ && ref1.ma_ == ma)) // is same
            return CompareResult::SAME;
        else
            return CompareResult::DIFFERENT;
    }

    // both set
    assert (!lower.is_not_a_date_time());
    assert (hasDataForExactTime(lower));
    TargetReport& ref1 = dataForExactTime(lower);

    assert (!upper.is_not_a_date_time());
    assert (hasDataForExactTime(upper));
    TargetReport& ref2 = dataForExactTime(upper);

    if (!has_ma)
    {
        if (!ref1.has_ma_ || !ref2.has_ma_) // both have no mode a
            return CompareResult::SAME;
        else
            return CompareResult::DIFFERENT; // no mode a here, but in other
    }

    // mode a exists
    if (!ref1.has_ma_ && !ref2.has_ma_)
        return CompareResult::DIFFERENT; // mode a here, but none in other

    if ((ref1.has_ma_ && ref1.ma_ == ma)
            || (ref2.has_ma_ && ref2.ma_ == ma)) // one of them is same
    {
        return CompareResult::SAME;
    }
    else
        return CompareResult::DIFFERENT;
}

std::tuple<vector<ptime>, vector<ptime>, vector<ptime>> Target::compareModeCCodes (
        const Target& other, const std::vector<ptime>& timestamps,
        time_duration max_time_diff, float max_alt_diff, bool debug) const
{
    vector<ptime> unknown;
    vector<ptime> same;
    vector<ptime> different;

    CompareResult cmp_res;

    TargetReport* tr;

    for (auto timestamp : timestamps)
    {
        assert (hasDataForExactTime(timestamp));
        tr = &dataForExactTime (timestamp);

        cmp_res = other.compareModeCCode(tr->has_mc_, tr->mc_, timestamp, max_time_diff, max_alt_diff, debug);

        if (debug)
            loginf << "tod " << Time::toString(timestamp) << " result " << (unsigned int) cmp_res;

        if (cmp_res == CompareResult::UNKNOWN)
            unknown.push_back(timestamp);
        else if (cmp_res == CompareResult::SAME)
            same.push_back(timestamp);
        else if (cmp_res == CompareResult::DIFFERENT)
            different.push_back(timestamp);
    }

    assert (timestamps.size() == unknown.size()+same.size()+different.size());

    return std::tuple<vector<ptime>, vector<ptime>, vector<ptime>>(unknown, same, different);
}

CompareResult Target::compareModeCCode (bool has_mc, float mc, ptime timestamp,
                                        time_duration max_time_diff, float max_alt_diff, bool debug) const
{
    if (!hasDataForTime(timestamp, max_time_diff))
        return CompareResult::UNKNOWN;

    ptime lower, upper;

    tie(lower, upper) = timesFor(timestamp, max_time_diff);

    if (lower.is_not_a_date_time() && upper.is_not_a_date_time())
    {
        if (debug)
            loginf << "Target: compareModeCCode: unknown, no times found";

        return CompareResult::UNKNOWN;
    }

    if ((lower.is_not_a_date_time() && !upper.is_not_a_date_time())
            || (!lower.is_not_a_date_time() && upper.is_not_a_date_time())) // only 1
    {
        if (debug)
            loginf << "Target: compareModeCCode: only 1";

        TargetReport& ref1 = (!lower.is_not_a_date_time()) ? dataForExactTime(lower)
                                           : dataForExactTime(upper);

        if (!has_mc)
        {
            if (!ref1.has_mc_ ) // both have no mode c
            {
                if (debug)
                    loginf << "Target: compareModeCCode: same, both have no mode c";
                return CompareResult::SAME;
            }
            else
            {
                if (debug)
                    loginf << "Target: compareModeCCode: different, no mode c but in ref1";
                return CompareResult::DIFFERENT;
            }
        }

        // mode c exists
        if (!ref1.has_mc_)
        {
            if (debug)
                loginf << "Target: compareModeCCode: different, mode c but not in ref1";
            return CompareResult::DIFFERENT;  // mode c here, but none in other
        }

        if ((ref1.has_mc_ && fabs(ref1.mc_ - mc) < max_alt_diff)) // is same
        {
            if (debug)
                loginf << "Target: compareModeCCode: same, diff check passed";
            return CompareResult::SAME;
        }
        else
        {
            if (debug)
                loginf << "Target: compareModeCCode: different, diff check failed";
            return CompareResult::DIFFERENT;
        }
    }

    // both set

    if (debug)
        loginf << "Target: compareModeCCode: both";

    assert (!lower.is_not_a_date_time());
    assert (hasDataForExactTime(lower));
    TargetReport& ref1 = dataForExactTime(lower);

    assert (!upper.is_not_a_date_time());
    assert (hasDataForExactTime(upper));
    TargetReport& ref2 = dataForExactTime(upper);

    if (!has_mc)
    {
        if (!ref1.has_mc_ || !ref2.has_mc_) // both have no mode c
        {
            if (debug)
                loginf << "Target: compareModeCCode: same, both have no mode c";
            return CompareResult::SAME;
        }
        else
        {
            if (debug)
                loginf << "Target: compareModeCCode: different, no mode c here, but in one of refs";
            return CompareResult::DIFFERENT; // no mode c here, but in other
        }
    }

    // mode a exists
    if (!ref1.has_mc_ && !ref2.has_mc_)
    {
        if (debug)
            loginf << "Target: compareModeCCode: different, mode c here, but none in refs";
        return CompareResult::DIFFERENT; // mode c here, but none in other
    }

    if ((ref1.has_mc_ && fabs(ref1.mc_ - mc) < max_alt_diff)
            || (ref2.has_mc_ && fabs(ref2.mc_ - mc) < max_alt_diff)) // one of them is same
    {
        if (debug)
            loginf << "Target: compareModeCCode: same, diff check passed";
        return CompareResult::SAME;
    }
    else
    {
        if (debug)
        {
            loginf << "Target: compareModeCCode: different, diff check failed";
            //                loginf << "\t mc " << mc;
            //                loginf << "\t ref1.has_mc_ " << ref1.has_mc_;
            //                loginf << "\t ref1.mc_ " << ref1.mc_;
            //                loginf << "\t fabs(ref1.mc_ - mc) " << fabs(ref1.mc_ - mc);
            //                loginf << "\t ref2.has_mc_ " << ref2.has_mc_;
            //                loginf << "\t ref2.mc_ " << ref2.mc_;
            //                loginf << "\t fabs(ref2.mc_ - mc) " << fabs(ref2.mc_ - mc);
            //                loginf << "\t max_alt_diff " << max_alt_diff;
        }
        return CompareResult::DIFFERENT;
    }
}

void Target::calculateSpeeds()
{
    has_speed_ = false;

    ptime timestamp;
    double latitude {0};
    double longitude {0};
    TargetReport* tr;

    ptime timestamp_prev;
    double latitude_prev {0};
    double longitude_prev {0};

    //        OGRSpatialReference wgs84;
    //        wgs84.SetWellKnownGeogCS("WGS84");
    //        OGRSpatialReference local;

    Transformation trafo;

    double x_pos, y_pos;
    bool ok;

    float d_t;
    double v_x, v_y;
    double spd;
    double spd_sum {0};
    unsigned int num_spd {0};

    bool first = true;
    for (auto& time_it : timed_indexes_)
    {
        timestamp_prev = timestamp;
        latitude_prev = latitude;
        longitude_prev = longitude;

        timestamp = time_it.first;

        assert (time_it.second < assoc_trs_.size());
        tr = assoc_trs_.at(time_it.second);

        latitude = tr->latitude_;
        longitude = tr->longitude_;

        if (first)
        {
            first = false;
            continue;
        }

        d_t = Time::partialSeconds(timestamp - timestamp_prev);
        assert (d_t >= 0);

        tie(ok, x_pos, y_pos) = trafo.distanceCart(
                    latitude, longitude, latitude_prev, longitude_prev);

        if (!ok)
            continue;

        v_x = x_pos/d_t;
        v_y = y_pos/d_t;

        spd = sqrt(pow(v_x,2)+pow(v_y,2)) * M_S2KNOTS;

        if (!has_speed_)
        {
            has_speed_ = true;

            speed_min_ = spd;
            speed_max_ = spd;
        }
        else
        {
            speed_min_ = min(speed_min_, spd);
            speed_max_ = max(speed_max_, spd);
        }

        spd_sum += spd;
        ++num_spd;
    }

    if (num_spd)
    {
        speed_avg_ = spd_sum/(float)num_spd;
    }
}

void Target::removeNonModeSTRs()
{
    vector<TargetReport*> tmp_trs = assoc_trs_;

    if (!tmp_)
    {
        loginf << "Target: removeNonModeSTRs: " << asStr();

        for (auto tr_it : tmp_trs)
            tr_it->removeAssociated(this);
    }

    assoc_trs_.clear();

    tas_.clear();
    ids_.clear();
    mas_.clear();
    mops_versions_.clear();
    has_timestamps_ = false;
    has_speed_ = false;
    timed_indexes_.clear();
    ds_ids_.clear();
    track_nums_.clear();

    for (auto tr_it : tmp_trs)
    {
        if (tr_it->has_ta_)
            addAssociated(tr_it);
    }
}

std::map <std::string, unsigned int> Target::getDBContentCounts()
{
    std::map <std::string, unsigned int> counts;

    for (auto& tr_it : assoc_trs_)
    {
        counts[tr_it->dbcontent_name_] += 1;
    }

    return counts;
}

bool Target::hasADSBMOPSVersion()
{
    return mops_versions_.size();
}

std::set<unsigned int> Target::getADSBMOPSVersions()
{
    assert (hasADSBMOPSVersion());

    return mops_versions_;
}
}
