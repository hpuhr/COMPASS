#ifndef ASTERIXFILEDECODER_H
#define ASTERIXFILEDECODER_H

#include "asterixdecoderbase.h"
#include "asteriximporttask.h"

#include <boost/date_time/posix_time/posix_time.hpp>

class ASTERIXFileDecoder : public ASTERIXDecoderBase
{
public:
    ASTERIXFileDecoder(ASTERIXDecodeJob& job, ASTERIXImportTask& task, const ASTERIXImportTaskSettings& settings);
    virtual ~ASTERIXFileDecoder();

    virtual void start() override;
    virtual void stop() override;

    virtual bool hasStatusInfo() override { return true; };
    virtual std::string statusInfoString() override;
    virtual float statusInfoProgress() override; // percent

private:
    unsigned int current_file_count_ {0};
    std::vector<ASTERIXFileInfo> files_info_;

    size_t total_file_size_{0};
    size_t done_files_total_size_{0};
    size_t current_file_max_index_{0};

     boost::posix_time::ptime start_time_;

    size_t num_records_total_ {0};

    float getRecordsPerSecond() const;
    float getRemainingTime() const;

    bool hasCurrentFileToDo(); // still something to decode
    void doCurrentFile();
    std::string getCurrentFilename();
};

#endif // ASTERIXFILEDECODER_H
