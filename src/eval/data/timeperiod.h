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

#ifndef TIMEPERIOD_H
#define TIMEPERIOD_H

#include "stringconv.h"
#include "util/timeconv.h"

#include "boost/date_time/posix_time/ptime.hpp"
#include "boost/date_time/time_duration.hpp"

#include <vector>
#include <cassert>

#include <math.h>

class TimePeriod
{
public:
    TimePeriod (boost::posix_time::ptime begin, boost::posix_time::ptime end)
        : begin_(begin), end_(end)
    {
        assert (end_ >= begin_);
    }

    bool isInside (boost::posix_time::ptime time) const
    {
        return (begin_ <= time) && (time <= end_);
    }

    bool isCloseToEnd (boost::posix_time::ptime time, boost::posix_time::time_duration max_time_d) const
    {
        assert (time >= end_);
        return (time-end_).abs() <= max_time_d;
    }

    //    void addToBegin (float time)
    //    {
    //        begin_ = time;
    //    }

    void extend (boost::posix_time::ptime time)
    {
        assert (time >= end_);
        end_ = time;
    }

    boost::posix_time::ptime begin() const
    {
        return begin_;
    }

    boost::posix_time::ptime end() const
    {
        return end_;
    }

    boost::posix_time::time_duration duration() const
    {
        return end_ - begin_;
    }

    std::string str() const
    {
        return "["+Utils::Time::toString(begin_)+","+Utils::Time::toString(end_)+"]";
    }

protected:
    boost::posix_time::ptime begin_;
    boost::posix_time::ptime end_;
};

class TimePeriodCollection
{
public:
    TimePeriodCollection()
    {}

    void add (TimePeriod&& period)
    {
        if (periods_.size())
            assert (periods_.rbegin()->end() < period.begin());

        periods_.push_back(period);
    }

    bool isInside (boost::posix_time::ptime time)
    {
        for (const auto& period_it : periods_)
            if (period_it.isInside(time))
                return true;

        return false;
    }

    unsigned int getPeriodIndex (boost::posix_time::ptime time)
    {
        for (unsigned int cnt=0; cnt < periods_.size(); ++cnt)
            if (periods_.at(cnt).isInside(time))
                return cnt;

        throw std::runtime_error("TimePeriodCollection: time "+Utils::Time::toString(time)+" not inside");
    }

    int getPeriodMaxIndexBefore (boost::posix_time::ptime time) // -1 if none
    {
        int index {-1};

        for (unsigned int cnt=0; cnt < periods_.size(); ++cnt)
            if (periods_.at(cnt).end() < time)
                index = cnt;

        return index;
    }

    unsigned int size() { return periods_.size(); }

    using TimePeriodIterator =
    typename std::vector<TimePeriod>::iterator;
    TimePeriodIterator begin() { return periods_.begin(); }
    TimePeriodIterator end() { return periods_.end(); }

    boost::posix_time::ptime totalBegin()
    {
        assert (periods_.size());
        return periods_.at(0).begin();
    }

    boost::posix_time::ptime totalEnd()
    {
        assert (periods_.size());
        return periods_.rbegin()->end();
    }

    TimePeriod& period (unsigned int index)
    {
        assert (index < periods_.size());
        return periods_.at(index);
    }

    TimePeriod& lastPeriod()
    {
        assert (periods_.size());
        return *periods_.rbegin();
    }

    std::string print(bool line_breaks=false)
    {
        std::string tmp;

        for (auto& period_it : periods_)
        {
            if (tmp.size())
            {
                if (line_breaks)
                    tmp += ",\n";
                else
                    tmp += ", ";
            }

            tmp += period_it.str();
        }

        return tmp;
    }

    void removeSmallPeriods (boost::posix_time::time_duration min_duraiton)
    {
        std::vector<TimePeriod> tmp_periods;

        for (auto& period_it : periods_)
        {
            if (period_it.duration() >= min_duraiton)
                tmp_periods.push_back(period_it);
        }

        periods_ = tmp_periods;
    }

    unsigned int getUIs (float update_interval)
    {
        unsigned int sum_uis = 0;

        for (auto& period_it : periods_)
            sum_uis += floor(Utils::Time::partialSeconds(period_it.duration())/update_interval);

        return sum_uis;
    }

protected:
    std::vector<TimePeriod> periods_;
};


#endif // TIMEPERIOD_H


