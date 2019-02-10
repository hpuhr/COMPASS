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

    bool parsed;
    bool parsed_any = false;

    logdbg << "JSONMappingJob: run: mapping json";
    for (auto& j_it : json_objects_)
    {
        parsed = false;
        parsed_any = false;

        for (auto& map_it : parsers_)
        {
            logdbg << "JSONMappingJob: run: mapping json: obj " << map_it.second.dbObject().name();
            parsed = map_it.second.parseJSON(j_it, buffers_.at(map_it.second.dbObject().name()));
            parsed_any |= parsed;
        }
        if (parsed_any)
            ++num_mapped_;
        else
            ++num_not_mapped_;
    }

    logdbg << "JSONMappingJob: run: creating buffers";
    for (auto& parser_it : parsers_)
    {
        assert (buffers_.count(parser_it.second.dbObject().name()));

        logdbg << "JSONMappingJob: run: creating buffer for " << parser_it.second.dbObject().name();

        std::shared_ptr<Buffer> buffer = buffers_.at(parser_it.second.dbObject().name());

        if (buffer && buffer->size())
        {
            parser_it.second.transformBuffer(buffer, key_count_);
            num_created_ += buffer->size();
        }
    }
    done_ = true;
    logdbg << "JSONMappingJob: run: done: mapped " << num_created_ << " skipped " << num_not_mapped_;
}

//std::vector <JsonMapping>&& JSONMappingJob::mappings()
//{
//    return std::move(mappings_);
//}

size_t JSONMappingJob::numMapped() const
{
    return num_mapped_;
}


size_t JSONMappingJob::numNotMapped() const
{
    return num_not_mapped_;
}

size_t JSONMappingJob::numCreated() const
{
    return num_created_;
}
