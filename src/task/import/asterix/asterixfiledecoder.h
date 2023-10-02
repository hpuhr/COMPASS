#ifndef ASTERIXFILEDECODER_H
#define ASTERIXFILEDECODER_H

#include "asterixdecoderbase.h"

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

    //    float getFileDecodingProgress() const;
    //    float getRecordsPerSecond() const;
    //    float getRemainingTime() const;

private:

    size_t file_size_{0};
    size_t max_index_{0};
};

#endif // ASTERIXFILEDECODER_H
