#ifndef ASTERIXDECODERBASE_H
#define ASTERIXDECODERBASE_H

#include <string>

class ASTERIXImportTask;
class ASTERIXDecodeJob;
class ASTERIXImportTaskSettings;

class ASTERIXDecoderBase
{
public:
    ASTERIXDecoderBase(ASTERIXDecodeJob& job, ASTERIXImportTask& task, const ASTERIXImportTaskSettings& settings);
    virtual ~ASTERIXDecoderBase();

    virtual void start() = 0;
    virtual void stop() = 0;

    size_t numErrors() const;

    bool error() const;
    std::string errorMessage() const;

    virtual bool hasStatusInfo() { return false; };
    virtual std::string statusInfoString() { return ""; }
    virtual float statusInfoProgress() { return 0; } // percent

protected:
    ASTERIXDecodeJob& job_;
    ASTERIXImportTask& task_;
    const ASTERIXImportTaskSettings& settings_;

    bool running_ {false};

    size_t num_errors_{0};

    bool error_{false};
    std::string error_message_;

};

#endif // ASTERIXDECODERBASE_H
