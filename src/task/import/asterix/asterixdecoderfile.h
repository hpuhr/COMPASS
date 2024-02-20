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

#pragma once 

#include "asterixdecoderbase.h"

#include <string>

class ASTERIXImportTask;
class ASTERIXImportTaskSettings;

/**
 * File-based ASTERIX decoder.
 */
class ASTERIXDecoderFile : public ASTERIXDecoderBase
{
public:
    ASTERIXDecoderFile(ASTERIXImportSource& source,
                       ASTERIXImportTask& task, 
                       const ASTERIXImportTaskSettings& settings);
    virtual ~ASTERIXDecoderFile();

    virtual ASTERIXImportSource::SourceType fileSourceType() const = 0;

    virtual bool hasStatusInfo() const override { return true; };
    virtual std::string statusInfoString() const override;
    virtual float statusInfoProgress() const override;

    void checkDecoding(ASTERIXImportFileInfo& file_info,
                       bool force_recompute) const;

    std::string getCurrentFilename() const;

    unsigned int DecodeCheckRecordLimit = 10000;
    unsigned int DecodeCheckMaxBytes    = 10000000;

protected:
    virtual bool canRun_impl() const override;
    virtual bool canDecode_impl() const override;
    virtual void checkDecoding_impl(bool force_recompute) const override;

    virtual void start_impl() override;

    virtual bool checkFile(ASTERIXImportFileInfo& file_info, std::string& error) const { return true; }
    virtual bool checkDecoding(ASTERIXImportFileInfo& file_info, int section_idx, std::string& error) const = 0;
    virtual void processFile(ASTERIXImportFileInfo& file_info) = 0;

    void bytesRead(size_t bytes, bool add = true);
    void recordsRead(size_t num_records_read);

private:
    bool nextFile();
    bool atEnd() const;
    void processCurrentFile();

    bool checkDecoding(ASTERIXImportFileInfo& file_info, 
                       int section_idx, 
                       ASTERIXImportFileError& error) const;

    float getRecordsPerSecond() const;
    float getRemainingTime() const;

    int current_file_idx_ = -1;

    size_t total_file_size_         = 0;
    size_t done_file_size_          = 0;
    size_t current_file_bytes_read_ = 0;
    size_t total_records_read_      = 0;
};
