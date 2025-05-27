#pragma once

#include "json.hpp"

#include <QObject>

#include <vector>

#include "boost/date_time/posix_time/posix_time.hpp"

namespace Utils
{

class TimeWindow
{
public:
    TimeWindow();
    TimeWindow(const boost::posix_time::ptime& begin, const boost::posix_time::ptime& end);

    bool valid() const;

    void setFrom(const nlohmann::json& json);
    nlohmann::json getAsJson() const;
    std::string asStr() const;

    bool contains(const boost::posix_time::ptime& ts) const;
    bool contains(const TimeWindow& tw) const;

    const boost::posix_time::ptime& begin() const;
    const boost::posix_time::ptime& end() const;

protected:
    std::pair<boost::posix_time::ptime,boost::posix_time::ptime> time_window_;
};

// changes have to be stored manually
class TimeWindowCollection
{
public:
    TimeWindowCollection();
    virtual ~TimeWindowCollection()=default;

    bool valid() const;
    bool contains(const boost::posix_time::ptime& ts) const;

    void setFrom(nlohmann::json& json);
    nlohmann::json asJSON() const;
    std::string asString() const;

    const Utils::TimeWindow& get(unsigned int index);

    // later functions save to json_ptr_ if available
    void add(const TimeWindow& time_window);
    bool contains(const TimeWindow& time_window);
    void erase(unsigned int index);
    void clear();


    unsigned int size() const { return time_windows_.size(); }

    // Iterators for range-based for loop support (C++11 compatible)
    std::vector<Utils::TimeWindow>::iterator begin() { return time_windows_.begin(); }
    std::vector<Utils::TimeWindow>::iterator end() { return time_windows_.end(); }

    std::vector<Utils::TimeWindow>::const_iterator begin() const { return time_windows_.cbegin(); }
    std::vector<Utils::TimeWindow>::const_iterator end() const { return time_windows_.cend(); }

    std::vector<Utils::TimeWindow>::const_iterator cbegin() const { return time_windows_.cbegin(); }
    std::vector<Utils::TimeWindow>::const_iterator cend() const { return time_windows_.cend(); }


protected:
    std::vector<Utils::TimeWindow> time_windows_;
};

}

