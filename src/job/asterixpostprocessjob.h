#ifndef ASTERIXPOSTPROCESSJOB_H
#define ASTERIXPOSTPROCESSJOB_H

#include "job.h"

#include <memory>
#include <map>


class Buffer;

class ASTERIXPostprocessJob : public Job
{
public:
    ASTERIXPostprocessJob(std::map<std::string, std::shared_ptr<Buffer>> buffers, bool do_timestamp_checks);

    virtual ~ASTERIXPostprocessJob();

    virtual void run() override;

    std::map<std::string, std::shared_ptr<Buffer>> buffers() { return std::move(buffers_); }

private:
    std::map<std::string, std::shared_ptr<Buffer>> buffers_;
    bool do_timestamp_checks_;

    float network_time_offset_ {0};

    void doFutureTimestampsCheck();
    void doRadarPlotPositionCalculations();
};

#endif // ASTERIXPOSTPROCESSJOB_H
