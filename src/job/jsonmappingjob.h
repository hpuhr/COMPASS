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
    JSONMappingJob(std::vector<nlohmann::json>&& json_objects, const std::map <std::string, JSONObjectParser>& mappings,
                   size_t key_count);
    // json obj moved, mappings referenced
    virtual ~JSONMappingJob();

    virtual void run ();

    //std::vector <JsonMapping>&& mappings(); // to be moved out

    size_t numMapped() const;
    size_t numNotMapped() const;
    size_t numCreated() const;

    std::map<std::string, std::shared_ptr<Buffer>>&& buffers () { return std::move(buffers_); }

private:
    size_t num_mapped_ {0}; // number of parsed where a parse was successful
    size_t num_not_mapped_ {0}; // number of parsed where no parse was successful
    size_t num_created_ {0}; // number of created objects from parsing

    std::vector<nlohmann::json> json_objects_;
    const std::map <std::string, JSONObjectParser>& parsers_;
    size_t key_count_;

    std::map<std::string, std::shared_ptr<Buffer>> buffers_;
};

#endif // JSONMAPPINGJOB_H
