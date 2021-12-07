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

#include <functional>

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

#include "job.h"
#include "json.hpp"

class ASTERIXImportTask;
class ASTERIXPostProcess;

const unsigned int MAX_READ_SIZE=1024*1024;

class ASTERIXDecodeJob : public Job
{
    Q_OBJECT
  signals:
    void decodedASTERIXSignal();

  public:
    ASTERIXDecodeJob(ASTERIXImportTask& task, bool test, ASTERIXPostProcess& post_process);
    virtual ~ASTERIXDecodeJob();

    void setDecodeFile (const std::string& filename,
                        const std::string& framing);

    void setDecodeUDPStreams (const std::vector<std::string>& udp_ips); // includes ips

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
    bool test_{false};
    ASTERIXPostProcess& post_process_;

    bool decode_file_ {false};
    std::string filename_;
    std::string framing_;

    bool decode_udp_streams_ {false};
    std::vector<std::string> udp_ips_;

    volatile bool pause_{false};

    size_t num_frames_{0};
    size_t num_records_{0};
    size_t num_errors_{0};

    bool error_{false};
    std::string error_message_;

    boost::interprocess::interprocess_semaphore receive_semaphore_;

    std::vector<boost::asio::ip::udp::socket> sockets_;
    std::vector<boost::asio::ip::udp::endpoint> end_points_;
    std::vector<boost::array<char, MAX_READ_SIZE>> recv_buffers_;
    unsigned int read_socket_num_;
    size_t read_bytes_;

    boost::array<char, MAX_READ_SIZE> receive_buffer_;
    size_t receive_buffer_size_ {0};
    boost::posix_time::ptime last_receive_decode_time_;

    std::unique_ptr<nlohmann::json> extracted_data_;

    std::map<unsigned int, size_t> category_counts_;

    void doFileDecoding();
    void doUDPStreamDecoding();

    void handleReceive(unsigned int socket_num, const boost::system::error_code& error, size_t bytes_transferred);

    void jasterix_callback(std::unique_ptr<nlohmann::json> data, size_t num_frames,
                           size_t num_records, size_t numErrors);
    void countRecord(unsigned int category, nlohmann::json& record);
    // checks that SAC/SIC are set in all records in same data block
    void checkCAT001SacSics(nlohmann::json& data_block);
};

#endif  // ASTERIXDECODEJOB_H
