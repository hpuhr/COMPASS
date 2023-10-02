#include "asterixnetworkdecoder.h"
#include "asteriximporttask.h"
#include "stringconv.h"
#include "compass.h"
#include "datasourcemanager.h"
#include "udpreceiver.h"

#include <jasterix/jasterix.h>

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

using namespace nlohmann;
using namespace Utils;
using namespace std;

ASTERIXNetworkDecoder::ASTERIXNetworkDecoder(
        ASTERIXDecodeJob& job, ASTERIXImportTask& task, const ASTERIXImportTaskSettings& settings)
    : ASTERIXDecoderBase(job, task, settings), receive_semaphore_((unsigned int) 0)
{
    ds_lines_ = COMPASS::instance().dataSourceManager().getNetworkLines();

    for (auto& ds_it : ds_lines_)
    {
        loginf << ds_it.first << ":";

        for (auto& line_it : ds_it.second)
            loginf << "\t" << line_it.first << " " << line_it.second->asString();
    }
}

ASTERIXNetworkDecoder::~ASTERIXNetworkDecoder()
{

}

void ASTERIXNetworkDecoder::start()
{
    assert (!running_);

    running_ = true;

    boost::asio::io_context io_context;

    unsigned int line;

    vector<unique_ptr<UDPReceiver>> udp_receivers;

    int max_lines = settings_.max_network_lines_;

    loginf << "ASTERIXNetworkDecoder: start: max lines " << max_lines;

    for (auto& ds_it : ds_lines_)
    {
        //loginf << ds_it.first << ":";

        unsigned int line_cnt = 0;

        for (auto& line_it : ds_it.second)
        {
            line = String::getAppendedInt(line_it.first);
            assert (line >= 1 && line <= 4);
            line--; // technical counting starts at 0

            loginf << "ASTERIXNetworkDecoder: start: setting up ds_id " << ds_it.first
                   << " line " << line << " info " << line_it.second->asString();

            auto data_callback = [this,line](const char* data, unsigned int length) {
                this->storeReceivedData(line, data, length);
            };

            udp_receivers.emplace_back(new UDPReceiver(io_context, line_it.second, data_callback, MAX_UDP_READ_SIZE));

            ++line_cnt;

            if (max_lines != -1 && line_cnt == (unsigned int) max_lines)
                break; // HACK only do first line
        }
    }

    loginf << "ASTERIXNetworkDecoder: start: running iocontext";

    boost::thread t(boost::bind(&boost::asio::io_context::run, &io_context));
    t.detach();

    last_receive_decode_time_ = boost::posix_time::microsec_clock::local_time();

    unsigned int line_id = 0;

    while (running_)
    {
        receive_semaphore_.wait();

        if (!running_)
            break;

        {
            boost::mutex::scoped_lock lock(receive_buffers_mutex_);

            if (receive_buffer_sizes_.size() // not paused, any data received, 1sec passed
                    && (boost::posix_time::microsec_clock::local_time()
                        - last_receive_decode_time_).total_milliseconds() > 1000)
            {
                loginf << "ASTERIXNetworkDecoder: start: copying data "
                       << receive_buffer_sizes_.size() << " buffers  max " << MAX_ALL_RECEIVE_SIZE;

                // copy data
                for (auto& size_it : receive_buffer_sizes_)
                {
                    line_id = size_it.first;

                    assert (receive_buffers_.count(line_id));

                    assert (receive_buffer_sizes_.at(line_id) <= MAX_ALL_RECEIVE_SIZE);

                    if (!receive_buffers_copy_.count(line_id))
                        receive_buffers_copy_[line_id].reset(new boost::array<char, MAX_ALL_RECEIVE_SIZE>());

                    *receive_buffers_copy_.at(line_id) = *receive_buffers_.at(line_id);
                    receive_copy_buffer_sizes_[line_id] = size_it.second;

                }

                receive_buffer_sizes_.clear();

                lock.unlock();

                last_receive_decode_time_ = boost::posix_time::microsec_clock::local_time();

                loginf << "ASTERIXNetworkDecoder: start: processing copied data";

                for (auto& size_it : receive_copy_buffer_sizes_)
                {
                    line_id = size_it.first;

                    assert (receive_buffers_copy_.count(line_id));

                    auto callback = [this, line_id](std::unique_ptr<nlohmann::json> data, size_t num_frames,
                            size_t num_records, size_t numErrors) {
                        job_.netJasterixCallback(std::move(data), line_id, num_frames, num_records, numErrors);
                    };

                    task_.jASTERIX()->decodeData((char*) receive_buffers_copy_.at(line_id)->data(),
                                                 size_it.second, callback);
                }

                loginf << "ASTERIXNetworkDecoder: start: done";

                receive_copy_buffer_sizes_.clear();

                job_.forceBlockingDataProcessing();
            }
        }
    }

    loginf << "ASTERIXNetworkDecoder: start: shutting down iocontext";

    io_context.stop();
    assert (io_context.stopped());

    t.timed_join(100);

    //done_ = true; // done set in outer run function

    loginf << "ASTERIXNetworkDecoder: start: done";
}

void ASTERIXNetworkDecoder::stop()
{
    if (running_)
    {
        running_ = false;

        // stop decoding
        receive_semaphore_.post(); // wake up loop
    }
}

void ASTERIXNetworkDecoder::storeReceivedData (unsigned int line, const char* data, unsigned int length) // const std::string& sender_id,
{
    if (!running_)
        return;

    //loginf << "ASTERIXDecoderBase: storeReceivedData: sender " << sender_id;

    boost::mutex::scoped_lock lock(receive_buffers_mutex_);

    if (length + receive_buffer_sizes_[line] >= MAX_ALL_RECEIVE_SIZE)
    {
        logerr << "ASTERIXNetworkDecoder: storeReceivedData: overload, too much data in buffer";
        return;
    }

    if (!receive_buffers_.count(line))
        receive_buffers_[line].reset(new boost::array<char, MAX_ALL_RECEIVE_SIZE>());

    assert (receive_buffers_[line]);

    for (unsigned int cnt=0; cnt < length; ++cnt)
        receive_buffers_[line]->at(receive_buffer_sizes_[line]+cnt) = data[cnt];

    receive_buffer_sizes_[line] += length;

    lock.unlock();

    receive_semaphore_.post();
}
