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

#pragma once

#include "log4cpp/AppenderSkeleton.hh"
#include "log4cpp/Category.hh"

#include "json.h"

#include <boost/optional.hpp>
#include <boost/thread.hpp>
#include <boost/circular_buffer.hpp>

#include <vector>
#include <map>
#include <queue>

namespace logger
{

/**
 * Logger event.
 */
struct Event
{
    bool consume()
    {
        if (!fresh)
            return false;
        fresh = false;
        return true;
    }

    bool        fresh = true;
    uint32_t    id;
    int         timestamp;
    std::string message;
};

/**
 * Query for obtaining logger events.
 */
struct EventQuery
{
    enum class Type
    {
        All = 0,
        Newest
    };

    EventQuery(bool q_fresh_only = false,
               Type q_type = Type::All, 
               size_t q_num_items = 10) : fresh_only(q_fresh_only), type(q_type), num_items(q_num_items) {}

    bool   fresh_only;
    Type   type;
    size_t num_items;
};

/**
 * Threadsafe event log.
 */
class EventLog
{
public:
    typedef std::vector<Event>                           Events;
    typedef std::map<int, Events>                        EventMap;
    typedef std::map<int, boost::circular_buffer<Event>> EventMapInternal;
    
    EventLog(size_t max_events_per_category);
    virtual ~EventLog() = default;

    void addEvent(const Event& evt, int priority);

    EventMap getEvents(const EventQuery& query = EventQuery()) const;
    nlohmann::json getEventsAsJSON(const EventQuery& query = EventQuery()) const;

private:
    mutable EventMapInternal events_;
    mutable boost::mutex     event_mutex_;

    size_t max_events_;

    static uint32_t          event_ids_;
};

/**
*/
class EventAppender : public log4cpp::AppenderSkeleton
{
public:
    EventAppender(const std::string& name, EventLog* event_log) : log4cpp::AppenderSkeleton(name), event_log_(event_log) {}
    virtual ~EventAppender() = default;

    virtual bool reopen() override { return true; }
    virtual void close() override {}
    virtual bool requiresLayout() const override { return false; }
    virtual void setLayout(log4cpp::Layout* layout) override {}

protected:
    virtual void _append(const log4cpp::LoggingEvent& event) override;

private:
    EventLog* event_log_ = nullptr;
    
};

} // namespace logger
