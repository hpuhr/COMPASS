#ifndef JSONMAPPINGJOB_H
#define JSONMAPPINGJOB_H

#include "job.h"
#include "json.hpp"

#include <vector>
#include <memory>

class JsonMapping;
class Buffer;

class JSONMappingJob : public Job
{
public:
    JSONMappingJob(std::vector<nlohmann::json>&& json_objects, std::vector <JsonMapping>& mappings); // json obj moved, mappings copied
    virtual ~JSONMappingJob();

    virtual void run ();

    std::vector <JsonMapping>&& mappings(); // to be moved out

    size_t numSkipped() const;

private:
    size_t num_skipped_ {0};

    std::vector<nlohmann::json> json_objects_;
    std::vector <JsonMapping> mappings_;
};

#endif // JSONMAPPINGJOB_H
