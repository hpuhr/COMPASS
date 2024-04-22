#include "reconstructortarget.h"
#include "reconstructorbase.h"
#include "compass.h"
#include "dbcontentmanager.h"
#include "dbcontent.h"
#include "dbcontent/variable/variable.h"
#include "buffer.h"
#include "util/number.h"
#include "util/timeconv.h"
#include "global.h"
#include "kalman_online_tracker.h"

#include <boost/optional/optional_io.hpp>

using namespace std;
using namespace Utils;
using namespace boost::posix_time;

namespace dbContent {

ReconstructorTarget::ReconstructorTarget(ReconstructorBase& reconstructor, unsigned int utn, bool tmp_utn)
    : reconstructor_(reconstructor), utn_(utn), tmp_utn_(tmp_utn)
{
}

ReconstructorTarget::~ReconstructorTarget()
{
}

void ReconstructorTarget::addTargetReport (unsigned long rec_num, bool add_to_tracker)
{
    assert (reconstructor_.target_reports_.count(rec_num));

    const dbContent::targetReport::ReconstructorInfo& tr = reconstructor_.target_reports_.at(rec_num);

//    if (!timestamp_max_.is_not_a_date_time()) // there is time
//    {
//        if (tr.timestamp_ < timestamp_max_)
//            logerr << "ReconstructorTarget: addTargetReport: old max " << Time::toString(timestamp_max_)
//                   << " tr ts " << Time::toString(tr.timestamp_);

//        assert (tr.timestamp_ >= timestamp_max_);
//    }

    bool ts_newer = true;

            // update min/max
    if (!target_reports_.size())
    {
        timestamp_min_ = tr.timestamp_;
        timestamp_max_ = tr.timestamp_;
    }
    else
    {
        ts_newer = tr.timestamp_ >= timestamp_max_;

        timestamp_min_ = min(timestamp_min_, tr.timestamp_);
        timestamp_max_ = max(timestamp_max_, tr.timestamp_);
    }

    if (tr.barometric_altitude_ && tr.barometric_altitude_->hasReliableValue())
    {
        if (mode_c_min_ && mode_c_max_)
        {
            mode_c_min_ = min(*mode_c_min_, tr.barometric_altitude_->altitude_);
            mode_c_max_ = max(*mode_c_max_, tr.barometric_altitude_->altitude_);
        }
        else
        {
            mode_c_min_ = tr.barometric_altitude_->altitude_;
            mode_c_max_ = tr.barometric_altitude_->altitude_;
        }
    }

    if (!ds_ids_.count(tr.ds_id_))
        ds_ids_.insert(tr.ds_id_);

            //    if (tr.track_number_ && !track_nums_.count({tr.ds_id_, *tr.track_number_}))
            //        track_nums_.insert({tr.ds_id_, *tr.track_number_});

    if (tr.mode_a_code_ && !mode_as_.count(tr.mode_a_code_->code_))
        mode_as_.insert(tr.mode_a_code_->code_);

    target_reports_.push_back(tr.record_num_);
    tr_timestamps_.insert({tr.timestamp_,tr.record_num_});
    // all sources sorted by time, ts -> record_num
    tr_ds_timestamps_[Number::recNumGetDBContId(tr.record_num_)][tr.ds_id_][tr.line_id_].insert(
        {tr.timestamp_, tr.record_num_});
    // dbcontent id -> ds_id -> ts -> record_num

    if (tr.acad_)
    {
        if (acads_.size() && !acads_.count(*tr.acad_))
        {
            logwrn << "Target: addAssociated: acad mismatch, target " << asStr() << " tr '" << tr.asStr() << "'";
        }

        if (!acads_.count(*tr.acad_))
            acads_.insert(*tr.acad_);
    }

    if (tr.acid_)
    {
        string acid = String::trim(*tr.acid_);

        if (!acids_.count(acid))
            acids_.insert(acid);
    }

            //    if (tr.has_adsb_info_ && tr.has_mops_version_)
            //    {
            //        if (!mops_versions_.count(tr.mops_version_))
            //            mops_versions_.insert(tr.mops_version_);
            //    }

            //    if (!tmp_)
            //        tr.addAssociated(this);

    if (add_to_tracker && !tr.do_not_use_position_)
    {
        if (!tracker_)
            reinitTracker();

        if (ts_newer)
            addToTracker(tr);
    }
}

void ReconstructorTarget::addTargetReports (std::vector<unsigned long> rec_nums, bool add_to_tracker)
{
    for (auto& rn_it : rec_nums)
        addTargetReport(rn_it, add_to_tracker);
}

unsigned int ReconstructorTarget::numAssociated() const
{
    return target_reports_.size();
}

unsigned long ReconstructorTarget::lastAssociated() const
{
    assert (target_reports_.size());
    return target_reports_.back();
}

bool ReconstructorTarget::hasACAD () const
{
    return acads_.size();
}

bool ReconstructorTarget::hasACAD (unsigned int ta) const
{
    return acads_.count(ta);
}

bool ReconstructorTarget::hasAllOfACADs (std::set<unsigned int> tas) const
{
    assert (hasACAD() && tas.size());

    for (auto other_ta : tas)
    {
        if (!acads_.count(other_ta))
            return false;
    }

    return true;
}

bool ReconstructorTarget::hasAnyOfACADs (std::set<unsigned int> tas) const
{
    assert (hasACAD() && tas.size());

    for (auto other_ta : tas)
    {
        if (acads_.count(other_ta))
            return true;
    }
    return false;
}

bool ReconstructorTarget::hasModeA () const
{
    return mode_as_.size();
}

bool ReconstructorTarget::hasModeA (unsigned int code)  const
{
    return mode_as_.count(code);
}

bool ReconstructorTarget::hasModeC () const
{
    return mode_c_min_ && mode_c_max_;
}

std::string ReconstructorTarget::asStr() const
{
    stringstream ss;

    ss << "utn " << utn_ << " tmp_utn " << tmp_utn_;  //<< " tmp " << tmp_

    if (acads_.size())
    {
        ss << " acads '";

        bool first {true};
        for (auto ta_it : acads_)
        {
            if (first)
                ss << String::hexStringFromInt(ta_it, 6, '0');
            else
                ss << ", " << String::hexStringFromInt(ta_it, 6, '0');

            first = false;
        }

        ss << "'";
    }

    if (acids_.size())
    {
        ss << " acids ";

        bool first {true};
        for (auto& it : acads_)
        {
            if (first)
                ss << "'" << it << "'";
            else
                ss << ", " << "'" << it << "'";

            first = false;
        }
    }

    if (mode_as_.size())
    {
        ss << " m3as '";

        bool first {true};
        for (auto it : acads_)
        {
            if (first)
                ss << String::octStringFromInt(it, 4, '0');
            else
                ss << ", " << String::octStringFromInt(it, 4, '0');

            first = false;
        }

        ss << "'";
    }


            //    if (track_nums_.size())
            //    {
            //        ss << " tns ";

            //        bool first {true};
            //        for (auto tn_it : track_nums_)
            //        {
            //            if (first)
            //                ss << "(" << tn_it.first << "," << tn_it.second << ")";
            //            else
            //                ss << ", " << "(" << tn_it.first << "," << tn_it.second << ")";

            //            first = false;
            //        }
            //    }

    return ss.str();
}

std::string ReconstructorTarget::timeStr() const
{
    if (timestamp_min_.is_not_a_date_time() || timestamp_min_.is_not_a_date_time())
        return "[]";

    return "["+Time::toString(timestamp_min_)+" - "+Time::toString(timestamp_max_)+"]";
}

bool ReconstructorTarget::hasTimestamps() const
{
    return !timestamp_min_.is_not_a_date_time() && !timestamp_min_.is_not_a_date_time();
}

bool ReconstructorTarget::isTimeInside (boost::posix_time::ptime timestamp) const
{
    if (timestamp_min_.is_not_a_date_time() || timestamp_min_.is_not_a_date_time())
        return false;

    return timestamp >= timestamp_min_ && timestamp <= timestamp_max_;
}

bool ReconstructorTarget::isTimeInside (boost::posix_time::ptime timestamp, boost::posix_time::time_duration d_max) const
{
    if (timestamp_min_.is_not_a_date_time() || timestamp_min_.is_not_a_date_time())
        return false;

    return timestamp >= timestamp_min_ - d_max && timestamp <= timestamp_max_ + d_max;
}

bool ReconstructorTarget::hasDataForTime (ptime timestamp, time_duration d_max) const
{
    if (!isTimeInside(timestamp))
        return false;

    if (tr_timestamps_.count(timestamp))
        return true; // contains exact value

            //    Return iterator to lower bound
            //    Returns an iterator pointing to the first element in the container whose key is not considered to go
            //    before k (i.e., either it is equivalent or goes after).

    auto lb_it = tr_timestamps_.lower_bound(timestamp);

    if (lb_it == tr_timestamps_.end())
        return false;

    assert (lb_it->first >= timestamp);

    if (lb_it->first - timestamp > d_max)
        return false; // too much time difference

            // save value
    ptime upper = lb_it->first;

            // TODO lb_it--; ?
    while (lb_it != tr_timestamps_.end() && timestamp < lb_it->first)
    {
        if (lb_it == tr_timestamps_.begin()) // exit condition on first value
        {
            if (timestamp < lb_it->first) // set as not found
                lb_it = tr_timestamps_.end();

            break;
        }

        lb_it--;
    }

    if (lb_it == tr_timestamps_.end())
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

ReconstructorTarget::ReconstructorInfoPair ReconstructorTarget::dataFor (ptime timestamp, 
                                                                         time_duration d_max,
                                                                         const InfoValidFunc& tr_valid_func) const
// lower/upper times, invalid ts if not existing
{
    std::multimap<boost::posix_time::ptime, unsigned long>::const_iterator it_lower, it_upper;
    bool has_lower = false;
    bool has_upper = false;

    auto it_available = tr_timestamps_.find(timestamp);
    if (it_available != tr_timestamps_.end())
    {
        //timestamp in map => start from timestamp
        it_lower  = it_available;
        it_upper  = it_available;
        has_lower = true;
        has_upper = true;
    }
    else
    {
        //get lower bound
        it_upper = tr_timestamps_.lower_bound(timestamp);

        //lower bound not found => stop
        if (it_upper == tr_timestamps_.end())
            return {nullptr, nullptr};

        assert (it_upper->first >= timestamp);

        //too much time difference?
        if (it_upper->first - timestamp <= d_max)
            has_upper = true;

        //our upper element is the first element => no lower element
        if (it_upper != tr_timestamps_.begin())
        {
            //set lower iterator to last elem
            it_lower = it_upper;
            --it_lower;

            assert (it_lower->first < timestamp);

            //lower item too far away?
            if (timestamp - it_lower->first <= d_max)
                has_lower = true;
        }
    }

    if (!tr_valid_func)
        return {has_lower ? &dataFor(it_lower->second) : nullptr, has_upper ? &dataFor(it_upper->second) : nullptr};

    if (has_upper)
    {
        has_upper = false;
        for (auto it = it_upper; it != tr_timestamps_.end(); ++it)
        {
            if (has_upper || it->first - timestamp > d_max)
                break;

            if (tr_valid_func(dataFor(it->second)))
            {
                it_upper  = it;
                has_upper = true;
                break;
            }
        }
    }

    if (has_lower)
    {
        has_lower = false;
        auto it = it_lower;
        while (1)
        {
            if (timestamp - it->first > d_max)
                break;

            if (tr_valid_func(dataFor(it->second)))
            {
                it_lower  = it;
                has_lower = true;
                break;
            }

            if (it == tr_timestamps_.begin())
                break;
            
            --it;
        }
    }

    return {has_lower ? &dataFor(it_lower->second) : nullptr,
            has_upper ? &dataFor(it_upper->second) : nullptr};
}

ReconstructorTarget::ReferencePair ReconstructorTarget::refDataFor (ptime timestamp, time_duration d_max) const
// lower/upper times, invalid ts if not existing
{
    if (references_.count(timestamp))
        return {&references_.at(timestamp), nullptr}; // contains exact value

            //    Return iterator to lower bound
            //    Returns an iterator pointing to the first element in the container whose key is not considered to go
            //    before k (i.e., either it is equivalent or goes after).

    auto lb_it = references_.lower_bound(timestamp);

    if (lb_it == references_.end())
        return {nullptr, nullptr};

    assert (lb_it->first >= timestamp);

    if (lb_it->first - timestamp > d_max)
        return {nullptr, nullptr}; // too much time difference

            // save value
    ptime upper = lb_it->first;

            // TODO lb_it--; ?
    while (lb_it != references_.end() && timestamp < lb_it->first)
    {
        if (lb_it == references_.begin()) // exit condition on first value
        {
            if (timestamp < lb_it->first) // set as not found
                lb_it = references_.end();

            break;
        }

        lb_it--;
    }

    if (lb_it == references_.end())
        return {nullptr, &references_.at(upper)};

    assert (timestamp >= lb_it->first);

    if (timestamp - lb_it->first > d_max)
        return {nullptr, &references_.at(upper)}; // too much time difference

    ptime lower = lb_it->first;

    return {&references_.at(lower), &references_.at(upper)};
}

//std::pair<ptime, ptime> ReconstructorTarget::timesFor (
//    ptime timestamp, time_duration  d_max) const
//// lower/upper times, invalid ts if not existing
//{
//    if (tr_timestamps_.count(timestamp))
//        return {timestamp, {}}; // contains exact value

//            //    Return iterator to lower bound
//            //    Returns an iterator pointing to the first element in the container whose key is not considered to go
//            //    before k (i.e., either it is equivalent or goes after).

//    auto lb_it = tr_timestamps_.lower_bound(timestamp);

//    if (lb_it == tr_timestamps_.end())
//        return {{}, {}};

//    assert (lb_it->first >= timestamp);

//    if (lb_it->first - timestamp > d_max)
//        return {{}, {}}; // too much time difference

//            // save value
//    ptime upper = lb_it->first;

//            // TODO lb_it--; ?
//    while (lb_it != tr_timestamps_.end() && timestamp < lb_it->first)
//    {
//        if (lb_it == tr_timestamps_.begin()) // exit condition on first value
//        {
//            if (timestamp < lb_it->first) // set as not found
//                lb_it = tr_timestamps_.end();

//            break;
//        }

//        lb_it--;
//    }

//    if (lb_it == tr_timestamps_.end())
//        return {{}, upper};

//    assert (timestamp >= lb_it->first);

//    if (timestamp - lb_it->first > d_max)
//        return {{}, upper}; // too much time difference

//    ptime lower = lb_it->first;

//    return {lower, upper};
//}

std::pair<dbContent::targetReport::Position, bool> ReconstructorTarget::interpolatedPosForTime (
    ptime timestamp, time_duration d_max) const
{
    dbContent::targetReport::ReconstructorInfo* lower, *upper;

    tie(lower, upper) = dataFor(timestamp, d_max);

    if (lower && !upper) // exact time
    {
        if (lower->position_)
            return {*lower->position_, true};
        else
            return {{}, false};
    }

    if (!lower || !lower->position_ || !upper || !upper->position_)
        return {{}, false};

    dbContent::targetReport::Position pos1 = *lower->position_;
    dbContent::targetReport::Position pos2 = *upper->position_;
    float d_t = Time::partialSeconds(upper->timestamp_ - lower->timestamp_);

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

    float d_t2 = Time::partialSeconds(timestamp - lower->timestamp_);
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

            // TODO no alt 4 u!

            //    bool has_altitude = false;
            //    float altitude = 0.0;

            //    if (pos1.has_altitude_ && !pos2.has_altitude_)
            //    {
            //        has_altitude = true;
            //        altitude = pos1.altitude_;
            //    }
            //    else if (!pos1.has_altitude_ && pos2.has_altitude_)
            //    {
            //        has_altitude = true;
            //        altitude = pos2.altitude_;
            //    }
            //    else if (pos1.has_altitude_ && pos2.has_altitude_)
            //    {
            //        float v_alt = (pos2.altitude_ - pos1.altitude_)/d_t;
            //        has_altitude = true;
            //        altitude = pos1.altitude_ + v_alt*d_t2;
            //    }

            //    logdbg << "Target: interpolatedPosForTime: pos1 has alt "
            //           << pos1.has_altitude_ << " alt " << pos1.altitude_
            //           << " pos2 has alt " << pos2.has_altitude_ << " alt " << pos2.altitude_
            //           << " interpolated has alt " << has_altitude << " alt " << altitude;

            //            //        if (in_appimage_) // inside appimage
            //            //            return {{y_pos, x_pos, has_altitude, true, altitude}, true};
            //            //        else
            //    return {{x_pos, y_pos, has_altitude, true, altitude}, true};

    return {{x_pos, y_pos}, true};
}

std::pair<dbContent::targetReport::Position, bool> ReconstructorTarget::interpolatedPosForTimeFast (
    ptime timestamp, time_duration d_max) const
{
    dbContent::targetReport::ReconstructorInfo* lower_rec_num, *upper_rec_num;

    tie(lower_rec_num, upper_rec_num) = dataFor(timestamp, d_max, [ & ] (const dbContent::targetReport::ReconstructorInfo& ti) { return !ti.do_not_use_position_; });

    if (lower_rec_num && !upper_rec_num) // exact time
    {
        if (lower_rec_num->position_)
            return {*lower_rec_num->position_, true};
        else
            return {{}, false};
    }

    if (!lower_rec_num || !lower_rec_num->position_ || !upper_rec_num || !upper_rec_num->position_)
        return {{}, false};

    dbContent::targetReport::Position pos1 = *lower_rec_num->position_;
    dbContent::targetReport::Position pos2 = *upper_rec_num->position_;
    float d_t = Time::partialSeconds(upper_rec_num->timestamp_ - lower_rec_num->timestamp_);

    logdbg << "Target: interpolatedPosForTimeFast: d_t " << d_t;

    assert (d_t >= 0);

    if (pos1.latitude_ == pos2.latitude_
        && pos1.longitude_ == pos2.longitude_) // same pos
        return {pos1, true};

    if (lower_rec_num == upper_rec_num) // same time
    {
        logwrn << "Target: interpolatedPosForTimeFast: ref has same time twice";
        return {{}, false};
    }

    double v_lat = (pos2.latitude_ - pos1.latitude_)/d_t;
    double v_long = (pos2.longitude_ - pos1.longitude_)/d_t;
    logdbg << "Target: interpolatedPosForTimeFast: v_x " << v_lat << " v_y " << v_long;

    float d_t2 = Time::partialSeconds(timestamp - lower_rec_num->timestamp_);
    logdbg << "Target: interpolatedPosForTimeFast: d_t2 " << d_t2;

    assert (d_t2 >= 0);

    double int_lat = pos1.latitude_ + v_lat * d_t2;
    double int_long = pos1.longitude_ + v_long * d_t2;

    logdbg << "Target: interpolatedPosForTimeFast: interpolated lat " << int_lat << " long " << int_long;

            // calculate altitude
    //    bool has_altitude = false;
    //    float altitude = 0.0;

            //    if (pos1.has_altitude_ && !pos2.has_altitude_)
            //    {
            //        has_altitude = true;
            //        altitude = pos1.altitude_;
            //    }
            //    else if (!pos1.has_altitude_ && pos2.has_altitude_)
            //    {
            //        has_altitude = true;
            //        altitude = pos2.altitude_;
            //    }
            //    else if (pos1.has_altitude_ && pos2.has_altitude_)
            //    {
            //        float v_alt = (pos2.altitude_ - pos1.altitude_)/d_t;
            //        has_altitude = true;
            //        altitude = pos1.altitude_ + v_alt*d_t2;
            //    }

            //    logdbg << "Target: interpolatedPosForTimeFast: pos1 has alt "
            //           << pos1.has_altitude_ << " alt " << pos1.altitude_
            //           << " pos2 has alt " << pos2.has_altitude_ << " alt " << pos2.altitude_
            //           << " interpolated has alt " << has_altitude << " alt " << altitude;

            //    return {{int_lat, int_long, has_altitude, true, altitude}, true};

    return {{int_lat, int_long}, true};
}

boost::optional<dbContent::targetReport::Position> ReconstructorTarget::interpolatedRefPosForTimeFast (
    ptime timestamp, time_duration d_max) const
{
    const reconstruction::Reference* lower_ref, *upper_ref;

    tie(lower_ref, upper_ref) = refDataFor(timestamp, d_max);

    if (lower_ref && !upper_ref) // exact time
        return lower_ref->position();

    if (!lower_ref || !upper_ref)
        return {};

    dbContent::targetReport::Position pos1 = lower_ref->position();
    dbContent::targetReport::Position pos2 = upper_ref->position();
    float d_t = Time::partialSeconds(upper_ref->t - lower_ref->t);

    logdbg << "Target: interpolatedRefPosForTimeFast: d_t " << d_t;

    assert (d_t >= 0);

    if (pos1.latitude_ == pos2.latitude_
        && pos1.longitude_ == pos2.longitude_) // same pos
        return pos1;

    if (lower_ref == upper_ref) // same time
    {
        logwrn << "Target: interpolatedRefPosForTimeFast: ref has same time twice";
        return {};
    }

    double v_lat = (pos2.latitude_ - pos1.latitude_)/d_t;
    double v_long = (pos2.longitude_ - pos1.longitude_)/d_t;
    logdbg << "Target: interpolatedRefPosForTimeFast: v_x " << v_lat << " v_y " << v_long;

    float d_t2 = Time::partialSeconds(timestamp - lower_ref->t);
    logdbg << "Target: interpolatedRefPosForTimeFast: d_t2 " << d_t2;

    assert (d_t2 >= 0);

    double int_lat = pos1.latitude_ + v_lat * d_t2;
    double int_long = pos1.longitude_ + v_long * d_t2;

    logdbg << "Target: interpolatedRefPosForTimeFast: interpolated lat " << int_lat << " long " << int_long;

    return dbContent::targetReport::Position{int_lat, int_long};
}

//bool ReconstructorTarget::hasDataForExactTime (ptime timestamp) const
//{
//    return tr_timestamps_.count(timestamp);
//}

//unsigned long  ReconstructorTarget::dataForExactTime (ptime timestamp) const
//{
//    assert (hasDataForExactTime(timestamp));
//    assert (target_reports_.size() > tr_timestamps_.at(timestamp));
//    return *target_reports_.at(tr_timestamps_.at(timestamp));
//}

//dbContent::targetReport::Position ReconstructorTarget::posForExactTime (ptime timestamp) const
//{
//    assert (hasDataForExactTime(timestamp));

//    TargetReport& tr = dataForExactTime(timestamp);

//    dbContent::targetReport::Position pos;

//    pos.latitude_ = tr.latitude_;
//    pos.longitude_ = tr.longitude_;
//    pos.has_altitude_ = tr.has_mc_;
//    pos.altitude_ = tr.mc_;

//    return pos;
//}

bool ReconstructorTarget::hasDataFor (unsigned long rec_num) const
{
    return reconstructor_.target_reports_.count(rec_num);
}

dbContent::targetReport::ReconstructorInfo& ReconstructorTarget::dataFor (unsigned long rec_num) const
{
    assert (reconstructor_.target_reports_.count(rec_num));
    return reconstructor_.target_reports_.at(rec_num);
}

bool ReconstructorTarget::hasPositionFor (unsigned long rec_num) const
{
    return dataFor(rec_num).position_.has_value();
}

dbContent::targetReport::Position ReconstructorTarget::positionFor (unsigned long rec_num) const
{
    assert (hasPositionFor(rec_num));
    return *dataFor(rec_num).position_;
}


float ReconstructorTarget::duration () const
{
    if (timestamp_max_.is_not_a_date_time() || timestamp_min_.is_not_a_date_time())
        return 0;

    return Time::partialSeconds(timestamp_max_ - timestamp_min_);
}

bool ReconstructorTarget::timeOverlaps (const ReconstructorTarget& other) const
{
    if (!target_reports_.size() || !other.target_reports_.size())
        return false;

    assert (!timestamp_max_.is_not_a_date_time() && !timestamp_min_.is_not_a_date_time());

            //a.start < b.end && b.start < a.end;
    return timestamp_min_ < other.timestamp_max_ && other.timestamp_min_ < timestamp_max_;
}

float ReconstructorTarget::probTimeOverlaps (const ReconstructorTarget& other) const
{
    if (timestamp_max_.is_not_a_date_time() || timestamp_min_.is_not_a_date_time())
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

std::tuple<vector<unsigned long>, vector<unsigned long>, vector<unsigned long>> ReconstructorTarget::compareModeACodes (
    const ReconstructorTarget& other, boost::posix_time::time_duration max_time_diff) const
{
    vector<unsigned long> unknown;
    vector<unsigned long> same;
    vector<unsigned long> different;

    ComparisonResult cmp_res;

    for (auto tr_it : target_reports_)
    {
        dbContent::targetReport::ReconstructorInfo& tr = dataFor(tr_it);

        cmp_res = other.compareModeACode(tr, max_time_diff);

        if (cmp_res == ComparisonResult::UNKNOWN)
            unknown.push_back(tr.record_num_);
        else if (cmp_res == ComparisonResult::SAME)
            same.push_back(tr.record_num_);
        else if (cmp_res == ComparisonResult::DIFFERENT)
            different.push_back(tr.record_num_);
    }

    assert (target_reports_.size() == unknown.size()+same.size()+different.size());

    return std::tuple<vector<unsigned long>, vector<unsigned long>, vector<unsigned long>>(unknown, same, different);
}

ComparisonResult ReconstructorTarget::compareModeACode (
    const dbContent::targetReport::ReconstructorInfo& tr, time_duration max_time_diff) const
{
    if (tr.mode_a_code_.has_value() && !tr.mode_a_code_->hasReliableValue()) // check if reliable value
        return ComparisonResult::UNKNOWN;

    if (!hasDataForTime(tr.timestamp_, max_time_diff))
        return ComparisonResult::UNKNOWN;

    dbContent::targetReport::ReconstructorInfo* lower_tr, *upper_tr;

    tie(lower_tr, upper_tr) = dataFor(tr.timestamp_, max_time_diff);

    bool lower_no_m3a = lower_tr && !lower_tr->mode_a_code_.has_value();
    bool upper_no_m3a = upper_tr && !upper_tr->mode_a_code_.has_value();

            // no mode a, and one missing in one of the others
    if (!tr.mode_a_code_.has_value() && (lower_no_m3a || upper_no_m3a)) // TODO check if data sources m3a capable
        return ComparisonResult::SAME;

    bool lower_m3a_usable = lower_tr && lower_tr->mode_a_code_.has_value() && lower_tr->mode_a_code_->hasReliableValue();
    bool upper_m3a_usable = upper_tr && upper_tr->mode_a_code_.has_value() && upper_tr->mode_a_code_->hasReliableValue();

            // no able to compare
    if (!lower_m3a_usable && !upper_m3a_usable)
        return ComparisonResult::UNKNOWN;

    if ((lower_m3a_usable && !upper_m3a_usable)
        || (!lower_m3a_usable && upper_m3a_usable)) // only 1 usable
    {
        dbContent::targetReport::ReconstructorInfo& ref1 = lower_m3a_usable ? *lower_tr : *upper_tr;

        if (!tr.mode_a_code_.has_value())
            return ComparisonResult::DIFFERENT;

                // mode a exists
        if (tr.mode_a_code_->code_ == ref1.mode_a_code_->code_) // is same
            return ComparisonResult::SAME;
        else
            return ComparisonResult::DIFFERENT;
    }

            // both set & reliable
    dbContent::targetReport::ReconstructorInfo& ref1 = *lower_tr;
    dbContent::targetReport::ReconstructorInfo& ref2 = *upper_tr;

    if (!tr.mode_a_code_.has_value())
        return ComparisonResult::DIFFERENT; // no mode a here, but in other

            // everything exists

    if ((tr.mode_a_code_->code_ == ref1.mode_a_code_->code_)
        || (tr.mode_a_code_->code_ == ref2.mode_a_code_->code_)) // one of them is same
    {
        return ComparisonResult::SAME;
    }
    else
        return ComparisonResult::DIFFERENT;
}

std::tuple<vector<unsigned long>, vector<unsigned long>, vector<unsigned long>> ReconstructorTarget::compareModeCCodes (
    const ReconstructorTarget& other, const std::vector<unsigned long>& rec_nums,
    time_duration max_time_diff, float max_alt_diff, bool debug) const
{
    vector<unsigned long> unknown;
    vector<unsigned long> same;
    vector<unsigned long> different;

    ComparisonResult cmp_res;

    for (auto rn_it : rec_nums)
    {
        assert (hasDataFor(rn_it));
        dbContent::targetReport::ReconstructorInfo& tr = dataFor(rn_it);

        cmp_res = other.compareModeCCode(tr, max_time_diff, max_alt_diff, debug);

        if (cmp_res == ComparisonResult::UNKNOWN)
            unknown.push_back(tr.record_num_);
        else if (cmp_res == ComparisonResult::SAME)
            same.push_back(tr.record_num_);
        else if (cmp_res == ComparisonResult::DIFFERENT)
            different.push_back(tr.record_num_);
    }

    assert (rec_nums.size() == unknown.size()+same.size()+different.size());

    return std::tuple<vector<unsigned long>, vector<unsigned long>, vector<unsigned long>>(unknown, same, different);
}

ComparisonResult ReconstructorTarget::compareModeCCode (
    const dbContent::targetReport::ReconstructorInfo& tr,
    time_duration max_time_diff, float max_alt_diff, bool debug) const
{
    if (tr.barometric_altitude_.has_value() && !tr.barometric_altitude_->hasReliableValue()) // check if reliable value
        return ComparisonResult::UNKNOWN;

    if (!hasDataForTime(tr.timestamp_, max_time_diff))
        return ComparisonResult::UNKNOWN;

    dbContent::targetReport::ReconstructorInfo* lower_tr, *upper_tr;

    tie(lower_tr, upper_tr) = dataFor(tr.timestamp_, max_time_diff);

    bool lower_no_mc = lower_tr && !lower_tr->barometric_altitude_.has_value();
    bool upper_no_mc = upper_tr && !upper_tr->barometric_altitude_.has_value();

            // no mode a, and one missing in one of the others
    if (!tr.barometric_altitude_.has_value() && (lower_no_mc || upper_no_mc)) // TODO check if data sources m3a capable
        return ComparisonResult::SAME;

    bool lower_mc_usable = lower_tr && lower_tr->barometric_altitude_.has_value()
                           && lower_tr->barometric_altitude_->hasReliableValue();

    bool upper_mc_usable = upper_tr && upper_tr->barometric_altitude_.has_value()
                           && upper_tr->barometric_altitude_->hasReliableValue();

            // no able to compare
    if (!lower_mc_usable && !upper_mc_usable)
        return ComparisonResult::UNKNOWN;

    if ((lower_mc_usable && !upper_mc_usable)
        || (!lower_mc_usable && upper_mc_usable)) // only 1 usable
    {
        dbContent::targetReport::ReconstructorInfo& ref1 = lower_mc_usable ? *lower_tr : *upper_tr;
        assert (ref1.barometric_altitude_);

        if (!tr.barometric_altitude_.has_value())
            return ComparisonResult::DIFFERENT;

                // barometric_altitude exists
        if (fabs(tr.barometric_altitude_->altitude_ - ref1.barometric_altitude_->altitude_) < max_alt_diff) // is same
            return ComparisonResult::SAME;
        else
            return ComparisonResult::DIFFERENT;
    }

            // both set & reliable
    dbContent::targetReport::ReconstructorInfo& ref1 = *lower_tr;
    dbContent::targetReport::ReconstructorInfo& ref2 = *upper_tr;

    if (!tr.barometric_altitude_.has_value())
        return ComparisonResult::DIFFERENT; // no mode a here, but in other

            // everything exists

    if ((fabs(tr.barometric_altitude_->altitude_ - ref1.barometric_altitude_->altitude_) < max_alt_diff)
        || (fabs(tr.barometric_altitude_->altitude_ - ref2.barometric_altitude_->altitude_)) < max_alt_diff) // one of them is same
    {
        return ComparisonResult::SAME;
    }
    else
        return ComparisonResult::DIFFERENT;
}

void ReconstructorTarget::updateCounts()
{
    for (auto& rn_it : target_reports_)
    {
        assert (reconstructor_.target_reports_.count(rn_it));
        dbContent::targetReport::ReconstructorInfo& tr = reconstructor_.target_reports_.at(rn_it);

        if (tr.timestamp_ >= reconstructor_.currentSlice().write_before_time_) // tr.in_current_slice_
            continue;

        counts_[Number::recNumGetDBContId(rn_it)] += 1;
    }
}

std::map <std::string, unsigned int> ReconstructorTarget::getDBContentCounts()
{
    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    std::map <std::string, unsigned int> counts;

    for (auto& cnt_it : counts_)
    {
        counts[dbcont_man.dbContentWithId(cnt_it.first)] = cnt_it.second;
    }

    return counts;
}

std::shared_ptr<Buffer> ReconstructorTarget::getReferenceBuffer()
{
    logdbg << "ReconstructorTarget: getReferenceBuffer: utn " << utn_ << " ref size " << references_.size();

    string dbcontent_name = "RefTraj";
    unsigned int dbcontent_id = 255;

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    PropertyList buffer_list;

            // basics
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ds_id_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_sac_id_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_sic_id_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_line_id_));

    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_timestamp_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_time_of_day_));

            // pos
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_latitude_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_longitude_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_));

    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ground_bit_));

    // track num begin, end
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_num_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_begin_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_end_));

            // spd
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_vx_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_vy_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ground_speed_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_angle_));

    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_rocd_));

    // accs
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ax_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ay_));

            // stddevs
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_x_stddev_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_y_stddev_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_xy_cov_));

            // secondary
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_acad_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_acid_));

    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_utn_));

    // yo mom so acc
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mom_long_acc_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mom_trans_acc_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mom_vert_rate_));

    std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>(buffer_list, dbcontent_name);

    NullableVector<unsigned int>& ds_id_vec = buffer->get<unsigned int> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ds_id_).name());
    NullableVector<unsigned char>& sac_vec = buffer->get<unsigned char> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_sac_id_).name());
    NullableVector<unsigned char>& sic_vec = buffer->get<unsigned char> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_sic_id_).name());
    NullableVector<unsigned int>& line_vec = buffer->get<unsigned int> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_line_id_).name());

    NullableVector<float>& tod_vec = buffer->get<float> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_time_of_day_).name());
    NullableVector<ptime>& ts_vec = buffer->get<ptime> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_timestamp_).name());

    NullableVector<double>& lat_vec = buffer->get<double> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_latitude_).name());
    NullableVector<double>& lon_vec = buffer->get<double> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_longitude_).name());
    NullableVector<float>& mc_vec = buffer->get<float> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_).name());

    // track num begin, end
    NullableVector<bool>& track_begin_vec = buffer->get<bool> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_begin_).name());
    NullableVector<bool>& track_end_vec = buffer->get<bool> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_end_).name());
    NullableVector<unsigned int>& track_num_vec = buffer->get<unsigned int> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_num_).name());

            // speed, track angle
    NullableVector<double>& vx_vec = buffer->get<double> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_vx_).name());
    NullableVector<double>& vy_vec = buffer->get<double> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_vy_).name());

    NullableVector<double>& speed_vec = buffer->get<double> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ground_speed_).name());
    NullableVector<double>& track_angle_vec = buffer->get<double> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_angle_).name());

    NullableVector<float>& rocd_vec = buffer->get<float> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_rocd_).name());

    // accs
    NullableVector<double>& ax_vec = buffer->get<double> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ax_).name());
    NullableVector<double>& ay_vec = buffer->get<double> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ay_).name());

            // stddevs
    NullableVector<double>& x_stddev_vec = buffer->get<double> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_x_stddev_).name());
    NullableVector<double>& y_stddev_vec = buffer->get<double> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_y_stddev_).name());
    NullableVector<double>& xy_cov_vec = buffer->get<double> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_xy_cov_).name());

            // ground bit
    NullableVector<bool>& gb_vec = buffer->get<bool> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ground_bit_).name());

    NullableVector<unsigned int>& m3a_vec = buffer->get<unsigned int> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_).name());
    NullableVector<unsigned int>& acad_vec = buffer->get<unsigned int> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_acad_).name());
    NullableVector<string>& acid_vec = buffer->get<string> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_acid_).name());

    // mom
    NullableVector<unsigned char>& mom_long_acc_vec = buffer->get<unsigned char> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mom_long_acc_).name());
    NullableVector<unsigned char>& mom_trans_acc_vec = buffer->get<unsigned char> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mom_trans_acc_).name());
    NullableVector<unsigned char>& mom_vert_rate_vec = buffer->get<unsigned char> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mom_vert_rate_).name());

    NullableVector<unsigned int>& utn_vec = buffer->get<unsigned int> (
        dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_utn_).name());

    unsigned int sac = reconstructor_.baseSettings().ds_sac;
    unsigned int sic = reconstructor_.baseSettings().ds_sic;
    unsigned int ds_id = Number::dsIdFrom(sac, sic);
    assert (reconstructor_.baseSettings().ds_line >= 0 && reconstructor_.baseSettings().ds_line <= 3);
    //std::vector<unsigned int> assoc_val ({utn_});

    double speed_ms, bearing_rad, xy_cov;
    double ax, ay, bearing_new_rad, turnrate_rad, a_ms2;
    double rocd_ft_s;
    double dt;

    unsigned int buffer_cnt = 0;

    boost::posix_time::time_duration d_max = Time::partialSeconds(10);
    boost::posix_time::time_duration track_end_time = Time::partialSeconds(30);

    //boost::posix_time::ptime ts_prev;

    for (auto& ref_it : references_)
    {
        //const reconstruction::Reference& ref = references_.at(i);

//        loginf << "ReconstructorTarget: getReferenceBuffer: utn " << utn_ << " ref ts " << Time::toString(ref.t)
//               << " wbt " << Time::toString(reconstructor_.write_before_time_) <<
//            " skip " << (ref.t >= reconstructor_.write_before_time_);

        if (ref_it.second.t >= reconstructor_.currentSlice().write_before_time_)
            continue;

        if (!ts_prev_.is_not_a_date_time())
            assert (ref_it.second.t > ts_prev_);

        ds_id_vec.set(buffer_cnt, ds_id);
        sac_vec.set(buffer_cnt, sac);
        sic_vec.set(buffer_cnt, sic);
        line_vec.set(buffer_cnt, reconstructor_.baseSettings().ds_line);

        ts_vec.set(buffer_cnt, ref_it.second.t);
        tod_vec.set(buffer_cnt, ref_it.second.t.time_of_day().total_milliseconds() / 1000.0);

        lat_vec.set(buffer_cnt, ref_it.second.lat);
        lon_vec.set(buffer_cnt, ref_it.second.lon);

        utn_vec.set(buffer_cnt, utn_);

        track_num_vec.set(buffer_cnt, utn_);

        // track end
        if (!ts_prev_.is_not_a_date_time()
            && ref_it.second.t - ts_prev_ > track_end_time) // have time before and dt > track end time
        {
            if (buffer_cnt != 0)
                track_end_vec.set(buffer_cnt-1, true); // set at previous update if possible

            track_begin_ = true;

            has_prev_v_ = false; // info too old
            has_prev_baro_alt_ = false;
        }

        track_end_vec.set(buffer_cnt, false);
        track_begin_vec.set(buffer_cnt, track_begin_);

        track_begin_ = false;

                // set speed

        if (ref_it.second.vx.has_value() && ref_it.second.vy.has_value())
        {
            vx_vec.set(buffer_cnt, *ref_it.second.vx);
            vy_vec.set(buffer_cnt, *ref_it.second.vy);

            speed_ms = sqrt(pow(*ref_it.second.vx, 2)+pow(*ref_it.second.vy, 2)); // for 1s
            bearing_rad = atan2(*ref_it.second.vx, *ref_it.second.vy);

            speed_vec.set(buffer_cnt, speed_ms * M_S2KNOTS);
            track_angle_vec.set(buffer_cnt, bearing_rad * RAD2DEG);

            // set ax, ay
            if (has_prev_v_)
            {
                ax = *ref_it.second.vx - v_x_prev_;
                ay = *ref_it.second.vy - v_y_prev_;

                ax_vec.set(buffer_cnt, ax);
                ay_vec.set(buffer_cnt, ay);

                a_ms2 = sqrt(pow(ax, 2)+pow(ay, 2)); // for 1s2

                        // LONG ACC
                if (fabs(a_ms2) < 1) // like 0
                    mom_long_acc_vec.set(buffer_cnt, (unsigned char) MOM_LONG_ACC::ConstantGroundspeed);
                else if (a_ms2 > 0)
                    mom_long_acc_vec.set(buffer_cnt, (unsigned char) MOM_LONG_ACC::IncreasingGroundspeed);
                else if (a_ms2 < 0)
                    mom_long_acc_vec.set(buffer_cnt, (unsigned char) MOM_LONG_ACC::DecreasingGroundspeed);

                bearing_new_rad = atan2(*ref_it.second.vx + ax, *ref_it.second.vy + ay); // for 1s

                turnrate_rad = bearing_new_rad - bearing_rad;

                //loginf << " turnrate_rad " << turnrate_rad;

                while (turnrate_rad > M_PI)
                    turnrate_rad -= 2*M_PI;
                while (turnrate_rad < -M_PI)
                    turnrate_rad += 2*M_PI;

                assert (fabs(turnrate_rad) <= M_PI);

                // TRANS ACC
                if (fabs(turnrate_rad) < M_PI/20) // like 0
                    mom_trans_acc_vec.set(buffer_cnt, (unsigned char) MOM_TRANS_ACC::ConstantCourse);
                else if (turnrate_rad > 0)
                    mom_trans_acc_vec.set(buffer_cnt, (unsigned char) MOM_TRANS_ACC::RightTurn);
                else if (turnrate_rad < 0)
                    mom_trans_acc_vec.set(buffer_cnt, (unsigned char) MOM_TRANS_ACC::LeftTurn);
            }

            v_x_prev_ = *ref_it.second.vx;
            v_y_prev_ = *ref_it.second.vy;
            has_prev_v_ = true;
        }
        else
            has_prev_v_ = false;


        if (mom_long_acc_vec.isNull(buffer_cnt))
            mom_long_acc_vec.set(buffer_cnt, (unsigned char) MOM_LONG_ACC::Undetermined);

        if (mom_trans_acc_vec.isNull(buffer_cnt))
            mom_trans_acc_vec.set(buffer_cnt, (unsigned char) MOM_TRANS_ACC::Undetermined);

                // set stddevs

        if (ref_it.second.x_stddev.has_value() && ref_it.second.y_stddev.has_value() && ref_it.second.xy_cov.has_value())
        {
            x_stddev_vec.set(buffer_cnt, *ref_it.second.x_stddev);
            y_stddev_vec.set(buffer_cnt, *ref_it.second.y_stddev);

            xy_cov = *ref_it.second.xy_cov;

                    // to inverse of this asterix rep
                    // if (xy_cov < 0)
                    //     xy_cov = - pow(xy_cov, 2);
                    // else
                    //     xy_cov = pow(xy_cov, 2);

            if (xy_cov < 0)
                xy_cov_vec.set(buffer_cnt, -sqrt(-xy_cov));
            else
                xy_cov_vec.set(buffer_cnt, sqrt(xy_cov));
        }

                // set other data
                // TODO crappy

        ReconstructorInfoPair info = dataFor(ref_it.second.t, d_max);

        if (info.first && info.first->barometric_altitude_
            && info.first->barometric_altitude_->hasReliableValue())
        {
            if (mc_vec.isNull(buffer_cnt))
                mc_vec.set(buffer_cnt, info.first->barometric_altitude_->altitude_);
        }

        if (info.second && info.second->barometric_altitude_
            && info.second->barometric_altitude_->hasReliableValue())
        {
            if (mc_vec.isNull(buffer_cnt))
                mc_vec.set(buffer_cnt, info.second->barometric_altitude_->altitude_);
            else
                mc_vec.set(buffer_cnt,
                           (info.second->barometric_altitude_->altitude_ + mc_vec.get(buffer_cnt))/2.0);
        }

        if (!mc_vec.isNull(buffer_cnt)) // has current alt
        {
            if (has_prev_baro_alt_)
            {
                dt = (ts_vec.get(buffer_cnt) - ts_prev_).total_milliseconds() / 1000.0;

                rocd_ft_s = (mc_vec.get(buffer_cnt) - baro_alt_prev_) / dt;
                rocd_vec.set(buffer_cnt, rocd_ft_s * 60); // ft per minute

                // MOM Vertical Rate
                if (rocd_ft_s == 0)
                    mom_vert_rate_vec.set(buffer_cnt, (unsigned char) MOM_VERT_RATE::Level);
                else if (rocd_ft_s > 0)
                    mom_vert_rate_vec.set(buffer_cnt, (unsigned char) MOM_VERT_RATE::Climb);
                else if (rocd_ft_s < 0)
                    mom_vert_rate_vec.set(buffer_cnt, (unsigned char) MOM_VERT_RATE::Descent);
            }

            has_prev_baro_alt_ = true;
            baro_alt_prev_ = mc_vec.get(buffer_cnt);
        }
        else
            has_prev_baro_alt_ = false;

        if (mom_vert_rate_vec.isNull(buffer_cnt))
            mom_vert_rate_vec.set(buffer_cnt, (unsigned char) MOM_VERT_RATE::Undetermined);

        if (info.first)
        {
            if (info.first->mode_a_code_ && info.first->mode_a_code_->hasReliableValue() && m3a_vec.isNull(buffer_cnt))
                m3a_vec.set(buffer_cnt, info.first->mode_a_code_->code_);

            if (info.first->acad_ && acad_vec.isNull(buffer_cnt))
                acad_vec.set(buffer_cnt, *info.first->acad_);

            if (info.first->acid_ && acid_vec.isNull(buffer_cnt))
                acid_vec.set(buffer_cnt, *info.first->acid_);

            if (info.first->ground_bit_ && gb_vec.isNull(buffer_cnt))
                gb_vec.set(buffer_cnt, *info.first->ground_bit_);
        }

        if (info.second)
        {
            if (info.second->mode_a_code_ && info.second->mode_a_code_->hasReliableValue() && m3a_vec.isNull(buffer_cnt))
                m3a_vec.set(buffer_cnt, info.second->mode_a_code_->code_);

            if (info.second->acad_ && acad_vec.isNull(buffer_cnt))
                acad_vec.set(buffer_cnt, *info.second->acad_);

            if (info.second->acid_ && acid_vec.isNull(buffer_cnt))
                acid_vec.set(buffer_cnt, *info.second->acid_);

            if (info.second->ground_bit_ && gb_vec.isNull(buffer_cnt))
                gb_vec.set(buffer_cnt, *info.second->ground_bit_);
        }

        ++buffer_cnt;

        ts_prev_ = ref_it.second.t;
    }

    // check last update for track end
    if (buffer->size())
    {
        unsigned int last_index = buffer_cnt - 1;

        if (reconstructor_.currentSlice().next_slice_begin_ - ts_vec.get(last_index) > track_end_time)
            track_end_vec.set(last_index, true);
    }

    counts_[dbcontent_id] += buffer->size();

    logdbg << "ReconstructorTarget: getReferenceBuffer: utn " << utn_ << " buffer size " << buffer->size();
    //assert (buffer->size());

    return buffer;
}

