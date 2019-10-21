#ifndef JSONMAPPINGSTUBSJOB_H
#define JSONMAPPINGSTUBSJOB_H

#include "job.h"
#include "json.hpp"

#include <vector>
#include <memory>

class JSONObjectParser;
class Buffer;

class JSONMappingStubsJob : public Job
{
public:
    JSONMappingStubsJob(std::unique_ptr<std::vector<nlohmann::json>> extracted_records,
                   std::map <std::string, JSONObjectParser>& parsers);
    // json obj moved, mappings referenced
    virtual ~JSONMappingStubsJob();

    virtual void run ();

private:
    std::unique_ptr<std::vector<nlohmann::json>> extracted_records_;
    std::map <std::string, JSONObjectParser>& parsers_;
};

#endif // JSONMAPPINGSTUBSJOB_H
