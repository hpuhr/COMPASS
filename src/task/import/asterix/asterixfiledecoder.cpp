#include "asterixfiledecoder.h"
#include "asterixdecoderbase.h"
#include "asteriximporttask.h"
#include "util/files.h"

#include <jasterix/jasterix.h>

using namespace Utils;
using namespace std;

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

    running_ = true;

    loginf << "ASTERIXFileDecoder: start: file '" << settings_.currentFilename()
           << "' framing '" << settings_.current_file_framing_ << "'";

    auto callback = [this](std::unique_ptr<nlohmann::json> data, size_t num_frames,
            size_t num_records, size_t numErrors) {
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
    return "UGA";

    string text = "File '"+settings_.currentFilename()+"'";
    string rec_text;
    string rem_text;

//    if (decode_job_)
//    {
//        rec_text = "\n\nRecords/s: Unknown";
//        rem_text = "Remaining: Unknown";

//        //file_progress_dialog_->setValue(decode_job_->getFileDecodingProgress());

//        //rec_text = "\n\nRecords/s: "+to_string((unsigned int) decode_job_->getRecordsPerSecond());
//        //rem_text = "Remaining: "+String::timeStringFromDouble(decode_job_->getRemainingTime() + 1.0, false);
//    }
//    else
//    {
//        rec_text = "\n\nRecords/s: Unknown";
//        rem_text = "Remaining: Unknown";
//    }

//    //string pack_text = "\npacks: "+to_string(num_packets_in_processing_) + " total " + to_string(num_packets_total_);

//    int num_filler = text.size() - rec_text.size() - rem_text.size();
//    if (num_filler < 1)
//        num_filler = 1;

//    file_progress_dialog_->setLabelText((text + rec_text + std::string(num_filler, ' ') + rem_text).c_str());
}

float ASTERIXFileDecoder::statusInfoProgress() // percent
{
    return 50;
}

//float ASTERIXDecodeJob::getFileDecodingProgress() const
//{
//    assert (decode_file_ && file_size_);

//    return 100.0 * (float) max_index_/(float) file_size_;
//}

//float ASTERIXDecodeJob::getRecordsPerSecond() const
//{
//    float elapsed_s = (float )(boost::posix_time::microsec_clock::local_time()
//                               - start_time_).total_milliseconds()/1000.0;

//    return (float) count_total_ / elapsed_s;
//}

//float ASTERIXDecodeJob::getRemainingTime() const
//{
//    assert (decode_file_ && file_size_);

//    size_t remaining_rec = file_size_ - max_index_;

//    float elapsed_s = (float )(boost::posix_time::microsec_clock::local_time()
//                               - start_time_).total_milliseconds()/1000.0;

//    float index_per_s = (float) max_index_ / elapsed_s;

//    return (float) remaining_rec / index_per_s;
//}
