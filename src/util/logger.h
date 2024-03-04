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

#include "singleton.h"

#include "log4cpp/Appender.hh"
#include "log4cpp/Category.hh"

#include <cstdint>

#define logerr log4cpp::Category::getRoot().errorStream()
#define logwrn log4cpp::Category::getRoot().warnStream()
#define loginf log4cpp::Category::getRoot().infoStream()
//#define logdbg log4cpp::Category::getRoot().debugStream()
#define logdbg \
    if (false) \
    log4cpp::Category::getRoot().debugStream()  // for improved performance

namespace logger
{
    class EventLog;
}

/**
 * @brief Thread-safe logger
 *
 * Uses log4cpp.
 */
class Logger : public Singleton
{
  public:
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

    typedef std::map<int, std::vector<Event>> Events;

    const logger::EventLog* getEventLog() const;

  protected:
    static Logger* log_instance_;
    log4cpp::Appender* console_appender_;
    log4cpp::Appender* file_appender_;

    logger::EventLog* event_log_ = nullptr;

    Logger();

  public:
    static Logger& getInstance()
    {
        static Logger instance;
        return instance;
    }

    void init(const std::string& log_config_filename, bool enable_event_log = false);

    virtual ~Logger();
};

