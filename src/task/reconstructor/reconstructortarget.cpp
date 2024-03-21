#include "reconstructortarget.h"
#include "simplereconstructor.h"
#include "compass.h"
#include "dbcontentmanager.h"
#include "util/number.h"
#include "util/timeconv.h"

#include <boost/optional/optional_io.hpp>

using namespace std;
using namespace Utils;
using namespace boost::posix_time;

namespace dbContent {

ReconstructorTarget::ReconstructorTarget(SimpleReconstructor& reconstructor)
    : reconstructor_(reconstructor)
{

}

ReconstructorTarget::~ReconstructorTarget()
{
   //    if (!tmp_)
   //    {
   //        for (auto& tr_it : target_reports_)
   //            tr_it->removeAssociated(this);
   //    }
}

void ReconstructorTarget::addTargetReport (unsigned long rec_num)
{
    assert (reconstructor_.target_reports_.count(rec_num));

    const dbContent::targetReport::ReconstructorInfo& tr = reconstructor_.target_reports_.at(rec_num);

            // update min/max
    if (!target_reports_.size())
    {
        timestamp_min_ = tr.timestamp_;
        timestamp_max_ = tr.timestamp_;
    }
    else
    {
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
    tr_ds_timestamps_[Number::recNumGetDBContId(tr.record_num_)][tr.ds_id_].insert({tr.timestamp_, tr.record_num_});
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
}

void ReconstructorTarget::addTargetReports (std::vector<unsigned long> rec_nums)
{
    for (auto& rn_it : rec_nums)
        addTargetReport(rn_it);
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

bool ReconstructorTarget::hasModeA (unsigned int ma)  const
{
    return mode_as_.count(ma);
}

std::string ReconstructorTarget::asStr() const
{
    stringstream ss;

    ss << "utn " << utn_ ? to_string(*utn_) : "none";  //<< " tmp " << tmp_

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

ReconstructorTarget::ReconstructorInfoPair ReconstructorTarget::dataFor (ptime timestamp, time_duration d_max) const
// lower/upper times, invalid ts if not existing
{
    if (tr_timestamps_.count(timestamp))
        return {&dataFor(tr_timestamps_.find(timestamp)->second), nullptr}; // contains exact value

            //    Return iterator to lower bound
            //    Returns an iterator pointing to the first element in the container whose key is not considered to go
            //    before k (i.e., either it is equivalent or goes after).

    auto lb_it = tr_timestamps_.lower_bound(timestamp);

    if (lb_it == tr_timestamps_.end())
        return {nullptr, nullptr};

    assert (lb_it->first >= timestamp);

    if (lb_it->first - timestamp > d_max)
        return {nullptr, nullptr}; // too much time difference

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
        return {nullptr, &dataFor(tr_timestamps_.find(upper)->second)};

    assert (timestamp >= lb_it->first);

    if (timestamp - lb_it->first > d_max)
        return {nullptr, &dataFor(tr_timestamps_.find(upper)->second)}; // too much time difference

    ptime lower = lb_it->first;

    return {&dataFor(tr_timestamps_.find(lower)->second), &dataFor(tr_timestamps_.find(upper)->second)};
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

    if (!lower || !lower->position_ || !upper || upper->position_)
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

    tie(lower_rec_num, upper_rec_num) = dataFor(timestamp, d_max);

    if (lower_rec_num && !upper_rec_num) // exact time
    {
        if (lower_rec_num->position_)
            return {*lower_rec_num->position_, true};
        else
            return {{}, false};
    }

    if (!lower_rec_num || !lower_rec_num->position_ || !upper_rec_num || upper_rec_num->position_)
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
    dbContent::targetReport::ReconstructorInfo& tr, time_duration max_time_diff) const
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

ComparisonResult ReconstructorTarget::compareModeCCode (dbContent::targetReport::ReconstructorInfo& tr,
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

        if (!tr.mode_a_code_.has_value())
            return ComparisonResult::DIFFERENT;

                // mode a exists
        if (fabs(tr.mode_a_code_->code_ - ref1.mode_a_code_->code_) < max_alt_diff) // is same
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

    if ((fabs(tr.mode_a_code_->code_ - ref1.mode_a_code_->code_) < max_alt_diff)
        || (fabs(tr.mode_a_code_->code_ - ref2.mode_a_code_->code_)) < max_alt_diff) // one of them is same
    {
        return ComparisonResult::SAME;
    }
    else
        return ComparisonResult::DIFFERENT;
}

//void ReconstructorTarget::calculateSpeeds()
//{
//    has_speed_ = false;

//    ptime timestamp;
//    double latitude {0};
//    double longitude {0};
//    TargetReport* tr;

//    ptime timestamp_prev;
//    double latitude_prev {0};
//    double longitude_prev {0};

//            //        OGRSpatialReference wgs84;
//            //        wgs84.SetWellKnownGeogCS("WGS84");
//            //        OGRSpatialReference local;

//    Transformation trafo;

//    double x_pos, y_pos;
//    bool ok;

//    float d_t;
//    double v_x, v_y;
//    double spd;
//    double spd_sum {0};
//    unsigned int num_spd {0};

//    bool first = true;
//    for (auto& time_it : tr_timestamps_)
//    {
//        timestamp_prev = timestamp;
//        latitude_prev = latitude;
//        longitude_prev = longitude;

//        timestamp = time_it.first;

//        assert (time_it.second < target_reports_.size());
//        tr = target_reports_.at(time_it.second);

//        latitude = info.latitude_;
//        longitude = info.longitude_;

//        if (first)
//        {
//            first = false;
//            continue;
//        }

//        d_t = Time::partialSeconds(timestamp - timestamp_prev);
//        assert (d_t >= 0);

//        tie(ok, x_pos, y_pos) = trafo.distanceCart(
//            latitude, longitude, latitude_prev, longitude_prev);

//        if (!ok)
//            continue;

//        v_x = x_pos/d_t;
//        v_y = y_pos/d_t;

//        spd = sqrt(pow(v_x,2)+pow(v_y,2)) * M_S2KNOTS;

//        if (!has_speed_)
//        {
//            has_speed_ = true;

//            speed_min_ = spd;
//            speed_max_ = spd;
//        }
//        else
//        {
//            speed_min_ = min(speed_min_, spd);
//            speed_max_ = max(speed_max_, spd);
//        }

//        spd_sum += spd;
//        ++num_spd;
//    }

//    if (num_spd)
//    {
//        speed_avg_ = spd_sum/(float)num_spd;
//    }
//}

//void ReconstructorTarget::removeNonModeSTRs()
//{
//    vector<TargetReport*> tmp_trs = target_reports_;

//    if (!tmp_)
//    {
//        loginf << "Target: removeNonModeSTRs: " << asStr();

//        for (auto tr_it : tmp_trs)
//            tr_it->removeAssociated(this);
//    }

//    target_reports_.clear();

//    tas_.clear();
//    ids_.clear();
//    mas_.clear();
//    mops_versions_.clear();
//    has_timestamps_ = false;
//    has_speed_ = false;
//    tr_timestamps_.clear();
//    ds_ids_.clear();
//    track_nums_.clear();

//    for (auto tr_it : tmp_trs)
//    {
//        if (tr_it->has_ta_)
//            addAssociated(tr_it);
//    }
//}

std::map <std::string, unsigned int> ReconstructorTarget::getDBContentCounts()
{
    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    std::map <std::string, unsigned int> counts;
    unsigned int dbcont_id;

    for (auto& rn_it : target_reports_)
    {
        dbcont_id = Number::recNumGetDBContId(dataFor(rn_it).record_num_);
        counts[dbcont_man.dbContentWithId(dbcont_id)] += 1;
    }

    return counts;
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