void ReconstructorTarget::removeOutdatedTargetReports()
{
    auto tmp_tr_timestamps = std::move(tr_timestamps_);

    target_reports_.clear();
    tr_timestamps_.clear();
    tr_ds_timestamps_.clear();

    for (auto ts_it : tmp_tr_timestamps)
    {
        if (reconstructor_.target_reports_.count(ts_it.second))
            addTargetReport(ts_it.second, false);
    }

    references_.clear();
}

void ReconstructorTarget::reinitTracker()
{
    //reset, reconfigure and initialize tracker
    tracker_.reset(new reconstruction::KalmanOnlineTracker);
    tracker_->settings() = reconstructor_.referenceCalculatorSettings().kalmanEstimatorSettings();
    tracker_->init(reconstructor_.referenceCalculatorSettings().kalman_type);

    //@TODO: init tracker from last slice's final state (where is it stored?)
    //tracker_->kalmanInit(mm)
}

void ReconstructorTarget::addToTracker(const dbContent::targetReport::ReconstructorInfo& tr)
{
    assert(tracker_);
    
    reconstruction::Measurement mm;
    reconstructor_.createMeasurement(mm, tr);

    tracker_->track(mm);
}

bool ReconstructorTarget::canPredict(boost::posix_time::ptime timestamp) const
{
    if (!tracker_)
        return false;

    if (!tr_timestamps_.size())
        return false;

    assert (timestamp >= tr_timestamps_.rbegin()->first);

    if (Time::partialSeconds(timestamp - tr_timestamps_.rbegin()->first) > 30.0)
        return false;

    return true;
}

bool ReconstructorTarget::predict(reconstruction::Measurement& mm, 
                                  const dbContent::targetReport::ReconstructorInfo& tr) const
{
    assert(tracker_);
    return tracker_->predict(mm, tr.timestamp_);
}

//bool ReconstructorTarget::hasADSBMOPSVersion()
//{
//    return mops_versions_.size();
//}

//std::set<unsigned int> ReconstructorTarget::getADSBMOPSVersions()
//{
//    assert (hasADSBMOPSVersion());

//    return mops_versions_;
//}


} // namespace dbContent
