#ifndef ASTERIXPOSTPROCESSJOB_H
#define ASTERIXPOSTPROCESSJOB_H

#include "job.h"

#include "boost/date_time/posix_time/ptime.hpp"

#include <memory>
#include <map>

class Buffer;
class ASTERIXImportTask;

class ASTERIXPostprocessJob : public Job
{
public:
    ASTERIXPostprocessJob(std::map<std::string, std::shared_ptr<Buffer>> buffers,
                          boost::posix_time::ptime date,
                          bool override_tod_active, float override_tod_offset,
                          bool ignore_time_jumps, bool do_timestamp_checks,
                          bool filter_tod_active, float filter_tod_min, float filter_tod_max,
                          bool filter_position_active,
                          float filter_latitude_min, float filter_latitude_max,
                          float filter_longitude_min, float filter_longitude_max,
                          bool filter_modec_active,
                          float filter_modec_min, float filter_modec_max);

    ASTERIXPostprocessJob(std::map<std::string, std::shared_ptr<Buffer>> buffers,
                          boost::posix_time::ptime date); // ctor with no checks/overrides/filters

    virtual ~ASTERIXPostprocessJob();

    virtual void run() override;

    std::map<std::string, std::shared_ptr<Buffer>> buffers() { return std::move(buffers_); }

    static void clearCurrentDate();

private:
    std::map<std::string, std::shared_ptr<Buffer>> buffers_;

    bool override_tod_active_{false};
    double override_tod_offset_{0};

    bool ignore_time_jumps_{false};
    bool do_timestamp_checks_{false};

    bool filter_tod_active_{false};
    float filter_tod_min_{0};
    float filter_tod_max_{0};

    bool filter_position_active_{false};
    float filter_latitude_min_{0};
    float filter_latitude_max_{0};
    float filter_longitude_min_{0};
    float filter_longitude_max_{0};

    bool filter_modec_active_{false};
    float filter_modec_min_{0};
    float filter_modec_max_{0};

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
    void doADSBPositionPorcessing();
    void doGroundSpeedCalculations();
    void doFilters();
};

#endif // ASTERIXPOSTPROCESSJOB_H
