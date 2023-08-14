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

#include "config.h"
#include "log4cpp/BasicLayout.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/Layout.hh"
#include "log4cpp/OstreamAppender.hh"
#include "log4cpp/Priority.hh"
#include "log4cpp/PropertyConfigurator.hh"
#include "log4cpp/AppenderSkeleton.hh"
#include "log4cpp/LoggingEvent.hh"

//#define LOGGER_FIXED_LEVEL logINFO

Logger::Logger() : console_appender_(0), file_appender_(0) {}

namespace log_tools
{
    /**
     * Adds events to the logger events where they can be browsed as needed.
     */
    class EventAppender : public log4cpp::AppenderSkeleton
    {
    public:
        EventAppender(const std::string& name, Logger::Events* events) : log4cpp::AppenderSkeleton(name), events_(events) {}
        virtual ~EventAppender() {}

        virtual bool reopen() override { return true; }
        virtual void close() override {}
        virtual bool requiresLayout() const override { return false; }
        virtual void setLayout(log4cpp::Layout* layout) override {}

    protected:
        virtual void _append(const log4cpp::LoggingEvent& event) override
        {
            if (events_ && (event.priority == log4cpp::Priority::WARN ||
                            event.priority == log4cpp::Priority::ERROR))
            {
                //std::cout << "adding type " << event.priority << " log message '" << event.message << "' to archive" << std::endl;

                Logger::Event evt;
                evt.id        = event_ids_++;
                evt.timestamp = event.timeStamp.getSeconds();
                evt.message   = event.message;

                (*events_)[ event.priority ].push_back(evt);
            }
        }

    private:
        Logger::Events* events_    = nullptr;
        uint32_t        event_ids_ = 0;
    };
}

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
        log4cpp::Category::getRoot().addAppender(new log_tools::EventAppender("events", &events_));
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
}

/**
 * Retuns all collected events. 
 */
const Logger::Events& Logger::getEvents() const 
{ 
    return events_; 
}

/**
 * Returns all events collected since the last call to getFreshEvents().
 */
Logger::Events Logger::getFreshEvents() const
{
    Events events;

    for (auto& priority_events : events_)
        for (auto& evt : priority_events.second)
            if (evt.consume())
                events[priority_events.first].push_back(evt);

    return events;
}
