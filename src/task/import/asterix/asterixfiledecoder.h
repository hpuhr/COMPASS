#ifndef ASTERIXFILEDECODER_H
#define ASTERIXFILEDECODER_H

#include "asterixdecoderbase.h"

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

    size_t file_size_{0};
    size_t max_index_{0};

     boost::posix_time::ptime start_time_;

    size_t num_records_total_ {0};

    float getRecordsPerSecond() const;
    float getRemainingTime() const;

};

#endif // ASTERIXFILEDECODER_H
