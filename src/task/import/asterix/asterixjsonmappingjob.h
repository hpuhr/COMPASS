#pragma once

#include "job.h"
#include "json_fwd.hpp"

#include <boost/asio.hpp>

#include <memory>
#include <vector>

class ASTERIXJSONParser;
class Buffer;

class ASTERIXJSONMappingJob : public Job
{
  public:
    ASTERIXJSONMappingJob(std::vector<std::unique_ptr<nlohmann::json>> data,
                            const std::string& source_name,
                            const std::vector<std::string>& data_record_keys,
                            const std::map<unsigned int, std::unique_ptr<ASTERIXJSONParser>>& parsers);
      // json obj moved, mappings referenced
    virtual ~ASTERIXJSONMappingJob();

    size_t numMapped() const;
    size_t numNotMapped() const;
    size_t numErrors() const;
    size_t numCreated() const;

    std::map<std::string, std::shared_ptr<Buffer>> buffers() { return std::move(buffers_); }

    std::map<unsigned int, std::pair<size_t, size_t>> categoryMappedCounts() const;

    std::string sourceName() const;

protected:
    void run_impl() override;

private:
    std::map<unsigned int, std::pair<size_t, size_t>>
        category_mapped_counts_;  // mapped, not mapped
    size_t num_mapped_{0};        // number of parsed where a parse was successful
    size_t num_not_mapped_{0};    // number of parsed where no parse was successful
    size_t num_errors_{0};        // number of failed parses
    size_t num_created_{0};       // number of created objects from parsing

    std::vector<std::unique_ptr<nlohmann::json>> data_;
    std::string source_name_;

    const std::vector<std::string> data_record_keys_;

    const std::map<unsigned int, std::unique_ptr<ASTERIXJSONParser>>& parsers_;

    std::map<std::string, std::shared_ptr<Buffer>> buffers_;
};


