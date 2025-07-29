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

#include "asterixpcapdecoder.h"
#include "asteriximporttask.h"
#include "packetsniffer.h"

#include <jasterix/jasterix.h>

/**
 * @param source Import source to retrieve data from.
 * @param settings If set, external settings will be applied, otherwise settings will be retrieved from the import task.
*/
ASTERIXPCAPDecoder::ASTERIXPCAPDecoder(ASTERIXImportSource& source,
                                       const ASTERIXImportTaskSettings* settings)
:   ASTERIXDecoderFile(ASTERIXImportSource::SourceType::FilePCAP, source, settings)
{
}

/**
*/
ASTERIXPCAPDecoder::~ASTERIXPCAPDecoder() = default;

/**
*/
void ASTERIXPCAPDecoder::stop_impl()
{
    task().jASTERIX()->stopFileDecoding();
}

/**
*/
bool ASTERIXPCAPDecoder::checkFile(ASTERIXImportFileInfo& file_info, 
                                   std::string& error) const
{
    file_info.sections.clear();
    error = "";

    //already has error?
    if (file_info.hasError())
    {   
        error = "File info error encountered";
        return false;
    }

    //open pcap file
    PacketSniffer sniffer;
    if (!sniffer.openPCAP(file_info.filename))
    {
        error = "Could not open PCAP file";
        return false;
    }

    //parse file and accumulate a little portion of data per detected signature
    if (!sniffer.readFile(PacketSniffer::ReadStyle::PerSignature,
                          {}, //we want complete packet stats, so no packet filter
                          PacketSniffer::DataFilter().maxBytesPerSignature(DecodeCheckMaxBytes)))
    {
        error = "Could not parse PCAP file";
        return false;
    }

    //check if unrecognized headers have been encountered
    if (sniffer.hasUnknownPacketHeaders())
    {
        error = "Unknown packet headers encountered";
        return false;
    }

    const auto& data = sniffer.dataPerSignature();

    //anything read?
    if (sniffer.numPacketsRead() == 0 || data.empty())
    {
        error = "No packets read from pcap file";
        return false;
    }

    //add pcap sections to file info
    for (const auto& d : data)
    {
        ASTERIXImportFileSection section;
        section.id               = PacketSniffer::signatureToString(d.first);
        section.description      = PacketSniffer::signatureToString(d.first);
        section.total_size_bytes = d.second.size;

        if (d.second.valid())
        {
            size_t n = d.second.data.size();
            section.raw_data.resize(n);
            memcpy(section.raw_data.data(), d.second.data.data(), n * sizeof(char));
        }

        loginf << "Adding section '" << section.id << "': "
               << "total size = " << section.total_size_bytes << " " 
               << "read size = " << section.raw_data.size();

        file_info.sections.push_back(section);
    }

    return true;
}

/**
*/
bool ASTERIXPCAPDecoder::checkDecoding(ASTERIXImportFileInfo& file_info, 
                                       int section_idx, 
                                       std::string& error) const
{
    assert(section_idx >= 0 && section_idx < (int)file_info.sections.size());

    error = "";

    auto& section = file_info.sections.at(section_idx);

    auto jasterix = task().jASTERIX(true);

    std::unique_ptr<nlohmann::json> analysis_info;

    analysis_info = jasterix->analyzeData(section.raw_data.data(), section.raw_data.size(), DecodeCheckRecordLimit);
    assert(analysis_info);

    auto& section_error = section.error;

    section_error.analysis_info = *analysis_info;

    //loginf << "file '" << file_info.filename << "' "
    //       << "section " << section.id << " "
    //       << "json '" << section_error.analysis_info.dump(4) << "'";
    //            json '{
    //               "data_items": {},
    //               "num_errors": 12,
    //               "num_records": 919,
    //               "sensor_counts": {}
    //           }'

    assert (section_error.analysis_info.contains("num_errors"));
    assert (section_error.analysis_info.contains("num_records"));

    unsigned int num_errors  = section_error.analysis_info.at("num_errors");
    unsigned int num_records = section_error.analysis_info.at("num_records");

    if (num_errors || !num_records) // decoder errors or no data
    {
        error = "Decoding failed";
        return false;
    }

    return true;
}

/**
*/
void ASTERIXPCAPDecoder::processFile(ASTERIXImportFileInfo& file_info)
{
    std::string  current_filename  = file_info.filename;
    unsigned int current_file_line = settings().file_line_id_; //files_info_.at(current_file_count_).line_id_;

    loginf << "file '" << current_filename << "'";

    //collect used signatures
    std::set<PacketSniffer::Signature> signatures;
    for (const auto& section : file_info.sections)
    {
        if (section.used)
        {
            assert(!section.error.hasError());
            signatures.insert(PacketSniffer::signatureFromString(section.id));

            loginf << "importing section '" << section.id << "'";
        }
    }

    PacketSniffer sniffer;
    bool file_open = sniffer.openPCAP(file_info.filename);

    //this should have been checked and caught beforehand
    assert(file_open);

    auto callback = [this, current_file_line] (std::unique_ptr<nlohmann::json> data, 
                                               size_t num_frames,
                                               size_t num_records, 
                                               size_t numErrors) 
    {
        // get last index

        if (settings().current_file_framing_ == "")
        {
            assert(data->contains("data_blocks"));
            assert(data->at("data_blocks").is_array());

            if (data->at("data_blocks").size())
            {
                nlohmann::json& data_block = data->at("data_blocks").back();

                assert(data_block.contains("content"));
                assert(data_block.at("content").is_object());
                assert(data_block.at("content").contains("index"));

                setChunkBytesRead(data_block.at("content").at("index"));
            }
        }
        else
        {
            assert(data->contains("frames"));
            assert(data->at("frames").is_array());

            if (data->at("frames").size())
            {
                nlohmann::json& frame = data->at("frames").back();

                if (frame.contains("content"))
                {
                    assert(frame.at("content").is_object());
                    assert (frame.at("content").contains("index"));

                    setChunkBytesRead(frame.at("content").at("index"));
                }
            }
        }

        addRecordsRead(num_records);

        if (job() && !job()->obsolete())
            job()->fileJasterixCallback(std::move(data), current_file_line, num_frames, num_records, numErrors);
    };

    size_t max_packets = std::numeric_limits<size_t>::max();
    size_t max_bytes   = FileChunkSize;

    bool eof = false;

    //read chunks from PCAP until file is at end
    while (!eof)
    {
        //get next big chunk
        auto data = sniffer.readFileNext(max_packets, max_bytes, signatures);

        //check for errors
        if (!data.has_value())
        {
            logerr << "Could not read data chunk from PCAP";
            logError("Could not read data chunk from PCAP");
            break;
        }

        eof = data.value().eof;

        //more data? => munch munch
        if (!eof)
        {
            //decode chunk
            const auto& chunk = data.value().chunk_data.data;
            size_t num_bytes = chunk.size();

            loginf << "processing " << num_bytes << " byte(s)"; 
            assert(num_bytes > 0);
            
            std::vector<char> vec(num_bytes);
            memcpy(vec.data(), chunk.data(), num_bytes * sizeof(char));

            task().jASTERIX(true)->decodeData(vec.data(), vec.size(), callback);

            chunkFinished();
        }
    }
}
