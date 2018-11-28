#include "jsonmappingjob.h"
#include "jsonobjectparser.h"
#include "buffer.h"
#include "dbobject.h"

JSONMappingJob::JSONMappingJob(std::vector<nlohmann::json>&& json_objects, const std::vector <JSONObjectParser>& mappings,
                               size_t key_count)
    : Job ("JSONMappingJob"), json_objects_(json_objects), mappings_(mappings), key_count_(key_count)
{

}

JSONMappingJob::~JSONMappingJob()
{

}

void JSONMappingJob::run ()
{
    logdbg << "JSONMappingJob: run";

    started_ = true;

    for (auto& map_it : mappings_)
        buffers_[map_it.dbObject().name()] = map_it.getNewBuffer();

    for (auto& j_it : json_objects_)
        for (auto& map_it : mappings_)
            map_it.parseJSON(j_it, buffers_.at(map_it.dbObject().name()));

    unsigned int num_mapped {0};

    for (auto& map_it : mappings_)
    {
        assert (buffers_.count(map_it.dbObject().name()));
        std::shared_ptr<Buffer> buffer = buffers_.at(map_it.dbObject().name());

        if (buffer && buffer->size())
        {
            map_it.transformBuffer(buffer, key_count_);
            num_mapped += buffer->size();
        }
    }
    num_skipped_ = json_objects_.size() - num_mapped;

    done_ = true;
    logdbg << "JSONMappingJob: run: done: mapped " << num_mapped << " skipped " << num_skipped_;
}

//std::vector <JsonMapping>&& JSONMappingJob::mappings()
//{
//    return std::move(mappings_);
//}

size_t JSONMappingJob::numSkipped() const
{
    return num_skipped_;
}
