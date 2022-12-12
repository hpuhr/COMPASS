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
#include "compass.h"
#include "mainwindow.h"
#include "util/files.h"
#include "asterixudpreceiver.h"

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

ASTERIXDecodeJob::ASTERIXDecodeJob(ASTERIXImportTask& task, bool test,
                                   ASTERIXPostProcess& post_process)
    : Job("ASTERIXDecodeJob"),
      task_(task),
      test_(test),
      post_process_(post_process), receive_semaphore_((unsigned int) 0)
{
    logdbg << "ASTERIXDecodeJob: ctor";
}

ASTERIXDecodeJob::~ASTERIXDecodeJob()
{
    loginf << "ASTERIXDecodeJob: dtor";
    assert (done_);
}

void ASTERIXDecodeJob::setDecodeFile (const std::string& filename,
                                      const std::string& framing)
{
    loginf << "ASTERIXDecodeJob: setDecodeFile: file '" << filename << "' framing '" << framing << "'";

    filename_ = filename;
    file_line_id_ = task_.fileLineID();
    framing_ = framing;

    assert (Files::fileExists(filename));
    file_size_ = Files::fileSize(filename);

    decode_file_ = true;
    assert (!decode_udp_streams_);
}


void ASTERIXDecodeJob::setDecodeUDPStreams (
        const std::map<unsigned int, std::map<std::string, std::shared_ptr<DataSourceLineInfo>>>& ds_lines)
{
    ds_lines_ = ds_lines;

    loginf << "ASTERIXDecodeJob: setDecodeUDPStreams: streams:";

    for (auto& ds_it : ds_lines_)
    {
        loginf << ds_it.first << ":";

        for (auto& line_it : ds_it.second)
            loginf << "\t" << line_it.first << " " << line_it.second->asString();
    }

    decode_udp_streams_ = true;
    assert (!decode_file_);

    framing_ = ""; // only netto content
}

void ASTERIXDecodeJob::run()
{
    loginf << "ASTERIXDecodeJob: run";

    assert (decode_file_ || decode_udp_streams_);

    start_time_ = boost::posix_time::microsec_clock::local_time();;

    started_ = true;
    done_ = false;

    if (decode_file_)
        doFileDecoding();
    else if (decode_udp_streams_)
        doUDPStreamDecoding();

    if (!obsolete_)
        assert(extracted_data_.size() == 0);

    done_ = true;

    loginf << "ASTERIXDecodeJob: run: done";
}

void ASTERIXDecodeJob::setObsolete()
{
    loginf << "ASTERIXDecodeJob: setObsolete";

    Job::setObsolete();

    if (decode_file_)
        task_.jASTERIX()->stopFileDecoding();
    else
        receive_semaphore_.post(); // wake up loop
}

