/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ASTERIXDECODEJOB_H
#define ASTERIXDECODEJOB_H

#include <functional>

#include "job.h"
#include "json.hpp"

class ASTERIXImportTask;
class ASTERIXPostProcess;

class ASTERIXDecodeJob : public Job
{
    Q_OBJECT
  signals:
    void decodedASTERIXSignal();

  public:
    ASTERIXDecodeJob(ASTERIXImportTask& task, const std::string& filename,
                     const std::string& framing, bool test, ASTERIXPostProcess& post_process);
    virtual ~ASTERIXDecodeJob();

    virtual void run();

    size_t numFrames() const;
    size_t numRecords() const;
    size_t numErrors() const;

    void pause() { pause_ = true; }
    void unpause() { pause_ = false; }

    bool error() const;
    std::string errorMessage() const;

    std::map<unsigned int, size_t> categoryCounts() const;

    std::unique_ptr<nlohmann::json> extractedData() { return std::move(extracted_data_); }

  private:
    ASTERIXImportTask& task_;
    std::string filename_;
    std::string framing_;
    bool test_{false};
    ASTERIXPostProcess& post_process_;

    volatile bool pause_{false};

    size_t num_frames_{0};
    size_t num_records_{0};
    size_t num_errors_{0};

    bool error_{false};
    std::string error_message_;

    std::unique_ptr<nlohmann::json> extracted_data_;

    std::map<unsigned int, size_t> category_counts_;

    void jasterix_callback(std::unique_ptr<nlohmann::json> data, size_t num_frames,
                           size_t num_records, size_t numErrors);
    void countRecord(unsigned int category, nlohmann::json& record);
    // checks that SAC/SIC are set in all records in same data block
    void checkCAT001SacSics(nlohmann::json& data_block);
};

#endif  // ASTERIXDECODEJOB_H
