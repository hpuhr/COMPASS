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

#include "asterixfiledecoder.h"
#include "asterixdecoderbase.h"
#include "asteriximporttask.h"
#include "util/files.h"

#include <jasterix/jasterix.h>

using namespace Utils;
using namespace std;
using namespace nlohmann;

/**
*/
ASTERIXFileDecoder::ASTERIXFileDecoder(ASTERIXImportSource& source,
                                       ASTERIXImportTask& task, 
                                       const ASTERIXImportTaskSettings& settings)
:   ASTERIXDecoderFile(ASTERIXImportSource::SourceType::FileASTERIX, source, task, settings)
{
}

/**
*/
ASTERIXFileDecoder::~ASTERIXFileDecoder() = default;

/**
*/
void ASTERIXFileDecoder::stop_impl()
{
    // stop decoding
    task().jASTERIX()->stopFileDecoding();
}

/**
*/
bool ASTERIXFileDecoder::checkDecoding(ASTERIXImportFileInfo& file_info, int section_idx, std::string& error) const
{
    //get a fresh jasterix instance
    auto jasterix = task().jASTERIX(true);

    bool has_framing = settings_.current_file_framing_.size() > 0;

    loginf << "ASTERIXFileDecoder: checkDecoding: file '" << file_info.filename << "' decoding now...";

    //analyze asterix file
    std::unique_ptr<nlohmann::json> analysis_info;
    analysis_info = has_framing ? jasterix->analyzeFile(file_info.filename, settings_.current_file_framing_, DecodeCheckRecordLimit) :
                                  jasterix->analyzeFile(file_info.filename, DecodeCheckRecordLimit);
    assert(analysis_info);

    auto& file_error = file_info.error;

    //store analysis info for later usage
    file_error.analysis_info = *analysis_info;

    loginf << "ASTERIXFileDecoder: checkDecoding: file '" << file_info.filename << "' json '" << file_error.analysis_info.dump(4) << "'";
    //            json '{
    //               "data_items": {},
    //               "num_errors": 12,
    //               "num_records": 919,
    //               "sensor_counts": {}
    //           }'

    //no error info? => strange
    if (!file_error.analysis_info.contains("num_errors") ||
        !file_error.analysis_info.contains("num_records"))
    {
        error = "Decoding failed";
        return false;
    }

    //decoding succeeded?
    unsigned int num_errors  = file_error.analysis_info.at("num_errors");
    unsigned int num_records = file_error.analysis_info.at("num_records");

    if (num_errors || !num_records) // decoder errors or no data
    {
        error = "Decoding failed";
        return false;
    }

    return true;
}

/**
*/
void ASTERIXFileDecoder::processFile(ASTERIXImportFileInfo& file_info)
{
    //get a fresh jasterix instance
    task().jASTERIX(true);

    string       current_filename  = file_info.filename;
    unsigned int current_file_line = settings_.file_line_id_; //files_info_.at(current_file_count_).line_id_;

    loginf << "ASTERIXFileDecoder: processFile: file '" << current_filename << "' framing '" << settings_.current_file_framing_ << "'";

    //jasterix callback
    auto callback = [this, current_file_line] (std::unique_ptr<nlohmann::json> data, 
                                               size_t num_frames,
                                               size_t num_records, 
                                               size_t numErrors) 
    {
        // get last index

        if (settings_.current_file_framing_ == "")
        {
            assert(data->contains("data_blocks"));
            assert(data->at("data_blocks").is_array());

            if (data->at("data_blocks").size())
            {
                json& data_block = data->at("data_blocks").back();

                assert(data_block.contains("content"));
                assert(data_block.at("content").is_object());
                assert(data_block.at("content").contains("index"));

                setFileBytesRead(data_block.at("content").at("index"));
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

                    setFileBytesRead(frame.at("content").at("index"));
                }
            }
        }

        addRecordsRead(num_records);

        //invoke job callback
        job()->fileJasterixCallback(std::move(data), current_file_line, num_frames, num_records, numErrors);
    };

    //start decoding
    if (settings_.current_file_framing_ == "")
        task().jASTERIX()->decodeFile(current_filename, callback);
    else
        task().jASTERIX()->decodeFile(current_filename, settings_.current_file_framing_, callback);
}