void ASTERIXDecodeJob::doFileDecoding()
{
    loginf << "ASTERIXDecodeJob: doFileDecoding: file '" << filename_ << "' framing '" << framing_ << "'";

    assert (decode_file_);

    auto callback = [this](std::unique_ptr<nlohmann::json> data, size_t num_frames,
            size_t num_records, size_t numErrors) {
        this->fileJasterixCallback(std::move(data), this->file_line_id_, num_frames, num_records, numErrors);
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
    assert (decode_udp_streams_);

    boost::asio::io_context io_context;

    unsigned int line;

    vector<unique_ptr<ASTERIXUDPReceiver>> udp_receivers;

    int max_lines = COMPASS::instance().mainWindow().importAsterixNetworkMaxLines();

    loginf << "ASTERIXDecodeJob: doUDPStreamDecoding: max lines " << max_lines;

    for (auto& ds_it : ds_lines_)
    {
        //loginf << ds_it.first << ":";

        unsigned int line_cnt = 0;

        for (auto& line_it : ds_it.second)
        {
            line = String::getAppendedInt(line_it.first);
            assert (line >= 1 && line <= 4);
            line--; // technical counting starts at 0

            loginf << "ASTERIXDecodeJob: doUDPStreamDecoding: setting up ds_id " << ds_it.first
                   << " line " << line << " info " << line_it.second->asString();

            auto data_callback = [this,line](const char* data, unsigned int length) {
                this->storeReceivedData(line, data, length);
            };

            udp_receivers.emplace_back(new ASTERIXUDPReceiver(io_context, line_it.second, data_callback));

            ++line_cnt;

            if (max_lines != -1 && line_cnt == (unsigned int) max_lines)
                break; // HACK only do first line
        }
    }

    loginf << "ASTERIXDecodeJob: doUDPStreamDecoding: running iocontext";

    boost::thread t(boost::bind(&boost::asio::io_context::run, &io_context));
    t.detach();

    last_receive_decode_time_ = boost::posix_time::microsec_clock::local_time();

    unsigned int line_id = 0;

    while (!obsolete_)
    {
        receive_semaphore_.wait();

        if (obsolete_)
            break;

        {
            boost::mutex::scoped_lock lock(receive_buffers_mutex_);

            if (!in_live_paused_state_ && receive_buffer_sizes_.size() // not paused, any data received, 1sec passed
                    && (boost::posix_time::microsec_clock::local_time()
                        - last_receive_decode_time_).total_milliseconds() > 1000)
            {
                loginf << "ASTERIXDecodeJob: doUDPStreamDecoding: copying data "
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

                loginf << "ASTERIXDecodeJob: doUDPStreamDecoding: processing copied data";

                for (auto& size_it : receive_copy_buffer_sizes_)
                {
                    line_id = size_it.first;

                    assert (receive_buffers_copy_.count(line_id));

                    auto callback = [this, line_id](std::unique_ptr<nlohmann::json> data, size_t num_frames,
                            size_t num_records, size_t numErrors) {
                        this->netJasterixCallback(std::move(data), line_id, num_frames, num_records, numErrors);
                    };

                    task_.jASTERIX()->decodeData((char*) receive_buffers_copy_.at(line_id)->data(),
                                                 size_it.second, callback);
                }

                loginf << "ASTERIXDecodeJob: doUDPStreamDecoding: done";

                receive_copy_buffer_sizes_.clear();

                loginf << "ASTERIXDecodeJob: doUDPStreamDecoding: emitting signal";
                emit decodedASTERIXSignal();

                while (!obsolete_ && extracted_data_.size())  // block decoder until extracted records have been moved out
                    QThread::msleep(1);

//                if (!obsolete_)
//                    assert(extracted_data_.size() == 0);
//                else
//                    extracted_data_ = nullptr;

                resuming_cached_data_ = false; // in case of resume action
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

void ASTERIXDecodeJob::storeReceivedData (unsigned int line, const char* data, unsigned int length) // const std::string& sender_id,
{
    if (obsolete_)
        return;

    //loginf << "ASTERIXDecodeJob: storeReceivedData: sender " << sender_id;

    boost::mutex::scoped_lock lock(receive_buffers_mutex_);

    if (in_live_paused_state_)
        loginf << "ASTERIXDecodeJob: storeReceivedData: line " << line << " existing "
               << receive_buffer_sizes_[line] << " adding " << length;

    if (length + receive_buffer_sizes_[line] >= MAX_ALL_RECEIVE_SIZE)
    {
        logerr << "ASTERIXDecodeJob: storeReceivedData: overload, too much data in buffer";
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

void ASTERIXDecodeJob::fileJasterixCallback(std::unique_ptr<nlohmann::json> data, unsigned int line_id, size_t num_frames,
                                         size_t num_records, size_t num_errors)
{
    if (obsolete_)
        return;

    if (error_)
    {
        loginf << "ASTERIXDecodeJob: fileJasterixCallback: errors state";
        return;
    }

    //loginf << "ASTERIXDecodeJob: fileJasterixCallback: data '" << data->dump(2) << "'";
    loginf << "ASTERIXDecodeJob: fileJasterixCallback: line_id " << line_id << " num_records " << num_records;

    if (num_records == 0)
    {
        loginf << "ASTERIXDecodeJob: fileJasterixCallback: omitting zero data";
        return;
    }

    assert(!extracted_data_.size());
    extracted_data_.emplace_back(std::move(data));
    assert(extracted_data_.size());
    assert(extracted_data_.back());
    assert(extracted_data_.back()->is_object());

    num_frames_ = num_frames;
    num_records_ = num_records;
    num_errors_ = num_errors;

    if (num_errors_)
        logwrn << "ASTERIXDecodeJob: fileJasterixCallback: num errors " << num_errors_;

    unsigned int category{0};

    auto count_lambda = [this, &category](nlohmann::json& record) {
        countRecord(category, record);
    };

    auto process_lambda = [this, line_id, &category](nlohmann::json& record) {
        record["line_id"] = line_id;
        post_process_.postProcess(category, record);
    };

    max_index_ = 0;

    if (framing_ == "")
    {
        assert(extracted_data_.back()->contains("data_blocks"));
        assert(extracted_data_.back()->at("data_blocks").is_array());

        std::vector<std::string> keys{"content", "records"};

        for (json& data_block : extracted_data_.back()->at("data_blocks"))
        {
            if (!data_block.contains("category"))
            {
                logwrn
                        << "ASTERIXDecodeJob: fileJasterixCallback: data block without asterix category";
                continue;
            }

            category = data_block.at("category");

            assert (data_block.contains("content"));
            assert(data_block.at("content").is_object());
            assert (data_block.at("content").contains("index"));
            max_index_ = data_block.at("content").at("index");

            if (category == 1)
                checkCAT001SacSics(data_block);

            logdbg << "ASTERIXDecodeJob: fileJasterixCallback: applying JSON function without framing";
            JSON::applyFunctionToValues(data_block, keys, keys.begin(), process_lambda, false);
            JSON::applyFunctionToValues(data_block, keys, keys.begin(), count_lambda, false);
        }
    }
    else
    {
        assert(extracted_data_.back()->contains("frames"));
        assert(extracted_data_.back()->at("frames").is_array());

        std::vector<std::string> keys{"content", "records"};

        for (json& frame : extracted_data_.back()->at("frames"))
        {
            if (!frame.contains("content"))  // frame with errors
                continue;

            assert(frame.at("content").is_object());

            assert(frame.at("content").is_object());
            assert (frame.at("content").contains("index"));
            max_index_ = frame.at("content").at("index");

            if (!frame.at("content").contains("data_blocks"))  // frame with errors
                continue;

            assert(frame.at("content").at("data_blocks").is_array());

            for (json& data_block : frame.at("content").at("data_blocks"))
            {
                if (!data_block.contains("category"))  // data block with errors
                {
                    logwrn << "ASTERIXDecodeJob: fileJasterixCallback: data block without asterix "
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

    if (decode_file_ && file_size_)
        logdbg << "ASTERIXDecodeJob: fileJasterixCallback: max_index " << max_index_
               << " perc " <<  String::percentToString((float) max_index_/(float) file_size_);

//    while (!obsolete_ && pause_)  // block decoder until unpaused
//        QThread::msleep(1);

    emit decodedASTERIXSignal();

    while (!obsolete_ && extracted_data_.size())  // block decoder until extracted records have been moved out
        QThread::msleep(1);

//    if (!obsolete_)
//        assert(!extracted_data_);
//    else
//        extracted_data_ = nullptr;
}

void ASTERIXDecodeJob::netJasterixCallback(std::unique_ptr<nlohmann::json> data, unsigned int line_id, size_t num_frames,
                                         size_t num_records, size_t num_errors)
{
    if (obsolete_)
        return;

    if (error_)
    {
        loginf << "ASTERIXDecodeJob: netJasterixCallback: errors state";
        return;
    }

    //loginf << "ASTERIXDecodeJob: fileJasterixCallback: data '" << data->dump(2) << "'";
    loginf << "ASTERIXDecodeJob: netJasterixCallback: line_id " << line_id << " num_records " << num_records;

    //std::unique_ptr<nlohmann::json> tmp_extracted_data = std::move(data);

    num_frames_ = num_frames;
    num_records_ = num_records;
    num_errors_ = num_errors;

    if (num_errors_)
        logwrn << "ASTERIXDecodeJob: netJasterixCallback: num errors " << num_errors_;

    unsigned int category{0};

    auto count_lambda = [this, &category](nlohmann::json& record) {
        countRecord(category, record);
    };

    auto process_lambda = [this, line_id, &category](nlohmann::json& record) {
        record["line_id"] = line_id;
        post_process_.postProcess(category, record);
    };

    max_index_ = 0;

    assert (framing_ == "");
    assert(data->contains("data_blocks"));
    assert(data->at("data_blocks").is_array());

    std::vector<std::string> keys{"content", "records"};

    for (json& data_block : data->at("data_blocks"))
    {
        if (!data_block.contains("category"))
        {
            logwrn
                    << "ASTERIXDecodeJob: netJasterixCallback: data block without asterix category";
            continue;
        }

        category = data_block.at("category");

        assert (data_block.contains("content"));
        assert(data_block.at("content").is_object());
        assert (data_block.at("content").contains("index"));
        max_index_ = data_block.at("content").at("index");

        if (category == 1)
            checkCAT001SacSics(data_block);

        logdbg << "ASTERIXDecodeJob: netJasterixCallback: applying JSON function without framing";
        JSON::applyFunctionToValues(data_block, keys, keys.begin(), process_lambda, false);
        JSON::applyFunctionToValues(data_block, keys, keys.begin(), count_lambda, false);
    }

    //    while (!obsolete_ && pause_)  // block decoder until unpaused
    //        QThread::msleep(1);

    if (data->at("data_blocks").size())
    {
//        if (extracted_data_)
//        {
//            // add to existing data
//            assert(extracted_data_->is_object());
//            assert(extracted_data_->contains("data_blocks"));
//            assert(extracted_data_->at("data_blocks").is_array());
//            assert(tmp_extracted_data->at("data_blocks").is_array());

//            if (tmp_extracted_data->at("data_blocks").size())
//                extracted_data_->at("data_blocks").insert(extracted_data_->at("data_blocks").end(),
//                                                          tmp_extracted_data->at("data_blocks").begin(),
//                                                          tmp_extracted_data->at("data_blocks").end());
//        }
//        else
//            extracted_data_ = std::move(tmp_extracted_data);

        extracted_data_.emplace_back(std::move(data));
    }

}

size_t ASTERIXDecodeJob::numFrames() const { return num_frames_; }

size_t ASTERIXDecodeJob::numRecords() const { return num_records_; }

bool ASTERIXDecodeJob::error() const { return error_; }

void ASTERIXDecodeJob::countRecord(unsigned int category, nlohmann::json& record)
{
    logdbg << "ASTERIXDecodeJob: countRecord: cat " << category << " record '" << record.dump(4)
           << "'";

    count_total_++;
    category_counts_[category] += 1;
}

std::map<unsigned int, size_t> ASTERIXDecodeJob::categoryCounts() const { return category_counts_; }

float ASTERIXDecodeJob::getFileDecodingProgress() const
{
    assert (decode_file_ && file_size_);

    return 100.0 * (float) max_index_/(float) file_size_;
}

float ASTERIXDecodeJob::getRecordsPerSecond() const
{
    float elapsed_s = (float )(boost::posix_time::microsec_clock::local_time()
                               - start_time_).total_milliseconds()/1000.0;

    return (float) count_total_ / elapsed_s;
}

float ASTERIXDecodeJob::getRemainingTime() const
{
    assert (decode_file_ && file_size_);

    size_t remaining_rec = file_size_ - max_index_;

    float elapsed_s = (float )(boost::posix_time::microsec_clock::local_time()
                               - start_time_).total_milliseconds()/1000.0;

    float index_per_s = (float) max_index_ / elapsed_s;

    return (float) remaining_rec / index_per_s;
}


size_t ASTERIXDecodeJob::countTotal() const
{
    return count_total_;
}

void ASTERIXDecodeJob::cacheLiveNetworkData()
{
    loginf << "ASTERIXDecodeJob: cacheLiveNetworkData";

    in_live_paused_state_ = true;
}

void ASTERIXDecodeJob::resumeLiveNetworkData(bool discard_cache)
{
    loginf << "ASTERIXDecodeJob: resumeLiveNetworkData: discard cache " << discard_cache;

    if (discard_cache)
    {
        boost::mutex::scoped_lock lock(receive_buffers_mutex_);

        receive_buffer_sizes_.clear();
    }
    else
        resuming_cached_data_ = true;

    in_live_paused_state_ = false;

    receive_semaphore_.post(); // wake up loop
}

bool ASTERIXDecodeJob::resumingCachedData() const
{
    return resuming_cached_data_;
}

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
