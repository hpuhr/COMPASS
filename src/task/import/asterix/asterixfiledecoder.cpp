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
    assert (Files::fileExists(settings_.currentFilename()));
    file_size_ = Files::fileSize(settings_.currentFilename());
}


ASTERIXFileDecoder::~ASTERIXFileDecoder()
{

}

void ASTERIXFileDecoder::start()
{
    assert (!running_);

    start_time_ = boost::posix_time::microsec_clock::local_time();

    running_ = true;

    loginf << "ASTERIXFileDecoder: start: file '" << settings_.currentFilename()
           << "' framing '" << settings_.current_file_framing_ << "'";

    auto callback = [this](std::unique_ptr<nlohmann::json> data, size_t num_frames,
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
                max_index_ = data_block.at("content").at("index");
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
                    max_index_ = frame.at("content").at("index");
                }
            }
        }

        num_records_total_ += num_records;

        job_.fileJasterixCallback(std::move(data), settings_.file_line_id_, num_frames, num_records, numErrors);
    };

    try
    {
        if (settings_.current_file_framing_ == "")
            task_.jASTERIX()->decodeFile(settings_.currentFilename(), callback);
        else
            task_.jASTERIX()->decodeFile(settings_.currentFilename(), settings_.current_file_framing_, callback);
    }
    catch (std::exception& e)
    {
        logerr << "ASTERIXFileDecoder: start: decoding error '" << e.what() << "'";
        error_ = true;
        error_message_ = e.what();
    }
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
    string text = "File '"+settings_.currentFilename()+"'";
    string rec_text;
    string rem_text;

    rec_text = "\n\nRecords/s: "+to_string((unsigned int) getRecordsPerSecond());
    rem_text = "Remaining: "+String::timeStringFromDouble(getRemainingTime() + 1.0, false);

    int num_filler = text.size() - rec_text.size() - rem_text.size();

    if (num_filler < 1)
        num_filler = 1;

    return text + rec_text + std::string(num_filler, ' ') + rem_text;
}

float ASTERIXFileDecoder::statusInfoProgress() // percent
{
    return 100.0 * (float) max_index_/(float) file_size_;
}

float ASTERIXFileDecoder::getRecordsPerSecond() const
{
    float elapsed_s = (float )(boost::posix_time::microsec_clock::local_time()
                               - start_time_).total_milliseconds()/1000.0;

    return (float) num_records_total_ / elapsed_s;
}

float ASTERIXFileDecoder::getRemainingTime() const
{
    size_t remaining_rec = file_size_ - max_index_;

    float elapsed_s = (float )(boost::posix_time::microsec_clock::local_time()
                               - start_time_).total_milliseconds()/1000.0;

    float index_per_s = (float) max_index_ / elapsed_s;

    return (float) remaining_rec / index_per_s;
}
