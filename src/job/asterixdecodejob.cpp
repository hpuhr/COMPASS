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

#include "asterixdecodejob.h"
#include "asteriximporttask.h"
#include "json.h"
#include "logger.h"
#include "stringconv.h"

#include <jasterix/jasterix.h>

#include <QThread>

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <chrono>
#include <thread>


#include <memory>

using namespace nlohmann;
using namespace Utils;
using namespace std;

//using boost::asio::ip::udp;

class UDPReceiver
{
public:
    UDPReceiver(boost::asio::io_context& io_context, string sender_ip, unsigned int port,
           std::function<void(const std::string&, const char*, unsigned int)> data_callback)
        : socket_(io_context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port)),
          sender_id_(sender_ip+":"+to_string(port)),
          sender_ip_(sender_ip), port_(port), data_callback_(data_callback)
    {
        //loginf << "ctor: " << sender_ip_ << ":" << port_;

        socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
        socket_.set_option(boost::asio::ip::multicast::join_group(
                               boost::asio::ip::address::from_string(sender_ip_)));

        //loginf << "ctor: first async rec";

        socket_.async_receive_from(
                    boost::asio::buffer(data_, max_length), sender_endpoint_,
                    boost::bind(&UDPReceiver::handle_receive_from, this,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));
    }

    void handle_receive_from(const boost::system::error_code& error,
                             size_t bytes_recvd)
    {
        //loginf << "handle_receive_from: from " << sender_ip_ << ":" << port_ << " bytes " << bytes_recvd;

        if (error && error != boost::asio::error::message_size)
        {
            logerr << "UDPReceiver: handle_receive_from: from " << sender_ip_ << ":" << port_ << " error " << error;
            return;
        }
        else
        {
            data_callback_(sender_id_, data_, bytes_recvd);
        }

        socket_.async_receive_from(
                    boost::asio::buffer(data_, max_length), sender_endpoint_,
                    boost::bind(&UDPReceiver::handle_receive_from, this,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));
    }

private:
    boost::asio::ip::udp::endpoint sender_endpoint_;
    boost::asio::ip::udp::socket socket_;
    string sender_id_;
    string sender_ip_;
    unsigned int port_;

    std::function<void(const std::string&, const char*, unsigned int)> data_callback_;
    enum { max_length = 512*1024 };
    char data_[max_length];
};

ASTERIXDecodeJob::ASTERIXDecodeJob(ASTERIXImportTask& task, bool test,
                                   ASTERIXPostProcess& post_process)
    : Job("ASTERIXDecodeJob"),
      task_(task),
      test_(test),
      post_process_(post_process), receive_semaphore_((unsigned int) 0)
{
    logdbg << "ASTERIXDecodeJob: ctor";
}

ASTERIXDecodeJob::~ASTERIXDecodeJob() { logdbg << "ASTERIXDecodeJob: dtor"; }

void ASTERIXDecodeJob::setDecodeFile (const std::string& filename,
                                      const std::string& framing)
{
    loginf << "ASTERIXDecodeJob: setDecodeFile: file '" << filename << "' framing '" << framing << "'";

    filename_ = filename;
    framing_ = framing;

    decode_file_ = true;
    assert (!decode_udp_streams_);
}


void ASTERIXDecodeJob::setDecodeUDPStreams (
        const std::map<unsigned int, std::vector <std::pair<std::string, unsigned int>>>& ds_lines)
{
    ds_lines_ = ds_lines;

    loginf << "ASTERIXDecodeJob: setDecodeUDPStreams: streams:";

    for (auto& ds_it : ds_lines_)
    {
        loginf << ds_it.first << ":";

        for (auto& line_it : ds_it.second)
            loginf << "\t" << line_it.first << ":" << line_it.second;
    }

    decode_udp_streams_ = true;
    assert (!decode_file_);

    framing_ = ""; // only netto content
}

void ASTERIXDecodeJob::run()
{
    logdbg << "ASTERIXDecodeJob: run";

    assert (decode_file_ || decode_udp_streams_);

    started_ = true;

    if (decode_file_)
        doFileDecoding();
    else if (decode_udp_streams_)
        doUDPStreamDecoding();

    assert(extracted_data_ == nullptr);

    done_ = true;

    logdbg << "ASTERIXDecodeJob: run: done";
}

void ASTERIXDecodeJob::setObsolete()
{
    Job::setObsolete();

    receive_semaphore_.post(); // wake up loop
}

