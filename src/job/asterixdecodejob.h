#ifndef ASTERIXDECODEJOB_H
#define ASTERIXDECODEJOB_H

#include "job.h"
#include "json.hpp"

class ASTERIXImporterTask;

class ASTERIXDecodeJob : public Job
{
    Q_OBJECT
signals:
    void decodedASTERIXSignal (std::shared_ptr<nlohmann::json> data);

public:
    ASTERIXDecodeJob(ASTERIXImporterTask& task, const std::string& filename, const std::string& framing, bool test);
    virtual ~ASTERIXDecodeJob() {}

    virtual void run ();

    size_t numFrames() const;
    size_t numRecords() const;

    void pause() { pause_ = true; }
    void unpause() { pause_ = false; }

private:
    ASTERIXImporterTask& task_;
    std::string filename_;
    std::string framing_;
    bool test_ {false};

    volatile bool pause_{false};

    size_t num_frames_{0};
    size_t num_records_{0};

    void jasterix_callback(nlohmann::json& data, size_t num_frames, size_t num_records);
};

#endif // ASTERIXDECODEJOB_H
