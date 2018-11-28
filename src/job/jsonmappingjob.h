#ifndef JSONMAPPINGJOB_H
#define JSONMAPPINGJOB_H

#include "job.h"
#include "json.hpp"

#include <vector>
#include <memory>

class JSONObjectParser;
class Buffer;

class JSONMappingJob : public Job
{
public:
    JSONMappingJob(std::vector<nlohmann::json>&& json_objects, const std::vector <JSONObjectParser>& mappings,
                   size_t key_count);
    // json obj moved, mappings referenced
    virtual ~JSONMappingJob();

    virtual void run ();

    //std::vector <JsonMapping>&& mappings(); // to be moved out

    size_t numSkipped() const;

    std::map<std::string, std::shared_ptr<Buffer>>&& buffers () { return std::move(buffers_); }

private:
    size_t num_skipped_ {0};

    std::vector<nlohmann::json> json_objects_;
    const std::vector <JSONObjectParser>& mappings_;
    size_t key_count_;

    std::map<std::string, std::shared_ptr<Buffer>> buffers_;
};

#endif // JSONMAPPINGJOB_H
