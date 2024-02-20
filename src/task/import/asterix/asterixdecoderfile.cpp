/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "asterixdecoderfile.h"

#include "logger.h"

#include "files.h"
#include "stringconv.h"

/**
*/
ASTERIXDecoderFile::ASTERIXDecoderFile(ASTERIXImportSource& source,
                                       ASTERIXImportTask& task, 
                                       const ASTERIXImportTaskSettings& settings)
:   ASTERIXDecoderBase(source, task, settings)
{
    assert(source_.isFileType());
}

/**
*/
ASTERIXDecoderFile::~ASTERIXDecoderFile() = default;

/**
*/
void ASTERIXDecoderFile::start_impl()
{
    total_file_size_         = source_.totalFileSizeInBytes();
    total_records_read_      = 0;
    current_file_bytes_read_ = 0;
    done_file_size_          = 0;
    
    current_file_idx_        = 0;
    
    while (isRunning() && nextFile())
        processCurrentFile();
}

/**
*/
bool ASTERIXDecoderFile::nextFile()
{
    if (atEnd())
        return false;

    ++current_file_idx_;

    return !atEnd();
}

/**
*/
bool ASTERIXDecoderFile::atEnd() const
{
    return (current_file_idx_ >= (int)source_.files().size());
}

/**
*/
void ASTERIXDecoderFile::processCurrentFile()
{
    assert(!atEnd());

    auto& current_file = source_.file_infos_.at(current_file_idx_);
    try
    {
        processFile(current_file);

        //another file done
        done_file_size_          += current_file.sizeInBytes();
        current_file_bytes_read_  = 0;
    }
    catch(const std::exception& e)
    {
        logerr << "ASTERIXDecoderFile: processCurrentFile: decode error '" << e.what() << "'";
        logError(e.what());
    }
    catch(...)
    {
        logerr << "ASTERIXDecoderFile: processCurrentFile: unknown decode error";
        logError("Unknown decode error");
    }
}

/**
*/
bool ASTERIXDecoderFile::canRun_impl() const
{
    assert(source_.sourceType() == fileSourceType());

    //no files to decode
    if (source_.files().empty())
        return false;

    return true;
}

/**
 */
bool ASTERIXDecoderFile::canDecode_impl() const
{
    assert(source_.sourceType() == fileSourceType());

    //check on all files
    for (const auto& fi : source_.file_infos_)
        if (!fi.canDecode())
            return false;

    return true;
}

/**
 */
void ASTERIXDecoderFile::checkDecoding_impl(bool force_recompute) const
{
    assert(source_.sourceType() == fileSourceType());

    //run decode check on all files
    for (auto& fi : source_.file_infos_)
        checkDecoding(fi, force_recompute);
}

/**
 * Checks if a file can be decoded.
*/
void ASTERIXDecoderFile::checkDecoding(ASTERIXImportFileInfo& file_info,
                                       bool force_recompute) const
{
    if (file_info.decoding_tested && !force_recompute)
        return;

    //reset information
    file_info.reset();

    file_info.decoding_tested = true;

    //check file validity first
    if (file_info.filename.empty() || !Utils::Files::fileExists(file_info.filename))
    {
        file_info.error.errtype = ASTERIXImportFileError::ErrorType::Invalid;
        file_info.error.errinfo = "File does not exist";
        return;
    }

    std::string error;
    bool file_ok = false;

    //check file (initially parse file, collect file sections, etc)
    try
    {
        file_ok = checkFile(file_info, error);
    }
    catch(const std::exception& e)
    {
        file_info.error.errinfo = "File check interrupt: " + std::string(e.what());
        file_ok = false;
    }
    catch(...)
    {
        file_info.error.errinfo = "File check interrupt: Unknown error";
        file_ok = false;
    }
    
    if (!file_ok)
    {
        file_info.error.errtype = ASTERIXImportFileError::ErrorType::CouldNotParse;
        file_info.error.errinfo = error;
        return;
    }

    //if file has subsections check these
    if (file_info.hasSections())
    {
        for (size_t i = 0; i < file_info.sections.size(); ++i)
        {
            bool section_ok = checkDecoding(file_info, (int)i, file_info.sections[ i ].error);

            //set section to unused?
            if (!section_ok)
                file_info.sections[ i ].used = false;
        }
        return;
    }

    //check decoding on complete file
    checkDecoding(file_info, -1, file_info.error);
}

/**
 * Checks if either a file or one of its sections can be decoded and
 * stores related error information.
*/
bool ASTERIXDecoderFile::checkDecoding(ASTERIXImportFileInfo& file_info, 
                                       int section_idx, 
                                       ASTERIXImportFileError& error) const
{
    bool ok = false;
    std::string err_msg;
            
    try
    {
        //invoke derived
        ok = checkDecoding(file_info, section_idx, err_msg);
    }
    catch(const std::exception& e)
    {
        err_msg = "Decoder interrupt: " + std::string(e.what());
        ok = false;
    }
    catch(...)
    {
        err_msg = "Decoder interrupt: Unknown error";
        ok = false;
    }
    
    if (!ok)
    {
        error.errtype = ASTERIXImportFileError::ErrorType::DecodingFailed;
        error.errinfo = err_msg;
    }

    return ok;
}

/**
*/
std::string ASTERIXDecoderFile::getCurrentFilename() const
{
    if (current_file_idx_ < (int)source_.file_infos_.size())
        return source_.file_infos_[ current_file_idx_ ].filename;
    return "";
}

/**
*/
void ASTERIXDecoderFile::bytesRead(size_t bytes, bool add)
{
    if (add)
        current_file_bytes_read_ += bytes;
    else
        current_file_bytes_read_ = bytes;
}

/**
*/
void ASTERIXDecoderFile::recordsRead(size_t num_records_read)
{
    total_records_read_ += num_records_read;
}

/**
*/
std::string ASTERIXDecoderFile::statusInfoString() const
{
    std::string text;

    const auto& file_infos = source_.files();

    for (const auto& file_info : file_infos)
    {
        if (file_info.filename == getCurrentFilename())
            text += "<p align=\"left\"><b>" + file_info.filename + "</b>";
        else
            text += "<p align=\"left\">"+file_info.filename + "";
    }

    text += "<br><p align=\"left\">Records/s: " + std::to_string((unsigned int) getRecordsPerSecond());
    text += "<p align=\"right\">Remaining: "+ Utils::String::timeStringFromDouble(getRemainingTime() + 1.0, false);

    return text ;
}

/**
*/
float ASTERIXDecoderFile::statusInfoProgress() const
{
    return 100.0 * (float)(done_file_size_ + current_file_bytes_read_) / (float)total_file_size_;
}

/**
*/
float ASTERIXDecoderFile::getRecordsPerSecond() const
{
    return (float) total_records_read_ / elapsedSeconds();
}

/**
*/
float ASTERIXDecoderFile::getRemainingTime() const
{
    size_t remaining_bytes = total_file_size_ - done_file_size_ - current_file_bytes_read_;
    float  bytes_per_s     = (float) (done_file_size_ + current_file_bytes_read_) / elapsedSeconds();

    return (float)remaining_bytes / bytes_per_s;
}
