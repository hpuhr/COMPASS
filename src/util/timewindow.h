#pragma once

#include "json.hpp"

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

    const boost::posix_time::ptime& begin() const;
    const boost::posix_time::ptime& end() const;

protected:
    std::pair<boost::posix_time::ptime,boost::posix_time::ptime> time_window_;
};

class TimeWindowCollection
{
public:
    TimeWindowCollection();

    bool valid() const;

    void setFrom(nlohmann::json& json); // saves as json_ptr_, auto stores changes!
    nlohmann::json asJSON() const;

    const Utils::TimeWindow& get(unsigned int index);

    // later functions save to json_ptr_ if available
    void add(const TimeWindow& time_window);
    void erase(unsigned int index);

    unsigned int size() const { return time_windows_.size(); }

protected:
    nlohmann::json* json_ptr_{nullptr};

    std::vector<Utils::TimeWindow> time_windows_;
};

}

