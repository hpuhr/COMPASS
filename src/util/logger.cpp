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
#include "logger.h"
#include "event_log.h"

//#include "config.h"
//#include "log4cpp/BasicLayout.hh"
//#include "log4cpp/FileAppender.hh"
//#include "log4cpp/Layout.hh"
//#include "log4cpp/OstreamAppender.hh"
//#include "log4cpp/Priority.hh"
#include "log4cpp/PropertyConfigurator.hh"
//#include "log4cpp/AppenderSkeleton.hh"
//#include "log4cpp/LoggingEvent.hh"

#include <boost/thread.hpp>

//#define LOGGER_FIXED_LEVEL logINFO

#define MAX_EVENTS_PER_CATEGORY 1000

Logger::Logger() : console_appender_(0), file_appender_(0), event_log_(new logger::EventLog(MAX_EVENTS_PER_CATEGORY)) {}

void Logger::init(const std::string& log_config_filename, bool enable_event_log)
{
#ifdef LOGGER_FIXED_LEVEL
    log4cpp::Appender* console_appender_ = new log4cpp::OstreamAppender("console", &std::cout);
    console_appender_->setLayout(new log4cpp::BasicLayout());

    log4cpp::Appender* file_appender_ = new log4cpp::FileAppender("default", "log.txt");
    file_appender_->setLayout(new log4cpp::BasicLayout());

    log4cpp::Category& root = log4cpp::Category::getRoot();
    root.setPriority(log4cpp::Priority::INFO);
    root.addAppender(console_appender_);
    root.addAppender(file_appender_);
#else
    log4cpp::PropertyConfigurator::configure(log_config_filename);

    if (enable_event_log)
    {
        log4cpp::Category::getRoot().addAppender(new logger::EventAppender("events", event_log_));
    }
#endif
}

Logger::~Logger()
{
    if (console_appender_)
    {
        delete console_appender_;
        console_appender_ = 0;
    }
    if (file_appender_)
    {
        delete file_appender_;
        file_appender_ = 0;
    }

    delete event_log_;
    event_log_ = nullptr;
}

/**
 * Retuns the event log.
 */
const logger::EventLog* Logger::getEventLog() const
{
    return event_log_;
}
