#include "asterixfiledecoder.h"
#include "asterixdecoderbase.h"
#include "asteriximporttask.h"
#include "util/files.h"

#include <jasterix/jasterix.h>

using namespace Utils;
using namespace std;
using namespace nlohmann;

ASTERIXFileDecoder::ASTERIXFileDecoder (
        ASTERIXDecodeJob& job, ASTERIXImportTask& task, const ASTERIXImportTaskSettings& settings)
    : ASTERIXDecoderBase(job, task, settings)
{
    total_file_size_ = 0;

    files_info_ = settings_.filesInfo();
    assert (files_info_.size());

    for (const auto& file_info : files_info_)
    {
        assert (Files::fileExists(file_info.filename_));
        total_file_size_ += Files::fileSize(file_info.filename_);
    }
}


ASTERIXFileDecoder::~ASTERIXFileDecoder()
{

}

void ASTERIXFileDecoder::start()
{
    assert (!running_);

    start_time_ = boost::posix_time::microsec_clock::local_time();

    running_ = true;

    while (running_ && hasCurrentFileToDo())
        doCurrentFile();

}

void ASTERIXFileDecoder::stop()
{
    if (running_)
    {
        running_ = false;

        // stop decoding
        task_.jASTERIX()->stopFileDecoding();
    }
}

std::string ASTERIXFileDecoder::statusInfoString()
{
    string text;

    for (const auto& file_info : files_info_)
    {
        if (file_info.filename_ == getCurrentFilename())
            text += "<p align=\"left\"><b>" + file_info.filename_ + "</b>";
        else
            text += "<p align=\"left\">"+file_info.filename_ + "";
    }


    text += "<br><p align=\"left\">Records/s: "+to_string((unsigned int) getRecordsPerSecond());
    text += "<p align=\"right\">Remaining: "+String::timeStringFromDouble(getRemainingTime() + 1.0, false);

    return text ;
}

float ASTERIXFileDecoder::statusInfoProgress() // percent
{
    return 100.0 * (float) (done_files_total_size_ + current_file_max_index_)/(float) total_file_size_;
}

float ASTERIXFileDecoder::getRecordsPerSecond() const
{
    float elapsed_s = (float )(boost::posix_time::microsec_clock::local_time()
                               - start_time_).total_milliseconds()/1000.0;

    return (float) num_records_total_ / elapsed_s;
}

float ASTERIXFileDecoder::getRemainingTime() const
{
    size_t remaining_rec = total_file_size_ - done_files_total_size_ - current_file_max_index_;

    float elapsed_s = (float )(boost::posix_time::microsec_clock::local_time()
                               - start_time_).total_milliseconds()/1000.0;

    float index_per_s = (float) (done_files_total_size_ + current_file_max_index_) / elapsed_s;

    return (float) remaining_rec / index_per_s;
}

bool ASTERIXFileDecoder::hasCurrentFileToDo() // still something to decode
{
    return current_file_count_ < files_info_.size();
}

void ASTERIXFileDecoder::doCurrentFile()
{
    assert (hasCurrentFileToDo());

    task_.refreshjASTERIX();

    string current_filename = files_info_.at(current_file_count_).filename_;
    unsigned int current_file_line = settings_.file_line_id_; //files_info_.at(current_file_count_).line_id_;

    loginf << "ASTERIXFileDecoder: doCurrentFile: file '" << current_filename
           << "' framing '" << settings_.current_file_framing_ << "'";

    auto callback = [this, current_file_line](std::unique_ptr<nlohmann::json> data, size_t num_frames,
            size_t num_records, size_t numErrors) {

        // get last index

        if (settings_.current_file_framing_ == "")
        {
            assert(data->contains("data_blocks"));
            assert(data->at("data_blocks").is_array());

            if (data->at("data_blocks").size())
            {
                json& data_block = data->at("data_blocks").back();

                assert (data_block.contains("content"));
                assert(data_block.at("content").is_object());
                assert (data_block.at("content").contains("index"));
                current_file_max_index_ = data_block.at("content").at("index");
            }

        }
        else
        {
            assert(data->contains("frames"));
            assert(data->at("frames").is_array());

            if (data->at("frames").size())
            {
                json& frame = data->at("frames").back();

                if (frame.contains("content"))
                {
                    assert(frame.at("content").is_object());
                    assert (frame.at("content").contains("index"));
                    current_file_max_index_ = frame.at("content").at("index");
                }
            }
        }

        num_records_total_ += num_records;

        job_.fileJasterixCallback(std::move(data), current_file_line, num_frames, num_records, numErrors);
    };

    try
    {
        if (settings_.current_file_framing_ == "")
            task_.jASTERIX()->decodeFile(current_filename, callback);
        else
            task_.jASTERIX()->decodeFile(current_filename, settings_.current_file_framing_, callback);
    }
    catch (std::exception& e)
    {
        logerr << "ASTERIXFileDecoder: doCurrentFile: decoding error '" << e.what() << "'";
        error_ = true;
        error_message_ = e.what();
    }

    done_files_total_size_ += Files::fileSize(files_info_.at(current_file_count_).filename_);
    current_file_max_index_ = 0;
    ++current_file_count_;
}

std::string ASTERIXFileDecoder::getCurrentFilename()
{
    assert (current_file_count_ < files_info_.size());
    return files_info_.at(current_file_count_).filename_;
}
