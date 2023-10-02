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
#include "asterixfiledecoder.h"
#include "asterixnetworkdecoder.h"

#include <QThread>

#include <chrono>
#include <thread>
#include <memory>

using namespace nlohmann;
using namespace Utils;
using namespace std;

ASTERIXDecodeJob::ASTERIXDecodeJob(ASTERIXImportTask& task, const ASTERIXImportTaskSettings& settings,
                                   ASTERIXPostProcess& post_process)
    : Job("ASTERIXDecodeJob"),
      task_(task), settings_(settings),
      post_process_(post_process)
{
    logdbg << "ASTERIXDecodeJob: ctor";


    if (settings_.importFile())
        decoder_.reset(new ASTERIXFileDecoder(*this, task_, settings_));
    else
        decoder_.reset(new ASTERIXNetworkDecoder(*this, task_, settings_));
}

ASTERIXDecodeJob::~ASTERIXDecodeJob()
{
    loginf << "ASTERIXDecodeJob: dtor";
    assert (done_);
}

void ASTERIXDecodeJob::run()
{
    loginf << "ASTERIXDecodeJob: run";

    start_time_ = boost::posix_time::microsec_clock::local_time();;

    started_ = true;
    done_ = false;

    assert (decoder_);
    decoder_->start();

    if (!obsolete_)
        assert(extracted_data_.size() == 0);

    done_ = true;

    loginf << "ASTERIXDecodeJob: run: done";
}

void ASTERIXDecodeJob::setObsolete()
{
    loginf << "ASTERIXDecodeJob: setObsolete";

    Job::setObsolete();

    assert (decoder_);
    decoder_->stop();
}

void ASTERIXDecodeJob::fileJasterixCallback(std::unique_ptr<nlohmann::json> data, unsigned int line_id, size_t num_frames,
                                         size_t num_records, size_t num_errors)
{
    if (obsolete_)
        return;

    if (decoder_ && decoder_->error())
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

    //max_index_ = 0;

    if (settings_.current_file_framing_ == "")
    {
        assert(extracted_data_.back()->contains("data_blocks"));
        assert(extracted_data_.back()->at("data_blocks").is_array());

        std::vector<std::string> keys{"content", "records"};

        for (json& data_block : extracted_data_.back()->at("data_blocks"))
        {
            if (!data_block.contains("category"))
            {
                logwrn << "ASTERIXDecodeJob: fileJasterixCallback: data block without asterix category";
                continue;
            }

            category = data_block.at("category");

            assert (data_block.contains("content"));
            assert(data_block.at("content").is_object());
            //assert (data_block.at("content").contains("index"));
            //max_index_ = data_block.at("content").at("index");

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
            //assert (frame.at("content").contains("index"));
            //max_index_ = frame.at("content").at("index");

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

//    if (decode_file_ && file_size_)
//        logdbg << "ASTERIXDecodeJob: fileJasterixCallback: max_index " << max_index_
//               << " perc " <<  String::percentToString((float) max_index_/(float) file_size_);

    ++signal_count_;

    logdbg << "ASTERIXDecodeJob: fileJasterixCallback: emitting signal " << signal_count_;

    emit decodedASTERIXSignal();

    logdbg << "ASTERIXDecodeJob: fileJasterixCallback: wait " << signal_count_;

    while (!obsolete_ && extracted_data_.size())  // block decoder until extracted records have been moved out
        QThread::msleep(1);

    logdbg << "ASTERIXDecodeJob: fileJasterixCallback: waiting done " << signal_count_;

}

void ASTERIXDecodeJob::netJasterixCallback(std::unique_ptr<nlohmann::json> data, unsigned int line_id, size_t num_frames,
                                         size_t num_records, size_t num_errors)
{
    if (obsolete_)
        return;

    if (decoder_ && decoder_->error())
    {
        loginf << "ASTERIXDecodeJob: netJasterixCallback: errors state";
        return;
    }

    //loginf << "ASTERIXDecodeJob: fileJasterixCallback: data '" << data->dump(2) << "'";
    loginf << "ASTERIXDecodeJob: netJasterixCallback: line_id " << line_id << " num_records " << num_records;

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

    //max_index_ = 0;

    assert (settings_.current_file_framing_ == "");
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
        //assert (data_block.at("content").contains("index"));
        //max_index_ = data_block.at("content").at("index");

        if (category == 1)
            checkCAT001SacSics(data_block);

        logdbg << "ASTERIXDecodeJob: netJasterixCallback: applying JSON function without framing";
        JSON::applyFunctionToValues(data_block, keys, keys.begin(), process_lambda, false);
        JSON::applyFunctionToValues(data_block, keys, keys.begin(), count_lambda, false);
    }

    if (data->at("data_blocks").size())
    {
        extracted_data_.emplace_back(std::move(data));
    }

}

size_t ASTERIXDecodeJob::numFrames() const { return num_frames_; }

size_t ASTERIXDecodeJob::numRecords() const { return num_records_; }

bool ASTERIXDecodeJob::error() const { return decoder_ && decoder_->error(); }

void ASTERIXDecodeJob::countRecord(unsigned int category, nlohmann::json& record)
{
    logdbg << "ASTERIXDecodeJob: countRecord: cat " << category << " record '" << record.dump(4)
           << "'";

    count_total_++;
    category_counts_[category] += 1;
}

std::map<unsigned int, size_t> ASTERIXDecodeJob::categoryCounts() const
{
    return category_counts_;
}

std::vector<std::unique_ptr<nlohmann::json>> ASTERIXDecodeJob::extractedData()
{
    logdbg << "ASTERIXDecodeJob: extractedData: signal cnt " << signal_count_;

    return std::move(extracted_data_);
}


size_t ASTERIXDecodeJob::countTotal() const
{
    return count_total_;
}

void ASTERIXDecodeJob::forceBlockingDataProcessing()
{
    loginf << "ASTERIXDecodeJob: forceBlockingDataProcessing: emitting signal";
    emit decodedASTERIXSignal();

    while (!obsolete_ && extracted_data_.size())  // block decoder until extracted records have been moved out
        QThread::msleep(1);
}

size_t ASTERIXDecodeJob::numErrors() const { return num_errors_; }

std::string ASTERIXDecodeJob::errorMessage() const { return (decoder_ ? decoder_->errorMessage() : ""); }

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
