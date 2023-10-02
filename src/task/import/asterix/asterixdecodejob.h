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

#ifndef ASTERIXDECODEJOB_H
#define ASTERIXDECODEJOB_H

#include "job.h"
#include "asterixdecoderbase.h"

#include "json.hpp"

//#include <boost/array.hpp>
//#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
//#include <boost/thread/mutex.hpp>

//#include <functional>

class ASTERIXImportTask;
class ASTERIXImportTaskSettings;
class ASTERIXPostProcess;

//const unsigned int MAX_UDP_READ_SIZE=1024*1024;
//const unsigned int MAX_ALL_RECEIVE_SIZE=100*1024*1024;

// is a job, creates specifc reader/decoder, does post-processing
class ASTERIXDecodeJob : public Job
{
    Q_OBJECT

signals:
    void decodedASTERIXSignal();

public:
    ASTERIXDecodeJob(ASTERIXImportTask& task, const ASTERIXImportTaskSettings& settings, ASTERIXPostProcess& post_process);
    virtual ~ASTERIXDecodeJob();

    virtual void run() override;
    virtual void setObsolete() override;

    size_t numFrames() const;
    size_t numRecords() const;
    size_t numErrors() const;

    bool hasData() { return extracted_data_.size();}

    bool error() const;
    std::string errorMessage() const;

    std::map<unsigned int, size_t> categoryCounts() const;

    std::vector<std::unique_ptr<nlohmann::json>> extractedData();

//    float getFileDecodingProgress() const;
//    float getRecordsPerSecond() const;
//    float getRemainingTime() const;

    size_t countTotal() const;

    void forceBlockingDataProcessing();

private:
    friend class ASTERIXFileDecoder;
    friend class ASTERIXNetworkDecoder;

    ASTERIXImportTask& task_;
    const ASTERIXImportTaskSettings& settings_;

    std::unique_ptr<ASTERIXDecoderBase> decoder_;

    ASTERIXPostProcess& post_process_;

//    bool decode_file_ {false};

//    bool decode_udp_streams_ {false};
//    std::map<unsigned int, std::map<std::string, std::shared_ptr<DataSourceLineInfo>>> ds_lines_;
//    // ds_id -> line str ->(ip, port)

    boost::posix_time::ptime start_time_;

    size_t num_frames_{0};
    size_t num_records_{0};
    size_t num_errors_{0};

//    size_t file_size_{0};
//    size_t max_index_{0};

//    bool error_{false};
//    std::string error_message_;

//    boost::interprocess::interprocess_semaphore receive_semaphore_;
//    std::map<unsigned int, std::unique_ptr<boost::array<char, MAX_ALL_RECEIVE_SIZE>>> receive_buffers_copy_; // line->buf
//    std::map<unsigned int, size_t> receive_copy_buffer_sizes_; // line -> len

//    boost::mutex receive_buffers_mutex_;
//    std::map<unsigned int, std::unique_ptr<boost::array<char, MAX_ALL_RECEIVE_SIZE>>> receive_buffers_; // line -> buf
//    std::map<unsigned int, size_t> receive_buffer_sizes_; // line -> len

//    boost::posix_time::ptime last_receive_decode_time_;

    std::vector<std::unique_ptr<nlohmann::json>> extracted_data_;

    size_t count_total_ {0};
    std::map<unsigned int, size_t> category_counts_;

    unsigned int signal_count_ {0};

//    void doFileDecoding();
//    void doUDPStreamDecoding();

//    void storeReceivedData (unsigned int line, const char* data, unsigned int length);

    void fileJasterixCallback(std::unique_ptr<nlohmann::json> data, unsigned int line_id, size_t num_frames,
                              size_t num_records, size_t numErrors);
    void netJasterixCallback(std::unique_ptr<nlohmann::json> data, unsigned int line_id, size_t num_frames,
                             size_t num_records, size_t numErrors);

    void countRecord(unsigned int category, nlohmann::json& record);
    // checks that SAC/SIC are set in all records in same data block
    void checkCAT001SacSics(nlohmann::json& data_block);
};

#endif  // ASTERIXDECODEJOB_H
