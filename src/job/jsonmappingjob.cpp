#include "jsonmappingjob.h"
#include "jsonmapping.h"
#include "buffer.h"

JSONMappingJob::JSONMappingJob(std::vector<nlohmann::json>&& json_objects, std::vector <JsonMapping>& mappings)
    : Job ("JSONMappingJob"), json_objects_(json_objects), mappings_(mappings)
{

}

JSONMappingJob::~JSONMappingJob()
{

}

void JSONMappingJob::run ()
{
    logdbg << "JSONMappingJob: run";

    started_ = true;

    for (auto& j_it : json_objects_)
        for (auto& map_it : mappings_)
            map_it.parseJSON(j_it);

    unsigned int num_mapped {0};

    for (auto& map_it : mappings_)
        if (map_it.hasFilledBuffer())
        {
            map_it.transformBuffer();
            num_mapped += map_it.buffer()->size();
        }

    num_skipped_ = json_objects_.size() - num_mapped;

    done_ = true;
    logdbg << "JSONMappingJob: run: done";
}

std::vector <JsonMapping>&& JSONMappingJob::mappings()
{
    return std::move(mappings_);
}

size_t JSONMappingJob::numSkipped() const
{
    return num_skipped_;
}
