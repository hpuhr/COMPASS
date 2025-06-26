#pragma once

#include <boost/date_time/posix_time/ptime.hpp>

#include <map>
#include <memory>

class Buffer;

class ASTERIXTimestampCalculator
{
public:
    ASTERIXTimestampCalculator();

    void setBuffers(std::map<std::string, std::shared_ptr<Buffer>> buffers);
    void calculate(std::string source_name,
                   boost::posix_time::ptime date, bool reset_date_between_files,
                   bool override_tod_active, float override_tod_offset,
                   bool ignore_time_jumps, bool do_timestamp_checks);
    bool processing() const {return processing_; }
    void setProcessingDone();
    void reset();

    void resetDateInfo();
    void clearTimeStats();
    void logLastTimestamp();

    bool first_time_{true};
    boost::posix_time::ptime timestamp_first_;
    float tod_first_{0};
    boost::posix_time::ptime timestamp_last_;
    float tod_last_{0};
    float last_reported_tod_{-3600};

    std::map<std::string, std::shared_ptr<Buffer>> buffers();

private:
    bool processing_{false};
    std::map<std::string, std::shared_ptr<Buffer>> buffers_;

    // timestamp / timejump handling
    std::string prev_source_name_;

    bool current_date_set_{false};
    boost::posix_time::ptime current_date_;
    boost::posix_time::ptime previous_date_;
    bool did_recent_time_jump_{false}; // indicator if recently a time jump was performed
    bool had_late_time_{false}; // indicator if time late before 24h mark occured

    void doADSBTimeProcessing();
    void doTodOverride(float override_tod_offset);
    void doFutureTimestampsCheck();
    void doTimeStampCalculation(bool ignore_time_jumps);
};


