#ifndef TIMEPERIOD_H
#define TIMEPERIOD_H

#include "stringconv.h"

#include <vector>
#include <cassert>

#include <math.h>

class TimePeriod
{
public:
    TimePeriod (float begin, float end)
        : begin_(begin), end_(end)
    {
        assert (end_ >= begin_);
    }

    bool isInside (float time) const
    {
        return (begin_ <= time) && (time <= end_);
    }

    bool isCloseToEnd (float time, float max_time_d) const
    {
        assert (time >= end_);
        return fabs(time-end_) <= max_time_d;
    }

    //    void addToBegin (float time)
    //    {
    //        begin_ = time;
    //    }

    void extend (float time)
    {
        assert (time >= end_);
        end_ = time;
    }

    float begin() const
    {
        return begin_;
    }

    float end() const
    {
        return end_;
    }

    float duration() const
    {
        return end_ - begin_;
    }

    std::string str() const
    {
        return "["+Utils::String::timeStringFromDouble(begin_)+","+Utils::String::timeStringFromDouble(end_)+"]";
    }

protected:
    float begin_ {0};
    float end_ {0};
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

    bool isInside (float time)
    {
        for (const auto& period_it : periods_)
            if (period_it.isInside(time))
                return true;

        return false;
    }

    unsigned int getPeriodIndex (float time)
    {
        for (unsigned int cnt=0; cnt < periods_.size(); ++cnt)
            if (periods_.at(cnt).isInside(time))
                return cnt;

        throw std::runtime_error("TimePeriodCollection: time "+std::to_string(time)+" not inside");
    }

    int getPeriodMaxIndexBefore (float time) // -1 if none
    {
        int index {-1};

        for (unsigned int cnt=0; cnt < periods_.size(); ++cnt)
            if (periods_.at(cnt).end() < time)
                index = cnt;

        return index;
    }

    unsigned int size() { return periods_.size(); }

    float totalBegin()
    {
        assert (periods_.size());
        return periods_.at(0).begin();
    }

    float totalEnd()
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

    void removeSmallPeriods (float min_duraiton)
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
            sum_uis += floor(period_it.duration()/update_interval);

        return sum_uis;
    }

protected:
    std::vector<TimePeriod> periods_;
};


#endif // TIMEPERIOD_H