void ASTERIXDecodeJob::doFileDecoding()
{
    assert (decode_file_);

    auto callback = [this](std::unique_ptr<nlohmann::json> data, size_t num_frames,
            size_t num_records, size_t numErrors) {
        this->jasterix_callback(std::move(data), num_frames, num_records, numErrors);
    };

    try
    {
        if (framing_ == "")
            task_.jASTERIX()->decodeFile(filename_, callback);
        else
            task_.jASTERIX()->decodeFile(filename_, framing_, callback);
    }
    catch (std::exception& e)
    {
        logerr << "ASTERIXDecodeJob: run: decoding error '" << e.what() << "'";
        error_ = true;
        error_message_ = e.what();
    }
}

void ASTERIXDecodeJob::doUDPStreamDecoding()
{
    loginf << "ASTERIXDecodeJob: doUDPStreamDecoding";

    assert (decode_udp_streams_);

    boost::asio::io_context io_context;

    auto data_callback = [this](const std::string& sender_id, const char* data, unsigned int length) {
        this->storeReceivedData(sender_id, data, length);
    };

    vector<unique_ptr<UDPReceiver>> servers;

    for (auto& ds_it : ds_lines_)
    {
        //loginf << ds_it.first << ":";

        for (auto& line_it : ds_it.second)
        {
            loginf << "ASTERIXDecodeJob: doUDPStreamDecoding: setting up ds_id " << ds_it.first
                   << " ip " << line_it.first << ":" << line_it.second;
            servers.emplace_back(new UDPReceiver(io_context, line_it.first, line_it.second, data_callback));
        }

    }

    loginf << "ASTERIXDecodeJob: doUDPStreamDecoding: running iocontext";

    //io_context.run();
    boost::thread t(boost::bind(&boost::asio::io_context::run, &io_context));
    t.detach();

    last_receive_decode_time_ = boost::posix_time::microsec_clock::local_time();

    auto callback = [this](std::unique_ptr<nlohmann::json> data, size_t num_frames,
            size_t num_records, size_t numErrors) {
        this->jasterix_callback(std::move(data), num_frames, num_records, numErrors);
    };

    while (!obsolete_)
    {
        receive_semaphore_.wait();

        //loginf << "ASTERIXDecodeJob: doUDPStreamDecoding: woke up";

        if (obsolete_)
            break;

        {
            boost::mutex::scoped_lock lock(receive_buffer_mutex_);

            if (receive_buffer_size_
                    && (boost::posix_time::microsec_clock::local_time()
                        - last_receive_decode_time_).total_milliseconds() > 1000)
            {
                loginf << "ASTERIXDecodeJob: doUDPStreamDecoding: processing buffer size "
                       << receive_buffer_size_ << " max " << MAX_READ_SIZE;

                boost::array<char, MAX_READ_SIZE> tmp_buffer = receive_buffer_;
                size_t tmp_buffer_size = receive_buffer_size_;

                receive_buffer_size_ = 0;

                lock.unlock();

                last_receive_decode_time_ = boost::posix_time::microsec_clock::local_time();

                task_.jASTERIX()->decodeData((char*) tmp_buffer.data(), tmp_buffer_size, callback);

            }
        }
    }

    loginf << "ASTERIXDecodeJob: doUDPStreamDecoding: shutting down iocontext";

    io_context.stop();
    assert (io_context.stopped());

    t.timed_join(100);

    //done_ = true; // done set in outer run function

    loginf << "ASTERIXDecodeJob: doUDPStreamDecoding: done";
}

void ASTERIXDecodeJob::storeReceivedData (const std::string& sender_id, const char* data, unsigned int length)
{
    if (obsolete_)
        return;

    boost::mutex::scoped_lock lock(receive_buffer_mutex_);

    assert (length + receive_buffer_size_ < MAX_READ_SIZE);

    for (unsigned int cnt=0; cnt < length; ++cnt)
        receive_buffer_.at(receive_buffer_size_+cnt) = data[cnt];

    receive_buffer_size_ += length;

    lock.unlock();

    receive_semaphore_.post();
}

