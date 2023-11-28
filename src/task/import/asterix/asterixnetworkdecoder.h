#ifndef ASTERIXNETWORKDECODER_H
#define ASTERIXNETWORKDECODER_H

#include "asterixdecoderbase.h"

#include "datasourcelineinfo.h"

#include <boost/array.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/mutex.hpp>

#include <cstddef>
#include <map>

const unsigned int MAX_UDP_READ_SIZE=1024*1024;
const unsigned int MAX_ALL_RECEIVE_SIZE=100*1024*1024;

class ASTERIXNetworkDecoder : public ASTERIXDecoderBase
{
public:
    ASTERIXNetworkDecoder(ASTERIXDecodeJob& job, ASTERIXImportTask& task, const ASTERIXImportTaskSettings& settings);
    virtual ~ASTERIXNetworkDecoder();

    virtual void start() override;
    virtual void stop() override;

private:

    std::map<unsigned int, std::map<std::string, std::shared_ptr<DataSourceLineInfo>>> ds_lines_;
    // ds_id -> line str ->(ip, port)

    boost::interprocess::interprocess_semaphore receive_semaphore_;
    std::map<unsigned int, std::unique_ptr<boost::array<char, MAX_ALL_RECEIVE_SIZE>>> receive_buffers_copy_; // line->buf
    std::map<unsigned int, size_t> receive_copy_buffer_sizes_; // line -> len

    boost::mutex receive_buffers_mutex_;
    std::map<unsigned int, std::unique_ptr<boost::array<char, MAX_ALL_RECEIVE_SIZE>>> receive_buffers_; // line -> buf
    std::map<unsigned int, size_t> receive_buffer_sizes_; // line -> len

    boost::posix_time::ptime last_receive_decode_time_;

    void storeReceivedData (unsigned int line, const char* data, unsigned int length);

};

#endif // ASTERIXNETWORKDECODER_H
