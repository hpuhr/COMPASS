#include "assoc/target.h"
#include "assoc/targetreport.h"
#include "logger.h"
#include "stringconv.h"

#include <cassert>
#include <sstream>

#include <ogr_spatialref.h>

using namespace std;
using namespace Utils;

namespace Association
{
    bool Target::in_appimage_ {getenv("APPDIR")};

    Target::Target(unsigned int utn, bool tmp)
        : utn_(utn), tmp_(tmp)
    {
    }

    Target::~Target()
    {
        if (!tmp_)
            for (auto& tr_it : assoc_trs_)
                tr_it->removeAssociated(this);
    }

    void Target::addAssociated (TargetReport* tr)
    {
        assert (tr);

        // update min/max
        if (!assoc_trs_.size())
        {
            tod_min_ = tr->tod_;
            tod_max_ = tr->tod_;
        }
        else
        {
            tod_min_ = min(tod_min_, tr->tod_);
            tod_max_ = max(tod_max_, tr->tod_);
        }

        if (!ds_ids_.count(tr->ds_id_))
            ds_ids_.insert(tr->ds_id_);

        timed_indexes_[tr->tod_] = assoc_trs_.size();
        assoc_trs_.push_back(tr);

        if (!has_ta_ && tr->has_ta_)
        {
            has_ta_ = true;
            ta_ = tr->ta_;
        }
        else if (has_ta_ && tr->has_ta_ && ta_ != tr->ta_)
        {
            logwrn << "Target: addAssociated: ta mismatch, target " << asStr()
                   << " tr " << tr->asStr();
        }

        if (!tmp_)
            tr->addAssociated(this);
    }

    void Target::addAssociated (vector<TargetReport*> trs)
    {
        for (auto& tr_it : trs)
            addAssociated(tr_it);
    }

    std::string Target::asStr()
    {
        stringstream ss;

        ss << "utn " << utn_ << " tmp " << tmp_;

        if (has_ta_)
            ss << " ta " << String::hexStringFromInt(ta_, 6, '0');

        return ss.str();
    }

    bool Target::hasDataForTime (float tod, float d_max) const
    {
        if (timed_indexes_.count(tod))
            return true; // contains exact value

        //    Return iterator to lower bound
        //    Returns an iterator pointing to the first element in the container whose key is not considered to go
        //    before k (i.e., either it is equivalent or goes after).

        auto lb_it = timed_indexes_.lower_bound(tod);

        if (lb_it == timed_indexes_.end())
            return false;

        assert (lb_it->first >= tod);

        if (lb_it->first - tod > d_max)
            return false; // too much time difference

        // save value
        float upper = lb_it->first;

        // TODO lb_it--; ?
        while (lb_it != timed_indexes_.end() && tod < lb_it->first)
        {
            if (lb_it == timed_indexes_.begin()) // exit condition on first value
            {
                if (tod < lb_it->first) // set as not found
                    lb_it = timed_indexes_.end();

                break;
            }

            lb_it--;
        }

        if (lb_it == timed_indexes_.end())
            return false;

        assert (tod >= lb_it->first);

        if (tod - lb_it->first > d_max)
            return false; // too much time difference

        float lower = lb_it->first;

        logdbg << "Target " << utn_ << ": hasDataForTime: found " << String::timeStringFromDouble(lower)
               << " <= " << String::timeStringFromDouble(tod)
               << " <= " << String::timeStringFromDouble(upper);

        return true;
    }

    std::pair<float, float> Target::timesFor (float tod, float d_max) const // lower/upper times, -1 if not existing
    {
        if (timed_indexes_.count(tod))
            return {tod, -1}; // contains exact value

        //    Return iterator to lower bound
        //    Returns an iterator pointing to the first element in the container whose key is not considered to go
        //    before k (i.e., either it is equivalent or goes after).

        auto lb_it = timed_indexes_.lower_bound(tod);

        if (lb_it == timed_indexes_.end())
            return {-1, -1};

        assert (lb_it->first >= tod);

        if (lb_it->first - tod > d_max)
            return {-1, -1}; // too much time difference

        // save value
        float upper = lb_it->first;

        // TODO lb_it--; ?
        while (lb_it != timed_indexes_.end() && tod < lb_it->first)
        {
            if (lb_it == timed_indexes_.begin()) // exit condition on first value
            {
                if (tod < lb_it->first) // set as not found
                    lb_it = timed_indexes_.end();

                break;
            }

            lb_it--;
        }

        if (lb_it == timed_indexes_.end())
            return {-1, upper};

        assert (tod >= lb_it->first);

        if (tod - lb_it->first > d_max)
            return {-1, upper}; // too much time difference

        float lower = lb_it->first;

        return {lower, upper};
    }

