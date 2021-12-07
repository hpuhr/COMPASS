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


#include <memory>

using namespace nlohmann;
using namespace Utils;
using namespace std;

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


void ASTERIXDecodeJob::setDecodeUDPStreams (const std::vector<std::string>& udp_ips)
{
    udp_ips_ = udp_ips;

    loginf << "ASTERIXDecodeJob: setDecodeUDPStreams: streams:";

    for (auto& udp_it : udp_ips_)
        loginf << "\t" << udp_it;

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

    //    UDP_OUTPUT 200 $ETHOUT 224.9.2.252 15080   \
    //    FILTER_SACSIC 100 200 50 80 \
    //    UDP_OUTPUT 202 $ETHOUT 224.9.2.252 15070   \
    //    FILTER_SACSIC 100 202 50 70    \
    //    UDP_OUTPUT 203 $ETHOUT 224.9.2.252 15071   \
    //    FILTER_SACSIC 100 203 50 71    \
    //     UDP_OUTPUT 204 $ETHOUT 224.9.2.252 15072   \
    //    FILTER_SACSIC 100 204 50 72    \
    //    UDP_OUTPUT 205 $ETHOUT 224.9.2.252 15073   \
    //    FILTER_SACSIC 100 205 50 73    \
    //    UDP_OUTPUT 206 $ETHOUT 224.9.2.252 15074   \
    //    FILTER_SACSIC 100 206 50 74    \
    //    UDP_OUTPUT 207 $ETHOUT 224.9.2.252 15075   \
    //    FILTER_SACSIC 100 207 50 75    \
    //    UDP_OUTPUT 208 $ETHOUT 224.9.2.252 15076   \
    //    FILTER_SACSIC 100 208 50 76    \
    //    UDP_OUTPUT 209 $ETHOUT 224.9.2.252 15077   \
    //    FILTER_SACSIC 100 209 50 77    \
    //    UDP_OUTPUT 210 $ETHOUT 224.9.2.252 15078   \
    //    FILTER_SACSIC 100 210 50 78    \
    //    UDP_OUTPUT 211 $ETHOUT 224.9.2.252 15081   \
    //    FILTER_SACSIC 100 211 50 81    \
    //

    std::vector <std::pair<std::string, unsigned int>> ips_and_ports;
    ips_and_ports.push_back({"224.9.2.252", 15080});
//    ips_and_ports.push_back({"224.9.2.252", 15070});
//    ips_and_ports.push_back({"224.9.2.252", 15071});
//    ips_and_ports.push_back({"224.9.2.252", 15072});
//    ips_and_ports.push_back({"224.9.2.252", 15073});
//    ips_and_ports.push_back({"224.9.2.252", 15074});
//    ips_and_ports.push_back({"224.9.2.252", 15075});
//    ips_and_ports.push_back({"224.9.2.252", 15076});
//    ips_and_ports.push_back({"224.9.2.252", 15077});
    //ips_and_ports.push_back({"224.9.2.252", 15078});
//    ips_and_ports.push_back({"224.9.2.252", 15081});
    ips_and_ports.push_back({"224.9.2.252", 150240});

    //std::vector<boost::asio::ip::udp::socket> sockets;

    boost::asio::io_service io_service;

    unsigned int socket_cnt = 0;

    for (auto& ip_port_it : ips_and_ports)
    {
        sockets_.emplace_back(io_service);
        // Create the socket so that multiple may be bound to the same address.
        sockets_.at(socket_cnt).open(boost::asio::ip::udp::v4());
        sockets_.at(socket_cnt).set_option(boost::asio::ip::udp::socket::reuse_address(true));

        end_points_.emplace_back(
                    boost::asio::ip::address::from_string(ip_port_it.first), ip_port_it.second);

        sockets_.at(socket_cnt).bind(end_points_.at(socket_cnt));

        // Join the multicast group.
        sockets_.at(socket_cnt).set_option(
                    boost::asio::ip::multicast::join_group(boost::asio::ip::address::from_string(ip_port_it.first)));

        recv_arrays_.emplace_back();

        sockets_.at(socket_cnt).async_receive_from(
                    boost::asio::buffer(recv_arrays_.at(socket_cnt)),
                    end_points_.at(socket_cnt),
                    boost::bind(&ASTERIXDecodeJob::handleReceive, this,
                                socket_cnt,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));

        ++socket_cnt;
    }

    last_receive_decode_time_ =  boost::posix_time::microsec_clock::local_time();

    loginf << "doing io service";

    shared_ptr<boost::asio::io_service::work> work(new boost::asio::io_service::work(io_service));
    boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));
    t.detach();

    loginf << "doing wait";

    auto callback = [this](std::unique_ptr<nlohmann::json> data, size_t num_frames,
            size_t num_records, size_t numErrors) {
        this->jasterix_callback(std::move(data), num_frames, num_records, numErrors);
    };

    while (!error_occured_ && !obsolete_)
    {
        receive_semaphore_.wait();

        if (receive_buffer_size_
                && (boost::posix_time::microsec_clock::local_time() - last_receive_decode_time_).total_milliseconds() > 1000)
        {
            loginf << "processing buffer size " << receive_buffer_size_ << " max " << MAX_READ_SIZE;

            last_receive_decode_time_ = boost::posix_time::microsec_clock::local_time();

            boost::mutex::scoped_lock lock(receive_buffer_mutex_);

            //task_.jASTERIX()->decodeData((char*) receive_buffer_.data(), receive_buffer_size_, callback);
            receive_buffer_size_ = 0;
        }

    }

    io_service.stop();
    pthread_cancel(t.native_handle());

    loginf << "ASTERIXDecodeJob: doUDPStreamDecoding: done";
}

