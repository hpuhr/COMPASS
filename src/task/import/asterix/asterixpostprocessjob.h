#ifndef ASTERIXPOSTPROCESSJOB_H
#define ASTERIXPOSTPROCESSJOB_H

#include "job.h"

#include "boost/date_time/posix_time/ptime.hpp"

#include <memory>
#include <map>


class Buffer;

class ASTERIXPostprocessJob : public Job
{
public:
    ASTERIXPostprocessJob(std::map<std::string, std::shared_ptr<Buffer>> buffers,
                          const boost::posix_time::ptime& date,
                          bool override_tod_active, float override_tod_offset, bool do_timestamp_checks);

    virtual ~ASTERIXPostprocessJob();

    virtual void run() override;

    std::map<std::string, std::shared_ptr<Buffer>> buffers() { return std::move(buffers_); }

private:
    std::map<std::string, std::shared_ptr<Buffer>> buffers_;

    bool override_tod_active_{false};
    float override_tod_offset_{0};

    bool do_timestamp_checks_;
    //float network_time_offset_ {0};

    // static vars for timestamp / timejump handling
    static bool current_date_set_;
    static boost::posix_time::ptime current_date_;
    static boost::posix_time::ptime previous_date_;
    static bool did_recent_time_jump_; // indicator if recently a time jump was performed
    static bool had_late_time_; // indicator if time late before 24h mark occured

    void doTodOverride();
    //void doNetworkTimeOverride();
    void doFutureTimestampsCheck();
    void doTimeStampCalculation();
    void doRadarPlotPositionCalculations();
    void doGroundSpeedCalculations();
};

#endif // ASTERIXPOSTPROCESSJOB_H
