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

    void setFrom(nlohmann::json& json);
    nlohmann::json asJSON() const;
    std::string asString() const;

    const Utils::TimeWindow& get(unsigned int index);

    // later functions save to json_ptr_ if available
    void add(const TimeWindow& time_window);
    void erase(unsigned int index);
    void clear();

    unsigned int size() const { return time_windows_.size(); }

protected:
    std::vector<Utils::TimeWindow> time_windows_;
};

}