void ASTERIXDecodeJob::handleReceive(unsigned int socket_num, const boost::system::error_code& error,
                                     size_t bytes_transferred)
{
    //loginf << "ASTERIXDecodeJob: handleReceive: socket " << socket_num << " bytes " << bytes_transferred;

    if (error && error != boost::asio::error::message_size)
    {
        logerr << "ASTERIXDecodeJob: handleReceive: socket " << socket_num << " error " << error;

        error_occured_ = true;
        error_code_ = error;

        return;
    }

    if (!bytes_transferred)
        return;

    assert (bytes_transferred < MAX_READ_SIZE);
    assert (bytes_transferred + receive_buffer_size_ < MAX_READ_SIZE);

    // collect data in receive buffer

    loginf << "ASTERIXDecodeJob: handleReceive: collect data " << socket_num << " transferred " << bytes_transferred
           << " receive_buffer_size " << receive_buffer_size_;

    {
        boost::mutex::scoped_lock lock(receive_buffer_mutex_);

        for (unsigned int cnt=0; cnt < bytes_transferred; ++cnt)
            receive_buffer_.at(receive_buffer_size_+cnt) = recv_arrays_.at(socket_num).at(cnt);
        receive_buffer_size_ += bytes_transferred;
    }

    loginf << "ASTERIXDecodeJob: handleReceive: collect data " << socket_num << " done";

    receive_semaphore_.post();

    if (!error || error == boost::asio::error::message_size)
    {
        loginf << "ASTERIXDecodeJob: handleReceive: new receive socket " << socket_num;

        assert (socket_num < sockets_.size());
        assert (socket_num < recv_arrays_.size());
        assert (socket_num < end_points_.size());

        sockets_.at(socket_num).async_receive_from(
                    boost::asio::buffer(recv_arrays_.at(socket_num)),
                    end_points_.at(socket_num),
                    boost::bind(&ASTERIXDecodeJob::handleReceive, this,
                                socket_num,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));
    }

    logdbg << "ASTERIXDecodeJob: handleReceive: done";

}

void ASTERIXDecodeJob::jasterix_callback(std::unique_ptr<nlohmann::json> data, size_t num_frames,
                                         size_t num_records, size_t num_errors)
{
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
