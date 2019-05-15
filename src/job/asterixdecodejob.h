#ifndef ASTERIXDECODEJOB_H
#define ASTERIXDECODEJOB_H

#include "job.h"
#include "json.hpp"

class ASTERIXImporterTask;

class ASTERIXDecodeJob : public Job
{
public:
    ASTERIXDecodeJob(ASTERIXImporterTask& task, const std::string& filename, const std::string& framing, bool test);
    virtual ~ASTERIXDecodeJob() {}

    virtual void run ();

    size_t numFrames() const;
    size_t numRecords() const;
    std::map<unsigned int, size_t> categoryCounts() const;

private:
    ASTERIXImporterTask& task_;
    std::string filename_;
    std::string framing_;
    bool test_ {false};

    size_t num_frames_{0};
    size_t num_records_{0};

    std::map<unsigned int, size_t> category_counts_;

    void jasterix_callback(nlohmann::json& data, size_t num_frames, size_t num_records);
};

#endif // ASTERIXDECODEJOB_H
