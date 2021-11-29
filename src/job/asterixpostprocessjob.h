#ifndef ASTERIXPOSTPROCESSJOB_H
#define ASTERIXPOSTPROCESSJOB_H

#include "job.h"

#include <memory>
#include <map>


class Buffer;

class ASTERIXPostprocessJob : public Job
{
public:
    ASTERIXPostprocessJob(std::map<std::string, std::shared_ptr<Buffer>> buffers);

    virtual ~ASTERIXPostprocessJob();

    virtual void run() override;

    std::map<std::string, std::shared_ptr<Buffer>> buffers() { return std::move(buffers_); }

private:
    std::map<std::string, std::shared_ptr<Buffer>> buffers_;
};

#endif // ASTERIXPOSTPROCESSJOB_H
