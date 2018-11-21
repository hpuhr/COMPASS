#ifndef JSONPARSEJOB_H
#define JSONPARSEJOB_H

#include "job.h"
#include "json.hpp"

class JSONParseJob : public Job
{
public:
    JSONParseJob(std::vector<std::string>&& objects); // is moved from objects
    virtual ~JSONParseJob();

    virtual void run ();

    std::vector<nlohmann::json>& jsonObjects(); // for move operation

    size_t objectsParsed() const;

private:
    std::vector<std::string> objects_;
    std::vector<nlohmann::json> json_objects_;

    size_t objects_parsed_ {0};
};

#endif // JSONPARSEJOB_H
