#ifndef JSONMAPPINGSTUBSJOB_H
#define JSONMAPPINGSTUBSJOB_H

#include <memory>
#include <vector>

#include "job.h"
#include "json.hpp"

class JSONObjectParser;
class Buffer;

class JSONMappingStubsJob : public Job
{
  public:
    JSONMappingStubsJob(std::unique_ptr<nlohmann::json> data,
                        const std::vector<std::string>& data_record_keys,
                        std::map<std::string, JSONObjectParser>& parsers);
    // json obj moved, mappings referenced
    virtual ~JSONMappingStubsJob();

    virtual void run();

  private:
    std::unique_ptr<nlohmann::json> data_;
    const std::vector<std::string> data_record_keys_;
    std::map<std::string, JSONObjectParser>& parsers_;
};

#endif  // JSONMAPPINGSTUBSJOB_H
