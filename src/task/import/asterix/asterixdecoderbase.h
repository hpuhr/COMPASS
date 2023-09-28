#ifndef ASTERIXDECODERBASE_H
#define ASTERIXDECODERBASE_H

#include "datasourcelineinfo.h"

#include <boost/array.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/mutex.hpp>

#include <cstddef>
#include <string>
#include <map>

class ASTERIXImportTask;
class ASTERIXDecodeJob;
class ASTERIXImportTaskSettings;

const unsigned int MAX_UDP_READ_SIZE=1024*1024;
const unsigned int MAX_ALL_RECEIVE_SIZE=100*1024*1024;

class ASTERIXDecoderBase
{
public:
    ASTERIXDecoderBase(ASTERIXDecodeJob& job, ASTERIXImportTask& task, const ASTERIXImportTaskSettings& settings);
    virtual ~ASTERIXDecoderBase();

    void start();
    void stop();

    size_t numErrors() const;

    bool error() const;
    std::string errorMessage() const;

//    float getFileDecodingProgress() const;
//    float getRecordsPerSecond() const;
//    float getRemainingTime() const;

private:
    ASTERIXDecodeJob& job_;
    ASTERIXImportTask& task_;
    const ASTERIXImportTaskSettings& settings_;

    bool running_ {false};

    bool decode_file_ {false};

    bool decode_udp_streams_ {false};
    std::map<unsigned int, std::map<std::string, std::shared_ptr<DataSourceLineInfo>>> ds_lines_;
    // ds_id -> line str ->(ip, port)

//    size_t num_frames_{0};
//    size_t num_records_{0};
    size_t num_errors_{0};

    size_t file_size_{0};
    size_t max_index_{0};

    bool error_{false};
    std::string error_message_;

    boost::interprocess::interprocess_semaphore receive_semaphore_;
    std::map<unsigned int, std::unique_ptr<boost::array<char, MAX_ALL_RECEIVE_SIZE>>> receive_buffers_copy_; // line->buf
    std::map<unsigned int, size_t> receive_copy_buffer_sizes_; // line -> len

    boost::mutex receive_buffers_mutex_;
    std::map<unsigned int, std::unique_ptr<boost::array<char, MAX_ALL_RECEIVE_SIZE>>> receive_buffers_; // line -> buf
    std::map<unsigned int, size_t> receive_buffer_sizes_; // line -> len

    boost::posix_time::ptime last_receive_decode_time_;

    void doFileDecoding();
    void doUDPStreamDecoding();

    void storeReceivedData (unsigned int line, const char* data, unsigned int length);

//    void fileJasterixCallback(std::unique_ptr<nlohmann::json> data, unsigned int line_id, size_t num_frames,
//                              size_t num_records, size_t numErrors);
//    void netJasterixCallback(std::unique_ptr<nlohmann::json> data, unsigned int line_id, size_t num_frames,
//                             size_t num_records, size_t numErrors);


};

#endif // ASTERIXDECODERBASE_H
