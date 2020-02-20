#ifndef JSONPARSEJOB_H
#define JSONPARSEJOB_H

#include "job.h"
#include "json.hpp"

#include <memory>

class JSONParseJob : public Job
{
public:
    JSONParseJob(std::vector<std::string> objects); // is moved from objects
    virtual ~JSONParseJob();

    virtual void run ();

    std::unique_ptr<nlohmann::json> jsonObjects(); // for move operation

    size_t objectsParsed() const;
    size_t parseErrors() const;

private:
    std::vector<std::string> objects_;
    std::unique_ptr<nlohmann::json> json_objects_;

    size_t objects_parsed_ {0};
    size_t parse_errors_ {0};
};

#endif // JSONPARSEJOB_H
