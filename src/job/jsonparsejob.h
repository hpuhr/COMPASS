#ifndef JSONPARSEJOB_H
#define JSONPARSEJOB_H

#include <memory>

#include "job.h"
#include "json.hpp"

class ASTERIXPostProcess;

class JSONParseJob : public Job
{
  public:
    JSONParseJob(std::vector<std::string> objects, const std::string& current_schema,
                 ASTERIXPostProcess& post_process);  // is moved from objects
    virtual ~JSONParseJob();

    virtual void run();

    std::unique_ptr<nlohmann::json> jsonObjects();  // for move operation

    size_t objectsParsed() const;
    size_t parseErrors() const;

  private:
    std::vector<std::string> objects_;
    std::string current_schema_;
    std::unique_ptr<nlohmann::json> json_objects_;

    ASTERIXPostProcess& post_process_;

    size_t objects_parsed_{0};
    size_t parse_errors_{0};

    void checkCAT001SacSics(nlohmann::json& data_block);
};

#endif  // JSONPARSEJOB_H
