#include "jsonmappingjob.h"
#include "jsonobjectparser.h"
#include "buffer.h"
#include "dbobject.h"

JSONMappingJob::JSONMappingJob(std::vector<nlohmann::json>&& json_objects,
                               const std::map <std::string, JSONObjectParser>& mappings, size_t key_count)
    : Job ("JSONMappingJob"), json_objects_(json_objects), parsers_(mappings), key_count_(key_count)
{

}

JSONMappingJob::~JSONMappingJob()
{

}

void JSONMappingJob::run ()
{
    logdbg << "JSONMappingJob: run";

    started_ = true;

    for (auto& parser_it : parsers_)
        buffers_[parser_it.second.dbObject().name()] = parser_it.second.getNewBuffer();

    std::pair <size_t, size_t> skipped_parsed;


    for (auto& j_it : json_objects_)
        for (auto& map_it : parsers_)
        {
            skipped_parsed = map_it.second.parseJSON(j_it, buffers_.at(map_it.second.dbObject().name()));
            num_skipped_ += skipped_parsed.first;
            num_parsed_ += skipped_parsed.second;
        }

    for (auto& parser_it : parsers_)
    {
        assert (buffers_.count(parser_it.second.dbObject().name()));
        std::shared_ptr<Buffer> buffer = buffers_.at(parser_it.second.dbObject().name());

        if (buffer && buffer->size())
        {
            parser_it.second.transformBuffer(buffer, key_count_);
            num_mapped_ += buffer->size();
        }
    }
    done_ = true;
    logdbg << "JSONMappingJob: run: done: mapped " << num_mapped_ << " skipped " << num_skipped_;
}

//std::vector <JsonMapping>&& JSONMappingJob::mappings()
//{
//    return std::move(mappings_);
//}

size_t JSONMappingJob::numParsed() const
{
    return num_parsed_;
}


size_t JSONMappingJob::numSkipped() const
{
    return num_skipped_;
}

size_t JSONMappingJob::numMapped() const
{
    return num_mapped_;
}