    std::pair<EvaluationTargetPosition, bool> Target::interpolatedPosForTime (float tod, float d_max) const
    {
        float lower, upper;

        tie(lower, upper) = timesFor(tod, d_max);

        if (lower == -1 || upper == -1)
            return {{}, false};

        EvaluationTargetPosition pos1 = posForExactTime(lower);
        EvaluationTargetPosition pos2 = posForExactTime(upper);
        float d_t = upper - lower;

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

        OGRSpatialReference wgs84;
        wgs84.SetWellKnownGeogCS("WGS84");
        OGRSpatialReference local;
        local.SetStereographic(pos1.latitude_, pos1.longitude_, 1.0, 0.0, 0.0);

        logdbg << "Target: interpolatedPosForTime: pos1 " << pos1.latitude_ << ", " << pos1.longitude_;
        logdbg << "Target: interpolatedPosForTime: pos2 " << pos2.latitude_ << ", " << pos2.longitude_;

        std::unique_ptr<OGRCoordinateTransformation> ogr_geo2cart {OGRCreateCoordinateTransformation(&wgs84, &local)};
        assert (ogr_geo2cart);
        std::unique_ptr<OGRCoordinateTransformation> ogr_cart2geo {OGRCreateCoordinateTransformation(&local, &wgs84)};
        assert (ogr_cart2geo);

        double x_pos, y_pos;

        if (in_appimage_) // inside appimage
        {
            x_pos = pos2.longitude_;
            y_pos = pos2.latitude_;
        }
        else
        {
            x_pos = pos2.latitude_;
            y_pos = pos2.longitude_;
        }

        logdbg << "Target: interpolatedPosForTime: geo2cart";
        bool ret = ogr_geo2cart->Transform(1, &x_pos, &y_pos); // wgs84 to cartesian offsets
        if (!ret)
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

        float d_t2 = tod - lower;
        logdbg << "Target: interpolatedPosForTime: d_t2 " << d_t2;

        assert (d_t2 >= 0);

        x_pos = v_x * d_t2;
        y_pos = v_y * d_t2;

        logdbg << "Target: interpolatedPosForTime: interpolated offsets x " << x_pos << " y " << y_pos;

        ret = ogr_cart2geo->Transform(1, &x_pos, &y_pos);

        // x_pos long, y_pos lat

        logdbg << "Target: interpolatedPosForTime: interpolated lat " << y_pos << " long " << x_pos;

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

        if (in_appimage_) // inside appimage
            return {{y_pos, x_pos, has_altitude, altitude}, true};
        else
            return {{x_pos, y_pos, has_altitude, altitude}, true};
    }

    bool Target::hasDataForExactTime (float tod) const
    {
        return timed_indexes_.count(tod);
    }

    TargetReport& Target::dataForExactTime (float tod) const
    {
        assert (hasDataForExactTime(tod));;
        assert (assoc_trs_.size() > timed_indexes_.at(tod));
        return *assoc_trs_.at(timed_indexes_.at(tod));
    }

    EvaluationTargetPosition Target::posForExactTime (float tod) const
    {
        assert (hasDataForExactTime(tod));

        TargetReport& tr = dataForExactTime(tod);

        EvaluationTargetPosition pos;

        pos.latitude_ = tr.latitude_;
        pos.longitude_ = tr.longitude_;
        pos.has_altitude_ = tr.has_mc_;
        pos.altitude_ = tr.mc_;

        return pos;
    }

    bool Target::timeOverlaps (Target& other) const
    {
        if (!assoc_trs_.size() || !other.assoc_trs_.size())
            return false;

        //a.start < b.end && b.start < a.end;
        return tod_min_ < other.tod_max_ && other.tod_min_ < tod_max_;
    }

    std::tuple<vector<float>, vector<float>, vector<float>> Target::compareModeACodes (Target& other) const
    {
        vector<float> unknown;
        vector<float> same;
        vector<float> different;

        float lower, upper;

        float tod;

        for (auto tr_it : assoc_trs_)
        {
            tod = tr_it->tod_;

            if (!other.hasDataForTime(tod, 15.0))
            {
                unknown.push_back(tod);
                continue;
            }

            tie(lower, upper) = other.timesFor(tod, 15.0);

            if (lower == -1 && upper == -1)
            {
                unknown.push_back(tod);
                continue;
            }

            if ((lower == -1 && upper != -1)
                    || (lower != -1 && upper == -1)) // only 1
            {
                TargetReport& ref1 = (lower != -1) ? other.dataForExactTime(lower)
                                                   : other.dataForExactTime(upper);

                if (!tr_it->has_ma_)
                {
                    if (!ref1.has_ma_ ) // both have no mode a
                    {
                        same.push_back(tod);
                        continue;
                    }
                    else
                    {
                        different.push_back(tod); // no mode a here, but in other
                        continue;
                    }
                }

                // mode a exists
                if (!ref1.has_ma_)
                {
                    different.push_back(tod); // mode a here, but none in other
                    continue;
                }

                if ((ref1.has_ma_ && ref1.ma_ == tr_it->ma_)) // is same
                {
                    same.push_back(tod);
                    continue;
                }
                else
                    different.push_back(tod);

               continue;
            }

            // both set
            assert (lower != -1);
            assert (other.hasDataForExactTime(lower));
            TargetReport& ref1 = other.dataForExactTime(lower);

            assert (upper != -1);
            assert (other.hasDataForExactTime(upper));
            TargetReport& ref2 = other.dataForExactTime(upper);

            if (!tr_it->has_ma_)
            {
                if (!ref1.has_ma_ || !ref2.has_ma_) // both have no mode a
                {
                    same.push_back(tod);
                    continue;
                }
                else
                {
                    different.push_back(tod); // no mode a here, but in other
                    continue;
                }
            }

            // mode a exists
            if (!ref1.has_ma_ && !ref2.has_ma_)
            {
                different.push_back(tod); // mode a here, but none in other
                continue;
            }

            if ((ref1.has_ma_ && ref1.ma_ == tr_it->ma_)
                    || (ref2.has_ma_ && ref2.ma_ == tr_it->ma_)) // one of them is same
            {
                same.push_back(tod);
                continue;
            }
            else
                different.push_back(tod);
        }

        assert (assoc_trs_.size() == unknown.size()+same.size()+different.size());

        return {unknown, same, different};
    }

}
