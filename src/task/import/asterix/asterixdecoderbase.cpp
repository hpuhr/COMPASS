#include "asterixdecoderbase.h"


ASTERIXDecoderBase::ASTERIXDecoderBase(
        ASTERIXDecodeJob& job, ASTERIXImportTask& task, const ASTERIXImportTaskSettings& settings)
    : job_(job), task_(task), settings_(settings)
{
}

ASTERIXDecoderBase::~ASTERIXDecoderBase()
{

}

size_t ASTERIXDecoderBase::numErrors() const
{
    return num_errors_;
}

bool ASTERIXDecoderBase::error() const
{
    return error_;
}

std::string ASTERIXDecoderBase::errorMessage() const
{
    return error_message_;
}