void ASTERIXDecodeJob::jasterix_callback(std::unique_ptr<nlohmann::json> data, size_t num_frames,
                                         size_t num_records, size_t num_errors)
{
    if (obsolete_)
        return;

    if (error_)
    {
        loginf << "ASTERIXDecodeJob: jasterix_callback: errors state";
        return;
    }

    //loginf << "ASTERIXDecodeJob: jasterix_callback: data '" << data->dump(2) << "'";

    assert(!extracted_data_);
    extracted_data_ = std::move(data);
    assert(extracted_data_);
    assert(extracted_data_->is_object());

    num_frames_ = num_frames;
    num_records_ = num_records;
    num_errors_ = num_errors;

    if (num_errors_)
        logwrn << "ASTERIXDecodeJob: jasterix_callback: num errors " << num_errors_;

    unsigned int category{0};

    auto count_lambda = [this, &category](nlohmann::json& record) {
        countRecord(category, record);
    };

    auto process_lambda = [this, &category](nlohmann::json& record) {
        post_process_.postProcess(category, record);
    };

    if (framing_ == "")
    {
        assert(extracted_data_->contains("data_blocks"));
        assert(extracted_data_->at("data_blocks").is_array());

        std::vector<std::string> keys{"content", "records"};

        for (json& data_block : extracted_data_->at("data_blocks"))
        {
            if (!data_block.contains("category"))
            {
                logwrn
                        << "ASTERIXDecodeJob: jasterix_callback: data block without asterix category";
                continue;
            }

            category = data_block.at("category");

            if (category == 1)
                checkCAT001SacSics(data_block);

            logdbg << "ASTERIXDecodeJob: jasterix_callback: applying JSON function without framing";
            JSON::applyFunctionToValues(data_block, keys, keys.begin(), process_lambda, false);
            JSON::applyFunctionToValues(data_block, keys, keys.begin(), count_lambda, false);
        }
    }
    else
    {
        assert(extracted_data_->contains("frames"));
        assert(extracted_data_->at("frames").is_array());

        std::vector<std::string> keys{"content", "records"};

        for (json& frame : extracted_data_->at("frames"))
        {
            if (!frame.contains("content"))  // frame with errors
                continue;

            assert(frame.at("content").is_object());

            if (!frame.at("content").contains("data_blocks"))  // frame with errors
                continue;

            assert(frame.at("content").at("data_blocks").is_array());

            for (json& data_block : frame.at("content").at("data_blocks"))
            {
                if (!data_block.contains("category"))  // data block with errors
                {
                    logwrn << "ASTERIXDecodeJob: jasterix_callback: data block without asterix "
                              "category";
                    continue;
                }

                category = data_block.at("category");

                if (category == 1)
                    checkCAT001SacSics(data_block);

                JSON::applyFunctionToValues(data_block, keys, keys.begin(), process_lambda, false);
                JSON::applyFunctionToValues(data_block, keys, keys.begin(), count_lambda, false);
            }
        }
    }

    while (pause_)  // block decoder until unpaused
        QThread::msleep(1);

    emit decodedASTERIXSignal();

    while (extracted_data_)  // block decoder until extracted records have been moved out
        QThread::msleep(1);

    assert(!extracted_data_);
}

size_t ASTERIXDecodeJob::numFrames() const { return num_frames_; }

size_t ASTERIXDecodeJob::numRecords() const { return num_records_; }

bool ASTERIXDecodeJob::error() const { return error_; }

void ASTERIXDecodeJob::countRecord(unsigned int category, nlohmann::json& record)
{
    logdbg << "ASTERIXDecodeJob: countRecord: cat " << category << " record '" << record.dump(4)
           << "'";

    category_counts_[category] += 1;
}

std::map<unsigned int, size_t> ASTERIXDecodeJob::categoryCounts() const { return category_counts_; }

size_t ASTERIXDecodeJob::numErrors() const { return num_errors_; }

std::string ASTERIXDecodeJob::errorMessage() const { return error_message_; }

// equivalent function in JSONParseJob
void ASTERIXDecodeJob::checkCAT001SacSics(nlohmann::json& data_block)
{
    if (!data_block.contains("content"))
    {
        logdbg << "ASTERIXDecodeJob: checkCAT001SacSics: no content in data block";
        return;
    }

    nlohmann::json& content = data_block.at("content");

    if (!content.contains("records"))
    {
        logdbg << "ASTERIXDecodeJob: checkCAT001SacSics: no records in content";
        return;
    }

    nlohmann::json& records = content.at("records");

    bool found_any_sac_sic = false;

    unsigned int sac = 0;
    unsigned int sic = 0;

    // check if any SAC/SIC info can be found
    for (nlohmann::json& record : records)
    {
        if (!found_any_sac_sic)
        {
            if (record.contains("010"))  // found, set as transferable values
            {
                sac = record.at("010").at("SAC");
                sic = record.at("010").at("SIC");
                found_any_sac_sic = true;
            }
            else  // not found, can not set values
                logwrn << "ASTERIXDecodeJob: checkCAT001SacSics: record without any SAC/SIC found";
        }
        else
        {
            if (record.contains("010"))  // found, check values
            {
                if (record.at("010").at("SAC") != sac || record.at("010").at("SIC") != sic)
                    logwrn << "ASTERIXDecodeJob: checkCAT001SacSics: record with differing "
                              "SAC/SICs found";
            }
            else  // not found, set values
            {
                record["010"]["SAC"] = sac;
                record["010"]["SIC"] = sic;
            }
        }
    }
}
