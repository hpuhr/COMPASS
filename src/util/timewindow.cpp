#include "timewindow.h"
#include "timeconv.h"

using namespace std;

namespace Utils
{

TimeWindow::TimeWindow() {}

TimeWindow::TimeWindow(const boost::posix_time::ptime& begin, const boost::posix_time::ptime& end)
{
    std::get<0>(time_window_) = begin;
    std::get<1>(time_window_) = end;

    assert (valid());
}

bool TimeWindow::valid() const
{
    if (std::get<0>(time_window_).is_not_a_date_time())
        return false;

    if (std::get<1>(time_window_).is_not_a_date_time())
        return false;

    return std::get<0>(time_window_) <= std::get<1>(time_window_);
}

void TimeWindow::setFrom(const nlohmann::json& json)
{
    assert (json.is_array());
    assert (json.size() == 2);

    assert (json.at(0).is_string());
    assert (json.at(1).is_string());

    time_window_ = {{},{}};

    std::get<0>(time_window_) = Time::fromString(json.at(0));
    std::get<1>(time_window_) = Time::fromString(json.at(1));

    assert (valid());
}

nlohmann::json TimeWindow::getAsJson() const
{
    assert (valid());

    nlohmann::json json_result = nlohmann::json::array();

    json_result.push_back(Time::toString(std::get<0>(time_window_)));
    json_result.push_back(Time::toString(std::get<1>(time_window_)));

    return json_result;
}

std::string TimeWindow::asStr() const
{
    return Time::toString(std::get<0>(time_window_)) + " - " + Time::toString(std::get<1>(time_window_));
}

const boost::posix_time::ptime& TimeWindow::begin() const
{
    return std::get<0>(time_window_);
}
const boost::posix_time::ptime& TimeWindow::end() const
{
    return std::get<1>(time_window_);
}


// --------------------

TimeWindowCollection::TimeWindowCollection()
{}

bool TimeWindowCollection::valid() const
{
    for (const auto& time_window : time_windows_)
        if (!time_window.valid())
            return false;

    return true;
}

void TimeWindowCollection::setFrom(nlohmann::json& json)
{
    time_windows_.clear();
    assert (json.is_array());

    for (const auto& time_window_array : json)
    {
        assert (time_window_array.is_array());

        TimeWindow time_window;

        time_window.setFrom(time_window_array);
        assert (time_window.valid());


        time_windows_.push_back(std::move(time_window));
    }

    assert (valid());
}

nlohmann::json TimeWindowCollection::asJSON() const
{
    nlohmann::json json_result = nlohmann::json::array();

    for (const auto& time_window : time_windows_)
        json_result.push_back(time_window.getAsJson());

    return json_result;
}

std::string TimeWindowCollection::asString() const
{
    ostringstream ss;

    for (const auto& time_window : time_windows_)
        ss << time_window.asStr() << endl;

    return ss.str();
}

const Utils::TimeWindow& TimeWindowCollection::get(unsigned int index)
{
    assert (index < time_windows_.size());
    return time_windows_.at(index);
}


void TimeWindowCollection::add(const TimeWindow& time_window)
{
    assert (time_window.valid());
    time_windows_.push_back(time_window);

    emit changedSignal();
}

void TimeWindowCollection::erase(unsigned int index)
{
    assert (index < time_windows_.size());
    time_windows_.erase(time_windows_.begin() + index);

    emit changedSignal();
}

void TimeWindowCollection::clear()
{
    time_windows_.clear();
}

}


