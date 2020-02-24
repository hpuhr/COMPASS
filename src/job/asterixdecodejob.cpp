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

#include "asterixdecodejob.h"
#include "asteriximporttask.h"
#include "stringconv.h"
#include "logger.h"
#include "json.h"

#include <jasterix/jasterix.h>

#include <memory>

#include <QThread>

using namespace nlohmann;
using namespace Utils;

ASTERIXDecodeJob::ASTERIXDecodeJob(
        ASTERIXImportTask& task, const std::string& filename, const std::string& framing, bool test,
        std::function<void(unsigned int category, nlohmann::json& record)> process_function,
        std::function<void(unsigned int category, nlohmann::json& record)> override_function)
    : Job ("ASTERIXDecodeJob"), task_(task), filename_(filename), framing_(framing), test_(test),
      process_function_(process_function), override_function_(override_function)
{
    logdbg << "ASTERIXDecodeJob: ctor";
}

ASTERIXDecodeJob::~ASTERIXDecodeJob()
{
    logdbg << "ASTERIXDecodeJob: dtor";
}

void ASTERIXDecodeJob::run ()
{
    logdbg << "ASTERIXDecodeJob: run";

    started_ = true;

    auto callback = [this](std::unique_ptr<nlohmann::json> data, size_t num_frames, size_t num_records,
            size_t numErrors) {
                    this->jasterix_callback(std::move(data), num_frames, num_records, numErrors);
              };

    try
    {
        if (framing_ == "")
            task_.jASTERIX()->decodeFile (filename_, callback);
        else
            task_.jASTERIX()->decodeFile (filename_, framing_, callback);
    }
    catch (std::exception& e)
    {
        logerr << "ASTERIXDecodeJob: run: decoding error '" << e.what() << "'";
        error_ = true;
        error_message_ = e.what();
    }

    assert (extracted_data_ == nullptr);

    done_ = true;

    logdbg << "ASTERIXDecodeJob: run: done";
}

void ASTERIXDecodeJob::jasterix_callback(std::unique_ptr<nlohmann::json> data, size_t num_frames, size_t num_records,
                                         size_t num_errors)
{
    if (error_)
    {
        loginf << "ASTERIXDecodeJob: jasterix_callback: errors state";
        return;
    }

    assert (!extracted_data_);
    extracted_data_ = std::move(data);
    assert (extracted_data_);
    assert (extracted_data_->is_object());

    num_frames_ = num_frames;
    num_records_ = num_records;
    num_errors_ = num_errors;

    if (num_errors_)
        logwrn << "ASTERIXDecodeJob: jasterix_callback: num errors " << num_errors_;

    unsigned int category;

    auto count_lambda = [this, &category](nlohmann::json& record)
    {
        countRecord (category, record);
    };

    auto process_lambda = [this, &category](nlohmann::json& record)
    {
        process_function_ (category, record);
    };

    auto override_lambda = [this, &category](nlohmann::json& record)
    {
        override_function_ (category, record);
    };

    if (framing_ == "")
    {
        assert (extracted_data_->contains("data_blocks"));

        std::vector<std::string> keys {"content", "records"};

        for (json& data_block : extracted_data_->at("data_blocks"))
        {
            if (!data_block.contains("category"))
            {
                logwrn << "ASTERIXDecodeJob: jasterix_callback: data block without asterix category";
                continue;
            }

            category = data_block.at("category");

            loginf << "ASTERIXDecodeJob: jasterix_callback: applying JSON function without framing";
            JSON::applyFunctionToValues(data_block, keys, keys.begin(), process_lambda, false);
            JSON::applyFunctionToValues(data_block, keys, keys.begin(), count_lambda, false);

            if (override_function_)
                JSON::applyFunctionToValues(data_block, keys, keys.begin(), override_lambda, false);
        }
    }
    else
    {
        assert (extracted_data_->contains("frames"));
        assert (extracted_data_->at("frames").is_array());

        std::vector<std::string> keys {"content", "records"};

        for (json& frame : extracted_data_->at("frames"))
        {
            if (!frame.contains("content")) // frame with errors
                continue;

            assert (frame.at("content").is_object());

            if (!frame.at("content").contains("data_blocks")) // frame with errors
                continue;

            assert (frame.at("content").at("data_blocks").is_array());

            for (json& data_block : frame.at("content").at("data_blocks"))
            {
                if (!data_block.contains("category")) // data block with errors
                {
                    logwrn << "ASTERIXDecodeJob: jasterix_callback: data block without asterix category";
                    continue;
                }

                category = data_block.at("category");

                JSON::applyFunctionToValues(data_block, keys, keys.begin(), process_lambda, false);
                JSON::applyFunctionToValues(data_block, keys, keys.begin(), count_lambda, false);

                if (override_function_)
                    JSON::applyFunctionToValues(data_block, keys, keys.begin(), override_lambda, false);
            }
        }
    }

    emit decodedASTERIXSignal();

    while (pause_ || extracted_data_) // block decoder until unpaused and extracted records moved out
    {
        QThread::msleep(1);
    }

    assert (!extracted_data_);
}


size_t ASTERIXDecodeJob::numFrames() const
{
    return num_frames_;
}

size_t ASTERIXDecodeJob::numRecords() const
{
    return num_records_;
}

bool ASTERIXDecodeJob::error() const
{
    return error_;
}

void ASTERIXDecodeJob::countRecord (unsigned int category, nlohmann::json& record)
{
    logdbg  << "ASTERIXDecodeJob: countRecord: cat " << category << " record '" << record.dump(4) << "'";

    category_counts_[category] += 1;
}

std::map<unsigned int, size_t> ASTERIXDecodeJob::categoryCounts() const
{
    return category_counts_;
}

size_t ASTERIXDecodeJob::numErrors() const
{
    return num_errors_;
}

std::string ASTERIXDecodeJob::errorMessage() const
{
    return error_message_;
}



